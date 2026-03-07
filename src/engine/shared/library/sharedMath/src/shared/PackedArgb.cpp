// ======================================================================
//
// PackedArgb.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PackedArgb.h"

#include "sharedMath/VectorArgb.h"

// ======================================================================

const real PackedArgb::oo255 = RECIP (255);

const PackedArgb PackedArgb::solidBlack   (255,   0,   0,   0);
const PackedArgb PackedArgb::solidBlue    (255,   0,   0, 255);
const PackedArgb PackedArgb::solidCyan    (255,   0, 255, 255);
const PackedArgb PackedArgb::solidGreen   (255,   0, 255,   0);
const PackedArgb PackedArgb::solidRed     (255, 255,   0,   0);
const PackedArgb PackedArgb::solidMagenta (255, 255,   0, 255);
const PackedArgb PackedArgb::solidYellow  (255, 255, 255,   0);
const PackedArgb PackedArgb::solidWhite   (255, 255, 255, 255);
const PackedArgb PackedArgb::solidGray    (255, 128, 128, 128);

// ======================================================================

PackedArgb const PackedArgb::linearInterpolate(PackedArgb const & color1, PackedArgb const & color2, float const t)
{
	return PackedArgb(
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getA()), static_cast<int>(color2.getA()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getR()), static_cast<int>(color2.getR()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getG()), static_cast<int>(color2.getG()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getB()), static_cast<int>(color2.getB()), t)));
}

// ======================================================================

/**
 * Construct a PackedArgb value.
 * @param argb The initial component values.
 */

//#include "sharedMath/VectorArgb.h"

PackedArgb::PackedArgb(const VectorArgb &argb)
: m_argb(convert(argb.a, argb.r, argb.g, argb.b))
{
}

// ----------------------------------------------------------------------
/**
 * Set the color.
 * @argb The new alpha and color value.
 */

void PackedArgb::setArgb(const VectorArgb &argb)
{
	m_argb = convert(argb.a, argb.r, argb.g, argb.b);
}

// ----------------------------------------------------------------------
/**
 * Parse an HTML color string and create a PackedArgb.
 * Supports formats: #RGB, #RRGGBB, #AARRGGBB (with or without #)
 * @param htmlColor  The HTML color string (e.g., "#FF5500" or "FF5500")
 * @return  The parsed color, or solidMagenta if invalid
 */
PackedArgb PackedArgb::fromHtmlString(const char * htmlColor)
{
	if (!htmlColor || !*htmlColor)
		return solidMagenta;

	// Skip leading # if present
	const char * p = htmlColor;
	if (*p == '#')
		++p;

	// Determine length
	int len = 0;
	const char * q = p;
	while (*q && len < 9)
	{
		char c = *q;
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
		{
			++len;
			++q;
		}
		else
		{
			break;
		}
	}

	// Parse based on length
	uint32 value = 0;
	for (int i = 0; i < len; ++i)
	{
		char c = p[i];
		int digit = 0;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = 10 + (c - 'a');
		else if (c >= 'A' && c <= 'F')
			digit = 10 + (c - 'A');

		value = (value << 4) | digit;
	}

	uint8 a = 255, r = 0, g = 0, b = 0;

	if (len == 3)
	{
		// #RGB -> #RRGGBB
		r = static_cast<uint8>(((value >> 8) & 0xF) * 17);
		g = static_cast<uint8>(((value >> 4) & 0xF) * 17);
		b = static_cast<uint8>((value & 0xF) * 17);
	}
	else if (len == 6)
	{
		// #RRGGBB
		r = static_cast<uint8>((value >> 16) & 0xFF);
		g = static_cast<uint8>((value >> 8) & 0xFF);
		b = static_cast<uint8>(value & 0xFF);
	}
	else if (len == 8)
	{
		// #AARRGGBB
		a = static_cast<uint8>((value >> 24) & 0xFF);
		r = static_cast<uint8>((value >> 16) & 0xFF);
		g = static_cast<uint8>((value >> 8) & 0xFF);
		b = static_cast<uint8>(value & 0xFF);
	}
	else
	{
		return solidMagenta;  // Invalid format
	}

	return PackedArgb(a, r, g, b);
}

