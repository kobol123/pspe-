<<<<<<< HEAD
// Copyright (c) 2015- PSPe+ Project.
=======
﻿// Copyright (c) 2015- PSPe+ Project.
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/



#include "base/display.h"  // Only to check screen aspect ratio with pixel_yres/pixel_xres

#include "base/colorutil.h"
#include "base/timeutil.h"
#include "math/curves.h"
#include "gfx_es2/gpu_features.h"
#include "gfx_es2/draw_buffer.h"
#include "i18n/i18n.h"
#include "ui/view.h"
#include "ui/viewgroup.h"
#include "ui/ui_context.h"
#include "UI/EmuScreen.h"
#include "UI/GameSettingsScreen.h"
#include "UI/GameInfoCache.h"
#include "UI/GamepadEmu.h"
#include "UI/MiscScreens.h"
#include "UI/ControlMappingScreen.h"
#include "UI/DevScreens.h"
#include "UI/SavedataScreen.h"
#include "UI/TouchControlLayoutScreen.h"
#include "UI/TouchControlVisibilityScreen.h"
#include "UI/TiltAnalogSettingsScreen.h"
#include "UI/TiltEventProcessor.h"

#include "Common/KeyMap.h"
#include "Common/FileUtil.h"
#include "Core/Config.h"
#include "Core/Host.h"
#include "Core/System.h"
#include "Core/Reporting.h"
#include "android/jni/TestRunner.h"
#include "GPU/GPUInterface.h"
#include "GPU/GLES/Framebuffer.h"

#if defined(_WIN32)
#include "Windows/WndMainWindow.h"
#include <shlobj.h>
#include "util/text/utf8.h"
#include "Windows/W32Util/ShellUtil.h"
#include "Windows/W32Util/Misc.h"
using namespace std;

#endif

#ifdef IOS
extern bool iosCanUseJit;
#endif

GameSettingsScreen::GameSettingsScreen(std::string gamePath, std::string gameID, bool editThenRestore)
	: UIDialogScreenWithGameBackground(gamePath), gameID_(gameID), enableReports_(false), bEditThenRestore(editThenRestore) {
	lastVertical_ = UseVerticalLayout();
}

bool GameSettingsScreen::UseVerticalLayout() const {
	return dp_yres > dp_xres * 1.1f;
}

void GameSettingsScreen::CreateViews() {
	GameInfo *info = g_gameInfoCache.GetInfo(NULL, gamePath_, GAMEINFO_WANTBG | GAMEINFO_WANTSIZE);

	if (bEditThenRestore)
	{
		g_Config.loadGameConfig(gameID_);
	}

	cap60FPS_ = g_Config.iForceMaxEmulatedFPS == 60;

	iAlternateSpeedPercent_ = (g_Config.iFpsLimit * 100) / 60;

	bool vertical = UseVerticalLayout();

	// Information in the top left.
	// Back button to the bottom left.
	// Scrolling action menu to the right.
	using namespace UI;

	I18NCategory *di = GetI18NCategory("Dialog");
	I18NCategory *gr = GetI18NCategory("Graphics");
	I18NCategory *co = GetI18NCategory("Controls");
	I18NCategory *a = GetI18NCategory("Audio");
	I18NCategory *sa = GetI18NCategory("Savedata");
	I18NCategory *sy = GetI18NCategory("System");
	I18NCategory *n = GetI18NCategory("Networking");
	I18NCategory *ms = GetI18NCategory("MainSettings");
	I18NCategory *dev = GetI18NCategory("Developer");

	if (vertical) {
		root_ = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
	} else {
		root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
	}

	TabHolder *tabHolder;
	tabHolder = new TabHolder(ORIENT_HORIZONTAL, 200, new AnchorLayoutParams(10, 0, 10, 0, false));
			root_->Add(tabHolder);
			//root_->Add(new Choice(d->T("Back"), "", false, new AnchorLayoutParams(150, 64, 10, NONE, NONE, 10)))->OnClick.Handle<UIScreen>(this, &UIScreen::OnBack);


	root_->SetDefaultFocusView(tabHolder);

	// TODO: These currently point to global settings, not game specific ones.

	// Graphics
	ViewGroup *graphicsSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *graphicsSettings = new LinearLayout(ORIENT_VERTICAL);
	graphicsSettings->SetSpacing(0);
	graphicsSettingsScroll->Add(graphicsSettings);
	tabHolder->AddTab(ms->T("Graphics"), graphicsSettingsScroll);

	///graphicsSettings->Add(new ItemHeader(gr->T("Rendering Mode")));
#if defined(_WIN32)
	static const char *renderingBackend[] = { "OpenGL", "Direct3D9" };
	PopupMultiChoice *renderingBackendChoice = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iTempGPUBackend, gr->T("Backend"), renderingBackend, GPU_BACKEND_OPENGL, ARRAY_SIZE(renderingBackend), gr, screenManager()));
	renderingBackendChoice->OnChoice.Handle(this, &GameSettingsScreen::OnRenderingBackend);
#endif
	static const char *renderingMode[] = { "Buffered Rendering"};


	graphicsSettings->Add(new ItemHeader(gr->T("Frame Rate Control")));
<<<<<<< HEAD
	static const char *frameSkip[] = {"Off", "1", "2", "3", "4", "5", "6", "7", "8" ,"9", "10", "11" , "12", "13", "14","15","16"};
=======
	static const char *frameSkip[] = {"Off", "1", "2", "3", "4", "5", "6", "7", "8" ,"9", "10", "11" , "12"};
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	graphicsSettings->Add(new PopupMultiChoice(&g_Config.iFrameSkip, gr->T("Frame Skipping"), frameSkip, 0, ARRAY_SIZE(frameSkip), gr, screenManager()));
	frameSkipAuto_ = graphicsSettings->Add(new CheckBox(&g_Config.bAutoFrameSkip, gr->T("Auto FrameSkip")));
	frameSkipAuto_->OnClick.Handle(this, &GameSettingsScreen::OnAutoFrameskip);
	graphicsSettings->Add(new CheckBox(&cap60FPS_, gr->T("Force max 60 FPS")));

<<<<<<< HEAD
	graphicsSettings->Add(new PopupSliderChoice(&iAlternateSpeedPercent_, 0, 4000, gr->T("Alternative Speed", "Alternative Speed (in %, 0 = unlimited)"), 5, screenManager()));
=======
	graphicsSettings->Add(new PopupSliderChoice(&iAlternateSpeedPercent_, 0, 600, gr->T("Alternative Speed", "Alternative Speed (in %, 0 = unlimited)"), 5, screenManager()));
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

	graphicsSettings->Add(new ItemHeader(gr->T("Features")));
	I18NCategory *ps = GetI18NCategory("PostShaders");
	postProcChoice_ = graphicsSettings->Add(new ChoiceWithValueDisplay(&g_Config.sPostShaderName, gr->T("Postprocessing Shader"), ps));
	postProcChoice_->OnClick.Handle(this, &GameSettingsScreen::OnPostProcShader);
<<<<<<< HEAD
	postProcEnable_ = true;
=======
	postProcEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	postProcChoice_->SetEnabledPtr(&postProcEnable_);

#if !defined(MOBILE_DEVICE)
	graphicsSettings->Add(new CheckBox(&g_Config.bFullScreen, gr->T("FullScreen")))->OnClick.Handle(this, &GameSettingsScreen::OnFullscreenChange);
