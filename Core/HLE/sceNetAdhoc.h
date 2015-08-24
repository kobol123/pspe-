// Copyright (c) 2013- PPSSPP Project.

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

class PointerWrap;

void Register_sceNetAdhoc();

void __NetAdhocInit();
void __NetAdhocShutdown();
void __NetAdhocDoState(PointerWrap &p);
void __UpdateAdhocctlHandlers(u32 flags, u32 error);
void __UpdateMatchingHandler(u64 params);

// I have to call this from netdialog
int sceNetAdhocctlCreate(const char * groupName);

// May need to use these from sceNet.cpp
extern bool netAdhocInited;
extern bool netAdhocctlInited;
int sceNetAdhocctlTerm();
int sceNetAdhocTerm();
