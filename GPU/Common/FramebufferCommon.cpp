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

// Official git repository and contact information can be found at
<<<<<<< HEAD

=======
// 
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

#include <algorithm>
#include "Common/Common.h"
#include "Core/Config.h"
#include "Core/CoreParameter.h"
#include "Core/Reporting.h"
#include "Core/ELF/ParamSFO.h"
#include "Core/System.h"
#include "GPU/Common/FramebufferCommon.h"
#include "GPU/GPUInterface.h"
#include "GPU/GPUState.h"
<<<<<<< HEAD
#include "GPU/Common/gralloc/framebuffer_device.h"
#include "GPU/Common/gralloc/gralloc_module.cpp"



#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <log.h>
#include <atomic>
#include <GPU/Common/gralloc/hardware/hardware.h>
#include <GPU/Common/gralloc/hardware/gralloc.h>

#include <sys/ioctl.h>

#include "GPU/Common/gralloc/alloc_device.h"
#include "GPU/Common/gralloc/gralloc_priv.h"
#include "GPU/Common/gralloc/gralloc_helper.h"
#include "GPU/Common/gralloc/framebuffer_device.h"

#if GRALLOC_ARM_UMP_MODULE
#include <ump/ump.h>
#include <ump/ump_ref_drv.h>
#endif

#if GRALLOC_ARM_DMA_BUF_MODULE
//#include <linux/ion.h>
//#include <ion/ion.h>
#endif

#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))


#if GRALLOC_SIMULATE_FAILURES
#include <GPU/Common/gralloc/cutils/properties.h>

/* system property keys for controlling simulated UMP allocation failures */
#define PROP_MALI_TEST_GRALLOC_FAIL_FIRST     "mali.test.gralloc.fail_first"
#define PROP_MALI_TEST_GRALLOC_FAIL_INTERVAL  "mali.test.gralloc.fail_interval"
static int __ump_alloc_should_fail()
{
    
    static unsigned int call_count  = 0;
    unsigned int        first_fail  = 0;
    int                 fail_period = 0;
    int                 fail        = 0;
    
    ++call_count;
    
    /* read the system properties that control failure simulation */
    {
        char prop_value[PROPERTY_VALUE_MAX];
        
        if (property_get(PROP_MALI_TEST_GRALLOC_FAIL_FIRST, prop_value, "0") > 0)
        {
            sscanf(prop_value, "%11u", &first_fail);
        }
        
        if (property_get(PROP_MALI_TEST_GRALLOC_FAIL_INTERVAL, prop_value, "0") > 0)
        {
            sscanf(prop_value, "%11u", &fail_period);
        }
    }
    
    /* failure simulation is enabled by setting the first_fail property to non-zero */
    if (first_fail > 0)
    {
        LOGI("iteration %u (fail=%u, period=%u)\n", call_count, first_fail, fail_period);
        
        fail = (call_count == first_fail) ||
        (call_count > first_fail && fail_period > 0 && 0 == (call_count - first_fail) % fail_period);
        
        if (fail)
        {
            AERR("failed ump_ref_drv_allocate on iteration #%d\n", call_count);
        }
    }
    
    return fail;
}
#endif

void CenterRect(float *x, float *y, float *w, float *h, float origW, float origH, float frameW, float frameH, int rotation) {
	float outW;
	float outH;

	bool rotated = rotation == ROTATION_LOCKED_VERTICAL || rotation == ROTATION_LOCKED_VERTICAL180;

	if (g_Config.bStretchToDisplay) {
		outW = frameW;
		outH = frameH;
	} else {
		// Add special case for 1080p displays, cutting off the bottom and top 1-pixel rows from the original 480x272.
		// This will be what 99.9% of users want.
		if (origW == 480 && origH == 272 && frameW == 1920 && frameH == 1080 && !rotated) {
			*x = 0;
			*y = -4;
			*w = 1920;
			*h = 1088;
			return;
		}

		float origRatio = !rotated ? origW / origH : origH / origW;
		float frameRatio = frameW / frameH;

		if (origRatio > frameRatio) {
			// Image is wider than frame. Center vertically.
			outW = frameW;
			outH = frameW / origRatio;
			// Stretch a little bit
			if (!rotated && g_Config.bPartialStretch)
				outH = (frameH + outH) / 2.0f; // (408 + 720) / 2 = 564
		} else {
			// Image is taller than frame. Center horizontally.
			outW = frameH * origRatio;
			outH = frameH;
		}
	}

	if (g_Config.bSmallDisplay) {
		outW /= 2.0f;
		outH /= 2.0f;
	}

	*x = (frameW - outW) / 2.0f;
	*y = (frameH - outH) / 2.0f;
	*w = outW;
	*h = outH;
    
}

=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

FramebufferManagerCommon::FramebufferManagerCommon() :
	displayFramebufPtr_(0),
	displayStride_(0),
	displayFormat_(GE_FORMAT_565),
	displayFramebuf_(0),
	prevDisplayFramebuf_(0),
	prevPrevDisplayFramebuf_(0),
	frameLastFramebufUsed_(0),
	currentRenderVfb_(0),
	framebufRangeEnd_(0),
	hackForce04154000Download_(false) {
}

FramebufferManagerCommon::~FramebufferManagerCommon() {
}

void FramebufferManagerCommon::Init() {
<<<<<<< HEAD
       const std::string gameId = g_paramSFO.GetValueString("DISC_ID");
=======

	const std::string gameId = g_paramSFO.GetValueString("DISC_ID");
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	// This applies a hack to Dangan Ronpa, its demo, and its sequel.
	// The game draws solid colors to a small framebuffer, and then reads this directly in VRAM.
	// We force this framebuffer to 1x and force download it automatically.
	hackForce04154000Download_ = gameId == "NPJH50631" || gameId == "NPJH50372" || gameId == "NPJH90164" || gameId == "NPJH50515";

	// And an initial clear. We don't clear per frame as the games are supposed to handle that
	// by themselves.
	ClearBuffer();
<<<<<<< HEAD
    /*
     * HAL_MODULE_INFO_SYM will be initialized using the default constructor
     * implemented above
     */
    
    //init_frame_buffer(HAL_MODULE_INFO_SYM);
    if(g_Config.bMaliHackb){
        alloc_device_t *dev;
        
        dev = new alloc_device_t;
        
        
        
#if GRALLOC_ARM_UMP_MODULE
        ump_result ump_res = ump_open();
        
        
        
#endif
        
        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));
        
        /* initialize the procs */
        dev->common.tag = HARDWARE_DEVICE_TAG;
        dev->common.version = 0;
        
        dev->common.close = alloc_device_close;
        dev->alloc = alloc_device_alloc;
        dev->free = alloc_device_free;
        
    }
    
    
    
    
    
	BeginFrame();
    
=======

	BeginFrame();
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
}

void FramebufferManagerCommon::BeginFrame() {
	DecimateFBOs();
	currentRenderVfb_ = 0;
	useBufferedRendering_ = g_Config.iRenderingMode != FB_NON_BUFFERED_MODE;
	updateVRAM_ = !(g_Config.iRenderingMode == FB_NON_BUFFERED_MODE || g_Config.iRenderingMode == FB_BUFFERED_MODE);
}

