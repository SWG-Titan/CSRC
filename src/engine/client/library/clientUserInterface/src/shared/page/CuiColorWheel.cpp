//======================================================================
//
// CuiColorWheel.cpp
// copyright (c) 2026 Titan Development Team
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiColorWheel.h"

#include "UIButton.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

#include <cmath>
#include <cstdio>
#include <algorithm>

//======================================================================

namespace CuiColorWheelNamespace
{
	const float PI = 3.14159265358979323846f;
	const float TWO_PI = 2.0f * PI;
}

using namespace CuiColorWheelNamespace;

//======================================================================

const char * const CuiColorWheel::Messages::ColorChanged = "CuiColorWheel::ColorChanged";

// Static recent colors storage
PackedArgb CuiColorWheel::s_recentColors[MaxRecentColors];
int CuiColorWheel::s_recentColorCount = 0;

//======================================================================

CuiColorWheel::CuiColorWheel(UIPage & page) :
CuiMediator("CuiColorWheel", page),
UIEventCallback(),
m_currentColor(PackedArgb::solidWhite),
m_hue(0.0f),
m_saturation(0.0f),
m_value(1.0f),
m_pageWheel(nullptr),
m_pageSVSquare(nullptr),
m_cursorWheel(nullptr),
m_cursorSV(nullptr),
m_pagePreview(nullptr),
m_textboxHtml(nullptr),
m_sliderR(nullptr),
m_sliderG(nullptr),
m_sliderB(nullptr),
m_textR(nullptr),
m_textG(nullptr),
m_textB(nullptr),
m_pageRecentColors(nullptr),
m_colorChangedCallback(nullptr),
m_callbackContext(nullptr),
m_draggingWheel(false),
m_draggingSV(false)
{
	// Get UI elements - these are optional, will work with what's available
	getCodeDataObject(TUIPage, m_pageWheel, "pageWheel", true);
	getCodeDataObject(TUIPage, m_pageSVSquare, "pageSVSquare", true);
	getCodeDataObject(TUIWidget, m_cursorWheel, "cursorWheel", true);
	getCodeDataObject(TUIWidget, m_cursorSV, "cursorSV", true);
	getCodeDataObject(TUIPage, m_pagePreview, "pagePreview", true);
	getCodeDataObject(TUITextbox, m_textboxHtml, "textboxHtml", true);
	getCodeDataObject(TUISliderbar, m_sliderR, "sliderR", true);
	getCodeDataObject(TUISliderbar, m_sliderG, "sliderG", true);
	getCodeDataObject(TUISliderbar, m_sliderB, "sliderB", true);
	getCodeDataObject(TUIText, m_textR, "textR", true);
	getCodeDataObject(TUIText, m_textG, "textG", true);
	getCodeDataObject(TUIText, m_textB, "textB", true);
	getCodeDataObject(TUIPage, m_pageRecentColors, "pageRecentColors", true);

	// Initialize recent color buttons
	for (int i = 0; i < MaxRecentColors; ++i)
	{
		m_recentColorButtons[i] = nullptr;
		char buttonName[32];
		snprintf(buttonName, sizeof(buttonName), "buttonRecent%d", i);
		getCodeDataObject(TUIButton, m_recentColorButtons[i], buttonName, true);
	}

	// Configure sliders
	if (m_sliderR)
	{
		m_sliderR->SetLowerLimit(0);
		m_sliderR->SetUpperLimit(255);
	}
	if (m_sliderG)
	{
		m_sliderG->SetLowerLimit(0);
		m_sliderG->SetUpperLimit(255);
	}
	if (m_sliderB)
	{
		m_sliderB->SetLowerLimit(0);
		m_sliderB->SetUpperLimit(255);
	}

	setState(MS_closeable);
}

//----------------------------------------------------------------------

