// Copyright (c) 2014- PPSSPP Project.

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

#include "Core/Config.h"
#include "GPU/GLES/FragmentShaderGenerator.h"
#include "GPU/GLES/FragmentTestCache.h"
#include "GPU/GPUState.h"

// These are small, let's give them plenty of frames.
static const int FRAGTEST_TEXTURE_OLD_AGE = 307;

FragmentTestCache::FragmentTestCache() : textureCache_(NULL), lastTexture_(0) {
	scratchpad_ = new u8[256 * 4];
}

FragmentTestCache::~FragmentTestCache() {
	Clear();
	delete [] scratchpad_;
}

void FragmentTestCache::BindTestTexture(GLenum unit) {
	if (!g_Config.bFragmentTestCache) {
		return;
	}

	const FragmentTestID id = GenerateTestID();
	const auto cached = cache_.find(id);
	if (cached != cache_.end()) {
		cached->second.lastFrame = gpuStats.numFlips;
		GLuint tex = cached->second.texture;
		if (tex == lastTexture_) {
			// Already bound, hurray.
			return;
		}
		if (!gstate.isColorTestEnabled() && (IsAlphaTestAgainstZero() || IsAlphaTestTriviallyTrue())) {
			// Common case: testing against zero.  Just skip it.
			return;
		}
		glActiveTexture(unit);
		glBindTexture(GL_TEXTURE_2D, tex);
		// Always return to the default.
		glActiveTexture(GL_TEXTURE0);
		lastTexture_ = tex;
		return;
	}

	const u8 rRef = (gstate.getColorTestRef() >> 0) & 0xFF;
	const u8 rMask = (gstate.getColorTestMask() >> 0) & 0xFF;
	const u8 gRef = (gstate.getColorTestRef() >> 8) & 0xFF;
	const u8 gMask = (gstate.getColorTestMask() >> 8) & 0xFF;
	const u8 bRef = (gstate.getColorTestRef() >> 16) & 0xFF;
	const u8 bMask = (gstate.getColorTestMask() >> 16) & 0xFF;
	const u8 aRef = gstate.getAlphaTestRef();
	const u8 aMask = gstate.getAlphaTestMask();
	const u8 refs[4] = {rRef, gRef, bRef, aRef};
	const u8 masks[4] = {rMask, gMask, bMask, aMask};
	const GEComparison funcs[4] = {gstate.getColorTestFunction(), gstate.getColorTestFunction(), gstate.getColorTestFunction(), gstate.getAlphaTestFunction()};
	const bool valid[4] = {gstate.isColorTestEnabled(), gstate.isColorTestEnabled(), gstate.isColorTestEnabled(), gstate.isAlphaTestEnabled()};

	glActiveTexture(unit);
	// This will necessarily bind the texture.
	const GLuint tex = CreateTestTexture(funcs, refs, masks, valid);
	// Always return to the default.
	glActiveTexture(GL_TEXTURE0);
	lastTexture_ = tex;

	FragmentTestTexture item;
	item.lastFrame = gpuStats.numFlips;
	item.texture = tex;
	cache_[id] = item;
}

FragmentTestID FragmentTestCache::GenerateTestID() const {
	FragmentTestID id;
	// Let's just keep it simple, it's all in here.
	id.alpha = gstate.isAlphaTestEnabled() ? gstate.alphatest : 0;
	if (gstate.isColorTestEnabled()) {
		id.colorRefFunc = gstate.getColorTestFunction() | (gstate.getColorTestRef() << 8);
		id.colorMask = gstate.getColorTestMask();
	} else {
		id.colorRefFunc = 0;
		id.colorMask = 0;
	}
	return id;
}

GLuint FragmentTestCache::CreateTestTexture(const GEComparison funcs[4], const u8 refs[4], const u8 masks[4], const bool valid[4]) {
	// TODO: Might it be better to use GL_ALPHA for simple textures?
	// TODO: Experiment with 4-bit/etc. textures.

	// Build the logic map.
	for (int color = 0; color < 256; ++color) {
		for (int i = 0; i < 4; ++i) {
			bool res;
			if (valid[i]) {
				switch (funcs[i]) {
				case GE_COMP_NEVER:
					res = false;
					break;
				case GE_COMP_ALWAYS:
					res = true;
					break;
				case GE_COMP_EQUAL:
					res = (color & masks[i]) == (refs[i] & masks[i]);
					break;
				case GE_COMP_NOTEQUAL:
					res = (color & masks[i]) != (refs[i] & masks[i]);
					break;
				case GE_COMP_LESS:
					res = (color & masks[i]) < (refs[i] & masks[i]);
					break;
				case GE_COMP_LEQUAL:
					res = (color & masks[i]) <= (refs[i] & masks[i]);
					break;
				case GE_COMP_GREATER:
					res = (color & masks[i]) > (refs[i] & masks[i]);
					break;
				case GE_COMP_GEQUAL:
					res = (color & masks[i]) >= (refs[i] & masks[i]);
					break;
				}
			} else {
				res = true;
			}
			scratchpad_[color * 4 + i] = res ? 0xFF : 0;
		}
	}

	GLuint tex = textureCache_->AllocTextureName();
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, scratchpad_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return tex;
}

void FragmentTestCache::Clear(bool deleteThem) {
	if (deleteThem) {
		for (auto tex = cache_.begin(); tex != cache_.end(); ++tex) {
			glDeleteTextures(1, &tex->second.texture);
		}
	}
	cache_.clear();
	lastTexture_ = 0;
}

void FragmentTestCache::Decimate() {
	for (auto tex = cache_.begin(); tex != cache_.end(); ) {
		if (tex->second.lastFrame + FRAGTEST_TEXTURE_OLD_AGE < gpuStats.numFlips) {
			glDeleteTextures(1, &tex->second.texture);
			cache_.erase(tex++);
		} else {
			++tex;
		}
	}
	lastTexture_ = 0;
}