// ----------------------------------------------------------------------
/**
 * Check if a string is a valid HTML color format.
 * @param htmlColor  The string to validate
 * @return  true if valid HTML color format
 */
bool PackedArgb::isValidHtmlColor(const char * htmlColor)
{
	if (!htmlColor || !*htmlColor)
		return false;

	const char * p = htmlColor;
	if (*p == '#')
		++p;

	int len = 0;
	while (*p)
	{
		char c = *p;
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
		{
			++len;
			++p;
		}
		else
		{
			return false;
		}
	}

	return (len == 3 || len == 6 || len == 8);
}

// ----------------------------------------------------------------------
/**
 * Convert the color to an HTML color string.
 * @param buffer      Output buffer for the string
 * @param bufferSize  Size of the output buffer (should be at least 8 for #RRGGBB, 10 for #AARRGGBB)
 * @param includeAlpha  If true, output #AARRGGBB format; otherwise #RRGGBB
 */
void PackedArgb::toHtmlString(char * buffer, int bufferSize, bool includeAlpha) const
{
	if (!buffer || bufferSize < 2)
		return;

	if (includeAlpha)
	{
		if (bufferSize >= 10)
		{
			snprintf(buffer, bufferSize, "#%02X%02X%02X%02X", getA(), getR(), getG(), getB());
		}
	}
	else
	{
		if (bufferSize >= 8)
		{
			snprintf(buffer, bufferSize, "#%02X%02X%02X", getR(), getG(), getB());
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Encode a color as a special negative palette index.
 * This allows storing direct RGB colors in systems that expect palette indices.
 * Uses RGB565 encoding for 16-bit compatibility:
 * - R: 5 bits (0-31), G: 6 bits (0-63), B: 5 bits (0-31) = 16 bits total
 * Formula: index = -((r5 << 11) | (g6 << 5) | b5) - 1
 * Range: -1 to -65536
 * @param color  The color to encode
 * @return  The special negative index encoding this color
 */
int PackedArgb::encodeAsSpecialIndex(const PackedArgb & color)
{
	// Convert 8-bit RGB to RGB565 (5-6-5 bits)
	uint8 r5 = color.getR() >> 3;  // 8-bit to 5-bit
	uint8 g6 = color.getG() >> 2;  // 8-bit to 6-bit
	uint8 b5 = color.getB() >> 3;  // 8-bit to 5-bit

	int rgb565 = (static_cast<int>(r5) << 11) |
	             (static_cast<int>(g6) << 5) |
	             static_cast<int>(b5);
	return -(rgb565 + 1);
}

// ----------------------------------------------------------------------
/**
 * Decode a color from a special negative palette index.
 * Uses RGB565 decoding for 16-bit compatibility.
 * @param specialIndex  The special index (must be negative)
 * @return  The decoded color, or solidMagenta if not a valid special index
 */
PackedArgb PackedArgb::decodeFromSpecialIndex(int specialIndex)
{
	if (specialIndex >= 0)
		return solidMagenta;

	int rgb565 = -(specialIndex + 1);

	// Extract RGB565 components
	uint8 r5 = static_cast<uint8>((rgb565 >> 11) & 0x1F);
	uint8 g6 = static_cast<uint8>((rgb565 >> 5) & 0x3F);
	uint8 b5 = static_cast<uint8>(rgb565 & 0x1F);

	// Convert back to 8-bit (scale up with proper rounding)
	uint8 r = (r5 << 3) | (r5 >> 2);  // 5-bit to 8-bit
	uint8 g = (g6 << 2) | (g6 >> 4);  // 6-bit to 8-bit
	uint8 b = (b5 << 3) | (b5 >> 2);  // 5-bit to 8-bit

	return PackedArgb(255, r, g, b);
}

// ----------------------------------------------------------------------
/**
 * Check if an index is a special color-encoded index (negative).
 * @param index  The index to check
 * @return  true if this is a special color index
 */
bool PackedArgb::isSpecialColorIndex(int index)
{
	return index < 0;
}

// ======================================================================