CuiColorWheel::~CuiColorWheel()
{
	m_pageWheel = nullptr;
	m_pageSVSquare = nullptr;
	m_cursorWheel = nullptr;
	m_cursorSV = nullptr;
	m_pagePreview = nullptr;
	m_textboxHtml = nullptr;
	m_sliderR = nullptr;
	m_sliderG = nullptr;
	m_sliderB = nullptr;
	m_textR = nullptr;
	m_textG = nullptr;
	m_textB = nullptr;
	m_pageRecentColors = nullptr;

	for (int i = 0; i < MaxRecentColors; ++i)
		m_recentColorButtons[i] = nullptr;

	m_colorChangedCallback = nullptr;
	m_callbackContext = nullptr;
}

//----------------------------------------------------------------------

void CuiColorWheel::performActivate()
{
	// Register callbacks
	if (m_textboxHtml)
		m_textboxHtml->AddCallback(this);
	if (m_sliderR)
		m_sliderR->AddCallback(this);
	if (m_sliderG)
		m_sliderG->AddCallback(this);
	if (m_sliderB)
		m_sliderB->AddCallback(this);

	for (int i = 0; i < MaxRecentColors; ++i)
	{
		if (m_recentColorButtons[i])
			m_recentColorButtons[i]->AddCallback(this);
	}

	// Update UI to reflect current color
	updateUIFromColor();

	setIsUpdating(true);
}

//----------------------------------------------------------------------

