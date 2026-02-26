// ======================================================================
//
// FileServerTreeWindow.h
// copyright 2024 Sony Online Entertainment
//
// Pop-out window listing the file server's data/ directory tree,
// comparing local vs remote file status with multi-select checkboxes
// and Send / Retrieve / Info buttons.
//
// ======================================================================

#ifndef INCLUDED_FileServerTreeWindow_H
#define INCLUDED_FileServerTreeWindow_H

// ======================================================================

#include <qwidget.h>
#include <string>
#include <vector>

class QListView;
class QListViewItem;
class QPushButton;
class QLabel;
class QLineEdit;

// ======================================================================

class FileServerTreeWindow : public QWidget
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	explicit FileServerTreeWindow(QWidget * parent = 0, const char * name = 0);
	virtual ~FileServerTreeWindow();

	void refreshTree();
	void setRootScope(const std::string & rootPath);

	struct FileEntry
	{
		std::string relativePath;
		bool        localAvailable;
		bool        remoteAvailable;
		unsigned long localSize;
		unsigned long remoteSize;
		unsigned long localCrc;
		unsigned long remoteCrc;
	};

signals:
	void statusMessage(const char * msg);
	void selectedPathChanged(const std::string & path);

public slots:
	void onRefresh();
	void onSend();
	void onRetrieve();
	void onInfo();
	void onScopeChanged();
	void onSelectionChanged(QListViewItem * item);
	void onItemChecked(QListViewItem * item);

private:
	FileServerTreeWindow(const FileServerTreeWindow &);
	FileServerTreeWindow & operator=(const FileServerTreeWindow &);

	void buildTreeFromEntries(const std::vector<FileEntry> & entries);
	void updateButtonStates();

	std::string getStatusText(bool localAvail, bool remoteAvail, unsigned long localCrc, unsigned long remoteCrc) const;

	QListView *  m_treeView;
	QPushButton * m_sendButton;
	QPushButton * m_retrieveButton;
	QPushButton * m_infoButton;
	QPushButton * m_refreshButton;
	QLineEdit *   m_scopeEdit;
	QLabel *      m_statusLabel;

	std::string   m_rootScope;
	std::vector<FileEntry> m_entries;
	std::vector<std::string> m_checkedPaths;
};

// ======================================================================

#endif