#endif
	graphicsSettings->Add(new CheckBox(&g_Config.bStretchToDisplay, gr->T("Stretch to Display")));
	// Small Display: To avoid overlapping touch controls on large tablets. Better control over this will be coming later.
	graphicsSettings->Add(new CheckBox(&g_Config.bSmallDisplay, gr->T("Small Display")));
	if (pixel_xres < pixel_yres * 1.3) // Smaller than 4:3
		graphicsSettings->Add(new CheckBox(&g_Config.bPartialStretch, gr->T("Partial Vertical Stretch")));

#ifdef ANDROID
	// Hide Immersive Mode on pre-kitkat Android
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 19) {
		graphicsSettings->Add(new CheckBox(&g_Config.bImmersiveMode, gr->T("Immersive Mode")))->OnClick.Handle(this, &GameSettingsScreen::OnImmersiveModeChange);
	}
#endif

	graphicsSettings->Add(new ItemHeader(gr->T("Performance")));
#ifndef MOBILE_DEVICE
	static const char *internalResolutions[] = {"Auto (1:1)", "1x PSP", "2x PSP", "3x PSP", "4x PSP", "5x PSP", "6x PSP", "7x PSP", "8x PSP", "9x PSP", "10x PSP" };
#else
	static const char *internalResolutions[] = {"Auto (1:1)", "1x PSP", "2x PSP", "3x PSP", "4x PSP", "5x PSP" };
#endif
	resolutionChoice_ = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iInternalResolution, gr->T("Rendering Resolution"), internalResolutions, 0, ARRAY_SIZE(internalResolutions), gr, screenManager()));
	resolutionChoice_->OnChoice.Handle(this, &GameSettingsScreen::OnResolutionChange);
	resolutionEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);
	resolutionChoice_->SetEnabledPtr(&resolutionEnable_);


#ifdef ANDROID
	static const char *deviceResolutions[] = { "Native device resolution", "Auto (same as Rendering)", "1x PSP", "2x PSP", "3x PSP", "4x PSP", "5x PSP" };
	int max_res_temp = std::max(System_GetPropertyInt(SYSPROP_DISPLAY_XRES), System_GetPropertyInt(SYSPROP_DISPLAY_YRES)) / 480 + 2;
	int max_res = std::min(max_res_temp, (int)ARRAY_SIZE(deviceResolutions));
	UI::PopupMultiChoice *hwscale = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iAndroidHwScale, gr->T("Display Resolution (HW scaler)"), deviceResolutions, 0, max_res, gr, screenManager()));
	hwscale->OnChoice.Handle(this, &GameSettingsScreen::OnHwScaleChange);  // To refresh the display mode
#endif

#ifdef ANDROID
	graphicsSettings->Add(new CheckBox(&g_Config.bVSync, gr->T("VSync")));
#endif
	CheckBox *mipmapping = graphicsSettings->Add(new CheckBox(&g_Config.bMipMap, gr->T("Mipmapping")));
	mipmapping->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *hwTransform = graphicsSettings->Add(new CheckBox(&g_Config.bHardwareTransform, gr->T("Hardware Transform")));
	hwTransform->OnClick.Handle(this, &GameSettingsScreen::OnHardwareTransform);
	hwTransform->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *swSkin = graphicsSettings->Add(new CheckBox(&g_Config.bSoftwareSkinning, gr->T("Software Skinning")));
	swSkin->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *vtxCache = graphicsSettings->Add(new CheckBox(&g_Config.bVertexCache, gr->T("Vertex Cache")));
	vtxCacheEnable_ = !g_Config.bSoftwareRendering && g_Config.bHardwareTransform;
	vtxCache->SetEnabledPtr(&vtxCacheEnable_);

	CheckBox *texBackoff = graphicsSettings->Add(new CheckBox(&g_Config.bTextureBackoffCache, gr->T("Lazy texture caching", "Lazy texture caching (speedup)")));
	texBackoff->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *texSecondary_ = graphicsSettings->Add(new CheckBox(&g_Config.bTextureSecondaryCache, gr->T("Retain changed textures", "Retain changed textures (speedup, mem hog)")));
	texSecondary_->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *framebufferSlowEffects = graphicsSettings->Add(new CheckBox(&g_Config.bDisableSlowFramebufEffects, gr->T("Disable slower effects (speedup)")));
	framebufferSlowEffects->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *KernelWH = graphicsSettings->Add(new CheckBox(&g_Config.bKernelWH, gr->T("Enable Kernel Wait Process")));

	// Seems solid, so we hide the setting.
	 CheckBox *vtxJit = graphicsSettings->Add(new CheckBox(&g_Config.bVertexDecoderJit, gr->T("Vertex Decoder JIT")));

	 if (PSP_IsInited()) {
		 vtxJit->SetEnabled(false);
	 }

	static const char *quality[] = { "Low", "Medium", "High"};
	PopupMultiChoice *beziersChoice = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iSplineBezierQuality, gr->T("LowCurves", "Spline/Bezier curves quality"), quality, 0, ARRAY_SIZE(quality), gr, screenManager()));
	beziersChoice->SetDisabledPtr(&g_Config.bSoftwareRendering);
	
	// In case we're going to add few other antialiasing option like MSAA in the future.
	// graphicsSettings->Add(new CheckBox(&g_Config.bFXAA, gr->T("FXAA")));
	graphicsSettings->Add(new ItemHeader(gr->T("Texture Scaling")));
#ifndef MOBILE_DEVICE
	static const char *texScaleLevelsNPOT[] = {"Auto", "Off", "2x", "3x", "4x", "5x"};
	static const char *texScaleLevelsPOT[] = {"Auto", "Off", "2x", "4x"};
#else
	static const char *texScaleLevelsNPOT[] = {"Auto", "Off", "2x", "3x"};
	static const char *texScaleLevelsPOT[] = {"Auto", "Off", "2x"};
#endif

	static const char **texScaleLevels;
	static int numTexScaleLevels;
	if (gl_extensions.OES_texture_npot) {
		texScaleLevels = texScaleLevelsNPOT;
		numTexScaleLevels = ARRAY_SIZE(texScaleLevelsNPOT);
	} else {
		texScaleLevels = texScaleLevelsPOT;
		numTexScaleLevels = ARRAY_SIZE(texScaleLevelsPOT);
	}
	PopupMultiChoice *texScalingChoice = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iTexScalingLevel, gr->T("Upscale Level"), texScaleLevels, 0, numTexScaleLevels, gr, screenManager()));
	texScalingChoice->SetDisabledPtr(&g_Config.bSoftwareRendering);

	static const char *texScaleAlgos[] = { "xBRZ", "Hybrid", "Bicubic", "Hybrid + Bicubic", };
	PopupMultiChoice *texScalingType = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iTexScalingType, gr->T("Upscale Type"), texScaleAlgos, 0, ARRAY_SIZE(texScaleAlgos), gr, screenManager()));
	texScalingType->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *deposterize = graphicsSettings->Add(new CheckBox(&g_Config.bTexDeposterize, gr->T("Deposterize")));
	deposterize->SetDisabledPtr(&g_Config.bSoftwareRendering);

	graphicsSettings->Add(new ItemHeader(gr->T("Texture Filtering")));
	static const char *anisoLevels[] = { "Off", "2x", "4x", "8x", "16x" };
	PopupMultiChoice *anisoFiltering = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iAnisotropyLevel, gr->T("Anisotropic Filtering"), anisoLevels, 0, ARRAY_SIZE(anisoLevels), gr, screenManager()));
	anisoFiltering->SetDisabledPtr(&g_Config.bSoftwareRendering);

	static const char *texFilters[] = { "Auto", "Nearest", "Linear", "Linear on FMV", };
	PopupMultiChoice *texFilter = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iTexFiltering, gr->T("Texture Filter"), texFilters, 1, ARRAY_SIZE(texFilters), gr, screenManager()));
	texFilter->SetDisabledPtr(&g_Config.bSoftwareRendering);

	static const char *bufFilters[] = { "Linear", "Nearest", };
	PopupMultiChoice *bufFilter = graphicsSettings->Add(new PopupMultiChoice(&g_Config.iBufFilter, gr->T("Screen Scaling Filter"), bufFilters, 1, ARRAY_SIZE(bufFilters), gr, screenManager()));
	bufFilter->SetDisabledPtr(&g_Config.bSoftwareRendering);

