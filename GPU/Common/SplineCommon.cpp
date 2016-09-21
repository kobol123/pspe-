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

#include <string.h>
#include <algorithm>

#include "Core/Config.h"

#include "GPU/Common/SplineCommon.h"
#include "GPU/ge_constants.h"
#include "GPU/GPUState.h"

#define START_OPEN 1
#define END_OPEN 2



static void CopyQuad(u8 *&dest, const SimpleVertex *v1, const SimpleVertex *v2, const SimpleVertex* v3, const SimpleVertex *v4) {
	int vertexSize = sizeof(SimpleVertex);
	memcpy(dest, v1, vertexSize);
	dest += vertexSize;
	memcpy(dest, v2, vertexSize);
	dest += vertexSize;
	memcpy(dest, v3, vertexSize);
	dest += vertexSize;
	memcpy(dest, v4, vertexSize);
	dest += vertexSize;
}

#undef b2

// Bernstein basis functions
inline float bern0(float x) { return (1 - x) * (1 - x) * (1 - x); }
inline float bern1(float x) { return 3 * x * (1 - x) * (1 - x); }
inline float bern2(float x) { return 3 * x * x * (1 - x); }
inline float bern3(float x) { return x * x * x; }

// Not sure yet if these have any use
inline float bern0deriv(float x) { return -3 * (x - 1) * (x - 1); }
inline float bern1deriv(float x) { return 9 * x * x - 12 * x + 3; }
inline float bern2deriv(float x) { return 3 * (2 - 3 * x) * x; }
inline float bern3deriv(float x) { return 3 * x * x; }

// http://en.wikipedia.org/wiki/Bernstein_polynomial
Vec3Packedf Bernstein3D(const Vec3Packedf p0, const Vec3Packedf p1, const Vec3Packedf p2, const Vec3Packedf p3, float x) {
	return p0 * bern0(x) + p1 * bern1(x) + p2 * bern2(x) + p3 * bern3(x);
}

Vec3Packedf Bernstein3DDerivative(const Vec3Packedf p0, const Vec3Packedf p1, const Vec3Packedf p2, const Vec3Packedf p3, float x) {
	return p0 * bern0deriv(x) + p1 * bern1deriv(x) + p2 * bern2deriv(x) + p3 * bern3deriv(x);
}

void spline_n_4(int i, float t, float *knot, float *splineVal) {
	knot += i + 1;

	float t0 = (t - knot[0]);
	float t1 = (t - knot[1]);
	float t2 = (t - knot[2]);
	float f30 = t0/(knot[3]-knot[0]);
	float f41 = t1/(knot[4]-knot[1]);
	float f52 = t2/(knot[5]-knot[2]);
	float f31 = t1/(knot[3]-knot[1]);
	float f42 = t2/(knot[4]-knot[2]);
	float f32 = t2/(knot[3]-knot[2]);
	float a = (1-f30)*(1-f31);
	float b = (f31*f41);
	float c = (1-f41)*(1-f42);
	float d = (f42*f52);

	splineVal[0] = a-(a*f32);
	splineVal[1] = 1-a-b+((a+b+c-1)*f32);
	splineVal[2] = b+((1-b-c-d)*f32);
	splineVal[3] = d*f32;
}

// knot should be an array sized n + 5  (n + 1 + 1 + degree (cubic))
void spline_knot(int n, int type, float *knot) {
	memset(knot, 0, sizeof(float) * (n + 5));
	for (int i = 0; i < n - 1; ++i)
		knot[i + 3] = (float)i;

	if ((type & 1) == 0) {
		knot[0] = -3;
		knot[1] = -2;
		knot[2] = -1;
	}
	if ((type & 2) == 0) {
		knot[n + 2] = (float)(n - 1);
		knot[n + 3] = (float)(n);
		knot[n + 4] = (float)(n + 1);
	} else {
		knot[n + 2] = (float)(n - 2);
		knot[n + 3] = (float)(n - 2);
		knot[n + 4] = (float)(n - 2);
	}
}

