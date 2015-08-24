// Copyright (c) 2015- PPSSPP Project.

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

#include "base/functional.h"
#include "ui/ui_screen.h"
#include "ui/view.h"
#include "ui/viewgroup.h"

#include "UI/MiscScreens.h"

class SavedataBrowser : public UI::LinearLayout {
public:
	SavedataBrowser(std::string path, UI::LayoutParams *layoutParams = 0);
	~SavedataBrowser();

	UI::Event OnChoice;

private:
	void Refresh();
	UI::EventReturn SavedataButtonClick(UI::EventParams &e);

	std::string path_;
	UI::ViewGroup *gameList_;
};

class SavedataScreen : public UIDialogScreenWithGameBackground {
public:
	// gamePath can be empty, in that case this screen will show all savedata in the save directory.
	SavedataScreen(std::string gamePath);

	void dialogFinished(const Screen *dialog, DialogResult result) override;

protected:
	UI::EventReturn OnSavedataButtonClick(UI::EventParams &e);
	void CreateViews() override;
	bool gridStyle_;
	SavedataBrowser *browser_;
};
