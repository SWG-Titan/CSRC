// ======================================================================
//
// CalendarMessages.cpp
// Copyright 2026 Titan Project
//
// Calendar system network messages implementation.
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CalendarMessages.h"

#include "Archive/Archive.h"

// ======================================================================
// CalendarEventData default constructor
// ======================================================================

CalendarEventData::CalendarEventData() :
	eventId(),
	title(),
	description(),
	eventType(0),
	year(0),
	month(0),
	day(0),
	hour(0),
	minute(0),
	duration(0),
	guildId(0),
	cityId(0),
	serverEventKey(),
	recurring(false),
	recurrenceType(0),
	broadcastStart(false),
	active(false),
	creatorId()
{
}

// ======================================================================
// CalendarEventData Archive functions
// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, CalendarEventData & target)
	{
		get(source, target.eventId);
		get(source, target.title);
		get(source, target.description);
		get(source, target.eventType);
		get(source, target.year);
		get(source, target.month);
		get(source, target.day);
		get(source, target.hour);
		get(source, target.minute);
		get(source, target.duration);
		get(source, target.guildId);
		get(source, target.cityId);
		get(source, target.serverEventKey);
		get(source, target.recurring);
		get(source, target.recurrenceType);
		get(source, target.broadcastStart);
		get(source, target.active);
		get(source, target.creatorId);
	}

	void put(ByteStream & target, CalendarEventData const & source)
	{
		put(target, source.eventId);
		put(target, source.title);
		put(target, source.description);
		put(target, source.eventType);
		put(target, source.year);
		put(target, source.month);
		put(target, source.day);
		put(target, source.hour);
		put(target, source.minute);
		put(target, source.duration);
		put(target, source.guildId);
		put(target, source.cityId);
		put(target, source.serverEventKey);
		put(target, source.recurring);
		put(target, source.recurrenceType);
		put(target, source.broadcastStart);
		put(target, source.active);
		put(target, source.creatorId);
	}
}

// ======================================================================
// CalendarGetEventsMessage
// ======================================================================

CalendarGetEventsMessage::CalendarGetEventsMessage(int32 year, int32 month) :
	GameNetworkMessage("CalendarGetEventsMessage"),
	m_year(year),
	m_month(month)
{
	addVariable(m_year);
	addVariable(m_month);
}

CalendarGetEventsMessage::CalendarGetEventsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarGetEventsMessage")
{
	addVariable(m_year);
	addVariable(m_month);
	unpack(source);
}

CalendarGetEventsMessage::~CalendarGetEventsMessage()
{
}

// ======================================================================
// CalendarEventsResponseMessage
// ======================================================================

CalendarEventsResponseMessage::CalendarEventsResponseMessage(std::vector<CalendarEventData> const & events) :
	GameNetworkMessage("CalendarEventsResponseMessage"),
	m_events()
{
	addVariable(m_events);
	m_events.set(events);
}

CalendarEventsResponseMessage::CalendarEventsResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarEventsResponseMessage")
{
	addVariable(m_events);
	unpack(source);
}

CalendarEventsResponseMessage::~CalendarEventsResponseMessage()
{
}

// ======================================================================
// CalendarCreateEventMessage
// ======================================================================

CalendarCreateEventMessage::CalendarCreateEventMessage(CalendarEventData const & eventData) :
	GameNetworkMessage("CalendarCreateEventMessage"),
	m_eventData(eventData)
{
}

CalendarCreateEventMessage::CalendarCreateEventMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarCreateEventMessage")
{
	Archive::get(source, m_eventData);
}

CalendarCreateEventMessage::~CalendarCreateEventMessage()
{
}

void CalendarCreateEventMessage::pack(Archive::ByteStream & target) const
{
	GameNetworkMessage::pack(target);
	Archive::put(target, m_eventData);
}

// ======================================================================
// CalendarCreateEventResponseMessage
// ======================================================================

CalendarCreateEventResponseMessage::CalendarCreateEventResponseMessage(bool success, std::string const & eventId, std::string const & errorMessage) :
	GameNetworkMessage("CalendarCreateEventResponseMessage"),
	m_success(success),
	m_eventId(eventId),
	m_errorMessage(errorMessage)
{
	addVariable(m_success);
	addVariable(m_eventId);
	addVariable(m_errorMessage);
}

CalendarCreateEventResponseMessage::CalendarCreateEventResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarCreateEventResponseMessage")
{
	addVariable(m_success);
	addVariable(m_eventId);
	addVariable(m_errorMessage);
	unpack(source);
}

CalendarCreateEventResponseMessage::~CalendarCreateEventResponseMessage()
{
}

