//======================================================================
//
// CuiColorWheel.h
// copyright (c) 2026 Titan Development Team
//
// Color wheel widget for direct RGB color selection with HTML hex input.
// Provides HSV-based color wheel with saturation/value square.
//
//======================================================================

#ifndef INCLUDED_CuiColorWheel_H
#define INCLUDED_CuiColorWheel_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMath/PackedArgb.h"

class UIButton;
class UIImage;
class UIPage;
class UISliderbar;
class UIText;
class UITextbox;
class UIWidget;

// ======================================================================

/**
 * CuiColorWheel - A color wheel widget for selecting colors.
 *
 * Features:
 * - HSV color wheel with hue selection around perimeter
 * - Saturation/Value selection in center square
 * - HTML hex color input (#RRGGBB format)
 * - RGB component sliders
 * - Recent colors strip (8 slots)
 * - Preview of current and matched palette colors
 */
class CuiColorWheel :
	public CuiMediator,
	public UIEventCallback
{
public:

	typedef void (*ColorChangedCallback)(const PackedArgb &color, void *context);

	struct Messages
	{
		static const char * const ColorChanged;
	};

	explicit                      CuiColorWheel(UIPage & page);
	virtual                      ~CuiColorWheel();

	virtual void                  performActivate();
	virtual void                  performDeactivate();

	virtual void                  OnButtonPressed(UIWidget * context);
	virtual void                  OnSliderbarChanged(UIWidget * context);
	virtual void                  OnTextboxChanged(UIWidget * context);
	virtual bool                  OnMessage(UIWidget * context, const UIMessage & msg);

	// Color access
	void                          setColor(const PackedArgb & color);
	const PackedArgb &            getColor() const;

	// HTML color support
	void                          setColorFromHtml(const char * htmlColor);
	void                          getColorAsHtml(char * buffer, int bufferSize) const;

	// HSV support
	void                          setColorHSV(float hue, float saturation, float value);
	void                          getColorHSV(float & hue, float & saturation, float & value) const;

	// Callback
	void                          setColorChangedCallback(ColorChangedCallback callback, void * context);

	// Recent colors
	void                          addToRecentColors(const PackedArgb & color);
	static void                   clearRecentColors();

	// Factory
	static CuiColorWheel *        createInto(UIPage & parent);

private:
	                              CuiColorWheel();
	                              CuiColorWheel(const CuiColorWheel & rhs);
	CuiColorWheel &               operator=(const CuiColorWheel & rhs);

	void                          updateFromHSV();
	void                          updateFromRGB();
	void                          updateUIFromColor();
	void                          updateWheelCursor();
	void                          updateSVCursor();
	void                          notifyColorChanged();

	// Color conversion helpers
	static void                   rgbToHsv(uint8 r, uint8 g, uint8 b, float & h, float & s, float & v);
	static void                   hsvToRgb(float h, float s, float v, uint8 & r, uint8 & g, uint8 & b);

	// Handle mouse input on wheel/square
	void                          handleWheelInput(int x, int y);
	void                          handleSVInput(int x, int y);

private:

	// Current color state
	PackedArgb                    m_currentColor;
	float                         m_hue;         // 0.0 - 360.0
	float                         m_saturation;  // 0.0 - 1.0
	float                         m_value;       // 0.0 - 1.0

	// UI elements
	UIPage *                      m_pageWheel;         // Color wheel image/area
	UIPage *                      m_pageSVSquare;      // Saturation/Value selection square
	UIWidget *                    m_cursorWheel;       // Cursor on the wheel
	UIWidget *                    m_cursorSV;          // Cursor on SV square
	UIPage *                      m_pagePreview;       // Color preview swatch
	UITextbox *                   m_textboxHtml;       // HTML hex input
	UISliderbar *                 m_sliderR;           // Red slider
	UISliderbar *                 m_sliderG;           // Green slider
	UISliderbar *                 m_sliderB;           // Blue slider
	UIText *                      m_textR;             // Red value text
	UIText *                      m_textG;             // Green value text
	UIText *                      m_textB;             // Blue value text
	UIPage *                      m_pageRecentColors;  // Recent colors strip

	// Recent color buttons (up to 8)
	static const int              MaxRecentColors = 8;
	UIButton *                    m_recentColorButtons[MaxRecentColors];

	// Static recent colors (shared across all instances)
	static PackedArgb             s_recentColors[MaxRecentColors];
	static int                    s_recentColorCount;

	// Callback
	ColorChangedCallback          m_colorChangedCallback;
	void *                        m_callbackContext;

	// Input state
	bool                          m_draggingWheel;
	bool                          m_draggingSV;
};

//======================================================================

#endif