#ifdef ANDROID

#endif

	graphicsSettings->Add(new ItemHeader(gr->T("Hack Settings", "Hack Settings")));
	graphicsSettings->Add(new CheckBox(&g_Config.bTimerHack, gr->T("Timer Hack")));
	CheckBox *alphaHack = graphicsSettings->Add(new CheckBox(&g_Config.bDisableAlphaTest, gr->T("Disable Alpha Test (PowerVR speedup)")));
	alphaHack->OnClick.Handle(this, &GameSettingsScreen::OnShaderChange);
	alphaHack->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *stencilTest = graphicsSettings->Add(new CheckBox(&g_Config.bDisableStencilTest, gr->T("Disable Stencil Test")));
	stencilTest->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *depthWrite = graphicsSettings->Add(new CheckBox(&g_Config.bAlwaysDepthWrite, gr->T("Always Depth Write")));
	depthWrite->SetDisabledPtr(&g_Config.bSoftwareRendering);

	CheckBox *prescale = graphicsSettings->Add(new CheckBox(&g_Config.bPrescaleUV, gr->T("Texture Speedhack")));
	prescale->SetDisabledPtr(&g_Config.bSoftwareRendering);

<<<<<<< HEAD
CheckBox *RefreshDrawingHack = graphicsSettings->Add(new CheckBox(&g_Config.bRefreshDrawingHackb, gr->T("Disable Refresh Drawing - Speed in Games eg. COD but break Intro Scenes")));;
=======


>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

	CheckBox *blockHack = graphicsSettings->Add(new CheckBox(&g_Config.bBlockHack, gr->T("BlockHack - Speed in Games eg. Gow")));;
		CheckBox *threadHack = graphicsSettings->Add(new CheckBox(&g_Config.bThreadHackb, gr->T("ThreadHack - Speed in Games eg. Burnout but break sound")));;

<<<<<<< HEAD
    
    graphicsSettings->Add(new ItemHeader(gr->T("Mali Driver Settings Experimental", "Mali Driver Settings Experimental")));

CheckBox *MaliHack = graphicsSettings->Add(new CheckBox(&g_Config.bMaliHackb, gr->T("Enable driver hack - Should only work in Mali devices ")));;
    
    static const char *MaliHackCompress[] = { "2x", "4x", "8x"};
    graphicsSettings->Add(new PopupMultiChoice(&g_Config.iMaliHackCompressb, gr->T("Compress Data"), MaliHackCompress, 0, ARRAY_SIZE(MaliHackCompress), gr, screenManager()));
   
    
    
    static const char *MaliHackPixel[] = { "4", "8" , "16" , "32" , "64" , "128" , "256" , "512" , "1024", "2048"};
    graphicsSettings->Add(new PopupMultiChoice(&g_Config.iMaliHackPixelb, gr->T("PixelClock Increase"), MaliHackPixel, 0, ARRAY_SIZE(MaliHackPixel), gr, screenManager()));
    
    
    static const char *MaliHackBuffer[] = { "2x", "4x" , "8x","16x"};
    graphicsSettings->Add(new PopupMultiChoice(&g_Config.iMaliHackBufferb, gr->T("Decrease Buffer"), MaliHackBuffer, 0, ARRAY_SIZE(MaliHackBuffer), gr, screenManager()));
   
    
    
    
=======



>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	static const char *bloomHackOptions[] = { "Off", "Safe", "Balanced", "Aggressive" };

	bloomHackEnable_ = !g_Config.bSoftwareRendering && (g_Config.iInternalResolution != 1);


	graphicsSettings->Add(new ItemHeader(gr->T("Overlay Information")));
	static const char *fpsChoices[] = {
		"None", "Speed", "FPS", "Both"
#ifdef BLACKBERRY
     , "Statistics"
#endif
	};
	graphicsSettings->Add(new PopupMultiChoice(&g_Config.iShowFPSCounter, gr->T("Show FPS Counter"), fpsChoices, 0, ARRAY_SIZE(fpsChoices), gr, screenManager()));
	graphicsSettings->Add(new CheckBox(&g_Config.bShowDebugStats, gr->T("Show Debug Statistics")))->OnClick.Handle(this, &GameSettingsScreen::OnJitAffectingSetting);

	// Developer tools are not accessible ingame, so it goes here.
	graphicsSettings->Add(new ItemHeader(gr->T("Debugging")));
	Choice *dump = graphicsSettings->Add(new Choice(gr->T("Dump next frame to log")));
	dump->OnClick.Handle(this, &GameSettingsScreen::OnDumpNextFrameToLog);
	if (!PSP_IsInited())
		dump->SetEnabled(false);

	// We normally use software rendering to debug so put it in debugging.
	CheckBox *softwareGPU = graphicsSettings->Add(new CheckBox(&g_Config.bSoftwareRendering, gr->T("Software Rendering", "Software Rendering (experimental)")));
	softwareGPU->OnClick.Handle(this, &GameSettingsScreen::OnSoftwareRendering);

	if (PSP_IsInited() || g_Config.iGPUBackend != GPU_BACKEND_OPENGL)
		softwareGPU->SetEnabled(false);

	// Audio
	ViewGroup *audioSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *audioSettings = new LinearLayout(ORIENT_VERTICAL);
	audioSettings->SetSpacing(0);
	audioSettingsScroll->Add(audioSettings);
	tabHolder->AddTab(ms->T("Audio"), audioSettingsScroll);

	audioSettings->Add(new ItemHeader(ms->T("Audio")));

	audioSettings->Add(new CheckBox(&g_Config.bEnableSound, a->T("Enable Sound")));

#ifdef _WIN32
	if (IsVistaOrHigher()) {
		static const char *backend[] = { "Auto", "DSound (compatible)", "WASAPI (fast)" };
		PopupMultiChoice *audioBackend = audioSettings->Add(new PopupMultiChoice(&g_Config.iAudioBackend, a->T("Audio backend", "Audio backend (restart req.)"), backend, 0, ARRAY_SIZE(backend), a, screenManager()));
		audioBackend->SetEnabledPtr(&g_Config.bEnableSound);
	}
