// ======================================================================
//
// FileControlQueue.h
// copyright 2024 Sony Online Entertainment
//
// Sequential file queue turnstile for bidirectional transfers.
//
// ======================================================================

#ifndef INCLUDED_FileControlQueue_H
#define INCLUDED_FileControlQueue_H

// ======================================================================

#include <string>
#include <deque>
#include <vector>

// ======================================================================

class FileControlQueue
{
public:

	enum OperationType
	{
		OT_UPLOAD,
		OT_DOWNLOAD,
		OT_COMPARE,
		OT_TEST
	};

	struct QueueEntry
	{
		OperationType              operation;
		std::string                relativePath;
		std::vector<unsigned char> fileData;
		bool                       compressed;
		unsigned long              connectionId;

		QueueEntry() : operation(OT_UPLOAD), relativePath(), fileData(), compressed(false), connectionId(0) {}
	};

	FileControlQueue();
	~FileControlQueue();

	void          enqueue(const QueueEntry & entry);
	bool          dequeue(QueueEntry & entry);
	bool          isEmpty() const;
	int           getQueueSize() const;
	void          clear();

	bool          isProcessing() const;
	void          setProcessing(bool processing);

private:

	FileControlQueue(const FileControlQueue &);
	FileControlQueue & operator=(const FileControlQueue &);

	std::deque<QueueEntry> m_queue;
	bool                   m_processing;
};

// ======================================================================

inline bool FileControlQueue::isEmpty() const
{
	return m_queue.empty();
}

inline int FileControlQueue::getQueueSize() const
{
	return static_cast<int>(m_queue.size());
}

inline bool FileControlQueue::isProcessing() const
{
	return m_processing;
}

inline void FileControlQueue::setProcessing(bool processing)
{
	m_processing = processing;
}

// ======================================================================

#endif