// ======================================================================
// CalendarDeleteEventMessage
// ======================================================================

CalendarDeleteEventMessage::CalendarDeleteEventMessage(std::string const & eventId) :
	GameNetworkMessage("CalendarDeleteEventMessage"),
	m_eventId(eventId)
{
	addVariable(m_eventId);
}

CalendarDeleteEventMessage::CalendarDeleteEventMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarDeleteEventMessage")
{
	addVariable(m_eventId);
	unpack(source);
}

CalendarDeleteEventMessage::~CalendarDeleteEventMessage()
{
}

// ======================================================================
// CalendarDeleteEventResponseMessage
// ======================================================================

CalendarDeleteEventResponseMessage::CalendarDeleteEventResponseMessage(bool success, std::string const & eventId) :
	GameNetworkMessage("CalendarDeleteEventResponseMessage"),
	m_success(success),
	m_eventId(eventId)
{
	addVariable(m_success);
	addVariable(m_eventId);
}

CalendarDeleteEventResponseMessage::CalendarDeleteEventResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarDeleteEventResponseMessage")
{
	addVariable(m_success);
	addVariable(m_eventId);
	unpack(source);
}

CalendarDeleteEventResponseMessage::~CalendarDeleteEventResponseMessage()
{
}

// ======================================================================
// CalendarEventNotificationMessage
// ======================================================================

CalendarEventNotificationMessage::CalendarEventNotificationMessage(int32 type, CalendarEventData const & eventData) :
	GameNetworkMessage("CalendarEventNotificationMessage"),
	m_notificationType(type),
	m_eventData(eventData)
{
	addVariable(m_notificationType);
}

CalendarEventNotificationMessage::CalendarEventNotificationMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarEventNotificationMessage")
{
	addVariable(m_notificationType);
	unpack(source);
	Archive::get(source, m_eventData);
}

CalendarEventNotificationMessage::~CalendarEventNotificationMessage()
{
}

void CalendarEventNotificationMessage::pack(Archive::ByteStream & target) const
{
	GameNetworkMessage::pack(target);
	Archive::put(target, m_eventData);
}

// ======================================================================
// CalendarGetSettingsMessage
// ======================================================================

CalendarGetSettingsMessage::CalendarGetSettingsMessage() :
	GameNetworkMessage("CalendarGetSettingsMessage")
{
}

CalendarGetSettingsMessage::CalendarGetSettingsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarGetSettingsMessage")
{
	unpack(source);
}

CalendarGetSettingsMessage::~CalendarGetSettingsMessage()
{
}

// ======================================================================
// CalendarSettingsResponseMessage
// ======================================================================

CalendarSettingsResponseMessage::CalendarSettingsResponseMessage(std::string const & bgTexture, int32 srcX, int32 srcY, int32 srcW, int32 srcH) :
	GameNetworkMessage("CalendarSettingsResponseMessage"),
	m_bgTexture(bgTexture),
	m_srcX(srcX),
	m_srcY(srcY),
	m_srcW(srcW),
	m_srcH(srcH)
{
	addVariable(m_bgTexture);
	addVariable(m_srcX);
	addVariable(m_srcY);
	addVariable(m_srcW);
	addVariable(m_srcH);
}

CalendarSettingsResponseMessage::CalendarSettingsResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarSettingsResponseMessage")
{
	addVariable(m_bgTexture);
	addVariable(m_srcX);
	addVariable(m_srcY);
	addVariable(m_srcW);
	addVariable(m_srcH);
	unpack(source);
}

CalendarSettingsResponseMessage::~CalendarSettingsResponseMessage()
{
}

// ======================================================================
// CalendarApplySettingsMessage
// ======================================================================

CalendarApplySettingsMessage::CalendarApplySettingsMessage(std::string const & bgTexture, int32 srcX, int32 srcY, int32 srcW, int32 srcH) :
	GameNetworkMessage("CalendarApplySettingsMessage"),
	m_bgTexture(bgTexture),
	m_srcX(srcX),
	m_srcY(srcY),
	m_srcW(srcW),
	m_srcH(srcH)
{
	addVariable(m_bgTexture);
	addVariable(m_srcX);
	addVariable(m_srcY);
	addVariable(m_srcW);
	addVariable(m_srcH);
}

CalendarApplySettingsMessage::CalendarApplySettingsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CalendarApplySettingsMessage")
{
	addVariable(m_bgTexture);
	addVariable(m_srcX);
	addVariable(m_srcY);
	addVariable(m_srcW);
	addVariable(m_srcH);
	unpack(source);
}

CalendarApplySettingsMessage::~CalendarApplySettingsMessage()
{
}

// ======================================================================
