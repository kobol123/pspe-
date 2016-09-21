// This is generic code that is included in all Android apps that use the
// Native framework by Henrik Rydg�rd (https://github.com/hrydgard/native).

// It calls a set of methods defined in NativeApp.h. These should be implemented
// by your game or app.

#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <stdint.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <queue>

#include "base/basictypes.h"
#include "base/display.h"
#include "base/mutex.h"
#include "base/NativeApp.h"
#include "base/logging.h"
#include "base/timeutil.h"
#include "file/zip_read.h"
#include "input/input_state.h"
#include "audio/mixer.h"
#include "math/math_util.h"
#include "net/resolve.h"
#include "android/native_audio.h"
#include "gfx_es2/gl_state.h"

#include "app-android.h"

static JNIEnv *jniEnvUI;

struct FrameCommand {
	FrameCommand() {}
	FrameCommand(std::string cmd, std::string prm) : command(cmd), params(prm) {}

	std::string command;
	std::string params;
};

recursive_mutex frameCommandLock;
std::queue<FrameCommand> frameCommands;

std::string systemName;
std::string langRegion;

static float left_joystick_x_async;
static float left_joystick_y_async;
static float right_joystick_x_async;
static float right_joystick_y_async;
static float hat_joystick_x_async;
static float hat_joystick_y_async;

int optimalFramesPerBuffer = 0;
int optimalSampleRate = 0;
static int androidVersion;

// Should only be used for display detection during startup (for config defaults etc)
static int display_xres;
static int display_yres;

// Android implementation of callbacks to the Java part of the app
void SystemToast(const char *text) {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand("toast", text));
}

void ShowKeyboard() {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand("showKeyboard", ""));
}

void Vibrate(int length_ms) {
	lock_guard guard(frameCommandLock);
	char temp[32];
	sprintf(temp, "%i", length_ms);
	frameCommands.push(FrameCommand("vibrate", temp));
}

void LaunchBrowser(const char *url) {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand("launchBrowser", url));
}

void LaunchMarket(const char *url) {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand("launchMarket", url));
}

void LaunchEmail(const char *email_address) {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand("launchEmail", email_address));
}

void System_SendMessage(const char *command, const char *parameter) {
	lock_guard guard(frameCommandLock);
	frameCommands.push(FrameCommand(command, parameter));
}

std::string System_GetProperty(SystemProperty prop) {
	switch (prop) {
	case SYSPROP_NAME:
		return systemName;
	case SYSPROP_LANGREGION:  // "en_US"
		return langRegion;
	default:
		return "";
	}
}

int System_GetPropertyInt(SystemProperty prop) {
	switch (prop) {
	case SYSPROP_SYSTEMVERSION:
		return androidVersion;
	case SYSPROP_DISPLAY_XRES:
		return display_xres;
	case SYSPROP_DISPLAY_YRES:
		return display_yres;
	default:
		return -1;
	}
}

// Remember that all of these need initialization on init! The process
// may be reused when restarting the game. Globals are DANGEROUS.

float dp_xscale = 1.0f;
float dp_yscale = 1.0f;

InputState input_state;

static bool renderer_inited = false;
static bool first_lost = true;
static bool use_opensl_audio = false;
static std::string library_path;

std::string GetJavaString(JNIEnv *env, jstring jstr) {
	const char *str = env->GetStringUTFChars(jstr, 0);
	std::string cpp_string = std::string(str);
	env->ReleaseStringUTFChars(jstr, str);
	return cpp_string;
}

// This is now only used as a trigger for GetAppInfo as a function to all before Init.
// On Android we don't use any of the values it returns.
extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_isLandscape(JNIEnv *env, jclass) {
	std::string app_name, app_nice_name;
	bool landscape;
	NativeGetAppInfo(&app_name, &app_nice_name, &landscape);
	return landscape;
}

