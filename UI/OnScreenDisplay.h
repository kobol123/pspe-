// Copyright (c) 2015- PSPe+ Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/



#pragma once

#include <string>
#include <list>

#include "base/basictypes.h"
#include "math/geom2d.h"
#include "Common/StdMutex.h"

#include "ui/view.h"

class DrawBuffer;

class OnScreenMessages {
public:
	void Show(const std::string &message, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1, bool checkUnique = true);
	void ShowOnOff(const std::string &message, bool b, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1);
	bool IsEmpty() const { return messages_.empty(); }

	void Lock() {
		mutex_.lock();
	}
	void Unlock() {
		mutex_.unlock();
	}

	void Clean();

	struct Message {
		int icon;
		uint32_t color;
		std::string text;
		double endTime;
		double duration;
	};
	const std::list<Message> &Messages() { return messages_; }

private:

	std::list<Message> messages_;
	std::recursive_mutex mutex_;
};

class OnScreenMessagesView : public UI::InertView {
public:
	OnScreenMessagesView(UI::LayoutParams *layoutParams = nullptr) : UI::InertView(layoutParams) {}
	void Draw(UIContext &dc);
};

extern OnScreenMessages osm;