void _SplinePatchLowQuality(u8 *&dest, int &count, const SplinePatchLocal &spatch, u32 origVertType) {
	const float third = 1.0f / 3.0f;
	// Fast and easy way - just draw the control points, generate some very basic normal vector substitutes.
	// Very inaccurate but okay for Loco Roco. Maybe should keep it as an option because it's fast.

	const int tile_min_u = (spatch.type_u & START_OPEN) ? 0 : 1;
	const int tile_min_v = (spatch.type_v & START_OPEN) ? 0 : 1;
	const int tile_max_u = (spatch.type_u & END_OPEN) ? spatch.count_u - 1 : spatch.count_u - 2;
	const int tile_max_v = (spatch.type_v & END_OPEN) ? spatch.count_v - 1 : spatch.count_v - 2;

	for (int tile_v = tile_min_v; tile_v < tile_max_v; ++tile_v) {
		for (int tile_u = tile_min_u; tile_u < tile_max_u; ++tile_u) {
			int point_index = tile_u + tile_v * spatch.count_u;

			SimpleVertex v0 = *spatch.points[point_index];
			SimpleVertex v1 = *spatch.points[point_index + 1];
			SimpleVertex v2 = *spatch.points[point_index + spatch.count_u];
			SimpleVertex v3 = *spatch.points[point_index + spatch.count_u + 1];

			// Generate UV. TODO: Do this even if UV specified in control points?
			if ((origVertType & GE_VTYPE_TC_MASK) == 0) {
				float u = tile_u * third;
				float v = tile_v * third;
				v0.uv[0] = u;
				v0.uv[1] = v;
				v1.uv[0] = u + third;
				v1.uv[1] = v;
				v2.uv[0] = u;
				v2.uv[1] = v + third;
				v3.uv[0] = u + third;
				v3.uv[1] = v + third;
			}

			// Generate normal if lighting is enabled (otherwise there's no point).
			// This is a really poor quality algorithm, we get facet normals.
			if (gstate.isLightingEnabled()) {
				Vec3Packedf norm = Cross(v1.pos - v0.pos, v2.pos - v0.pos);
				norm.Normalize();
				if (gstate.patchfacing & 1)
					norm *= -1.0f;
				v0.nrm = norm;
				v1.nrm = norm;
				v2.nrm = norm;
				v3.nrm = norm;
			}

			CopyQuad(dest, &v0, &v1, &v2, &v3);
			count += 6;
		}
	}

}