#endif

	static const char *latency[] = { "Low", "Medium", "High" };
	PopupMultiChoice *lowAudio = audioSettings->Add(new PopupMultiChoice(&g_Config.iAudioLatency, a->T("Audio Latency"), latency, 0, ARRAY_SIZE(latency), gr, screenManager()));

	lowAudio->SetEnabledPtr(&g_Config.bEnableSound);


	audioSettings->Add(new ItemHeader(a->T("Audio hacks")));
	audioSettings->Add(new CheckBox(&g_Config.bSoundSpeedHack, a->T("Sound speed hack")));

	// Control
	ViewGroup *controlsSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *controlsSettings = new LinearLayout(ORIENT_VERTICAL);
	controlsSettings->SetSpacing(0);
	controlsSettingsScroll->Add(controlsSettings);
	tabHolder->AddTab(ms->T("Controls"), controlsSettingsScroll);
	controlsSettings->Add(new ItemHeader(ms->T("Controls")));
	controlsSettings->Add(new Choice(co->T("Control Mapping")))->OnClick.Handle(this, &GameSettingsScreen::OnControlMapping);

#if defined(USING_WIN_UI)
	controlsSettings->Add(new CheckBox(&g_Config.bGamepadOnlyFocused, co->T("Ignore gamepads when not focused")));
#endif

#if defined(MOBILE_DEVICE)
	controlsSettings->Add(new CheckBox(&g_Config.bHapticFeedback, co->T("HapticFeedback", "Haptic Feedback (vibration)")));
	static const char *tiltTypes[] = { "None (Disabled)", "Analog Stick", "D-PAD", "PSP Action Buttons"};
	controlsSettings->Add(new PopupMultiChoice(&g_Config.iTiltInputType, co->T("Tilt Input Type"), tiltTypes, 0, ARRAY_SIZE(tiltTypes), co, screenManager()))->OnClick.Handle(this, &GameSettingsScreen::OnTiltTypeChange);

	Choice *customizeTilt = controlsSettings->Add(new Choice(co->T("Customize tilt")));
	customizeTilt->OnClick.Handle(this, &GameSettingsScreen::OnTiltCustomize);
	customizeTilt->SetEnabledPtr((bool *)&g_Config.iTiltInputType); //<- dirty int-to-bool cast
#endif

	// TVs don't have touch control, at least not yet.
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) != DEVICE_TYPE_TV) {
		controlsSettings->Add(new ItemHeader(co->T("OnScreen", "On-Screen Touch Controls")));
		controlsSettings->Add(new CheckBox(&g_Config.bShowTouchControls, co->T("OnScreen", "On-Screen Touch Controls")));
<<<<<<< HEAD
		controlsSettings->Add(new CheckBox(&g_Config.CustomLR, co->T("CustomLR", "Custom L/R Buttons")));
=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		layoutEditorChoice_ = controlsSettings->Add(new Choice(co->T("Custom layout...")));
		layoutEditorChoice_->OnClick.Handle(this, &GameSettingsScreen::OnTouchControlLayout);
		layoutEditorChoice_->SetEnabledPtr(&g_Config.bShowTouchControls);

		// Re-centers itself to the touch location on touch-down.
		CheckBox *floatingAnalog = controlsSettings->Add(new CheckBox(&g_Config.bAutoCenterTouchAnalog, co->T("Auto-centering analog stick")));
		floatingAnalog->SetEnabledPtr(&g_Config.bShowTouchControls);

		// On systems that aren't Symbian, iOS, and Maemo, offer to let the user see this button.
		// Some Windows touch devices don't have a back button or other button to call up the menu.
#if !defined(__SYMBIAN32__) && !defined(IOS) && !defined(MAEMO)
		CheckBox *enablePauseBtn = controlsSettings->Add(new CheckBox(&g_Config.bShowTouchPause, co->T("Show Touch Pause Menu Button")));

		// Don't allow the user to disable it once in-game, so they can't lock themselves out of the menu.
		if (!PSP_IsInited()) {
			enablePauseBtn->SetEnabledPtr(&g_Config.bShowTouchControls);
		} else {
			enablePauseBtn->SetEnabled(false);
		}
#endif

		CheckBox *disableDiags = controlsSettings->Add(new CheckBox(&g_Config.bDisableDpadDiagonals, co->T("Disable D-Pad diagonals (4-way touch)")));
		disableDiags->SetEnabledPtr(&g_Config.bShowTouchControls);
		View *opacity = controlsSettings->Add(new PopupSliderChoice(&g_Config.iTouchButtonOpacity, 0, 100, co->T("Button Opacity"), screenManager()));
		opacity->SetEnabledPtr(&g_Config.bShowTouchControls);
		static const char *touchControlStyles[] = {"Classic", "Thin borders"};
		View *style = controlsSettings->Add(new PopupMultiChoice(&g_Config.iTouchButtonStyle, co->T("Button style"), touchControlStyles, 0, ARRAY_SIZE(touchControlStyles), co, screenManager()));
		style->SetEnabledPtr(&g_Config.bShowTouchControls);
	}

#ifdef _WIN32
	static const char *inverseDeadzoneModes[] = { "Off", "X", "Y", "X + Y" };

	controlsSettings->Add(new ItemHeader(co->T("DInput Analog Settings", "DInput Analog Settings")));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fDInputAnalogDeadzone, 0.0f, 1.0f, co->T("Deadzone Radius"), screenManager()));
	controlsSettings->Add(new PopupMultiChoice(&g_Config.iDInputAnalogInverseMode, co->T("Analog Mapper Mode"), inverseDeadzoneModes, 0, ARRAY_SIZE(inverseDeadzoneModes), co, screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fDInputAnalogInverseDeadzone, 0.0f, 1.0f, co->T("Analog Mapper Low End", "Analog Mapper Low End (Inverse Deadzone)"), screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fDInputAnalogSensitivity, 0.0f, 10.0f, co->T("Analog Mapper High End", "Analog Mapper High End (Axis Sensitivity)"), screenManager()));

	controlsSettings->Add(new ItemHeader(co->T("XInput Analog Settings", "XInput Analog Settings")));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fXInputAnalogDeadzone, 0.0f, 1.0f, co->T("Deadzone Radius"), screenManager()));
	controlsSettings->Add(new PopupMultiChoice(&g_Config.iXInputAnalogInverseMode, co->T("Analog Mapper Mode"), inverseDeadzoneModes, 0, ARRAY_SIZE(inverseDeadzoneModes), co, screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fXInputAnalogInverseDeadzone, 0.0f, 1.0f, co->T("Analog Mapper Low End", "Analog Mapper Low End (Inverse Deadzone)"), screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fXInputAnalogSensitivity, 0.0f, 10.0f, co->T("Analog Mapper High End", "Analog Mapper High End (Axis Sensitivity)"), screenManager()));
#endif

	controlsSettings->Add(new ItemHeader(co->T("Keyboard", "Keyboard Control Settings")));
#if defined(USING_WIN_UI)
	controlsSettings->Add(new CheckBox(&g_Config.bIgnoreWindowsKey, co->T("Ignore Windows Key")));
#endif // #if defined(USING_WIN_UI)
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_Config.fAnalogLimiterDeadzone, 0.0f, 1.0f, co->T("Analog Limiter"), 0.10f, screenManager()));

<<<<<<< HEAD
	ViewGroup *networkingSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *networkingSettings = new LinearLayout(ORIENT_VERTICAL);
	networkingSettings->SetSpacing(0);
	networkingSettingsScroll->Add(networkingSettings);
	tabHolder->AddTab(ms->T("Networking"), networkingSettingsScroll);

	networkingSettings->Add(new ItemHeader(ms->T("Networking")));



	networkingSettings->Add(new CheckBox(&g_Config.bEnableWlan, n->T("Enable networking", "Enable networking")));
