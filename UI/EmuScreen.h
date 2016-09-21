// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <string>
#include <vector>
#include <list>

#include "input/keycodes.h"
#include "ui/screen.h"
#include "ui/ui_screen.h"
#include "Common/KeyMap.h"

struct AxisInput;

class AsyncImageFileView;

class EmuScreen : public UIScreen {
public:
	EmuScreen(const std::string &filename);
	~EmuScreen();

	virtual void update(InputState &input) override;
	virtual void render() override;
	virtual void deviceLost() override;
	virtual void dialogFinished(const Screen *dialog, DialogResult result) override;
	virtual void sendMessage(const char *msg, const char *value) override;

	virtual bool touch(const TouchInput &touch) override;
	virtual bool key(const KeyInput &key) override;
	virtual bool axis(const AxisInput &axis) override;

protected:
	virtual void CreateViews();
	UI::EventReturn OnDevTools(UI::EventParams &params);

private:
	void bootGame(const std::string &filename);
	void bootComplete();
	void processAxis(const AxisInput &axis, int direction);

	void pspKey(int pspKeyCode, int flags);
	void onVKeyDown(int virtualKeyCode);
	void onVKeyUp(int virtualKeyCode);
	void setVKeyAnalogX(int stick, int virtualKeyMin, int virtualKeyMax);
	void setVKeyAnalogY(int stick, int virtualKeyMin, int virtualKeyMax);

	void releaseButtons();

	void autoLoad();
	void checkPowerDown();

	bool bootPending_;
	std::string gamePath_;

	// Something invalid was loaded, don't try to emulate
	bool invalid_;
	bool quit_;
	std::string errorMessage_;

	// If set, pauses at the end of the frame.
	bool pauseTrigger_;

	// To track mappable virtual keys. We can have as many as we want.
	bool virtKeys[VIRTKEY_COUNT];

	// In-memory save state used for freezeFrame, which is useful for debugging.
	std::vector<u8> freezeState_;

	std::string tag_;

	// De-noise mapped axis updates
	int axisState_[JOYSTICK_AXIS_MAX];

	double saveStatePreviewShownTime_;
	AsyncImageFileView *saveStatePreview_;
};