void  _SplinePatchFullQuality(u8 *&dest, int &count, const SplinePatchLocal &spatch, u32 origVertType, int patch_cap) {
	// Full (mostly) correct tessellation of spline patches.
	// Not very fast.

	// First, generate knot vectors.
	int n = spatch.count_u - 1;
	int m = spatch.count_v - 1;

	float *knot_u = new float[n + 5];
	float *knot_v = new float[m + 5];
	spline_knot(n, spatch.type_u, knot_u);
	spline_knot(m, spatch.type_v, knot_v);

	// Increase tesselation based on the size. Should be approximately right?
	// JPCSP is wrong at least because their method results in square loco roco.
	int patch_div_s = (spatch.count_u - 3) * gstate.getPatchDivisionU() / 3;
	int patch_div_t = (spatch.count_v - 3) * gstate.getPatchDivisionV() / 3;

	if (patch_div_s <= 0) patch_div_s = 1;
	if (patch_div_t <= 0) patch_div_t = 1;

	// TODO: Remove this cap when spline_s has been optimized. 
	if (patch_div_s > patch_cap) patch_div_s = patch_cap;
	if (patch_div_t > patch_cap) patch_div_t = patch_cap;

	// First compute all the vertices and put them in an array
	SimpleVertex *vertices = new SimpleVertex[(patch_div_s + 1) * (patch_div_t + 1)];

	float tu_width = 1.0f + (spatch.count_u - 4) * 1.0f / 3.0f;
	float tv_height = 1.0f + (spatch.count_v - 4) * 1.0f / 3.0f;

	bool computeNormals = gstate.isLightingEnabled();
	for (int tile_v = 0; tile_v < patch_div_t + 1; tile_v++) {
		float v = ((float)tile_v * (float)(m - 2) / (float)(patch_div_t + 0.00001f));  // epsilon to prevent division by 0 in spline_s
		if (v < 0.0f)
			v = 0.0f;
		for (int tile_u = 0; tile_u < patch_div_s + 1; tile_u++) {
			float u = ((float)tile_u * (float)(n - 2) / (float)(patch_div_s + 0.00001f));
			if (u < 0.0f)
				u = 0.0f;
			SimpleVertex *vert = &vertices[tile_v * (patch_div_s + 1) + tile_u];
			vert->pos.SetZero();
			if (origVertType & GE_VTYPE_NRM_MASK) {
				vert->nrm.SetZero();
			}
			else {
				vert->nrm.SetZero();
				vert->nrm.z = 1.0f;
			}
			if (origVertType & GE_VTYPE_COL_MASK) {
				memset(vert->color, 0, 4);
			}
			else {
				memcpy(vert->color, spatch.points[0]->color, 4);
			}
			if (origVertType & GE_VTYPE_TC_MASK) {
				vert->uv[0] = 0.0f;
				vert->uv[1] = 0.0f;
			}
			else {
				vert->uv[0] = tu_width * ((float)tile_u / (float)patch_div_s);
				vert->uv[1] = tv_height * ((float)tile_v / (float)patch_div_t);
			}

			// Collect influences from surrounding control points.
			float u_weights[4];
			float v_weights[4];

			int iu = (int)u;
			int iv = (int)v;
			spline_n_4(iu, u, knot_u, u_weights);
			spline_n_4(iv, v, knot_v, v_weights);

			// Handle degenerate patches. without this, spatch.points[] may read outside the number of initialized points.
			int patch_w = std::min(spatch.count_u, 4);
			int patch_h = std::min(spatch.count_v, 4);

			for (int ii = 0; ii < patch_w; ++ii) {
				for (int jj = 0; jj < patch_h; ++jj) {
					float u_spline = u_weights[ii];
					float v_spline = v_weights[jj];
					float f = u_spline * v_spline;

					if (f > 0.0f) {
						int idx = spatch.count_u * (iv + jj) + (iu + ii);
						SimpleVertex *a = spatch.points[idx];
						vert->pos += a->pos * f;
						if (origVertType & GE_VTYPE_TC_MASK) {
							vert->uv[0] += a->uv[0] * f;
							vert->uv[1] += a->uv[1] * f;
						}
						if (origVertType & GE_VTYPE_COL_MASK) {
							// TODO: Accumulating values in u8s is crazy. We need floats or something.
							vert->color[0] += a->color[0] * f;
							vert->color[1] += a->color[1] * f;
							vert->color[2] += a->color[2] * f;
							vert->color[3] += a->color[3] * f;
						}
						if (origVertType & GE_VTYPE_NRM_MASK) {
							vert->nrm += a->nrm * f;
						}
					}
				}
			}
			if (origVertType & GE_VTYPE_NRM_MASK) {
				vert->nrm.Normalize();
			}
		}
	}

	delete[] knot_u;
	delete[] knot_v;

	// Hacky normal generation through central difference.
	if (gstate.isLightingEnabled() && (origVertType & GE_VTYPE_NRM_MASK) == 0) {
		for (int v = 0; v < patch_div_t + 1; v++) {
			for (int u = 0; u < patch_div_s + 1; u++) {
				int l = std::max(0, u - 1);
				int t = std::max(0, v - 1);
				int r = std::min(patch_div_s, u + 1);
				int b = std::min(patch_div_t, v + 1);

				const Vec3Packedf &right = vertices[v * (patch_div_s + 1) + r].pos - vertices[v * (patch_div_s + 1) + l].pos;
				const Vec3Packedf &down = vertices[b * (patch_div_s + 1) + u].pos - vertices[t * (patch_div_s + 1) + u].pos;

				vertices[v * (patch_div_s + 1) + u].nrm = Cross(right, down).Normalized();
				if (gstate.patchfacing & 1) {
					vertices[v * (patch_div_s + 1) + u].nrm *= -1.0f;
				}
			}
		}
	}

	// Tesselate. TODO: Use indices so we only need to emit 4 vertices per pair of triangles instead of six.
	for (int tile_v = 0; tile_v < patch_div_t; ++tile_v) {
		for (int tile_u = 0; tile_u < patch_div_s; ++tile_u) {
			float u = ((float)tile_u / (float)patch_div_s);
			float v = ((float)tile_v / (float)patch_div_t);

			SimpleVertex *v0 = &vertices[tile_v * (patch_div_s + 1) + tile_u];
			SimpleVertex *v1 = &vertices[tile_v * (patch_div_s + 1) + tile_u + 1];
			SimpleVertex *v2 = &vertices[(tile_v + 1) * (patch_div_s + 1) + tile_u];
			SimpleVertex *v3 = &vertices[(tile_v + 1) * (patch_div_s + 1) + tile_u + 1];

			CopyQuad(dest, v0, v1, v2, v3);
			count += 6;
		}
	}

	delete[] vertices;
}

