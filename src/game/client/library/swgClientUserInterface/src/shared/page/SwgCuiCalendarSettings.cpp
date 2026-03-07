//======================================================================
//
// SwgCuiCalendarSettings.cpp
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Settings Dialog (Staff Only)
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCalendarSettings.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedNetworkMessages/CalendarMessages.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UITextbox.h"
#include "Unicode.h"

//======================================================================

SwgCuiCalendarSettings::SwgCuiCalendarSettings(UIPage & page) :
CuiMediator       ("SwgCuiCalendarSettings", page),
UIEventCallback   (),
m_textTexture     (0),
m_textRectX       (0),
m_textRectY       (0),
m_textRectW       (0),
m_textRectH       (0),
m_imagePreview    (0),
m_buttonPreview   (0),
m_buttonApply     (0),
m_buttonReset     (0),
m_buttonClose     (0)
{
	getCodeDataObject(TUITextbox, m_textTexture,   "textureSection.txtTexture", false);
	getCodeDataObject(TUITextbox, m_textRectX,     "rectSection.txtRectX", false);
	getCodeDataObject(TUITextbox, m_textRectY,     "rectSection.txtRectY", false);
	getCodeDataObject(TUITextbox, m_textRectW,     "rectSection.txtRectW", false);
	getCodeDataObject(TUITextbox, m_textRectH,     "rectSection.txtRectH", false);
	getCodeDataObject(TUIImage,   m_imagePreview,  "previewSection.previewPane.imgPreview", false);
	getCodeDataObject(TUIButton,  m_buttonPreview, "rectSection.btnPreview", false);
	getCodeDataObject(TUIButton,  m_buttonApply,   "btnApply", false);
	getCodeDataObject(TUIButton,  m_buttonReset,   "btnReset", false);
	getCodeDataObject(TUIButton,  m_buttonClose,   "btnClose", false);
}

//----------------------------------------------------------------------

SwgCuiCalendarSettings::~SwgCuiCalendarSettings()
{
	m_textTexture   = 0;
	m_textRectX     = 0;
	m_textRectY     = 0;
	m_textRectW     = 0;
	m_textRectH     = 0;
	m_imagePreview  = 0;
	m_buttonPreview = 0;
	m_buttonApply   = 0;
	m_buttonReset   = 0;
	m_buttonClose   = 0;
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::performActivate()
{
	if (m_buttonPreview) m_buttonPreview->AddCallback(this);
	if (m_buttonApply)   m_buttonApply->AddCallback(this);
	if (m_buttonReset)   m_buttonReset->AddCallback(this);
	if (m_buttonClose)   m_buttonClose->AddCallback(this);

	loadSettings();
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::performDeactivate()
{
	if (m_buttonPreview) m_buttonPreview->RemoveCallback(this);
	if (m_buttonApply)   m_buttonApply->RemoveCallback(this);
	if (m_buttonReset)   m_buttonReset->RemoveCallback(this);
	if (m_buttonClose)   m_buttonClose->RemoveCallback(this);
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::OnButtonPressed(UIWidget *context)
{
	if (context == m_buttonPreview)
	{
		updatePreview();
	}
	else if (context == m_buttonApply)
	{
		applySettings();
	}
	else if (context == m_buttonReset)
	{
		resetSettings();
	}
	else if (context == m_buttonClose)
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::loadSettings()
{
	// Set defaults
	if (m_textTexture) m_textTexture->SetText(Unicode::narrowToWide("ui_calendar_bg.dds"));
	if (m_textRectX)   m_textRectX->SetText(Unicode::narrowToWide("0"));
	if (m_textRectY)   m_textRectY->SetText(Unicode::narrowToWide("0"));
	if (m_textRectW)   m_textRectW->SetText(Unicode::narrowToWide("512"));
	if (m_textRectH)   m_textRectH->SetText(Unicode::narrowToWide("512"));
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::applySettings()
{
	// Gather values
	Unicode::String textureW, rectXW, rectYW, rectWW, rectHW;

	if (m_textTexture) m_textTexture->GetText(textureW);
	if (m_textRectX)   m_textRectX->GetText(rectXW);
	if (m_textRectY)   m_textRectY->GetText(rectYW);
	if (m_textRectW)   m_textRectW->GetText(rectWW);
	if (m_textRectH)   m_textRectH->GetText(rectHW);

	std::string texture = Unicode::wideToNarrow(textureW);
	int srcX = atoi(Unicode::wideToNarrow(rectXW).c_str());
	int srcY = atoi(Unicode::wideToNarrow(rectYW).c_str());
	int srcW = atoi(Unicode::wideToNarrow(rectWW).c_str());
	int srcH = atoi(Unicode::wideToNarrow(rectHW).c_str());

	// Send settings message to server
	if (GameNetwork::isConnectedToConnectionServer())
	{
		CalendarApplySettingsMessage msg(texture, srcX, srcY, srcW, srcH);
		GameNetwork::send(msg, true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::resetSettings()
{
	// Reset UI to defaults
	if (m_textTexture) m_textTexture->SetText(Unicode::narrowToWide("ui_calendar_bg.dds"));
	if (m_textRectX)   m_textRectX->SetText(Unicode::narrowToWide("0"));
	if (m_textRectY)   m_textRectY->SetText(Unicode::narrowToWide("0"));
	if (m_textRectW)   m_textRectW->SetText(Unicode::narrowToWide("512"));
	if (m_textRectH)   m_textRectH->SetText(Unicode::narrowToWide("512"));

	// Send reset (apply defaults) to server
	if (GameNetwork::isConnectedToConnectionServer())
	{
		CalendarApplySettingsMessage msg("ui_calendar_bg.dds", 0, 0, 512, 512);
		GameNetwork::send(msg, true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarSettings::updatePreview()
{
	// Preview is handled client-side only
}

//======================================================================
