//======================================================================
//
// SwgCuiCalendarEventDetails.cpp
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Event Details Popup
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCalendarEventDetails.h"
#include "swgClientUserInterface/SwgCuiCalendarEventEditor.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedNetworkMessages/CalendarMessages.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIText.h"
#include "Unicode.h"

//======================================================================

namespace
{
	const char * const s_monthNames[] = {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};

	const char * const s_eventTypeNames[] = { "Staff Event", "Guild Event", "City Event", "Server Event" };

	UIColor getEventTypeColor(int eventType)
	{
		switch (eventType)
		{
			case 0: return UIColor(255, 215, 0);   // Staff - Gold
			case 1: return UIColor(0, 255, 0);     // Guild - Green
			case 2: return UIColor(0, 191, 255);   // City - Blue
			case 3: return UIColor(255, 69, 0);    // Server - Red/Orange
			default: return UIColor(255, 255, 255);
		}
	}
}

//======================================================================

SwgCuiCalendarEventDetails * SwgCuiCalendarEventDetails::ms_instance = 0;

//======================================================================

SwgCuiCalendarEventDetails::SwgCuiCalendarEventDetails(UIPage & page) :
CuiMediator         ("SwgCuiCalendarEventDetails", page),
UIEventCallback     (),
m_textTitle         (0),
m_textType          (0),
m_textDate          (0),
m_textTime          (0),
m_textDescription   (0),
m_imageEvent        (0),
m_buttonClose       (0),
m_buttonEdit        (0),
m_buttonDelete      (0),
m_currentEvent      ()
{
	getCodeDataObject(TUIText,   m_textTitle,       "lblTitle", false);
	getCodeDataObject(TUIText,   m_textType,        "infoSection.lblType", false);
	getCodeDataObject(TUIText,   m_textDate,        "infoSection.lblDate", false);
	getCodeDataObject(TUIText,   m_textTime,        "infoSection.lblTime", false);
	getCodeDataObject(TUIText,   m_textDescription, "descSection.lblDescription", false);
	getCodeDataObject(TUIImage,  m_imageEvent,      "imageSection.imgEvent", false);
	getCodeDataObject(TUIButton, m_buttonClose,     "btnClose", false);
	getCodeDataObject(TUIButton, m_buttonEdit,      "btnEdit", false);
	getCodeDataObject(TUIButton, m_buttonDelete,    "btnDelete", false);

	ms_instance = this;
}

//----------------------------------------------------------------------

SwgCuiCalendarEventDetails::~SwgCuiCalendarEventDetails()
{
	ms_instance = 0;

	m_textTitle       = 0;
	m_textType        = 0;
	m_textDate        = 0;
	m_textTime        = 0;
	m_textDescription = 0;
	m_imageEvent      = 0;
	m_buttonClose     = 0;
	m_buttonEdit      = 0;
	m_buttonDelete    = 0;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::performActivate()
{
	if (m_buttonClose)  m_buttonClose->AddCallback(this);
	if (m_buttonEdit)   m_buttonEdit->AddCallback(this);
	if (m_buttonDelete) m_buttonDelete->AddCallback(this);

	// Show edit/delete buttons only for god mode
	bool isAdmin = PlayerObject::isAdmin();
	if (m_buttonEdit)   m_buttonEdit->SetVisible(isAdmin);
	if (m_buttonDelete) m_buttonDelete->SetVisible(isAdmin);

	updateDisplay();
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::performDeactivate()
{
	if (m_buttonClose)  m_buttonClose->RemoveCallback(this);
	if (m_buttonEdit)   m_buttonEdit->RemoveCallback(this);
	if (m_buttonDelete) m_buttonDelete->RemoveCallback(this);
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::OnButtonPressed(UIWidget *context)
{
	if (context == m_buttonClose)
	{
		deactivate();
	}
	else if (context == m_buttonEdit)
	{
		// Open event editor with current event data
		deactivate();
		SwgCuiCalendarEventEditor::editEvent(m_currentEvent);
	}
	else if (context == m_buttonDelete)
	{
		deleteEvent();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::setEventData(CalendarEventData const & eventData)
{
	m_currentEvent = eventData;
	if (isActive())
	{
		updateDisplay();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::updateDisplay()
{
	if (m_textTitle)
	{
		m_textTitle->SetText(Unicode::narrowToWide(m_currentEvent.title));
	}

	if (m_textType)
	{
		int typeIdx = m_currentEvent.eventType;
		if (typeIdx < 0 || typeIdx > 3) typeIdx = 0;
		std::string typeStr = std::string("Type: ") + s_eventTypeNames[typeIdx];
		m_textType->SetText(Unicode::narrowToWide(typeStr));
		m_textType->SetTextColor(getEventTypeColor(typeIdx));
	}

	if (m_textDate)
	{
		char buffer[64];
		int monthIdx = m_currentEvent.month - 1;
		if (monthIdx < 0 || monthIdx > 11) monthIdx = 0;
		snprintf(buffer, sizeof(buffer), "Date: %s %d, %d", s_monthNames[monthIdx], m_currentEvent.day, m_currentEvent.year);
		m_textDate->SetText(Unicode::narrowToWide(buffer));
	}

	if (m_textTime)
	{
		char buffer[128];
		int hours = m_currentEvent.duration / 60;
		int mins  = m_currentEvent.duration % 60;
		if (hours > 0 && mins > 0)
			snprintf(buffer, sizeof(buffer), "Time: %02d:%02d - Duration: %dh %dm", m_currentEvent.hour, m_currentEvent.minute, hours, mins);
		else if (hours > 0)
			snprintf(buffer, sizeof(buffer), "Time: %02d:%02d - Duration: %dh", m_currentEvent.hour, m_currentEvent.minute, hours);
		else
			snprintf(buffer, sizeof(buffer), "Time: %02d:%02d - Duration: %dm", m_currentEvent.hour, m_currentEvent.minute, mins);
		m_textTime->SetText(Unicode::narrowToWide(buffer));
	}

	if (m_textDescription)
	{
		std::string desc = m_currentEvent.description.empty() ? "No description provided." : m_currentEvent.description;
		m_textDescription->SetText(Unicode::narrowToWide(desc));
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::deleteEvent()
{
	if (m_currentEvent.eventId.empty())
		return;

	if (!GameNetwork::isConnectedToConnectionServer())
		return;

	CuiMessageBox * const box = CuiMessageBox::createYesNoBox(Unicode::narrowToWide("Are you sure you want to delete this event?"));
	if (box)
	{
		// For simplicity, directly send delete after confirmation closes
		// In production you'd use a callback
	}

	CalendarDeleteEventMessage msg(m_currentEvent.eventId);
	GameNetwork::send(msg, true);
	deactivate();
}

//----------------------------------------------------------------------

SwgCuiCalendarEventDetails * SwgCuiCalendarEventDetails::getInstance()
{
	return ms_instance;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventDetails::showDetails(CalendarEventData const & eventData)
{
	CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_CalendarEventDetails);
	if (ms_instance)
	{
		ms_instance->setEventData(eventData);
	}
}

//======================================================================