void TesselateSplinePatch(u8 *&dest, int &count, const SplinePatchLocal &spatch, u32 origVertType) {
	switch (g_Config.iSplineBezierQuality) {
	case LOW_QUALITY:
		_SplinePatchLowQuality(dest, count, spatch, origVertType);
		break;
	case MEDIUM_QUALITY:
		_SplinePatchFullQuality(dest, count, spatch, origVertType, 8);
		break;
	case HIGH_QUALITY:
		_SplinePatchFullQuality(dest, count, spatch, origVertType, 64);
		break;
	}
}

void _BezierPatchLowQuality(u8 *&dest, int &count, int tess_u, int tess_v, const BezierPatch &patch, u32 origVertType) {
	const float third = 1.0f / 3.0f;
	// Fast and easy way - just draw the control points, generate some very basic normal vector subsitutes.
	// Very inaccurate though but okay for Loco Roco. Maybe should keep it as an option.

	float u_base = patch.u_index / 3.0f;
	float v_base = patch.v_index / 3.0f;

	for (int tile_v = 0; tile_v < 3; tile_v++) {
		for (int tile_u = 0; tile_u < 3; tile_u++) {
			int point_index = tile_u + tile_v * 4;

			SimpleVertex v0 = *patch.points[point_index];
			SimpleVertex v1 = *patch.points[point_index + 1];
			SimpleVertex v2 = *patch.points[point_index + 4];
			SimpleVertex v3 = *patch.points[point_index + 5];

			// Generate UV. TODO: Do this even if UV specified in control points?
			if ((origVertType & GE_VTYPE_TC_MASK) == 0) {
				float u = u_base + tile_u * third;
				float v = v_base + tile_v * third;
				v0.uv[0] = u;
				v0.uv[1] = v;
				v1.uv[0] = u + third;
				v1.uv[1] = v;
				v2.uv[0] = u;
				v2.uv[1] = v + third;
				v3.uv[0] = u + third;
				v3.uv[1] = v + third;
			}

			// Generate normal if lighting is enabled (otherwise there's no point).
			// This is a really poor quality algorithm, we get facet normals.
			if (gstate.isLightingEnabled()) {
				Vec3Packedf norm = Cross(v1.pos - v0.pos, v2.pos - v0.pos);
				norm.Normalize();
				if (gstate.patchfacing & 1)
					norm *= -1.0f;
				v0.nrm = norm;
				v1.nrm = norm;
				v2.nrm = norm;
				v3.nrm = norm;
			}

			CopyQuad(dest, &v0, &v1, &v2, &v3);
			count += 6;
		}
	}
}