void CuiColorWheel::performDeactivate()
{
	// Unregister callbacks
	if (m_textboxHtml)
		m_textboxHtml->RemoveCallback(this);
	if (m_sliderR)
		m_sliderR->RemoveCallback(this);
	if (m_sliderG)
		m_sliderG->RemoveCallback(this);
	if (m_sliderB)
		m_sliderB->RemoveCallback(this);

	for (int i = 0; i < MaxRecentColors; ++i)
	{
		if (m_recentColorButtons[i])
			m_recentColorButtons[i]->RemoveCallback(this);
	}

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void CuiColorWheel::OnButtonPressed(UIWidget * context)
{
	// Check if it's a recent color button
	for (int i = 0; i < MaxRecentColors && i < s_recentColorCount; ++i)
	{
		if (context == m_recentColorButtons[i])
		{
			setColor(s_recentColors[i]);
			return;
		}
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::OnSliderbarChanged(UIWidget * context)
{
	if (context == m_sliderR || context == m_sliderG || context == m_sliderB)
	{
		// Get RGB values from sliders
		uint8 r = m_sliderR ? static_cast<uint8>(m_sliderR->GetValue()) : m_currentColor.getR();
		uint8 g = m_sliderG ? static_cast<uint8>(m_sliderG->GetValue()) : m_currentColor.getG();
		uint8 b = m_sliderB ? static_cast<uint8>(m_sliderB->GetValue()) : m_currentColor.getB();

		m_currentColor.setArgb(255, r, g, b);
		rgbToHsv(r, g, b, m_hue, m_saturation, m_value);

		updateUIFromColor();
		notifyColorChanged();
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::OnTextboxChanged(UIWidget * context)
{
	if (context == m_textboxHtml)
	{
		Unicode::String text;
		m_textboxHtml->GetLocalText(text);
		std::string htmlColor = Unicode::wideToNarrow(text);

		if (PackedArgb::isValidHtmlColor(htmlColor.c_str()))
		{
			PackedArgb color = PackedArgb::fromHtmlString(htmlColor.c_str());
			m_currentColor = color;
			rgbToHsv(color.getR(), color.getG(), color.getB(), m_hue, m_saturation, m_value);

			// Update everything except the textbox itself
			if (m_sliderR)
				m_sliderR->SetValue(color.getR(), false);
			if (m_sliderG)
				m_sliderG->SetValue(color.getG(), false);
			if (m_sliderB)
				m_sliderB->SetValue(color.getB(), false);

			updateWheelCursor();
			updateSVCursor();

			if (m_pagePreview)
				m_pagePreview->SetBackgroundTint(UIColor(color.getR(), color.getG(), color.getB()));

			notifyColorChanged();
		}
	}
}

//----------------------------------------------------------------------

bool CuiColorWheel::OnMessage(UIWidget * context, const UIMessage & msg)
{
	// Handle mouse input on the color wheel
	if (context == m_pageWheel)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			m_draggingWheel = true;
			handleWheelInput(msg.MouseCoords.x, msg.MouseCoords.y);
			return false;
		}
		else if (msg.Type == UIMessage::LeftMouseUp)
		{
			if (m_draggingWheel)
			{
				m_draggingWheel = false;
				addToRecentColors(m_currentColor);
			}
			return false;
		}
		else if (msg.Type == UIMessage::MouseMove && m_draggingWheel)
		{
			handleWheelInput(msg.MouseCoords.x, msg.MouseCoords.y);
			return false;
		}
	}

	// Handle mouse input on the SV square
	if (context == m_pageSVSquare)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			m_draggingSV = true;
			handleSVInput(msg.MouseCoords.x, msg.MouseCoords.y);
			return false;
		}
		else if (msg.Type == UIMessage::LeftMouseUp)
		{
			if (m_draggingSV)
			{
				m_draggingSV = false;
				addToRecentColors(m_currentColor);
			}
			return false;
		}
		else if (msg.Type == UIMessage::MouseMove && m_draggingSV)
		{
			handleSVInput(msg.MouseCoords.x, msg.MouseCoords.y);
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void CuiColorWheel::setColor(const PackedArgb & color)
{
	m_currentColor = color;
	rgbToHsv(color.getR(), color.getG(), color.getB(), m_hue, m_saturation, m_value);
	updateUIFromColor();
	notifyColorChanged();
}

//----------------------------------------------------------------------

const PackedArgb & CuiColorWheel::getColor() const
{
	return m_currentColor;
}

//----------------------------------------------------------------------

void CuiColorWheel::setColorFromHtml(const char * htmlColor)
{
	if (PackedArgb::isValidHtmlColor(htmlColor))
	{
		setColor(PackedArgb::fromHtmlString(htmlColor));
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::getColorAsHtml(char * buffer, int bufferSize) const
{
	m_currentColor.toHtmlString(buffer, bufferSize, false);
}

//----------------------------------------------------------------------

void CuiColorWheel::setColorHSV(float hue, float saturation, float value)
{
	m_hue = std::max(0.0f, std::min(360.0f, hue));
	m_saturation = std::max(0.0f, std::min(1.0f, saturation));
	m_value = std::max(0.0f, std::min(1.0f, value));

	updateFromHSV();
}

//----------------------------------------------------------------------

void CuiColorWheel::getColorHSV(float & hue, float & saturation, float & value) const
{
	hue = m_hue;
	saturation = m_saturation;
	value = m_value;
}

//----------------------------------------------------------------------

void CuiColorWheel::setColorChangedCallback(ColorChangedCallback callback, void * context)
{
	m_colorChangedCallback = callback;
	m_callbackContext = context;
}

//----------------------------------------------------------------------

void CuiColorWheel::addToRecentColors(const PackedArgb & color)
{
	// Check if color already exists in recent colors
	for (int i = 0; i < s_recentColorCount; ++i)
	{
		if (s_recentColors[i] == color)
		{
			// Move to front
			for (int j = i; j > 0; --j)
				s_recentColors[j] = s_recentColors[j - 1];
			s_recentColors[0] = color;
			return;
		}
	}

	// Shift existing colors
	int count = std::min(s_recentColorCount, MaxRecentColors - 1);
	for (int i = count; i > 0; --i)
		s_recentColors[i] = s_recentColors[i - 1];

	// Add new color at front
	s_recentColors[0] = color;
	if (s_recentColorCount < MaxRecentColors)
		++s_recentColorCount;
}

//----------------------------------------------------------------------

void CuiColorWheel::clearRecentColors()
{
	s_recentColorCount = 0;
}

//----------------------------------------------------------------------

void CuiColorWheel::updateFromHSV()
{
	uint8 r, g, b;
	hsvToRgb(m_hue, m_saturation, m_value, r, g, b);
	m_currentColor.setArgb(255, r, g, b);
	updateUIFromColor();
	notifyColorChanged();
}

//----------------------------------------------------------------------

void CuiColorWheel::updateFromRGB()
{
	rgbToHsv(m_currentColor.getR(), m_currentColor.getG(), m_currentColor.getB(),
	         m_hue, m_saturation, m_value);
	updateUIFromColor();
	notifyColorChanged();
}

//----------------------------------------------------------------------

void CuiColorWheel::updateUIFromColor()
{
	// Update sliders
	if (m_sliderR)
		m_sliderR->SetValue(m_currentColor.getR(), false);
	if (m_sliderG)
		m_sliderG->SetValue(m_currentColor.getG(), false);
	if (m_sliderB)
		m_sliderB->SetValue(m_currentColor.getB(), false);

	// Update text displays
	if (m_textR)
	{
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", m_currentColor.getR());
		m_textR->SetLocalText(Unicode::narrowToWide(buf));
	}
	if (m_textG)
	{
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", m_currentColor.getG());
		m_textG->SetLocalText(Unicode::narrowToWide(buf));
	}
	if (m_textB)
	{
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", m_currentColor.getB());
		m_textB->SetLocalText(Unicode::narrowToWide(buf));
	}

	// Update HTML textbox
	if (m_textboxHtml)
	{
		char htmlColor[16];
		m_currentColor.toHtmlString(htmlColor, sizeof(htmlColor), false);
		m_textboxHtml->SetLocalText(Unicode::narrowToWide(htmlColor));
	}

	// Update preview
	if (m_pagePreview)
	{
		m_pagePreview->SetBackgroundTint(UIColor(m_currentColor.getR(), m_currentColor.getG(), m_currentColor.getB()));
	}

	// Update cursors
	updateWheelCursor();
	updateSVCursor();

	// Update recent color buttons
	for (int i = 0; i < MaxRecentColors; ++i)
	{
		if (m_recentColorButtons[i])
		{
			if (i < s_recentColorCount)
			{
				m_recentColorButtons[i]->SetVisible(true);
				m_recentColorButtons[i]->SetBackgroundTint(UIColor(
					s_recentColors[i].getR(),
					s_recentColors[i].getG(),
					s_recentColors[i].getB()));
			}
			else
			{
				m_recentColorButtons[i]->SetVisible(false);
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::updateWheelCursor()
{
	if (!m_cursorWheel || !m_pageWheel)
		return;

	UISize wheelSize = m_pageWheel->GetSize();
	float centerX = wheelSize.x / 2.0f;
	float centerY = wheelSize.y / 2.0f;
	float radius = std::min(centerX, centerY) * 0.85f;  // 85% of half-size

	// Convert hue to angle (0 = right, counter-clockwise)
	float angle = m_hue * PI / 180.0f;

	int cursorX = static_cast<int>(centerX + radius * cosf(angle));
	int cursorY = static_cast<int>(centerY - radius * sinf(angle));  // Y is inverted

	UISize cursorSize = m_cursorWheel->GetSize();
	m_cursorWheel->SetLocation(cursorX - cursorSize.x / 2, cursorY - cursorSize.y / 2);
}

//----------------------------------------------------------------------

void CuiColorWheel::updateSVCursor()
{
	if (!m_cursorSV || !m_pageSVSquare)
		return;

	UISize svSize = m_pageSVSquare->GetSize();

	// Saturation goes left to right, Value goes bottom to top
	int cursorX = static_cast<int>(m_saturation * svSize.x);
	int cursorY = static_cast<int>((1.0f - m_value) * svSize.y);

	UISize cursorSize = m_cursorSV->GetSize();
	m_cursorSV->SetLocation(cursorX - cursorSize.x / 2, cursorY - cursorSize.y / 2);
}

//----------------------------------------------------------------------

void CuiColorWheel::notifyColorChanged()
{
	if (m_colorChangedCallback)
	{
		m_colorChangedCallback(m_currentColor, m_callbackContext);
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::rgbToHsv(uint8 r, uint8 g, uint8 b, float & h, float & s, float & v)
{
	float rf = r / 255.0f;
	float gf = g / 255.0f;
	float bf = b / 255.0f;

	float maxC = std::max(rf, std::max(gf, bf));
	float minC = std::min(rf, std::min(gf, bf));
	float delta = maxC - minC;

	v = maxC;

	if (maxC > 0.0f)
		s = delta / maxC;
	else
		s = 0.0f;

	if (delta < 0.00001f)
	{
		h = 0.0f;
	}
	else
	{
		if (rf >= maxC)
			h = (gf - bf) / delta;
		else if (gf >= maxC)
			h = 2.0f + (bf - rf) / delta;
		else
			h = 4.0f + (rf - gf) / delta;

		h *= 60.0f;
		if (h < 0.0f)
			h += 360.0f;
	}
}

//----------------------------------------------------------------------

void CuiColorWheel::hsvToRgb(float h, float s, float v, uint8 & r, uint8 & g, uint8 & b)
{
	if (s <= 0.0f)
	{
		r = g = b = static_cast<uint8>(v * 255.0f);
		return;
	}

	float hh = h;
	if (hh >= 360.0f)
		hh = 0.0f;
	hh /= 60.0f;

	int i = static_cast<int>(hh);
	float ff = hh - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - (s * ff));
	float t = v * (1.0f - (s * (1.0f - ff)));

	float rf, gf, bf;
	switch (i)
	{
	case 0:
		rf = v; gf = t; bf = p;
		break;
	case 1:
		rf = q; gf = v; bf = p;
		break;
	case 2:
		rf = p; gf = v; bf = t;
		break;
	case 3:
		rf = p; gf = q; bf = v;
		break;
	case 4:
		rf = t; gf = p; bf = v;
		break;
	default:
		rf = v; gf = p; bf = q;
		break;
	}

	r = static_cast<uint8>(rf * 255.0f);
	g = static_cast<uint8>(gf * 255.0f);
	b = static_cast<uint8>(bf * 255.0f);
}

//----------------------------------------------------------------------

void CuiColorWheel::handleWheelInput(int x, int y)
{
	if (!m_pageWheel)
		return;

	UISize wheelSize = m_pageWheel->GetSize();
	float centerX = wheelSize.x / 2.0f;
	float centerY = wheelSize.y / 2.0f;

	// Calculate angle from center
	float dx = x - centerX;
	float dy = centerY - y;  // Y is inverted

	float angle = atan2f(dy, dx);
	if (angle < 0.0f)
		angle += TWO_PI;

	m_hue = angle * 180.0f / PI;

	updateFromHSV();
}

//----------------------------------------------------------------------

void CuiColorWheel::handleSVInput(int x, int y)
{
	if (!m_pageSVSquare)
		return;

	UISize svSize = m_pageSVSquare->GetSize();

	// Clamp to bounds
	x = std::max(0, std::min(x, static_cast<int>(svSize.x)));
	y = std::max(0, std::min(y, static_cast<int>(svSize.y)));

	// Saturation goes left to right, Value goes bottom to top
	m_saturation = static_cast<float>(x) / svSize.x;
	m_value = 1.0f - static_cast<float>(y) / svSize.y;

	updateFromHSV();
}

//----------------------------------------------------------------------

CuiColorWheel * CuiColorWheel::createInto(UIPage & parent)
{
	// This would typically load from a UI definition file
	// For now, return nullptr - the UI page needs to be set up in the UI data
	UNREF(parent);
	return nullptr;
}

//======================================================================