=======
	//ViewGroup *networkingSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	//LinearLayout *networkingSettings = new LinearLayout(ORIENT_VERTICAL);
	//networkingSettings->SetSpacing(0);
	//networkingSettingsScroll->Add(networkingSettings);
	//tabHolder->AddTab(ms->T("Networking"), networkingSettingsScroll);

	//networkingSettings->Add(new ItemHeader(ms->T("Networking")));



	//networkingSettings->Add(new CheckBox(&g_Config.bEnableWlan, n->T("Enable networking", "Enable networking")));
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

#ifdef _WIN32
	networkingSettings->Add(new PopupTextInputChoice(&g_Config.proAdhocServer, n->T("Change proAdhocServer Address"), "", 255, screenManager()));
#else
<<<<<<< HEAD
networkingSettings->Add(new ChoiceWithValueDisplay(&g_Config.proAdhocServer, n->T("Change proAdhocServer Address"), nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeproAdhocServerAddress);
#endif
networkingSettings->Add(new CheckBox(&g_Config.bEnableAdhocServer, n->T("Enable built-in PRO Adhoc Server", "Enable built-in PRO Adhoc Server")));
	networkingSettings->Add(new ChoiceWithValueDisplay(&g_Config.sMACAddress, n->T("Change Mac Address"), nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeMacAddress);

	
=======

#endif

	//networkingSettings->Add(new ChoiceWithValueDisplay(&g_Config.sMACAddress, n->T("Change Mac Address"), nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeMacAddress);

>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

	// System
	ViewGroup *systemSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *systemSettings = new LinearLayout(ORIENT_VERTICAL);
	systemSettings->SetSpacing(0);
	systemSettingsScroll->Add(systemSettings);
	tabHolder->AddTab(ms->T("System"), systemSettingsScroll);

	systemSettings->Add(new ItemHeader(sy->T("UI Language")));
	systemSettings->Add(new Choice(dev->T("Language", "Language")))->OnClick.Handle(this, &GameSettingsScreen::OnLanguage);



	systemSettings->Add(new ItemHeader(sy->T("Emulation")));

	systemSettings->Add(new CheckBox(&g_Config.bFastMemory, sy->T("Fast Memory", "Fast Memory (Unstable)")))->OnClick.Handle(this, &GameSettingsScreen::OnJitAffectingSetting);

	systemSettings->Add(new CheckBox(&g_Config.bSeparateCPUThread, sy->T("Multithreaded")));
	systemSettings->Add(new CheckBox(&g_Config.bSeparateIOThread, sy->T("I/O on thread")))->SetEnabled(!PSP_IsInited());
	static const char *ioTimingMethods[] = { "Fast (lag on slow storage)", "Host (bugs, less lag)", "Simulate UMD delays" };

#ifndef MOBILE_DEVICE

#endif
	systemSettings->Add(new CheckBox(&g_Config.bSetRoundingMode, sy->T("Respect FPU rounding (disable for old GEB saves)")))->OnClick.Handle(this, &GameSettingsScreen::OnJitAffectingSetting);

	systemSettings->Add(new ItemHeader(sy->T("General")));

#ifdef ANDROID
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) == DEVICE_TYPE_MOBILE) {
		static const char *screenRotation[] = { "Landscape", "Landscape Reversed"};
		PopupMultiChoice *rot = systemSettings->Add(new PopupMultiChoice(&g_Config.iScreenRotation, co->T("Screen Rotation"), screenRotation, 0, ARRAY_SIZE(screenRotation), co, screenManager()));
		rot->OnChoice.Handle(this, &GameSettingsScreen::OnScreenRotation);
	}
#endif


	if (g_Config.iMaxRecent > 0)
		systemSettings->Add(new Choice(sy->T("Clear Recent Games List")))->OnClick.Handle(this, &GameSettingsScreen::OnClearRecents);
	systemSettings->Add(new Choice(sy->T("Restore Default Settings")))->OnClick.Handle(this, &GameSettingsScreen::OnRestoreDefaultSettings);
	systemSettings->Add(new CheckBox(&g_Config.bEnableAutoLoad, sy->T("Auto Load Newest Savestate")));

<<<<<<< HEAD
   
	
=======

	systemSettings->Add(new ItemHeader(sy->T("Networking")));
	systemSettings->Add(new CheckBox(&g_Config.bEnableWlan, sy->T("Enable networking", "Enable networking")));



	systemSettings->Add(new ChoiceWithValueDisplay(&g_Config.sMACAddress, sy->T("Change Mac Address"), nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeMacAddress);


>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2





#if defined(USING_WIN_UI)
	systemSettings->Add(new CheckBox(&g_Config.bBypassOSKWithKeyboard, sy->T("Enable Windows native keyboard", "Enable Windows native keyboard")));
#endif
#if defined(_WIN32)
	SavePathInMyDocumentChoice = systemSettings->Add(new CheckBox(&installed_, sy->T("Save path in My Documents", "Save path in My Documents")));
	SavePathInMyDocumentChoice->OnClick.Handle(this, &GameSettingsScreen::OnSavePathMydoc);
	SavePathInOtherChoice = systemSettings->Add(new CheckBox(&otherinstalled_, sy->T("Save path in installed.txt", "Save path in installed.txt")));
	SavePathInOtherChoice->SetEnabled(false);
	SavePathInOtherChoice->OnClick.Handle(this, &GameSettingsScreen::OnSavePathOther);
	wchar_t myDocumentsPath[MAX_PATH];
	const HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocumentsPath);
	const std::string PPSSPPpath = File::GetExeDirectory();
	const std::string installedFile = PPSSPPpath + "installed.txt";
	installed_ = File::Exists(installedFile);
	otherinstalled_ = false;
	if (!installed_ && result == S_OK) {
		if (File::CreateEmptyFile(PPSSPPpath + "installedTEMP.txt")) {
			// Disable the setting whether cannot create & delete file
			if (!(File::Delete(PPSSPPpath + "installedTEMP.txt")))
				SavePathInMyDocumentChoice->SetEnabled(false);
			else
				SavePathInOtherChoice->SetEnabled(true);
		} else
			SavePathInMyDocumentChoice->SetEnabled(false);
	} else {
		if (installed_ && (result == S_OK)) {
			std::ifstream inputFile(ConvertUTF8ToWString(installedFile));
			if (!inputFile.fail() && inputFile.is_open()) {
				std::string tempString;
				std::getline(inputFile, tempString);

				// Skip UTF-8 encoding bytes if there are any. There are 3 of them.
				if (tempString.substr(0, 3) == "\xEF\xBB\xBF")
					tempString = tempString.substr(3);
				SavePathInOtherChoice->SetEnabled(true);
				if (!(tempString == "")) {
					installed_ = false;
					otherinstalled_ = true;
				}
			}
			inputFile.close();
		} else if (result != S_OK)
			SavePathInMyDocumentChoice->SetEnabled(false);
	}
#endif

#if defined(_M_X64)
	systemSettings->Add(new CheckBox(&g_Config.bCacheFullIsoInRam, sy->T("Cache ISO in RAM", "Cache full ISO in RAM (slow startup)")));
#endif

//#ifndef ANDROID

//#endif
	systemSettings->SetSpacing(0);

	systemSettings->Add(new ItemHeader(sy->T("PSP Settings")));
	static const char *models[] = {"PSP-1000" , "PSP-2000/3000"};
	systemSettings->Add(new PopupMultiChoice(&g_Config.iPSPModel, sy->T("PSP Model"), models, 0, ARRAY_SIZE(models), sy, screenManager()))->SetEnabled(!PSP_IsInited());
	// TODO: Come up with a way to display a keyboard for mobile users,
	// so until then, this is Windows/Desktop only.