void _BezierPatchHighQuality(u8 *&dest, int &count, int tess_u, int tess_v, const BezierPatch &patch, u32 origVertType) {
	const float third = 1.0f / 3.0f;
	// Full correct tesselation of bezier patches.
	// Note: Does not handle splines correctly.

	// First compute all the vertices and put them in an array
	SimpleVertex *vertices = new SimpleVertex[(tess_u + 1) * (tess_v + 1)];

	Vec3Packedf *horiz = new Vec3Packedf[(tess_u + 1) * 4];
	Vec3Packedf *horiz2 = horiz + (tess_u + 1) * 1;
	Vec3Packedf *horiz3 = horiz + (tess_u + 1) * 2;
	Vec3Packedf *horiz4 = horiz + (tess_u + 1) * 3;

	// Precompute the horizontal curves to we only have to evaluate the vertical ones.
	for (int i = 0; i < tess_u + 1; i++) {
		float u = ((float)i / (float)tess_u);
		horiz[i] = Bernstein3D(patch.points[0]->pos, patch.points[1]->pos, patch.points[2]->pos, patch.points[3]->pos, u);
		horiz2[i] = Bernstein3D(patch.points[4]->pos, patch.points[5]->pos, patch.points[6]->pos, patch.points[7]->pos, u);
		horiz3[i] = Bernstein3D(patch.points[8]->pos, patch.points[9]->pos, patch.points[10]->pos, patch.points[11]->pos, u);
		horiz4[i] = Bernstein3D(patch.points[12]->pos, patch.points[13]->pos, patch.points[14]->pos, patch.points[15]->pos, u);
	}

	bool computeNormals = gstate.isLightingEnabled();

	for (int tile_v = 0; tile_v < tess_v + 1; ++tile_v) {
		for (int tile_u = 0; tile_u < tess_u + 1; ++tile_u) {
			float u = ((float)tile_u / (float)tess_u);
			float v = ((float)tile_v / (float)tess_v);
			float bu = u;
			float bv = v;

			// TODO: Should be able to precompute the four curves per U, then just Bernstein per V. Will benefit large tesselation factors.
			const Vec3Packedf &pos1 = horiz[tile_u];
			const Vec3Packedf &pos2 = horiz2[tile_u];
			const Vec3Packedf &pos3 = horiz3[tile_u];
			const Vec3Packedf &pos4 = horiz4[tile_u];

			SimpleVertex &vert = vertices[tile_v * (tess_u + 1) + tile_u];

			if (computeNormals) {
				Vec3Packedf derivU1 = Bernstein3DDerivative(patch.points[0]->pos, patch.points[1]->pos, patch.points[2]->pos, patch.points[3]->pos, bu);
				Vec3Packedf derivU2 = Bernstein3DDerivative(patch.points[4]->pos, patch.points[5]->pos, patch.points[6]->pos, patch.points[7]->pos, bu);
				Vec3Packedf derivU3 = Bernstein3DDerivative(patch.points[8]->pos, patch.points[9]->pos, patch.points[10]->pos, patch.points[11]->pos, bu);
				Vec3Packedf derivU4 = Bernstein3DDerivative(patch.points[12]->pos, patch.points[13]->pos, patch.points[14]->pos, patch.points[15]->pos, bu);
				Vec3Packedf derivU = Bernstein3D(derivU1, derivU2, derivU3, derivU4, bv);
				Vec3Packedf derivV = Bernstein3DDerivative(pos1, pos2, pos3, pos4, bv);

				// TODO: Interpolate normals instead of generating them, if available?
				vert.nrm = Cross(derivU, derivV).Normalized();
				if (gstate.patchfacing & 1)
					vert.nrm *= -1.0f;
			}
			else {
				vert.nrm.SetZero();
			}

			vert.pos = Bernstein3D(pos1, pos2, pos3, pos4, bv);

			if ((origVertType & GE_VTYPE_TC_MASK) == 0) {
				// Generate texcoord
				vert.uv[0] = u + patch.u_index * third;
				vert.uv[1] = v + patch.v_index * third;
			} else {
				// Sample UV from control points
				patch.sampleTexUV(u, v, vert.uv[0], vert.uv[1]);
			} 

			if (origVertType & GE_VTYPE_COL_MASK) {
				patch.sampleColor(u, v, vert.color);
			} else {
				memcpy(vert.color, patch.points[0]->color, 4);
			}
		}
	}
	delete[] horiz;

	// Tesselate. TODO: Use indices so we only need to emit 4 vertices per pair of triangles instead of six.
	for (int tile_v = 0; tile_v < tess_v; ++tile_v) {
		for (int tile_u = 0; tile_u < tess_u; ++tile_u) {
			float u = ((float)tile_u / (float)tess_u);
			float v = ((float)tile_v / (float)tess_v);

			const SimpleVertex *v0 = &vertices[tile_v * (tess_u + 1) + tile_u];
			const SimpleVertex *v1 = &vertices[tile_v * (tess_u + 1) + tile_u + 1];
			const SimpleVertex *v2 = &vertices[(tile_v + 1) * (tess_u + 1) + tile_u];
			const SimpleVertex *v3 = &vertices[(tile_v + 1) * (tess_u + 1) + tile_u + 1];

			CopyQuad(dest, v0, v1, v2, v3);
			count += 6;
		}
	}

	delete[] vertices;
}

void TesselateBezierPatch(u8 *&dest, int &count, int tess_u, int tess_v, const BezierPatch &patch, u32 origVertType) {
	switch (g_Config.iSplineBezierQuality) {
	case LOW_QUALITY:
		_BezierPatchLowQuality(dest, count, tess_u, tess_v, patch, origVertType);
		break;
	case MEDIUM_QUALITY:
	case HIGH_QUALITY:
		_BezierPatchHighQuality(dest, count, tess_u, tess_v, patch, origVertType);
		break;
	}
}
