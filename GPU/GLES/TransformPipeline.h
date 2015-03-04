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

#include <map>

#include "GPU/Common/GPUDebugInterface.h"
#include "GPU/Common/IndexGenerator.h"
#include "GPU/Common/VertexDecoderCommon.h"
#include "GPU/Common/DrawEngineCommon.h"
#include "GPU/GLES/FragmentShaderGenerator.h"
#include "gfx/gl_common.h"
#include "gfx/gl_lost_manager.h"

class LinkedShader;
class ShaderManager;
class TextureCache;
class FramebufferManager;
class FramebufferManagerCommon;
class TextureCacheCommon;
class FragmentTestCache;
struct TransformedVertex;

struct DecVtxFormat;

// States transitions:
// On creation: DRAWN_NEW
// DRAWN_NEW -> DRAWN_HASHING
// DRAWN_HASHING -> DRAWN_RELIABLE
// DRAWN_HASHING -> DRAWN_UNRELIABLE
// DRAWN_ONCE -> UNRELIABLE
// DRAWN_RELIABLE -> DRAWN_SAFE
// UNRELIABLE -> death
// DRAWN_ONCE -> death
// DRAWN_RELIABLE -> death

enum {
	VAI_FLAG_VERTEXFULLALPHA = 1,
};

// Try to keep this POD.
class VertexArrayInfo {
public:
	VertexArrayInfo() {
		status = VAI_NEW;
		vbo = 0;
		ebo = 0;
		prim = GE_PRIM_INVALID;
		numDraws = 0;
		numFrames = 0;
		lastFrame = gpuStats.numFlips;
		numVerts = 0;
		drawsUntilNextFullHash = 0;
		flags = 0;
	}
	~VertexArrayInfo();

	enum Status {
		VAI_NEW,
		VAI_HASHING,
		VAI_RELIABLE,  // cache, don't hash
		VAI_UNRELIABLE,  // never cache
	};

	u32 hash;
	u32 minihash;

	Status status;

	u32 vbo;
	u32 ebo;

	// Precalculated parameter for drawRangeElements
	u16 numVerts;
	u16 maxIndex;
	s8 prim;

	// ID information
	int numDraws;
	int numFrames;
	int lastFrame;  // So that we can forget.
	u16 drawsUntilNextFullHash;
	u8 flags;
};

// Handles transform, lighting and drawing.
class TransformDrawEngine : public DrawEngineCommon, public GfxResourceHolder {
public:
	TransformDrawEngine();
	virtual ~TransformDrawEngine();

	void SubmitPrim(void *verts, void *inds, GEPrimitiveType prim, int vertexCount, u32 vertType, int *bytesRead);
	void SubmitSpline(void* control_points, void* indices, int count_u, int count_v, int type_u, int type_v, GEPatchPrimType prim_type, u32 vertType);
	void SubmitBezier(void* control_points, void* indices, int count_u, int count_v, GEPatchPrimType prim_type, u32 vertType);

	void SetShaderManager(ShaderManager *shaderManager) {
		shaderManager_ = shaderManager;
	}
	void SetTextureCache(TextureCache *textureCache) {
		textureCache_ = textureCache;
	}
	void SetFramebufferManager(FramebufferManager *fbManager) {
		framebufferManager_ = fbManager;
	}
	void SetFragmentTestCache(FragmentTestCache *testCache) {
		fragmentTestCache_ = testCache;
	}
	void InitDeviceObjects();
	void DestroyDeviceObjects();
	void GLLost();
	void Resized();

	void DecimateTrackedVertexArrays();
	void ClearTrackedVertexArrays();

	void SetupVertexDecoder(u32 vertType);
	inline void SetupVertexDecoderInternal(u32 vertType);

	// This requires a SetupVertexDecoder call first.
	int EstimatePerVertexCost() {
		// TODO: This is transform cost, also account for rasterization cost somehow... although it probably
		// runs in parallel with transform.

		// Also, this is all pure guesswork. If we can find a way to do measurements, that would be great.

		// GTA wants a low value to run smooth, GoW wants a high value (otherwise it thinks things
		// went too fast and starts doing all the work over again).

		int cost = 20;
		if (gstate.isLightingEnabled()) {
			cost += 10;

			for (int i = 0; i < 4; i++) {
				if (gstate.isLightChanEnabled(i))
					cost += 10;
			}
		}

		if (gstate.getUVGenMode() != GE_TEXMAP_TEXTURE_COORDS) {
			cost += 20;
		}
		if (dec_ && dec_->morphcount > 1) {
			cost += 5 * dec_->morphcount;
		}

		return cost;
	}

	// So that this can be inlined
	void Flush() {
		if (!numDrawCalls)
			return;
		DoFlush();
	}

	bool IsCodePtrVertexDecoder(const u8 *ptr) const;

protected:
	// Preprocessing for spline/bezier
	virtual u32 NormalizeVertices(u8 *outPtr, u8 *bufPtr, const u8 *inPtr, int lowerBound, int upperBound, u32 vertType) override;

private:
	void DecodeVerts();
	void DecodeVertsStep();
	void DoFlush();
	void ApplyDrawState(int prim);
	void ApplyDrawStateLate();
	void ApplyBlendState();
	void ApplyStencilReplaceAndLogicOp(ReplaceAlphaType replaceAlphaWithStencil);
	bool ApplyShaderBlending();
	inline void ResetShaderBlending();
	GLuint AllocateBuffer();
	void FreeBuffer(GLuint buf);

	u32 ComputeMiniHash();
	u32 ComputeHash();  // Reads deferred vertex data.
	void MarkUnreliable(VertexArrayInfo *vai);

	VertexDecoder *GetVertexDecoder(u32 vtype);

	// Defer all vertex decoding to a Flush, so that we can hash and cache the
	// generated buffers without having to redecode them every time.
	struct DeferredDrawCall {
		void *verts;
		void *inds;
		u32 vertType;
		u8 indexType;
		u8 prim;
		u16 vertexCount;
		u16 indexLowerBound;
		u16 indexUpperBound;
	};

	// Vertex collector state
	IndexGenerator indexGen;
	int decodedVerts_;
	GEPrimitiveType prevPrim_;

	// Cached vertex decoders
	std::map<u32, VertexDecoder *> decoderMap_;
	VertexDecoder *dec_;
	VertexDecoderJitCache *decJitCache_;
	u32 lastVType_;

	TransformedVertex *transformed;
	TransformedVertex *transformedExpanded;

	std::map<u32, VertexArrayInfo *> vai_;

	// Fixed index buffer for easy quad generation from spline/bezier
	u16 *quadIndices_;

	// Vertex buffer objects
	// Element buffer objects
	std::vector<GLuint> bufferNameCache_;

	// Other
	ShaderManager *shaderManager_;
	TextureCache *textureCache_;
	FramebufferManager *framebufferManager_;
	FragmentTestCache *fragmentTestCache_;

	enum { MAX_DEFERRED_DRAW_CALLS = 128 };
	DeferredDrawCall drawCalls[MAX_DEFERRED_DRAW_CALLS];
	int numDrawCalls;
	int vertexCountInDrawCalls;

	int decimationCounter_;
	int decodeCounter_;
	u32 dcid_;

	UVScale *uvScale;

	bool fboTexBound_;
	VertexDecoderOptions decOptions_;
};
