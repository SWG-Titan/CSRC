// ======================================================================
//
// SseMath.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/SseMath.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <xmmintrin.h>
#include <intrin.h>

// ======================================================================

#define SSE_ALIGN  __declspec(align(16))

// ======================================================================

static inline float horizontalSum(__m128 v)
{
	SSE_ALIGN float tmp[4];
	_mm_store_ps(tmp, v);
	return tmp[0] + tmp[1] + tmp[2] + tmp[3];
}

static inline float horizontalSum3(__m128 v)
{
	SSE_ALIGN float tmp[4];
	_mm_store_ps(tmp, v);
	return tmp[0] + tmp[1] + tmp[2];
}

// ======================================================================

bool SseMath::canDoSseMath()
{
#ifdef _M_X64
	return true;
#else
	bool cpuHasSse = false;
	bool cpuHasSaveRestore = false;

	int cpuInfo[4] = {0};
	__cpuid(cpuInfo, 1);
	uint32 featureBits = static_cast<uint32>(cpuInfo[3]);

	cpuHasSse         = ((featureBits & 0x02000000) != 0);
	cpuHasSaveRestore = ((featureBits & 0x01000000) != 0);

	return cpuHasSse && cpuHasSaveRestore;
#endif
}

// ----------------------------------------------------------------------

Vector SseMath::rotateTranslateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
	const float * m = reinterpret_cast<const float *>(&transform);

	__m128 row0 = _mm_load_ps(m + 0);
	__m128 row1 = _mm_load_ps(m + 4);
	__m128 row2 = _mm_load_ps(m + 8);

	SSE_ALIGN float src[4] = { vector.x, vector.y, vector.z, 1.0f };
	__m128 vec = _mm_load_ps(src);

	__m128 scaleVec = _mm_set1_ps(scale);

	__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
	__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
	__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

	return Vector(horizontalSum(r0), horizontalSum(r1), horizontalSum(r2));
}

// ----------------------------------------------------------------------

Vector SseMath::rotateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
	const float * m = reinterpret_cast<const float *>(&transform);

	__m128 row0 = _mm_load_ps(m + 0);
	__m128 row1 = _mm_load_ps(m + 4);
	__m128 row2 = _mm_load_ps(m + 8);

	SSE_ALIGN float src[4] = { vector.x, vector.y, vector.z, 0.0f };
	__m128 vec = _mm_load_ps(src);

	__m128 scaleVec = _mm_set1_ps(scale);

	__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
	__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
	__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

	return Vector(horizontalSum3(r0), horizontalSum3(r1), horizontalSum3(r2));
}

// ----------------------------------------------------------------------

void SseMath::skinPositionNormal_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	const float * m = reinterpret_cast<const float *>(&transform);

	__m128 row0 = _mm_load_ps(m + 0);
	__m128 row1 = _mm_load_ps(m + 4);
	__m128 row2 = _mm_load_ps(m + 8);

	__m128 scaleVec = _mm_set1_ps(scale);

	// Transform position (w=1 for translation)
	{
		SSE_ALIGN float src[4] = { sourcePosition.x, sourcePosition.y, sourcePosition.z, 1.0f };
		__m128 vec = _mm_load_ps(src);

		__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
		__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
		__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

		destPosition.x = horizontalSum(r0);
		destPosition.y = horizontalSum(r1);
		destPosition.z = horizontalSum(r2);
	}

	// Transform normal (w=1 but only sum first 3 components — rotation only)
	{
		SSE_ALIGN float src[4] = { sourceNormal.x, sourceNormal.y, sourceNormal.z, 1.0f };
		__m128 vec = _mm_load_ps(src);

		__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
		__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
		__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

		destNormal.x = horizontalSum3(r0);
		destNormal.y = horizontalSum3(r1);
		destNormal.z = horizontalSum3(r2);
	}
}

// ----------------------------------------------------------------------

void SseMath::skinPositionNormalAdd_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	const float * m = reinterpret_cast<const float *>(&transform);

	__m128 row0 = _mm_load_ps(m + 0);
	__m128 row1 = _mm_load_ps(m + 4);
	__m128 row2 = _mm_load_ps(m + 8);

	__m128 scaleVec = _mm_set1_ps(scale);

	// Transform and accumulate position
	{
		SSE_ALIGN float src[4] = { sourcePosition.x, sourcePosition.y, sourcePosition.z, 1.0f };
		__m128 vec = _mm_load_ps(src);

		__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
		__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
		__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

		destPosition.x += horizontalSum(r0);
		destPosition.y += horizontalSum(r1);
		destPosition.z += horizontalSum(r2);
	}

	// Transform and accumulate normal
	{
		SSE_ALIGN float src[4] = { sourceNormal.x, sourceNormal.y, sourceNormal.z, 1.0f };
		__m128 vec = _mm_load_ps(src);

		__m128 r0 = _mm_mul_ps(_mm_mul_ps(row0, vec), scaleVec);
		__m128 r1 = _mm_mul_ps(_mm_mul_ps(row1, vec), scaleVec);
		__m128 r2 = _mm_mul_ps(_mm_mul_ps(row2, vec), scaleVec);

		destNormal.x += horizontalSum3(r0);
		destNormal.y += horizontalSum3(r1);
		destNormal.z += horizontalSum3(r2);
	}
}

// ======================================================================