void FramebufferManagerCommon::SetDisplayFramebuffer(u32 framebuf, u32 stride, GEBufferFormat format) {
	displayFramebufPtr_ = framebuf;
	displayStride_ = stride;
	displayFormat_ = format;
}

VirtualFramebuffer *FramebufferManagerCommon::GetVFBAt(u32 addr) {
	VirtualFramebuffer *match = NULL;
	for (size_t i = 0; i < vfbs_.size(); ++i) {
		VirtualFramebuffer *v = vfbs_[i];
		if (MaskedEqual(v->fb_address, addr)) {
			// Could check w too but whatever
			if (match == NULL || match->last_frame_render < v->last_frame_render) {
				match = v;
			}
		}
	}
	if (match != NULL) {
		return match;
	}

<<<<<<< HEAD
=======
	DEBUG_LOG(SCEGE, "Finding no FBO matching address %08x", addr);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	return 0;
}

bool FramebufferManagerCommon::MaskedEqual(u32 addr1, u32 addr2) {
	return (addr1 & 0x03FFFFFF) == (addr2 & 0x03FFFFFF);
}

u32 FramebufferManagerCommon::FramebufferByteSize(const VirtualFramebuffer *vfb) const {
	return vfb->fb_stride * vfb->height * (vfb->format == GE_FORMAT_8888 ? 4 : 2);
}

bool FramebufferManagerCommon::ShouldDownloadFramebuffer(const VirtualFramebuffer *vfb) const {
	return updateVRAM_ || (hackForce04154000Download_ && vfb->fb_address == 0x00154000);
}

