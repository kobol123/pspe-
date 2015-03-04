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

#include "Common/CommonTypes.h"

#include "VertexDecoderCommon.h"

class FramebufferManagerCommon;
class TextureCacheCommon;

enum SoftwareTransformAction {
	SW_DRAW_PRIMITIVES,
	SW_CLEAR,
};

struct SoftwareTransformResult {
	SoftwareTransformAction action;
	u32 color;
	float depth;

	bool setStencil;
	u8 stencilValue;
};

void SoftwareTransform(int prim, u8 *decoded, int vertexCount, u32 vertexType, void *inds, int indexType, const DecVtxFormat &decVtxFormat, int maxIndex, FramebufferManagerCommon *fbman, TextureCacheCommon *texCache, TransformedVertex *transformed, TransformedVertex *transformedExpanded, TransformedVertex *&drawBuffer, 
	int &numTrans, bool &drawIndexed, SoftwareTransformResult *result);