#if defined(_WIN32)  // TODO: Add all platforms where KEY_CHAR support is added
	systemSettings->Add(new PopupTextInputChoice(&g_Config.sNickName, sy->T("Change Nickname"), "", 32, screenManager()));
#elif defined(USING_QT_UI)
	systemSettings->Add(new Choice(sy->T("Change Nickname")))->OnClick.Handle(this, &GameSettingsScreen::OnChangeNickname);
#endif
#if defined(_WIN32) || (defined(USING_QT_UI) && !defined(MOBILE_DEVICE))
	// Screenshot functionality is not yet available on non-Windows/non-Qt
	systemSettings->Add(new CheckBox(&g_Config.bScreenshotsAsPNG, sy->T("Screenshots as PNG")));
#endif
	systemSettings->Add(new CheckBox(&g_Config.bDayLightSavings, sy->T("Day Light Saving")));
	static const char *dateFormat[] = { "YYYYMMDD", "MMDDYYYY", "DDMMYYYY"};
	systemSettings->Add(new PopupMultiChoice(&g_Config.iDateFormat, sy->T("Date Format"), dateFormat, 1, 3, sy, screenManager()));
	static const char *timeFormat[] = { "12HR", "24HR"};
	systemSettings->Add(new PopupMultiChoice(&g_Config.iTimeFormat, sy->T("Time Format"), timeFormat, 1, 2, sy, screenManager()));
	static const char *buttonPref[] = { "Use O to confirm", "Use X to confirm" };
	systemSettings->Add(new PopupMultiChoice(&g_Config.iButtonPreference, sy->T("Confirmation Button"), buttonPref, 0, 2, sy, screenManager()));
}

