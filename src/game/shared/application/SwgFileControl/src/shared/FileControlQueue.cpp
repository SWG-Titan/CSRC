// ======================================================================
//
// FileControlQueue.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgFileControl.h"
#include "FileControlQueue.h"

#include "sharedLog/Log.h"

// ======================================================================

FileControlQueue::FileControlQueue() :
m_queue(),
m_processing(false)
{
}

// ----------------------------------------------------------------------

FileControlQueue::~FileControlQueue()
{
	m_queue.clear();
}

// ----------------------------------------------------------------------

void FileControlQueue::enqueue(const QueueEntry & entry)
{
	m_queue.push_back(entry);

	const char * opName = "unknown";
	switch (entry.operation)
	{
	case OT_UPLOAD:   opName = "upload";   break;
	case OT_DOWNLOAD: opName = "download"; break;
	case OT_COMPARE:  opName = "compare";  break;
	case OT_TEST:     opName = "test";     break;
	}

	LOG("FileControl", ("Queued %s for [%s] (queue depth: %d)", opName, entry.relativePath.c_str(), getQueueSize()));
}

// ----------------------------------------------------------------------

bool FileControlQueue::dequeue(QueueEntry & entry)
{
	if (m_queue.empty())
		return false;

	entry = m_queue.front();
	m_queue.pop_front();
	return true;
}

// ----------------------------------------------------------------------

void FileControlQueue::clear()
{
	m_queue.clear();
	m_processing = false;
}

// ======================================================================