// Heuristics to figure out the size of FBO to create.
<<<<<<< HEAD
void FramebufferManagerCommon::EstimateDrawingSize(u32 fb_address, GEBufferFormat fb_format, int viewport_width, int viewport_height, int region_width, int region_height, int scissor_width, int scissor_height, int fb_stride, int &drawing_width, int &drawing_height) {
	static const int MAX_FRAMEBUF_HEIGHT = 512;
=======
void FramebufferManagerCommon::EstimateDrawingSize(int &drawing_width, int &drawing_height) {
	static const int MAX_FRAMEBUF_HEIGHT = 512;
	const int viewport_width = (int) gstate.getViewportX1();
	const int viewport_height = (int) gstate.getViewportY1();
	const int region_width = gstate.getRegionX2() + 1;
	const int region_height = gstate.getRegionY2() + 1;
	const int scissor_width = gstate.getScissorX2() + 1;
	const int scissor_height = gstate.getScissorY2() + 1;
	const int fb_stride = std::max(gstate.FrameBufStride(), 4);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

	// Games don't always set any of these.  Take the greatest parameter that looks valid based on stride.
	if (viewport_width > 4 && viewport_width <= fb_stride) {
		drawing_width = viewport_width;
		drawing_height = viewport_height;
		// Some games specify a viewport with 0.5, but don't have VRAM for 273.  480x272 is the buffer size.
		if (viewport_width == 481 && region_width == 480 && viewport_height == 273 && region_height == 272) {
			drawing_width = 480;
			drawing_height = 272;
		}
		// Sometimes region is set larger than the VRAM for the framebuffer.
<<<<<<< HEAD
		// However, in one game it's correctly set as a larger height (see #7277) with the same width.
		// A bit of a hack, but we try to handle that unusual case here.
		if (region_width <= fb_stride && (region_width > drawing_width || (region_width == drawing_width && region_height > drawing_height)) && region_height <= MAX_FRAMEBUF_HEIGHT) {
=======
		if (region_width <= fb_stride && region_width > drawing_width && region_height <= MAX_FRAMEBUF_HEIGHT) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
			drawing_width = region_width;
			drawing_height = std::max(drawing_height, region_height);
		}
		// Scissor is often set to a subsection of the framebuffer, so we pay the least attention to it.
		if (scissor_width <= fb_stride && scissor_width > drawing_width && scissor_height <= MAX_FRAMEBUF_HEIGHT) {
			drawing_width = scissor_width;
			drawing_height = std::max(drawing_height, scissor_height);
		}
	} else {
		// If viewport wasn't valid, let's just take the greatest anything regardless of stride.
		drawing_width = std::min(std::max(region_width, scissor_width), fb_stride);
		drawing_height = std::max(region_height, scissor_height);
	}

	// Assume no buffer is > 512 tall, it couldn't be textured or displayed fully if so.
	if (drawing_height >= MAX_FRAMEBUF_HEIGHT) {
		if (region_height < MAX_FRAMEBUF_HEIGHT) {
			drawing_height = region_height;
		} else if (scissor_height < MAX_FRAMEBUF_HEIGHT) {
			drawing_height = scissor_height;
		}
	}

	if (viewport_width != region_width) {
		// The majority of the time, these are equal.  If not, let's check what we know.
<<<<<<< HEAD
=======
		const u32 fb_address = gstate.getFrameBufAddress();
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		u32 nearest_address = 0xFFFFFFFF;
		for (size_t i = 0; i < vfbs_.size(); ++i) {
			const u32 other_address = vfbs_[i]->fb_address | 0x44000000;
			if (other_address > fb_address && other_address < nearest_address) {
				nearest_address = other_address;
			}
		}

		// Unless the game is using overlapping buffers, the next buffer should be far enough away.
		// This catches some cases where we can know this.
		// Hmm.  The problem is that we could only catch it for the first of two buffers...
<<<<<<< HEAD
		const u32 bpp = fb_format == GE_FORMAT_8888 ? 4 : 2;
=======
		const u32 bpp = gstate.FrameBufFormat() == GE_FORMAT_8888 ? 4 : 2;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		int avail_height = (nearest_address - fb_address) / (fb_stride * bpp);
		if (avail_height < drawing_height && avail_height == region_height) {
			drawing_width = std::min(region_width, fb_stride);
			drawing_height = avail_height;
		}

		// Some games draw buffers interleaved, with a high stride/region/scissor but default viewport.
		if (fb_stride == 1024 && region_width == 1024 && scissor_width == 1024) {
			drawing_width = 1024;
		}
	}

<<<<<<< HEAD
	DEBUG_LOG(G3D, "Est: %08x V: %ix%i, R: %ix%i, S: %ix%i, STR: %i, THR:%i, Z:%08x = %ix%i", fb_address, viewport_width,viewport_height, region_width, region_height, scissor_width, scissor_height, fb_stride, gstate.isModeThrough(), gstate.isDepthWriteEnabled() ? gstate.getDepthBufAddress() : 0, drawing_width, drawing_height);
}

void GetFramebufferHeuristicInputs(FramebufferHeuristicParams *params, const GPUgstate &gstate) {
	params->fb_addr = gstate.getFrameBufAddress();
	params->fb_address = gstate.getFrameBufRawAddress();
	params->fb_stride = gstate.FrameBufStride();

	params->z_address = gstate.getDepthBufRawAddress();
	params->z_stride = gstate.DepthBufStride();

	params->fmt = gstate.FrameBufFormat();

	params->isClearingDepth = gstate.isModeClear() && gstate.isClearModeDepthMask();
	// Technically, it may write depth later, but we're trying to detect it only when it's really true.
	if (gstate.isModeClear()) {
		// Not quite seeing how this makes sense..
		params->isWritingDepth = !gstate.isClearModeDepthMask() && gstate.isDepthWriteEnabled();
	} else {
		params->isWritingDepth = gstate.isDepthWriteEnabled();
	}
	params->isDrawing = !gstate.isModeClear() || !gstate.isClearModeColorMask() || !gstate.isClearModeAlphaMask();
	params->isModeThrough = gstate.isModeThrough();

	// Viewport-X1 and Y1 are not the upper left corner, but half the width/height. A bit confusing.
	params->viewportWidth = (int)(fabsf(gstate.getViewportX1()*2.0f));
	params->viewportHeight = (int)(fabsf(gstate.getViewportY1()*2.0f));
	params->regionWidth = gstate.getRegionX2() + 1;
	params->regionHeight = gstate.getRegionY2() + 1;
	params->scissorWidth = gstate.getScissorX2() + 1;
	params->scissorHeight = gstate.getScissorY2() + 1;
}

VirtualFramebuffer *FramebufferManagerCommon::DoSetRenderFrameBuffer(const FramebufferHeuristicParams &params, u32 skipDrawReason) {
	gstate_c.framebufChanged = false;

	// Collect all parameters. This whole function has really become a cesspool of heuristics...
	// but it appears that's what it takes, unless we emulate VRAM layout more accurately somehow.
=======
	DEBUG_LOG(G3D, "Est: %08x V: %ix%i, R: %ix%i, S: %ix%i, STR: %i, THR:%i, Z:%08x = %ix%i", gstate.getFrameBufAddress(), viewport_width,viewport_height, region_width, region_height, scissor_width, scissor_height, fb_stride, gstate.isModeThrough(), gstate.isDepthWriteEnabled() ? gstate.getDepthBufAddress() : 0, drawing_width, drawing_height);
}

void FramebufferManagerCommon::DoSetRenderFrameBuffer() {
	/*
	if (useBufferedRendering_ && currentRenderVfb_) {
		// Hack is enabled, and there was a previous framebuffer.
		// Before we switch, let's do a series of trickery to copy one bit of stencil to
		// destination alpha. Or actually, this is just a bunch of hackery attempts on Wipeout.
		// Ignore for now.
		glstate.depthTest.disable();
		glstate.colorMask.set(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
		glstate.stencilTest.enable();
		glstate.stencilOp.set(GL_KEEP, GL_KEEP, GL_KEEP);  // don't modify stencil§
		glstate.stencilFunc.set(GL_GEQUAL, 0xFE, 0xFF);
		DrawPlainColor(0x00000000);
		//glstate.stencilFunc.set(GL_LESS, 0x80, 0xFF);
		//DrawPlainColor(0xFF000000);
		glstate.stencilTest.disable();
		glstate.colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glstate.depthTest.disable();
		glstate.colorMask.set(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
		DrawPlainColor(0x00000000);
		shaderManager_->DirtyLastShader();  // dirty lastShader_
	}
	*/


	gstate_c.framebufChanged = false;

	// Get parameters
	const u32 fb_address = gstate.getFrameBufRawAddress();
	const int fb_stride = gstate.FrameBufStride();

	const u32 z_address = gstate.getDepthBufRawAddress();
	const int z_stride = gstate.DepthBufStride();

	GEBufferFormat fmt = gstate.FrameBufFormat();
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2

	// As there are no clear "framebuffer width" and "framebuffer height" registers,
	// we need to infer the size of the current framebuffer somehow.
	int drawing_width, drawing_height;
<<<<<<< HEAD
	EstimateDrawingSize(params.fb_address, params.fmt, params.viewportWidth, params.viewportHeight, params.regionWidth, params.regionHeight, params.scissorWidth, params.scissorHeight, std::max(params.fb_stride, 4), drawing_width, drawing_height);

	gstate_c.curRTOffsetX = 0;
=======
	EstimateDrawingSize(drawing_width, drawing_height);

	gstate_c.cutRTOffsetX = 0;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	bool vfbFormatChanged = false;

	// Find a matching framebuffer
	VirtualFramebuffer *vfb = 0;
	for (size_t i = 0; i < vfbs_.size(); ++i) {
		VirtualFramebuffer *v = vfbs_[i];
<<<<<<< HEAD
		if (v->fb_address == params.fb_address) {
			vfb = v;
			// Update fb stride in case it changed
			if (vfb->fb_stride != params.fb_stride || vfb->format != params.fmt) {
				vfbFormatChanged = true;
				vfb->fb_stride = params.fb_stride;
				vfb->format = params.fmt;
			}
			// Heuristic: In throughmode, a higher height could be used.  Let's avoid shrinking the buffer.
			if (params.isModeThrough && (int)vfb->width < params.fb_stride) {
=======
		if (v->fb_address == fb_address) {
			vfb = v;
			// Update fb stride in case it changed
			if (vfb->fb_stride != fb_stride || vfb->format != fmt) {
				vfbFormatChanged = true;
				vfb->fb_stride = fb_stride;
				vfb->format = fmt;
			}
			// In throughmode, a higher height could be used.  Let's avoid shrinking the buffer.
			if (gstate.isModeThrough() && (int)vfb->width < fb_stride) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				vfb->width = std::max((int)vfb->width, drawing_width);
				vfb->height = std::max((int)vfb->height, drawing_height);
			} else {
				vfb->width = drawing_width;
				vfb->height = drawing_height;
			}
			break;
<<<<<<< HEAD
		} else if (v->fb_address < params.fb_address && v->fb_address + v->fb_stride * 4 > params.fb_address) {
			// Possibly a render-to-offset.
			const u32 bpp = v->format == GE_FORMAT_8888 ? 4 : 2;
			const int x_offset = (params.fb_address - v->fb_address) / bpp;
			if (v->format == params.fmt && v->fb_stride == params.fb_stride && x_offset < params.fb_stride && v->height >= drawing_height) {
				WARN_LOG_REPORT_ONCE(renderoffset, HLE, "Rendering to framebuffer offset: %08x +%dx%d", v->fb_address, x_offset, 0);
				vfb = v;
				gstate_c.curRTOffsetX = x_offset;
=======
		} else if (v->fb_address < fb_address && v->fb_address + v->fb_stride * 4 > fb_address) {
			// Possibly a render-to-offset.
			const u32 bpp = v->format == GE_FORMAT_8888 ? 4 : 2;
			const int x_offset = (fb_address - v->fb_address) / bpp;
			if (v->format == fmt && v->fb_stride == fb_stride && x_offset < fb_stride && v->height >= drawing_height) {
				WARN_LOG_REPORT_ONCE(renderoffset, HLE, "Rendering to framebuffer offset: %08x +%dx%d", v->fb_address, x_offset, 0);
				vfb = v;
				gstate_c.cutRTOffsetX = x_offset;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				vfb->width = std::max((int)vfb->width, x_offset + drawing_width);
				// To prevent the newSize code from being confused.
				drawing_width += x_offset;
				break;
			}
		}
	}

	if (vfb) {
		if ((drawing_width != vfb->bufferWidth || drawing_height != vfb->bufferHeight)) {
			// Even if it's not newly wrong, if this is larger we need to resize up.
			if (vfb->width > vfb->bufferWidth || vfb->height > vfb->bufferHeight) {
				ResizeFramebufFBO(vfb, vfb->width, vfb->height);
			} else if (vfb->newWidth != drawing_width || vfb->newHeight != drawing_height) {
				// If it's newly wrong, or changing every frame, just keep track.
				vfb->newWidth = drawing_width;
				vfb->newHeight = drawing_height;
				vfb->lastFrameNewSize = gpuStats.numFlips;
			} else if (vfb->lastFrameNewSize + FBO_OLD_AGE < gpuStats.numFlips) {
				// Okay, it's changed for a while (and stayed that way.)  Let's start over.
				// But only if we really need to, to avoid blinking.
<<<<<<< HEAD
				bool needsRecreate = vfb->bufferWidth > params.fb_stride;
=======
				bool needsRecreate = vfb->bufferWidth > fb_stride;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				needsRecreate = needsRecreate || vfb->newWidth > vfb->bufferWidth || vfb->newWidth * 2 < vfb->bufferWidth;
				needsRecreate = needsRecreate || vfb->newHeight > vfb->bufferHeight || vfb->newHeight * 2 < vfb->bufferHeight;
				if (needsRecreate) {
					ResizeFramebufFBO(vfb, vfb->width, vfb->height, true);
				}
			}
		} else {
			// It's not different, let's keep track of that too.
			vfb->lastFrameNewSize = gpuStats.numFlips;
		}
	}

	float renderWidthFactor = (float)PSP_CoreParameter().renderWidth / 480.0f;
	float renderHeightFactor = (float)PSP_CoreParameter().renderHeight / 272.0f;

<<<<<<< HEAD
	if (hackForce04154000Download_ && params.fb_address == 0x00154000) {
=======
	if (hackForce04154000Download_ && fb_address == 0x00154000) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		renderWidthFactor = 1.0;
		renderHeightFactor = 1.0;
	}

	// None found? Create one.
	if (!vfb) {
		vfb = new VirtualFramebuffer();
		vfb->fbo = 0;
<<<<<<< HEAD
		vfb->fb_address = params.fb_address;
		vfb->fb_stride = params.fb_stride;
		vfb->z_address = params.z_address;
		vfb->z_stride = params.z_stride;
=======
		vfb->fb_address = fb_address;
		vfb->fb_stride = fb_stride;
		vfb->z_address = z_address;
		vfb->z_stride = z_stride;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		vfb->width = drawing_width;
		vfb->height = drawing_height;
		vfb->newWidth = drawing_width;
		vfb->newHeight = drawing_height;
		vfb->lastFrameNewSize = gpuStats.numFlips;
		vfb->renderWidth = (u16)(drawing_width * renderWidthFactor);
		vfb->renderHeight = (u16)(drawing_height * renderHeightFactor);
		vfb->bufferWidth = drawing_width;
		vfb->bufferHeight = drawing_height;
<<<<<<< HEAD
		vfb->format = params.fmt;
		vfb->drawnWidth = 0;
		vfb->drawnHeight = 0;
		vfb->drawnFormat = params.fmt;
		vfb->usageFlags = FB_USAGE_RENDERTARGET;
		SetColorUpdated(vfb, skipDrawReason);
		vfb->depthUpdated = false;

		u32 byteSize = FramebufferByteSize(vfb);
		u32 fb_address_mem = (params.fb_address & 0x3FFFFFFF) | 0x04000000;
=======
		vfb->format = fmt;
		vfb->drawnWidth = 0;
		vfb->drawnHeight = 0;
		vfb->drawnFormat = fmt;
		vfb->usageFlags = FB_USAGE_RENDERTARGET;
		SetColorUpdated(vfb);
		vfb->depthUpdated = false;

		u32 byteSize = FramebufferByteSize(vfb);
		u32 fb_address_mem = (fb_address & 0x3FFFFFFF) | 0x04000000;
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		if (Memory::IsVRAMAddress(fb_address_mem) && fb_address_mem + byteSize > framebufRangeEnd_) {
			framebufRangeEnd_ = fb_address_mem + byteSize;
		}

		ResizeFramebufFBO(vfb, drawing_width, drawing_height, true);
		NotifyRenderFramebufferCreated(vfb);

		INFO_LOG(SCEGE, "Creating FBO for %08x : %i x %i x %i", vfb->fb_address, vfb->width, vfb->height, vfb->format);

		vfb->last_frame_render = gpuStats.numFlips;
		vfb->last_frame_used = 0;
		vfb->last_frame_attached = 0;
<<<<<<< HEAD
		vfb->last_frame_displayed = 0;
=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		frameLastFramebufUsed_ = gpuStats.numFlips;
		vfbs_.push_back(vfb);
		currentRenderVfb_ = vfb;

		if (useBufferedRendering_ && !updateVRAM_ && !g_Config.bDisableSlowFramebufEffects) {
			gpu->PerformMemoryUpload(fb_address_mem, byteSize);
			NotifyStencilUpload(fb_address_mem, byteSize, true);
			// TODO: Is it worth trying to upload the depth buffer?
		}

		// Let's check for depth buffer overlap.  Might be interesting.
		bool sharingReported = false;
<<<<<<< HEAD
		for (size_t i = 0, end = vfbs_.size(); i < end; ++i) {
			if (vfbs_[i]->z_stride != 0 && params.fb_address == vfbs_[i]->z_address) {
				// If it's clearing it, most likely it just needs more video memory.
				// Technically it could write something interesting and the other might not clear, but that's not likely.
				if (params.isDrawing) {
					if (params.fb_address != params.z_address && vfbs_[i]->fb_address != vfbs_[i]->z_address) {
						WARN_LOG_REPORT(SCEGE, "FBO created from existing depthbuffer as color, %08x/%08x and %08x/%08x", params.fb_address, params.z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
					}
				}
			} else if (params.z_stride != 0 && params.z_address == vfbs_[i]->fb_address) {
				// If it's clearing it, then it's probably just the reverse of the above case.
				if (params.isWritingDepth) {
					WARN_LOG_REPORT(SCEGE, "FBO using existing buffer as depthbuffer, %08x/%08x and %08x/%08x", params.fb_address, params.z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
				}
			} else if (vfbs_[i]->z_stride != 0 && params.z_address == vfbs_[i]->z_address && params.fb_address != vfbs_[i]->fb_address && !sharingReported) {
				// This happens a lot, but virtually always it's cleared.
				// It's possible the other might not clear, but when every game is reported it's not useful.
				if (params.isWritingDepth) {
					WARN_LOG_REPORT(SCEGE, "FBO reusing depthbuffer, %08x/%08x and %08x/%08x", params.fb_address, params.z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
=======
		bool writingDepth = true;
		// Technically, it may write depth later, but we're trying to detect it only when it's really true.
		if (gstate.isModeClear()) {
			writingDepth = !gstate.isClearModeDepthMask() && gstate.isDepthWriteEnabled();
		} else {
			writingDepth = gstate.isDepthWriteEnabled();
		}
		for (size_t i = 0, end = vfbs_.size(); i < end; ++i) {
			if (vfbs_[i]->z_stride != 0 && fb_address == vfbs_[i]->z_address) {
				// If it's clearing it, most likely it just needs more video memory.
				// Technically it could write something interesting and the other might not clear, but that's not likely.
				if (!gstate.isModeClear() || !gstate.isClearModeColorMask() || !gstate.isClearModeAlphaMask()) {
					if (fb_address != z_address && vfbs_[i]->fb_address != vfbs_[i]->z_address) {
						WARN_LOG_REPORT(SCEGE, "FBO created from existing depthbuffer as color, %08x/%08x and %08x/%08x", fb_address, z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
					}
				}
			} else if (z_stride != 0 && z_address == vfbs_[i]->fb_address) {
				// If it's clearing it, then it's probably just the reverse of the above case.
				if (writingDepth) {
					WARN_LOG_REPORT(SCEGE, "FBO using existing buffer as depthbuffer, %08x/%08x and %08x/%08x", fb_address, z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
				}
			} else if (vfbs_[i]->z_stride != 0 && z_address == vfbs_[i]->z_address && fb_address != vfbs_[i]->fb_address && !sharingReported) {
				// This happens a lot, but virtually always it's cleared.
				// It's possible the other might not clear, but when every game is reported it's not useful.
				if (writingDepth) {
					WARN_LOG_REPORT(SCEGE, "FBO reusing depthbuffer, %08x/%08x and %08x/%08x", fb_address, z_address, vfbs_[i]->fb_address, vfbs_[i]->z_address);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
					sharingReported = true;
				}
			}
		}

	// We already have it!
	} else if (vfb != currentRenderVfb_) {
		// Use it as a render target.
		DEBUG_LOG(SCEGE, "Switching render target to FBO for %08x: %i x %i x %i ", vfb->fb_address, vfb->width, vfb->height, vfb->format);
		vfb->usageFlags |= FB_USAGE_RENDERTARGET;
		vfb->last_frame_render = gpuStats.numFlips;
		frameLastFramebufUsed_ = gpuStats.numFlips;
		vfb->dirtyAfterDisplay = true;
<<<<<<< HEAD
		if ((skipDrawReason & SKIPDRAW_SKIPFRAME) == 0)
=======
		if ((gstate_c.skipDrawReason & SKIPDRAW_SKIPFRAME) == 0)
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
			vfb->reallyDirtyAfterDisplay = true;

		VirtualFramebuffer *prev = currentRenderVfb_;
		currentRenderVfb_ = vfb;
<<<<<<< HEAD
		NotifyRenderFramebufferSwitched(prev, vfb, params.isClearingDepth);
=======
		NotifyRenderFramebufferSwitched(prev, vfb);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	} else {
		vfb->last_frame_render = gpuStats.numFlips;
		frameLastFramebufUsed_ = gpuStats.numFlips;
		vfb->dirtyAfterDisplay = true;
<<<<<<< HEAD
		if ((skipDrawReason & SKIPDRAW_SKIPFRAME) == 0)
=======
		if ((gstate_c.skipDrawReason & SKIPDRAW_SKIPFRAME) == 0)
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
			vfb->reallyDirtyAfterDisplay = true;

		NotifyRenderFramebufferUpdated(vfb, vfbFormatChanged);
	}

	gstate_c.curRTWidth = vfb->width;
	gstate_c.curRTHeight = vfb->height;
	gstate_c.curRTRenderWidth = vfb->renderWidth;
	gstate_c.curRTRenderHeight = vfb->renderHeight;
<<<<<<< HEAD
	return vfb;
=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
}

void FramebufferManagerCommon::UpdateFromMemory(u32 addr, int size, bool safe) {
	addr &= ~0x40000000;
	// TODO: Could go through all FBOs, but probably not important?
	// TODO: Could also check for inner changes, but video is most important.
	bool isDisplayBuf = addr == DisplayFramebufAddr() || addr == PrevDisplayFramebufAddr();
	if (isDisplayBuf || safe) {
		// TODO: Deleting the FBO is a heavy hammer solution, so let's only do it if it'd help.
		if (!Memory::IsValidAddress(displayFramebufPtr_))
			return;
<<<<<<< HEAD
        if (!g_Config.bRefreshDrawingHackb){
=======

>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		for (size_t i = 0; i < vfbs_.size(); ++i) {
			VirtualFramebuffer *vfb = vfbs_[i];
			if (MaskedEqual(vfb->fb_address, addr)) {
				FlushBeforeCopy();

				if (useBufferedRendering_ && vfb->fbo) {
					DisableState();
					GEBufferFormat fmt = vfb->format;
					if (vfb->last_frame_render + 1 < gpuStats.numFlips && isDisplayBuf) {
						// If we're not rendering to it, format may be wrong.  Use displayFormat_ instead.
						fmt = displayFormat_;
					}
					DrawPixels(vfb, 0, 0, Memory::GetPointer(addr | 0x04000000), fmt, vfb->fb_stride, vfb->width, vfb->height);
<<<<<<< HEAD
					SetColorUpdated(vfb, gstate_c.skipDrawReason);
=======
					SetColorUpdated(vfb);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				} else {
					INFO_LOG(SCEGE, "Invalidating FBO for %08x (%i x %i x %i)", vfb->fb_address, vfb->width, vfb->height, vfb->format);
					DestroyFramebuf(vfb);
					vfbs_.erase(vfbs_.begin() + i--);
				}
			}
		}

		RebindFramebuffer();
<<<<<<< HEAD
        }
	}
}

bool FramebufferManagerCommon::NotifyFramebufferCopy(u32 src, u32 dst, int size, bool isMemset, u32 skipDrawReason) {
=======
	}
}

bool FramebufferManagerCommon::NotifyFramebufferCopy(u32 src, u32 dst, int size, bool isMemset) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	if (updateVRAM_ || size == 0) {
		return false;
	}

	dst &= 0x3FFFFFFF;
	src &= 0x3FFFFFFF;

	VirtualFramebuffer *dstBuffer = 0;
	VirtualFramebuffer *srcBuffer = 0;
	u32 dstY = (u32)-1;
	u32 dstH = 0;
	u32 srcY = (u32)-1;
	u32 srcH = 0;
	for (size_t i = 0; i < vfbs_.size(); ++i) {
		VirtualFramebuffer *vfb = vfbs_[i];
		if (vfb->fb_stride == 0) {
			continue;
		}

		const u32 vfb_address = (0x04000000 | vfb->fb_address) & 0x3FFFFFFF;
		const u32 vfb_size = FramebufferByteSize(vfb);
		const u32 vfb_bpp = vfb->format == GE_FORMAT_8888 ? 4 : 2;
		const u32 vfb_byteStride = vfb->fb_stride * vfb_bpp;
		const int vfb_byteWidth = vfb->width * vfb_bpp;

		if (dst >= vfb_address && (dst + size <= vfb_address + vfb_size || dst == vfb_address)) {
			const u32 offset = dst - vfb_address;
			const u32 yOffset = offset / vfb_byteStride;
			if ((offset % vfb_byteStride) == 0 && (size == vfb_byteWidth || (size % vfb_byteStride) == 0) && yOffset < dstY) {
				dstBuffer = vfb;
				dstY = yOffset;
				dstH = size == vfb_byteWidth ? 1 : std::min((u32)size / vfb_byteStride, (u32)vfb->height);
			}
		}

		if (src >= vfb_address && (src + size <= vfb_address + vfb_size || src == vfb_address)) {
			const u32 offset = src - vfb_address;
			const u32 yOffset = offset / vfb_byteStride;
			if ((offset % vfb_byteStride) == 0 && (size == vfb_byteWidth || (size % vfb_byteStride) == 0) && yOffset < srcY) {
				srcBuffer = vfb;
				srcY = yOffset;
				srcH = size == vfb_byteWidth ? 1 : std::min((u32)size / vfb_byteStride, (u32)vfb->height);
<<<<<<< HEAD
			} else if ((offset % vfb_byteStride) == 0 && size == vfb->fb_stride && yOffset < srcY) {
				// Valkyrie Profile reads 512 bytes at a time, rather than 2048.  So, let's whitelist fb_stride also.
				srcBuffer = vfb;
				srcY = yOffset;
				srcH = 1;
=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
			}
		}
	}

	if (srcBuffer && srcY == 0 && srcH == srcBuffer->height && !dstBuffer) {
		// MotoGP workaround - it copies a framebuffer to memory and then displays it.
		// TODO: It's rare anyway, but the game could modify the RAM and then we'd display the wrong thing.
		// Unfortunately, that would force 1x render resolution.
		if (Memory::IsRAMAddress(dst)) {
			knownFramebufferRAMCopies_.insert(std::pair<u32, u32>(src, dst));
		}
	}

	if (!useBufferedRendering_) {
		// If we're copying into a recently used display buf, it's probably destined for the screen.
		if (srcBuffer || (dstBuffer != displayFramebuf_ && dstBuffer != prevDisplayFramebuf_)) {
			return false;
		}
	}

	if (dstBuffer && srcBuffer && !isMemset) {
		if (srcBuffer == dstBuffer) {
			WARN_LOG_REPORT_ONCE(dstsrccpy, G3D, "Intra-buffer memcpy (not supported) %08x -> %08x", src, dst);
		} else {
			WARN_LOG_REPORT_ONCE(dstnotsrccpy, G3D, "Inter-buffer memcpy %08x -> %08x", src, dst);
			// Just do the blit!
			if (g_Config.bBlockTransferGPU) {
				BlitFramebuffer(dstBuffer, 0, dstY, srcBuffer, 0, srcY, srcBuffer->width, srcH, 0);
<<<<<<< HEAD
				SetColorUpdated(dstBuffer, skipDrawReason);
=======
				SetColorUpdated(dstBuffer);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				RebindFramebuffer();
			}
		}
		return false;
	} else if (dstBuffer) {
		WARN_LOG_ONCE(btucpy, G3D, "Memcpy fbo upload %08x -> %08x", src, dst);
		if (g_Config.bBlockTransferGPU) {
			FlushBeforeCopy();
			const u8 *srcBase = Memory::GetPointerUnchecked(src);
			DrawPixels(dstBuffer, 0, dstY, srcBase, dstBuffer->format, dstBuffer->fb_stride, dstBuffer->width, dstH);
<<<<<<< HEAD
			SetColorUpdated(dstBuffer, skipDrawReason);
=======
			SetColorUpdated(dstBuffer);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
			RebindFramebuffer();
			// This is a memcpy, let's still copy just in case.
			return false;
		}
		return false;
	} else if (srcBuffer) {
		WARN_LOG_ONCE(btdcpy, G3D, "Memcpy fbo download %08x -> %08x", src, dst);
		FlushBeforeCopy();
		if (srcH == 0 || srcY + srcH > srcBuffer->bufferHeight) {
			WARN_LOG_REPORT_ONCE(btdcpyheight, G3D, "Memcpy fbo download %08x -> %08x skipped, %d+%d is taller than %d", src, dst, srcY, srcH, srcBuffer->bufferHeight);
		} else if (g_Config.bBlockTransferGPU && !srcBuffer->memoryUpdated) {
			ReadFramebufferToMemory(srcBuffer, true, 0, srcY, srcBuffer->width, srcH);
		}
		return false;
	} else {
		return false;
	}
}

void FramebufferManagerCommon::FindTransferFramebuffers(VirtualFramebuffer *&dstBuffer, VirtualFramebuffer *&srcBuffer, u32 dstBasePtr, int dstStride, int &dstX, int &dstY, u32 srcBasePtr, int srcStride, int &srcX, int &srcY, int &srcWidth, int &srcHeight, int &dstWidth, int &dstHeight, int bpp) const {
	u32 dstYOffset = -1;
	u32 dstXOffset = -1;
	u32 srcYOffset = -1;
	u32 srcXOffset = -1;
	int width = srcWidth;
	int height = srcHeight;

	dstBasePtr &= 0x3FFFFFFF;
	srcBasePtr &= 0x3FFFFFFF;

	for (size_t i = 0; i < vfbs_.size(); ++i) {
		VirtualFramebuffer *vfb = vfbs_[i];
		const u32 vfb_address = (0x04000000 | vfb->fb_address) & 0x3FFFFFFF;
		const u32 vfb_size = FramebufferByteSize(vfb);
		const u32 vfb_bpp = vfb->format == GE_FORMAT_8888 ? 4 : 2;
		const u32 vfb_byteStride = vfb->fb_stride * vfb_bpp;
		const u32 vfb_byteWidth = vfb->width * vfb_bpp;

<<<<<<< HEAD
=======


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         /////               THIS IS EXTREMELY NOISIE BUT CAN RESULT GOOD WITHOUT ALL THIS CHECKS AND MAY SIMPLIFY   MAY RESULT A DEFORMED FB////////////////



>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
		// These heuristics are a bit annoying.
		// The goal is to avoid using GPU block transfers for things that ought to be memory.
		// Maybe we should even check for textures at these places instead?

		if (vfb_address <= dstBasePtr && dstBasePtr < vfb_address + vfb_size) {
			const u32 byteOffset = dstBasePtr - vfb_address;
			const u32 byteStride = dstStride * bpp;
			const u32 yOffset = byteOffset / byteStride;
			// Some games use mismatching bitdepths.  But make sure the stride matches.
			// If it doesn't, generally this means we detected the framebuffer with too large a height.
<<<<<<< HEAD
			bool match = yOffset < dstYOffset;
			if (match && vfb_byteStride != byteStride) {
				// Grand Knights History copies with a mismatching stride but a full line at a time.
				// Makes it hard to detect the wrong transfers in e.g. God of War.
				if (width != dstStride || (byteStride * height != vfb_byteStride && byteStride * height != vfb_byteWidth)) {
					match = false;
				} else {
					dstWidth = byteStride * height / vfb_bpp;
					dstHeight = 1;
				}
			} else if (match) {
				dstWidth = width;
				dstHeight = height;
			}
			if (match) {
				dstYOffset = yOffset;
				dstXOffset = dstStride == 0 ? 0 : (byteOffset / bpp) % dstStride;
				dstBuffer = vfb;
			}
		}
		if (vfb_address <= srcBasePtr && srcBasePtr < vfb_address + vfb_size) {
			const u32 byteOffset = srcBasePtr - vfb_address;
			const u32 byteStride = srcStride * bpp;
			const u32 yOffset = byteOffset / byteStride;
			bool match = yOffset < srcYOffset;
			if (match && vfb_byteStride != byteStride) {
				if (width != srcStride || (byteStride * height != vfb_byteStride && byteStride * height != vfb_byteWidth)) {
					match = false;
				} else {
					srcWidth = byteStride * height / vfb_bpp;
					srcHeight = 1;
				}
			} else if (match) {
				srcWidth = width;
				srcHeight = height;
			}
			if (match) {
				srcYOffset = yOffset;
				srcXOffset = srcStride == 0 ? 0 : (byteOffset / bpp) % srcStride;
				srcBuffer = vfb;
			}
		}
=======
	//		bool match = yOffset < dstYOffset;
		//	if (match && vfb_byteStride != byteStride) {
				// Grand Knights History copies with a mismatching stride but a full line at a time.
				// Makes it hard to detect the wrong transfers in e.g. God of War.
		//		if (width != dstStride || (byteStride * height != vfb_byteStride && byteStride * height != vfb_byteWidth)) {
		//			match = false;
		//		} else {
		//			dstWidth = byteStride * height / vfb_bpp;
		//			dstHeight = 1;
		//		}
		//	} else if (match) {
		//		dstWidth = width;
		//		dstHeight = height;
		//	}
		//	if (match) {
				dstYOffset = yOffset;
				dstXOffset = (byteOffset / bpp) % dstStride;
				dstBuffer = vfb;
		//	}
	//	}
	//	if (vfb_address <= srcBasePtr && srcBasePtr < vfb_address + vfb_size) {
	//		const u32 byteOffset = srcBasePtr - vfb_address;
	//		const u32 byteStride = srcStride * bpp;
	//		const u32 yOffset = byteOffset / byteStride;
	//		bool match = yOffset < srcYOffset;
	//		if (match && vfb_byteStride != byteStride) {
	//			if (width != srcStride || (byteStride * height != vfb_byteStride && byteStride * height != vfb_byteWidth)) {
	//				match = false;
	//			} else {
	//				srcWidth = byteStride * height / vfb_bpp;
	//				srcHeight = 1;
	//			}
	//		} else if (match) {
	//			srcWidth = width;
	//			srcHeight = height;
	//		}
	//		if (match) {
	//			srcYOffset = yOffset;
	//			srcXOffset = (byteOffset / bpp) % srcStride;
	//			srcBuffer = vfb;
	//		}
	//	}
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	}

	if (dstYOffset != (u32)-1) {
		dstY += dstYOffset;
		dstX += dstXOffset;
	}
	if (srcYOffset != (u32)-1) {
		srcY += srcYOffset;
		srcX += srcXOffset;
	}
}

<<<<<<< HEAD

void FramebufferManagerCommon::NotifyVideoUpload(u32 addr, int size, int width, GEBufferFormat fmt) {
    // Note: UpdateFromMemory() is still called later.
    // This is a special case where we have extra information prior to the invalidation.
    
    // TODO: Could possibly be an offset...
    VirtualFramebuffer *vfb = GetVFBAt(addr);
    if (vfb) {
        if (vfb->format != fmt || vfb->drawnFormat != fmt) {
            DEBUG_LOG(ME, "Changing format for %08x from %d to %d", addr, vfb->drawnFormat, fmt);
            vfb->format = fmt;
            vfb->drawnFormat = fmt;
            
            // Let's count this as a "render".  This will also force us to use the correct format.
            vfb->last_frame_render = gpuStats.numFlips;
        }
        
        if (vfb->fb_stride < width) {
            DEBUG_LOG(ME, "Changing stride for %08x from %d to %d", addr, vfb->fb_stride, width);
            const int bpp = fmt == GE_FORMAT_8888 ? 4 : 2;
            ResizeFramebufFBO(vfb, width, size / (bpp * width));
            vfb->fb_stride = width;
            // This might be a bit wider than necessary, but we'll redetect on next render.
            vfb->width = width;
        }
    }
}


bool FramebufferManagerCommon::NotifyBlockTransferBefore(u32 dstBasePtr, int dstStride, int dstX, int dstY, u32 srcBasePtr, int srcStride, int srcX, int srcY, int width, int height, int bpp, u32 skipDrawReason) {
=======
bool FramebufferManagerCommon::NotifyBlockTransferBefore(u32 dstBasePtr, int dstStride, int dstX, int dstY, u32 srcBasePtr, int srcStride, int srcX, int srcY, int width, int height, int bpp) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	if (!useBufferedRendering_ || updateVRAM_) {
		return false;
	}

	// Skip checking if there's no framebuffers in that area.
	if (!MayIntersectFramebuffer(srcBasePtr) && !MayIntersectFramebuffer(dstBasePtr)) {
		return false;
	}

	VirtualFramebuffer *dstBuffer = 0;
	VirtualFramebuffer *srcBuffer = 0;
	int srcWidth = width;
	int srcHeight = height;
	int dstWidth = width;
	int dstHeight = height;
	FindTransferFramebuffers(dstBuffer, srcBuffer, dstBasePtr, dstStride, dstX, dstY, srcBasePtr, srcStride, srcX, srcY, srcWidth, srcHeight, dstWidth, dstHeight, bpp);

	if (dstBuffer && srcBuffer) {
		if (srcBuffer == dstBuffer) {
			if (srcX != dstX || srcY != dstY) {
				WARN_LOG_ONCE(dstsrc, G3D, "Intra-buffer block transfer %08x -> %08x", srcBasePtr, dstBasePtr);
				if (g_Config.bBlockTransferGPU) {
					FlushBeforeCopy();
					BlitFramebuffer(dstBuffer, dstX, dstY, srcBuffer, srcX, srcY, dstWidth, dstHeight, bpp);
					RebindFramebuffer();
<<<<<<< HEAD
					SetColorUpdated(dstBuffer, skipDrawReason);
=======
					SetColorUpdated(dstBuffer);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
					return true;
				}
			} else {
				// Ignore, nothing to do.  Tales of Phantasia X does this by accident.
				if (g_Config.bBlockTransferGPU) {
					return true;
				}
			}
		} else {
			WARN_LOG_ONCE(dstnotsrc, G3D, "Inter-buffer block transfer %08x -> %08x", srcBasePtr, dstBasePtr);
			// Just do the blit!
			if (g_Config.bBlockTransferGPU) {
				FlushBeforeCopy();
				BlitFramebuffer(dstBuffer, dstX, dstY, srcBuffer, srcX, srcY, dstWidth, dstHeight, bpp);
				RebindFramebuffer();
<<<<<<< HEAD
				SetColorUpdated(dstBuffer, skipDrawReason);
=======
				SetColorUpdated(dstBuffer);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				return true;  // No need to actually do the memory copy behind, probably.
			}
		}
		return false;
	} else if (dstBuffer) {
		// Here we should just draw the pixels into the buffer.  Copy first.
		return false;
	} else if (srcBuffer) {
		WARN_LOG_ONCE(btd, G3D, "Block transfer download %08x -> %08x", srcBasePtr, dstBasePtr);
		FlushBeforeCopy();
		if (g_Config.bBlockTransferGPU && !srcBuffer->memoryUpdated) {
			const int srcBpp = srcBuffer->format == GE_FORMAT_8888 ? 4 : 2;
			const float srcXFactor = (float)bpp / srcBpp;
<<<<<<< HEAD
			const bool tooTall = srcY + srcHeight > srcBuffer->bufferHeight;
			if (srcHeight <= 0 || (tooTall && srcY != 0)) {
				WARN_LOG_ONCE(btdheight, G3D, "Block transfer download %08x -> %08x skipped, %d+%d is taller than %d", srcBasePtr, dstBasePtr, srcY, srcHeight, srcBuffer->bufferHeight);
			} else {
				if (tooTall)
					WARN_LOG_ONCE(btdheight, G3D, "Block transfer download %08x -> %08x dangerous, %d+%d is taller than %d", srcBasePtr, dstBasePtr, srcY, srcHeight, srcBuffer->bufferHeight);
=======
			if (srcHeight <= 0 || srcY + srcHeight > srcBuffer->bufferHeight) {
				WARN_LOG_ONCE(btdheight, G3D, "Block transfer download %08x -> %08x skipped, %d+%d is taller than %d", srcBasePtr, dstBasePtr, srcY, srcHeight, srcBuffer->bufferHeight);
			} else {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				ReadFramebufferToMemory(srcBuffer, true, static_cast<int>(srcX * srcXFactor), srcY, static_cast<int>(srcWidth * srcXFactor), srcHeight);
			}
		}
		return false;  // Let the bit copy happen
	} else {
		return false;
	}
}

<<<<<<< HEAD
void FramebufferManagerCommon::NotifyBlockTransferAfter(u32 dstBasePtr, int dstStride, int dstX, int dstY, u32 srcBasePtr, int srcStride, int srcX, int srcY, int width, int height, int bpp, u32 skipDrawReason) {
=======
void FramebufferManagerCommon::NotifyBlockTransferAfter(u32 dstBasePtr, int dstStride, int dstX, int dstY, u32 srcBasePtr, int srcStride, int srcX, int srcY, int width, int height, int bpp) {
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
	// A few games use this INSTEAD of actually drawing the video image to the screen, they just blast it to
	// the backbuffer. Detect this and have the framebuffermanager draw the pixels.

	u32 backBuffer = PrevDisplayFramebufAddr();
	u32 displayBuffer = DisplayFramebufAddr();

	// TODO: Is this not handled by upload?  Should we check !dstBuffer to avoid a double copy?
	if (((backBuffer != 0 && dstBasePtr == backBuffer) ||
		(displayBuffer != 0 && dstBasePtr == displayBuffer)) &&
		dstStride == 512 && height == 272 && !useBufferedRendering_) {
		FlushBeforeCopy();
		DrawFramebuffer(Memory::GetPointerUnchecked(dstBasePtr), displayFormat_, 512, false);
	}

	if (MayIntersectFramebuffer(srcBasePtr) || MayIntersectFramebuffer(dstBasePtr)) {
		VirtualFramebuffer *dstBuffer = 0;
		VirtualFramebuffer *srcBuffer = 0;
		int srcWidth = width;
		int srcHeight = height;
		int dstWidth = width;
		int dstHeight = height;
		FindTransferFramebuffers(dstBuffer, srcBuffer, dstBasePtr, dstStride, dstX, dstY, srcBasePtr, srcStride, srcX, srcY, srcWidth, srcHeight, dstWidth, dstHeight, bpp);

		if (!useBufferedRendering_ && currentRenderVfb_ != dstBuffer) {
			return;
		}

		if (dstBuffer && !srcBuffer) {
			WARN_LOG_ONCE(btu, G3D, "Block transfer upload %08x -> %08x", srcBasePtr, dstBasePtr);
			if (g_Config.bBlockTransferGPU) {
				FlushBeforeCopy();
				const u8 *srcBase = Memory::GetPointerUnchecked(srcBasePtr) + (srcX + srcY * srcStride) * bpp;
				int dstBpp = dstBuffer->format == GE_FORMAT_8888 ? 4 : 2;
				float dstXFactor = (float)bpp / dstBpp;
				DrawPixels(dstBuffer, static_cast<int>(dstX * dstXFactor), dstY, srcBase, dstBuffer->format, static_cast<int>(srcStride * dstXFactor), static_cast<int>(dstWidth * dstXFactor), dstHeight);
<<<<<<< HEAD
				SetColorUpdated(dstBuffer, skipDrawReason);
=======
				SetColorUpdated(dstBuffer);
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
				RebindFramebuffer();
			}
		}
	}
}
<<<<<<< HEAD

void FramebufferManagerCommon::SetRenderSize(VirtualFramebuffer *vfb) {
	float renderWidthFactor = (float)PSP_CoreParameter().renderWidth / 480.0f;
	float renderHeightFactor = (float)PSP_CoreParameter().renderHeight / 272.0f;
	bool force1x = false;
	switch (g_Config.iBloomHack) {
	case 1:
		force1x = vfb->bufferWidth <= 128 || vfb->bufferHeight <= 64;
		break;
	case 2:
		force1x = vfb->bufferWidth <= 256 || vfb->bufferHeight <= 128;
		break;
	case 3:
		force1x = vfb->bufferWidth < 480 || vfb->bufferHeight < 272;
		break;
	}

	if (force1x && g_Config.iInternalResolution != 1) {
		vfb->renderWidth = vfb->bufferWidth;
		vfb->renderHeight = vfb->bufferHeight;
	}
	else {
		vfb->renderWidth = (u16)(vfb->bufferWidth * renderWidthFactor);
		vfb->renderHeight = (u16)(vfb->bufferHeight * renderHeightFactor);
	}
}

void FramebufferManagerCommon::UpdateFramebufUsage(VirtualFramebuffer *vfb) {
	auto checkFlag = [&](u16 flag, int last_frame) {
		if (vfb->usageFlags & flag) {
			const int age = frameLastFramebufUsed_ - last_frame;
			if (age > FBO_OLD_USAGE_FLAG) {
				vfb->usageFlags &= ~flag;
			}
		}
	};
    

	checkFlag(FB_USAGE_DISPLAYED_FRAMEBUFFER, vfb->last_frame_displayed);
	checkFlag(FB_USAGE_TEXTURE, vfb->last_frame_used);
	checkFlag(FB_USAGE_RENDERTARGET, vfb->last_frame_render);
}
=======
>>>>>>> 63c6f00de0562ceae061dcd71caa7b575da4f1b2