UI::EventReturn GameSettingsScreen::OnAutoFrameskip(UI::EventParams &e) {
	if (g_Config.bAutoFrameSkip && g_Config.iFrameSkip == 0) {
		g_Config.iFrameSkip = 1;
	}

	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSoftwareRendering(UI::EventParams &e) {
	vtxCacheEnable_ = !g_Config.bSoftwareRendering && g_Config.bHardwareTransform;
	postProcEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);
	resolutionEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnHardwareTransform(UI::EventParams &e) {
	vtxCacheEnable_ = !g_Config.bSoftwareRendering && g_Config.bHardwareTransform;
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnScreenRotation(UI::EventParams &e) {
	System_SendMessage("rotate", "");
	return UI::EVENT_DONE;
}

static void RecreateActivity() {
	const int SYSTEM_JELLYBEAN = 16;
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= SYSTEM_JELLYBEAN) {
		System_SendMessage("recreate", "");
	} else {
		I18NCategory *gr = GetI18NCategory("Graphics");
		System_SendMessage("toast", gr->T("Must Restart", "You must restart PSPe+ for this change to take effect"));
	}
}

UI::EventReturn GameSettingsScreen::OnAdhocGuides(UI::EventParams &e) {
	LaunchBrowser("");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnImmersiveModeChange(UI::EventParams &e) {
	System_SendMessage("immersive", "");
	const int SYSTEM_JELLYBEAN = 16;
	// recreate doesn't seem reliable on earlier versions.
	if (g_Config.iAndroidHwScale != 0) {
		RecreateActivity();
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnRenderingMode(UI::EventParams &e) {
	// We do not want to report when rendering mode is Framebuffer to memory - so many issues
	// are caused by that (framebuffer copies overwriting display lists, etc).
	Reporting::UpdateConfig();
	enableReports_ = Reporting::IsEnabled();
	enableReportsCheckbox_->SetEnabled(Reporting::IsSupported());

	postProcEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);
	resolutionEnable_ = !g_Config.bSoftwareRendering && (g_Config.iRenderingMode != FB_NON_BUFFERED_MODE);

	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnJitAffectingSetting(UI::EventParams &e) {
	NativeMessageReceived("clear jit", "");
	return UI::EVENT_DONE;
}

#ifdef _WIN32

UI::EventReturn GameSettingsScreen::OnSavePathMydoc(UI::EventParams &e) {
	const std::string PPSSPPpath = File::GetExeDirectory();
	const std::string installedFile = PPSSPPpath + "installed.txt";
	installed_ = File::Exists(installedFile);
	if (otherinstalled_) {
		File::Delete(PPSSPPpath + "installed.txt");
		File::CreateEmptyFile(PPSSPPpath + "installed.txt");
		otherinstalled_ = false;
		wchar_t myDocumentsPath[MAX_PATH];
		const HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocumentsPath);
		const std::string myDocsPath = ConvertWStringToUTF8(myDocumentsPath) + "/PPSSPP/";
		g_Config.memStickDirectory = myDocsPath;
	}
	else if (installed_) {
		File::Delete(PPSSPPpath + "installed.txt");
		installed_ = false;
		g_Config.memStickDirectory = PPSSPPpath + "memstick/";
	}
	else {
		ofstream myfile;
		myfile.open(PPSSPPpath + "installed.txt");
		if (myfile.is_open()){
			myfile.close();
		}

		wchar_t myDocumentsPath[MAX_PATH];
		const HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocumentsPath);
		const std::string myDocsPath = ConvertWStringToUTF8(myDocumentsPath) + "/PPSSPP/";
		g_Config.memStickDirectory = myDocsPath;
		installed_ = true;
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSavePathOther(UI::EventParams &e) {
	const std::string PPSSPPpath = File::GetExeDirectory();	
	if (otherinstalled_) {
		I18NCategory *di = GetI18NCategory("Dialog");
		std::string folder = W32Util::BrowseForFolder(MainWindow::GetHWND(), di->T("Choose PPSSPP save folder"));
		if (folder.size()) {
			g_Config.memStickDirectory = folder;
			FILE *f = File::OpenCFile(PPSSPPpath + "installed.txt", "wb");
			if (f) {
				std::string utfstring("\xEF\xBB\xBF");
				utfstring.append(folder);
				fwrite(utfstring.c_str(), 1, utfstring.length(), f);
				fclose(f);
			}
			installed_ = false;
		}
		else
			otherinstalled_ = false;
	}
	else {
		File::Delete(PPSSPPpath + "installed.txt");
		SavePathInMyDocumentChoice->SetEnabled(true);
		otherinstalled_ = false;
		installed_ = false;
		g_Config.memStickDirectory = PPSSPPpath + "memstick/";
	}
	return UI::EVENT_DONE;
}

#endif

UI::EventReturn GameSettingsScreen::OnClearRecents(UI::EventParams &e) {
	g_Config.recentIsos.clear();
	OnRecentChanged.Trigger(e);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnReloadCheats(UI::EventParams &e) {
	// Hmm, strange mechanism.
	g_Config.bReloadCheats = true;
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnFullscreenChange(UI::EventParams &e) {
#if defined(USING_WIN_UI) || defined(USING_QT_UI)
	host->GoFullscreen(g_Config.bFullScreen);
#else
	// SDL, basically.
	System_SendMessage("toggle_fullscreen", "");
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnResolutionChange(UI::EventParams &e) {
	if (gpu) {
		gpu->Resized();
	}
	if (g_Config.iAndroidHwScale == 1) {
		RecreateActivity();
	}
	Reporting::UpdateConfig();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnHwScaleChange(UI::EventParams &e) {
	RecreateActivity();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnShaderChange(UI::EventParams &e) {
	if (gpu) {
		gpu->ClearShaderCache();
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnDumpNextFrameToLog(UI::EventParams &e) {
	if (gpu) {
		gpu->DumpNextFrame();
	}
	return UI::EVENT_DONE;
}

void GameSettingsScreen::update(InputState &input) {
	UIScreen::update(input);
	g_Config.iForceMaxEmulatedFPS = cap60FPS_ ? 60 : 0;

	g_Config.iFpsLimit = (iAlternateSpeedPercent_ * 60) / 100;

	bool vertical = UseVerticalLayout();
	if (vertical != lastVertical_) {
		RecreateViews();
		lastVertical_ = vertical;
	}
}

void GameSettingsScreen::sendMessage(const char *message, const char *value) {
	// Always call the base class method first to handle the most common messages.
	UIDialogScreenWithBackground::sendMessage(message, value);

	if (!strcmp(message, "control mapping")) {
		UpdateUIState(UISTATE_MENU);
		screenManager()->push(new ControlMappingScreen());
	}
}

void GameSettingsScreen::onFinish(DialogResult result) {
	if (g_Config.bEnableSound) {
		if (PSP_IsInited() && !IsAudioInitialised())
			Audio_Init();
	}

	Reporting::Enable(enableReports_, "");
	Reporting::UpdateConfig();
	g_Config.Save();


	host->UpdateUI();

	KeyMap::UpdateConfirmCancelKeys();
}

/*
void GlobalSettingsScreen::CreateViews() {
	using namespace UI;
	root_ = new ScrollView(ORIENT_VERTICAL);

	enableReports_ = Reporting::IsEnabled();
}*/

void GameSettingsScreen::CallbackRenderingBackend(bool yes) {
#if defined(_WIN32)
	// If the user ends up deciding not to restart, set the temporary variable back to the current backend
	// so it doesn't get switched by accident.
	if (yes) {
		g_Config.bRestartRequired = true;
		PostMessage(MainWindow::GetHWND(), WM_CLOSE, 0, 0);
	} else {
		g_Config.iTempGPUBackend = g_Config.iGPUBackend;
	}
#endif
}

UI::EventReturn GameSettingsScreen::OnRenderingBackend(UI::EventParams &e) {
#if defined(_WIN32)
	I18NCategory *di = GetI18NCategory("Dialog");

	// It only makes sense to show the restart prompt if the backend was actually changed.
	if (g_Config.iTempGPUBackend != g_Config.iGPUBackend) {
		screenManager()->push(new PromptScreen(di->T("ChangingGPUBackends", "Changing GPU backends requires PPSSPP to restart. Restart now?"), di->T("Yes"), di->T("No"),
			std::bind(&GameSettingsScreen::CallbackRenderingBackend, this, placeholder::_1)));
	}
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeNickname(UI::EventParams &e) {
#if defined(_WIN32) || defined(USING_QT_UI)
	const size_t name_len = 256;

	char name[name_len];
	memset(name, 0, sizeof(name));

	if (System_InputBoxGetString("Enter a new PSP nickname", g_Config.sNickName.c_str(), name, name_len)) {
		g_Config.sNickName = name;
	}
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeproAdhocServerAddress(UI::EventParams &e) {	
#if defined(_WIN32) || defined(USING_QT_UI)	
	if (!g_Config.bFullScreen) {
		const size_t name_len = 256;

		char name[name_len];
		memset(name, 0, sizeof(name));

		if (System_InputBoxGetString("Enter an IP address", g_Config.proAdhocServer.c_str(), name, name_len)) {
			g_Config.proAdhocServer = name;
		}
	}
	else
		screenManager()->push(new ProAdhocServerScreen);
#else
	screenManager()->push(new ProAdhocServerScreen);
#endif
	
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeMacAddress(UI::EventParams &e) {
	g_Config.sMACAddress = std::string(CreateRandMAC());

	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnLanguage(UI::EventParams &e) {
	I18NCategory *dev = GetI18NCategory("Developer");
	auto langScreen = new NewLanguageScreen(dev->T("Language"));
	langScreen->OnChoice.Handle(this, &GameSettingsScreen::OnLanguageChange);
	screenManager()->push(langScreen);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnLanguageChange(UI::EventParams &e) {
	screenManager()->RecreateAllViews();

	if (host) {
		host->UpdateUI();
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnPostProcShader(UI::EventParams &e) {
	I18NCategory *gr = GetI18NCategory("Graphics");
	auto procScreen = new PostProcScreen(gr->T("Postprocessing Shader"));
	procScreen->OnChoice.Handle(this, &GameSettingsScreen::OnPostProcShaderChange);
	screenManager()->push(procScreen);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnPostProcShaderChange(UI::EventParams &e) {
	if (gpu) {
		gpu->Resized();
	}
	Reporting::UpdateConfig();
	return UI::EVENT_DONE;
}
UI::EventReturn GameSettingsScreen::OnDeveloperTools(UI::EventParams &e) {
	screenManager()->push(new DeveloperToolsScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnControlMapping(UI::EventParams &e) {
	screenManager()->push(new ControlMappingScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnTouchControlLayout(UI::EventParams &e) {
	screenManager()->push(new TouchControlLayoutScreen());
	return UI::EVENT_DONE;
};

//when the tilt event type is modified, we need to reset all tilt settings.
//refer to the ResetTiltEvents() function for a detailed explanation.
UI::EventReturn GameSettingsScreen::OnTiltTypeChange(UI::EventParams &e){
	TiltEventProcessor::ResetTiltEvents();
	return UI::EVENT_DONE;
};

UI::EventReturn GameSettingsScreen::OnTiltCustomize(UI::EventParams &e){
	screenManager()->push(new TiltAnalogSettingsScreen());
	return UI::EVENT_DONE;
};

UI::EventReturn GameSettingsScreen::OnSavedataManager(UI::EventParams &e) {
	auto saveData = new SavedataScreen("");
	//screenManager()->push(saveData);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSysInfo(UI::EventParams &e) {
	screenManager()->push(new SystemInfoScreen());
	return UI::EVENT_DONE;
}

void DeveloperToolsScreen::CreateViews() {
	using namespace UI;
	root_ = new ScrollView(ORIENT_VERTICAL);

	I18NCategory *di = GetI18NCategory("Dialog");
	I18NCategory *dev = GetI18NCategory("Developer");
	I18NCategory *gr = GetI18NCategory("Graphics");
	I18NCategory *a = GetI18NCategory("Audio");
	I18NCategory *sy = GetI18NCategory("System");

	LinearLayout *list = root_->Add(new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(1.0f)));
	list->SetSpacing(0);
	list->Add(new ItemHeader(sy->T("General")));

	bool canUseJit = true;
#ifdef IOS
	if (!iosCanUseJit) {
		canUseJit = false;
		list->Add(new TextView(sy->T("DynarecisJailed", "Dynarec (JIT) - (Not jailbroken - JIT not available)")));
	}
#endif
	if (canUseJit) {
		list->Add(new CheckBox(&g_Config.bJit, sy->T("Dynarec", "Dynarec (JIT)")))->OnClick.Handle(this, &DeveloperToolsScreen::OnJitAffectingSetting);
	}

	list->Add(new CheckBox(&g_Config.bShowDeveloperMenu, dev->T("Show Developer Menu")));
	list->Add(new CheckBox(&g_Config.bDumpDecryptedEboot, dev->T("Dump Decrypted Eboot", "Dump Decrypted EBOOT.BIN (If Encrypted) When Booting Game")));

	Choice *cpuTests = new Choice(dev->T("Run CPU Tests"));
	list->Add(cpuTests)->OnClick.Handle(this, &DeveloperToolsScreen::OnRunCPUTests);
#ifdef IOS
	const std::string testDirectory = g_Config.flash0Directory + "../";
#else
	const std::string testDirectory = g_Config.memStickDirectory;
#endif
	if (!File::Exists(testDirectory + "pspautotests/tests/")) {
		cpuTests->SetEnabled(false);
	}

	list->Add(new CheckBox(&g_Config.bEnableLogging, dev->T("Enable Logging")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLoggingChanged);
	list->Add(new Choice(dev->T("Logging Channels")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLogConfig);
	list->Add(new ItemHeader(dev->T("Language")));
	list->Add(new Choice(dev->T("Load language ini")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLoadLanguageIni);
	list->Add(new Choice(dev->T("Save language ini")))->OnClick.Handle(this, &DeveloperToolsScreen::OnSaveLanguageIni);
	list->Add(new ItemHeader(""));
	list->Add(new Choice(di->T("Back")))->OnClick.Handle<UIScreen>(this, &UIScreen::OnBack);
}

void DeveloperToolsScreen::onFinish(DialogResult result) {
	g_Config.Save();
}

void GameSettingsScreen::CallbackRestoreDefaults(bool yes) {
	if (yes)
		g_Config.RestoreDefaults();
	host->UpdateUI();
}

UI::EventReturn GameSettingsScreen::OnRestoreDefaultSettings(UI::EventParams &e) {
	I18NCategory *dev = GetI18NCategory("Developer");
	I18NCategory *di = GetI18NCategory("Dialog");

		screenManager()->push(
			new PromptScreen(dev->T("RestoreGameDefaultSettings", "Are you sure you want to restore all settings(except control mapping)\nback to their defaults?\nYou can't undo this.\nPlease restart PSPe+ after restoring settings."), di->T("OK"), di->T("Cancel"),

			std::bind(&GameSettingsScreen::CallbackRestoreDefaults, this, placeholder::_1)));


	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLoggingChanged(UI::EventParams &e) {
	host->ToggleDebugConsoleVisibility();
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnRunCPUTests(UI::EventParams &e) {
	RunTests();
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnSaveLanguageIni(UI::EventParams &e) {
	i18nrepo.SaveIni(g_Config.sLanguageIni);
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLoadLanguageIni(UI::EventParams &e) {
	i18nrepo.LoadIni(g_Config.sLanguageIni);
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLogConfig(UI::EventParams &e) {
	screenManager()->push(new LogConfigScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnJitAffectingSetting(UI::EventParams &e) {
	NativeMessageReceived("clear jit", "");
	return UI::EVENT_DONE;
}

void ProAdhocServerScreen::CreateViews() {
	using namespace UI;	
	I18NCategory *sy = GetI18NCategory("System");
	I18NCategory *di = GetI18NCategory("Dialog");
	
	tempProAdhocServer = g_Config.proAdhocServer;
	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
	LinearLayout *leftColumn = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	
	leftColumn->Add(new ItemHeader(sy->T("proAdhocServer Address:")));
	addrView_ = new TextView(tempProAdhocServer, ALIGN_LEFT, false);
	leftColumn->Add(addrView_);
	LinearLayout *rightColumn = new LinearLayout(ORIENT_HORIZONTAL, new AnchorLayoutParams(0, 120, 10, NONE, NONE,10));
	rightColumn->Add(new Button("0"))->OnClick.Handle(this, &ProAdhocServerScreen::On0Click);
	rightColumn->Add(new Button("1"))->OnClick.Handle(this, &ProAdhocServerScreen::On1Click);
	rightColumn->Add(new Button("2"))->OnClick.Handle(this, &ProAdhocServerScreen::On2Click);
	rightColumn->Add(new Button("3"))->OnClick.Handle(this, &ProAdhocServerScreen::On3Click);
	rightColumn->Add(new Button("4"))->OnClick.Handle(this, &ProAdhocServerScreen::On4Click);
	rightColumn->Add(new Button("5"))->OnClick.Handle(this, &ProAdhocServerScreen::On5Click);
	rightColumn->Add(new Button("6"))->OnClick.Handle(this, &ProAdhocServerScreen::On6Click);
	rightColumn->Add(new Button("7"))->OnClick.Handle(this, &ProAdhocServerScreen::On7Click);
	rightColumn->Add(new Button("8"))->OnClick.Handle(this, &ProAdhocServerScreen::On8Click);
	rightColumn->Add(new Button("9"))->OnClick.Handle(this, &ProAdhocServerScreen::On9Click);
	rightColumn->Add(new Button("."))->OnClick.Handle(this, &ProAdhocServerScreen::OnPointClick);
	rightColumn->Add(new Button(di->T("Delete")))->OnClick.Handle(this, &ProAdhocServerScreen::OnDeleteClick);
	rightColumn->Add(new Button(di->T("Delete all")))->OnClick.Handle(this, &ProAdhocServerScreen::OnDeleteAllClick);
	rightColumn->Add(new Button(di->T("OK")))->OnClick.Handle(this, &ProAdhocServerScreen::OnOKClick);
	rightColumn->Add(new Button(di->T("Cancel")))->OnClick.Handle(this, &ProAdhocServerScreen::OnCancelClick);
	root_->Add(leftColumn);
	root_->Add(rightColumn);
}

UI::EventReturn ProAdhocServerScreen::On0Click(UI::EventParams &e) {
	if (tempProAdhocServer.length() > 0)
		tempProAdhocServer.append("0");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On1Click(UI::EventParams &e) {
	tempProAdhocServer.append("1");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On2Click(UI::EventParams &e) {
	tempProAdhocServer.append("2");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On3Click(UI::EventParams &e) {
	tempProAdhocServer.append("3");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On4Click(UI::EventParams &e) {
	tempProAdhocServer.append("4");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On5Click(UI::EventParams &e) {
	tempProAdhocServer.append("5");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On6Click(UI::EventParams &e) {
	tempProAdhocServer.append("6");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On7Click(UI::EventParams &e) {
	tempProAdhocServer.append("7");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On8Click(UI::EventParams &e) {
	tempProAdhocServer.append("8");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::On9Click(UI::EventParams &e) {
	tempProAdhocServer.append("9");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}


UI::EventReturn ProAdhocServerScreen::OnPointClick(UI::EventParams &e) {
	if (tempProAdhocServer.length() > 0 && tempProAdhocServer.at(tempProAdhocServer.length() - 1) != '.')
		tempProAdhocServer.append(".");
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::OnDeleteClick(UI::EventParams &e) {
	if (tempProAdhocServer.length() > 0)
		tempProAdhocServer.erase(tempProAdhocServer.length() -1, 1);
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::OnDeleteAllClick(UI::EventParams &e) {
	tempProAdhocServer = "";
	addrView_->SetText(tempProAdhocServer);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::OnOKClick(UI::EventParams &e) {
	g_Config.proAdhocServer = tempProAdhocServer;
	UIScreen::OnBack(e);
	return UI::EVENT_DONE;
}

UI::EventReturn ProAdhocServerScreen::OnCancelClick(UI::EventParams &e) {
	tempProAdhocServer = g_Config.proAdhocServer;
	UIScreen::OnBack(e);
	return UI::EVENT_DONE;
}