// Allow the app to intercept the back button.
extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_isAtTopLevel(JNIEnv *env, jclass) {
	return NativeIsAtTopLevel();
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_audioConfig
	(JNIEnv *env, jclass, jint optimalFPB, jint optimalSR) {
	optimalFramesPerBuffer = optimalFPB;
	optimalSampleRate = optimalSR;
}

extern "C" jstring Java_com_henrikrydgard_libnative_NativeApp_queryConfig
	(JNIEnv *env, jclass, jstring jquery) {
	std::string query = GetJavaString(env, jquery);
	std::string result = NativeQueryConfig(query);
	jstring jresult = env->NewStringUTF(result.c_str());
	return jresult;
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_init
  (JNIEnv *env, jclass, jstring jdevicetype, jint jxres, jint jyres, jstring jlangRegion, jstring japkpath,
		jstring jdataDir, jstring jexternalDir, jstring jlibraryDir, jstring jshortcutParam,
		jstring jinstallID, jboolean juseNativeAudio, jint jAndroidVersion) {
	jniEnvUI = env;

	ILOG("NativeApp.init() -- begin");

	memset(&input_state, 0, sizeof(input_state));
	renderer_inited = false;
	first_lost = true;
	androidVersion = jAndroidVersion;

	g_buttonTracker.Reset();

	left_joystick_x_async = 0;
	left_joystick_y_async = 0;
	right_joystick_x_async = 0;
	right_joystick_y_async = 0;
	hat_joystick_x_async = 0;
	hat_joystick_y_async = 0;
	display_xres = jxres;
	display_yres = jyres;

	std::string apkPath = GetJavaString(env, japkpath);
	VFSRegister("", new ZipAssetReader(apkPath.c_str(), "assets/"));

	systemName = GetJavaString(env, jdevicetype);
	langRegion = GetJavaString(env, jlangRegion);

	std::string externalDir = GetJavaString(env, jexternalDir);
	std::string user_data_path = GetJavaString(env, jdataDir) + "/";
	library_path = GetJavaString(env, jlibraryDir) + "/";
	std::string shortcut_param = GetJavaString(env, jshortcutParam);
	std::string installID = GetJavaString(env, jinstallID);

	ILOG("NativeApp.init(): External storage path: %s", externalDir.c_str());
	ILOG("NativeApp.init(): Launch shortcut parameter: %s", shortcut_param.c_str());

	std::string app_name;
	std::string app_nice_name;
	bool landscape;

	net::Init();

	NativeGetAppInfo(&app_name, &app_nice_name, &landscape);


	// If shortcut_param is not empty, pass it as additional varargs argument to NativeInit() method.
	// NativeInit() is expected to treat extra argument as boot_filename, which in turn will start game immediately.
	// NOTE: Will only work if ppsspp started from Activity.onCreate(). Won't work if ppsspp app start from onResume().

	if (shortcut_param.empty()) {
		const char *argv[2] = {app_name.c_str(), 0};
		NativeInit(1, argv, user_data_path.c_str(), externalDir.c_str(), installID.c_str());
	}
	else {
		const char *argv[3] = {app_name.c_str(), shortcut_param.c_str(), 0};
		NativeInit(2, argv, user_data_path.c_str(), externalDir.c_str(), installID.c_str());
	}

	use_opensl_audio = juseNativeAudio;
	ILOG("NativeApp.init() -- end");
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_audioInit(JNIEnv *, jclass) {
	if (use_opensl_audio) {
		// TODO: PPSSPP doesn't support 48khz yet so let's not use that yet.
		ILOG("NativeApp.audioInit() -- Using OpenSL audio! frames/buffer: %i   optimal sr: %i   actual sr: 44100", optimalFramesPerBuffer, optimalSampleRate);
		optimalSampleRate = 44100;
		AndroidAudio_Init(&NativeMix, library_path, optimalFramesPerBuffer, optimalSampleRate);
	} else {
		ILOG("NativeApp.audioInit() -- Using Java audio :(");
	}
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_audioShutdown(JNIEnv *, jclass) {
	ILOG("NativeApp.audioShutdown() -- begin");
	if (use_opensl_audio) {
		AndroidAudio_Shutdown();
	}
	ILOG("NativeApp.audioShutdown() -- end");
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_resume(JNIEnv *, jclass) {
	ILOG("NativeApp.resume() - resuming audio");
	if (use_opensl_audio) {
		AndroidAudio_Resume();
	}
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_pause(JNIEnv *, jclass) {
	ILOG("NativeApp.pause() - begin");
	if (use_opensl_audio) {
		AndroidAudio_Pause();
	}
	ILOG("NativeApp.pause() - end");
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_shutdown(JNIEnv *, jclass) {
	ILOG("NativeApp.shutdown() -- begin");
	NativeShutdown();
	VFSShutdown();
	net::Shutdown();
	ILOG("NativeApp.shutdown() -- end");
}

static jmethodID postCommand;

extern "C" void Java_com_henrikrydgard_libnative_NativeRenderer_displayInit(JNIEnv * env, jobject obj) {
	ILOG("NativeApp.displayInit()");
	if (!renderer_inited) {
		NativeInitGraphics();
		renderer_inited = true;
	} else {
		NativeDeviceLost();  // ???
		ILOG("displayInit: NativeDeviceLost completed.");
	}

	DLOG("(Re)-fetching method ID to postCommand...");
	postCommand = env->GetMethodID(env->GetObjectClass(obj), "postCommand", "(Ljava/lang/String;Ljava/lang/String;)V");
}

extern "C" void Java_com_henrikrydgard_libnative_NativeRenderer_displayResize(JNIEnv *, jobject clazz, jint w, jint h, jint dpi, jfloat refreshRate) {
	ILOG("NativeApp.displayResize(%i x %i, dpi=%i, refresh=%0.2f)", w, h, dpi, refreshRate);

	g_dpi = dpi;
	g_dpi_scale = 240.0f / (float)g_dpi;

	pixel_xres = w;
	pixel_yres = h;
	dp_xres = pixel_xres * g_dpi_scale;
	dp_yres = pixel_yres * g_dpi_scale;
	dp_xscale = (float)dp_xres / pixel_xres;
	dp_yscale = (float)dp_yres / pixel_yres;
	display_hz = refreshRate;

	NativeResized();
}

extern "C" void Java_com_henrikrydgard_libnative_NativeRenderer_displayRender(JNIEnv *env, jobject obj) {
	if (renderer_inited) {
		// TODO: Look into if these locks are a perf loss
		{
			lock_guard guard(input_state.lock);

			input_state.pad_lstick_x = left_joystick_x_async;
			input_state.pad_lstick_y = left_joystick_y_async;
			input_state.pad_rstick_x = right_joystick_x_async;
			input_state.pad_rstick_y = right_joystick_y_async;

			UpdateInputState(&input_state);
		}
		NativeUpdate(input_state);

		{
			lock_guard guard(input_state.lock);
			EndInputState(&input_state);
		}

		NativeRender();
		time_update();
	} else {
		ELOG("BAD: Ended up in nativeRender even though app has quit.%s", "");
		// Shouldn't really get here. Let's draw magenta.
		glstate.depthWrite.set(GL_TRUE);
		glstate.colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(1.0, 0.0, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	lock_guard guard(frameCommandLock);
	while (!frameCommands.empty()) {
		FrameCommand frameCmd;
		frameCmd = frameCommands.front();
		frameCommands.pop();

		DLOG("frameCommand %s %s", frameCmd.command.c_str(), frameCmd.params.c_str());

		jstring cmd = env->NewStringUTF(frameCmd.command.c_str());
		jstring param = env->NewStringUTF(frameCmd.params.c_str());
		env->CallVoidMethod(obj, postCommand, cmd, param);
	}
}

extern "C" void Java_com_henrikrydgard_libnative_NativeRenderer_displayShutdown(JNIEnv *env, jobject obj) {
	if (renderer_inited) {
		NativeDeviceLost();
		ILOG("NativeDeviceLost completed.");
		NativeShutdownGraphics();
		renderer_inited = false;
		NativeMessageReceived("recreateviews", "");
	}
}


// This path is not used if OpenSL ES is available.
extern "C" jint Java_com_henrikrydgard_libnative_NativeApp_audioRender(JNIEnv*	env, jclass clazz, jshortArray array) {
	// The audio thread can pretty safely enable Flush-to-Zero mode on the FPU.
	EnableFZ();

	int buf_size = env->GetArrayLength(array);
	if (buf_size) {
		short *data = env->GetShortArrayElements(array, 0);
		int samples = buf_size / 2;
		samples = NativeMix(data, samples);
		if (samples != 0) {
			env->ReleaseShortArrayElements(array, data, 0);
			return samples * 2;
		} else {
			env->ReleaseShortArrayElements(array, data, JNI_ABORT);
			return 0;
		}
	}
	return 0;
}

extern "C" jboolean JNICALL Java_com_henrikrydgard_libnative_NativeApp_touch
	(JNIEnv *, jclass, float x, float y, int code, int pointerId) {
	float scaledX = x * dp_xscale;
	float scaledY = y * dp_yscale;

	TouchInput touch;
	touch.id = pointerId;
	touch.x = scaledX;
	touch.y = scaledY;
	touch.flags = code;
	if (code & 2) {
		input_state.pointer_down[pointerId] = true;
	} else if (code & 4) {
		input_state.pointer_down[pointerId] = false;
	}

	bool retval = NativeTouch(touch);
	{
		lock_guard guard(input_state.lock);
		if (pointerId >= MAX_POINTERS) {
			ELOG("Too many pointers: %i", pointerId);
			return false;	// We ignore 8+ pointers entirely.
		}
		input_state.pointer_x[pointerId] = scaledX;
		input_state.pointer_y[pointerId] = scaledY;
		input_state.mouse_valid = true;
	}
	return retval;
}

extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_keyDown(JNIEnv *, jclass, jint deviceId, jint key, jboolean isRepeat) {
	KeyInput keyInput;
	keyInput.deviceId = deviceId;
	keyInput.keyCode = key;
	keyInput.flags = KEY_DOWN;
	if (isRepeat) {
		ILOG("Is repeat! %i", key);
		keyInput.flags |= KEY_IS_REPEAT;
	}
	return NativeKey(keyInput);
}

extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_keyUp(JNIEnv *, jclass, jint deviceId, jint key) {
	KeyInput keyInput;
	keyInput.deviceId = deviceId;
	keyInput.keyCode = key;
	keyInput.flags = KEY_UP;
	return NativeKey(keyInput);
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_beginJoystickEvent(
	JNIEnv *env, jclass) {
	// mutex lock?
}

extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_joystickAxis(
		JNIEnv *env, jclass, jint deviceId, jint axisId, jfloat value) {
	if (!renderer_inited)
		return false;
	switch (axisId) {
	case JOYSTICK_AXIS_X:
		left_joystick_x_async = value;
		break;
	case JOYSTICK_AXIS_Y:
		left_joystick_y_async = -value;
		break;
	case JOYSTICK_AXIS_Z:
		right_joystick_x_async = value;
		break;
	case JOYSTICK_AXIS_RZ:
		right_joystick_y_async = -value;
		break;
	case JOYSTICK_AXIS_HAT_X:
		hat_joystick_x_async = value;
		break;
	case JOYSTICK_AXIS_HAT_Y:
		hat_joystick_y_async = -value;
		break;
	}

	AxisInput axis;
	axis.axisId = axisId;
	axis.deviceId = deviceId;
	axis.value = value;
	return NativeAxis(axis);
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_endJoystickEvent(
	JNIEnv *env, jclass) {
	// mutex unlock?
}


extern "C" jboolean Java_com_henrikrydgard_libnative_NativeApp_mouseWheelEvent(
	JNIEnv *env, jclass, jint stick, jfloat x, jfloat y) {
	// TODO: Support mousewheel for android
	return true;
}

extern "C" jboolean JNICALL Java_com_henrikrydgard_libnative_NativeApp_accelerometer(JNIEnv *, jclass, float x, float y, float z) {
	if (!renderer_inited)
		return false;

	// Theoretically this needs locking but I doubt it matters. Worst case, the X
	// from one "sensor frame" will be used together with Y from the next.
	// Should look into quantization though, for compressed movement storage.
	input_state.accelerometer_valid = true;
	input_state.acc.x = x;
	input_state.acc.y = y;
	input_state.acc.z = z;

	AxisInput axis;
	axis.deviceId = DEVICE_ID_ACCELEROMETER;
	axis.flags = 0;

	axis.axisId = JOYSTICK_AXIS_ACCELEROMETER_X;
	axis.value = x;
	bool retvalX = NativeAxis(axis);

	axis.axisId = JOYSTICK_AXIS_ACCELEROMETER_Y;
	axis.value = y;
	bool retvalY = NativeAxis(axis);

	axis.axisId = JOYSTICK_AXIS_ACCELEROMETER_Z;
	axis.value = z;
	bool retvalZ = NativeAxis(axis);

	return retvalX || retvalY || retvalZ;
}

extern "C" void Java_com_henrikrydgard_libnative_NativeApp_sendMessage(JNIEnv *env, jclass, jstring message, jstring param) {
	std::string msg = GetJavaString(env, message);
	std::string prm = GetJavaString(env, param);
	NativeMessageReceived(msg.c_str(), prm.c_str());
}
