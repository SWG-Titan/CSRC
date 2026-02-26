// ======================================================================
//
// FileServerTreeWindow.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FileServerTreeWindow.h"
#include "FileServerTreeWindow.moc"

#include "ActionsFileControl.h"
#include "MainFrame.h"

#include "FileControlServer.h"

#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

#include <sys/stat.h>

// ======================================================================

FileServerTreeWindow::FileServerTreeWindow(QWidget * parent, const char * name)
: QWidget(parent, name),
  m_treeView(0),
  m_sendButton(0),
  m_retrieveButton(0),
  m_infoButton(0),
  m_refreshButton(0),
  m_scopeEdit(0),
  m_statusLabel(0),
  m_rootScope("data/"),
  m_entries(),
  m_checkedPaths()
{
	QVBoxLayout * mainLayout = new QVBoxLayout(this, 4, 4);

	QHBoxLayout * scopeLayout = new QHBoxLayout(0, 0, 4);
	QLabel * scopeLabel = new QLabel("Scope:", this);
	m_scopeEdit = new QLineEdit("data/", this);
	m_refreshButton = new QPushButton("Refresh", this);
	scopeLayout->addWidget(scopeLabel);
	scopeLayout->addWidget(m_scopeEdit, 1);
	scopeLayout->addWidget(m_refreshButton);
	mainLayout->addLayout(scopeLayout);

	m_treeView = new QListView(this, "fileServerTree");
	m_treeView->addColumn("Path");
	m_treeView->addColumn("Local Status");
	m_treeView->addColumn("Remote Status");
	m_treeView->addColumn("Local Size");
	m_treeView->addColumn("Remote Size");
	m_treeView->setRootIsDecorated(true);
	m_treeView->setAllColumnsShowFocus(true);
	m_treeView->setSelectionMode(QListView::Extended);
	m_treeView->header()->setClickEnabled(true);
	m_treeView->setSorting(0, true);
	mainLayout->addWidget(m_treeView, 1);

	m_statusLabel = new QLabel("Select files and use Send, Retrieve, or Info.", this);
	mainLayout->addWidget(m_statusLabel);

	QHBoxLayout * buttonLayout = new QHBoxLayout(0, 0, 4);
	m_sendButton     = new QPushButton("Send", this);
	m_retrieveButton = new QPushButton("Retrieve", this);
	m_infoButton     = new QPushButton("Info", this);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(m_sendButton);
	buttonLayout->addWidget(m_retrieveButton);
	buttonLayout->addWidget(m_infoButton);
	mainLayout->addLayout(buttonLayout);

	m_sendButton->setEnabled(false);
	m_retrieveButton->setEnabled(false);
	m_infoButton->setEnabled(false);

	IGNORE_RETURN(connect(m_refreshButton,  SIGNAL(clicked()), this, SLOT(onRefresh())));
	IGNORE_RETURN(connect(m_sendButton,     SIGNAL(clicked()), this, SLOT(onSend())));
	IGNORE_RETURN(connect(m_retrieveButton, SIGNAL(clicked()), this, SLOT(onRetrieve())));
	IGNORE_RETURN(connect(m_infoButton,     SIGNAL(clicked()), this, SLOT(onInfo())));
	IGNORE_RETURN(connect(m_scopeEdit,      SIGNAL(returnPressed()), this, SLOT(onScopeChanged())));
	IGNORE_RETURN(connect(m_treeView,       SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(onSelectionChanged(QListViewItem *))));
}

// ----------------------------------------------------------------------

FileServerTreeWindow::~FileServerTreeWindow()
{
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::setRootScope(const std::string & rootPath)
{
	m_rootScope = rootPath;
	if (m_scopeEdit)
		m_scopeEdit->setText(rootPath.c_str());
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::refreshTree()
{
	if (!m_treeView)
		return;

	m_treeView->clear();
	m_checkedPaths.clear();
	m_entries.clear();

	m_statusLabel->setText("Refreshing from server...");

	std::vector<std::string> remoteFiles;
	std::vector<unsigned long> remoteSizes;
	std::vector<unsigned long> remoteCrcs;

	bool gotListing = FileControlServer::requestDirectoryListing(
		m_rootScope, remoteFiles, remoteSizes, remoteCrcs);

	if (!gotListing || remoteFiles.empty())
	{
		m_statusLabel->setText("No files returned from server (is server running?).");
		MainFrame::getInstance().textToConsole("FileControl: Directory listing returned empty.");
		updateButtonStates();
		return;
	}

	for (size_t i = 0; i < remoteFiles.size(); ++i)
	{
		FileEntry fe;
		fe.relativePath = remoteFiles[i];
		fe.remoteAvailable = true;
		fe.remoteSize = (i < remoteSizes.size()) ? remoteSizes[i] : 0;
		fe.remoteCrc  = (i < remoteCrcs.size())  ? remoteCrcs[i]  : 0;

		fe.localAvailable = false;
		fe.localSize = 0;
		fe.localCrc  = 0;

#if defined(PLATFORM_WIN32)
		struct _stat localStat;
		if (_stat(fe.relativePath.c_str(), &localStat) == 0)
#else
		struct stat localStat;
		if (stat(fe.relativePath.c_str(), &localStat) == 0)
#endif
		{
			fe.localAvailable = true;
			fe.localSize = static_cast<unsigned long>(localStat.st_size);
		}

		m_entries.push_back(fe);
	}

	buildTreeFromEntries(m_entries);
	updateButtonStates();

	m_statusLabel->setText(QString("Loaded %1 files from scope: %2")
		.arg(static_cast<int>(m_entries.size()))
		.arg(m_rootScope.c_str()));

	MainFrame::getInstance().textToConsole(
		QString("FileControl: Tree refreshed - %1 files in %2")
			.arg(static_cast<int>(m_entries.size()))
			.arg(m_rootScope.c_str()).latin1());
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::buildTreeFromEntries(const std::vector<FileEntry> & entries)
{
	for (size_t i = 0; i < entries.size(); ++i)
	{
		const FileEntry & fe = entries[i];

		std::string localStatus;
		std::string remoteStatus;

		if (fe.localAvailable && fe.remoteAvailable)
		{
			if (fe.localCrc == fe.remoteCrc && fe.localCrc != 0)
			{
				localStatus = "Available (match)";
				remoteStatus = "Available (match)";
			}
			else
			{
				localStatus = "Available (differs)";
				remoteStatus = "Available (differs)";
			}
		}
		else if (fe.localAvailable && !fe.remoteAvailable)
		{
			localStatus = "Available";
			remoteStatus = "Missing";
		}
		else if (!fe.localAvailable && fe.remoteAvailable)
		{
			localStatus = "Missing";
			remoteStatus = "Available";
		}
		else
		{
			localStatus = "Unavailable";
			remoteStatus = "Unavailable";
		}

		char localSizeBuf[64];
		char remoteSizeBuf[64];
		snprintf(localSizeBuf, sizeof(localSizeBuf), "%lu", fe.localSize);
		snprintf(remoteSizeBuf, sizeof(remoteSizeBuf), "%lu", fe.remoteSize);

		QListViewItem * item = new QListViewItem(m_treeView,
			fe.relativePath.c_str(),
			localStatus.c_str(),
			remoteStatus.c_str(),
			localSizeBuf,
			remoteSizeBuf);
		UNREF(item);
	}
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::updateButtonStates()
{
	bool hasSelection = (m_treeView && m_treeView->selectedItem() != 0);
	m_sendButton->setEnabled(hasSelection);
	m_retrieveButton->setEnabled(hasSelection);
	m_infoButton->setEnabled(hasSelection);
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onRefresh()
{
	onScopeChanged();
	refreshTree();
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onScopeChanged()
{
	if (m_scopeEdit)
		m_rootScope = m_scopeEdit->text().latin1();
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onSelectionChanged(QListViewItem * item)
{
	updateButtonStates();

	if (item)
	{
		std::string path = item->text(0).latin1();
		emit selectedPathChanged(path);

		ActionsFileControl::getInstance().onSelectedPathChanged(path);
	}
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onItemChecked(QListViewItem * item)
{
	UNREF(item);
	updateButtonStates();
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onSend()
{
	QListViewItem * item = m_treeView ? m_treeView->selectedItem() : 0;
	if (!item)
		return;

	std::string path = item->text(0).latin1();

	bool success = FileControlServer::requestSendAsset(path);

	if (success)
	{
		MainFrame::getInstance().textToConsole(("FileControl: Send -> " + path + " OK").c_str());
		emit statusMessage(("Sent: " + path).c_str());
	}
	else
	{
		MainFrame::getInstance().textToConsole(("FileControl: Send -> " + path + " FAILED").c_str());
		emit statusMessage(("Send failed: " + path).c_str());
	}
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onRetrieve()
{
	QListViewItem * item = m_treeView ? m_treeView->selectedItem() : 0;
	if (!item)
		return;

	std::string path = item->text(0).latin1();

	std::vector<unsigned char> data;
	bool success = FileControlServer::requestRetrieveAsset(path, data);

	if (success)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "FileControl: Retrieved %s (%d bytes)", path.c_str(), static_cast<int>(data.size()));
		MainFrame::getInstance().textToConsole(buf);
		emit statusMessage(("Retrieved: " + path).c_str());
	}
	else
	{
		MainFrame::getInstance().textToConsole(("FileControl: Retrieve -> " + path + " FAILED").c_str());
		emit statusMessage(("Retrieve failed: " + path).c_str());
	}
}

// ----------------------------------------------------------------------

void FileServerTreeWindow::onInfo()
{
	QListViewItem * item = m_treeView ? m_treeView->selectedItem() : 0;
	if (!item)
		return;

	std::string path = item->text(0).latin1();

	unsigned long size = 0;
	unsigned long crc = 0;
	bool verified = FileControlServer::requestVerifyAsset(path, size, crc);

	std::string localStatus = item->text(1).latin1();
	std::string remoteStatus = item->text(2).latin1();
	std::string localSize = item->text(3).latin1();
	std::string remoteSize = item->text(4).latin1();

	QString info = QString("Path: %1\n\nLocal: %2 (%3 bytes)\nRemote: %4 (%5 bytes)")
		.arg(path.c_str())
		.arg(localStatus.c_str())
		.arg(localSize.c_str())
		.arg(remoteStatus.c_str())
		.arg(remoteSize.c_str());

	if (verified)
	{
		info += QString("\n\nServer CRC: 0x%1\nServer Size: %2")
			.arg(crc, 8, 16, QChar('0'))
			.arg(size);
	}

	QMessageBox::information(&MainFrame::getInstance(), "Asset Info", info);
}

// ----------------------------------------------------------------------

std::string FileServerTreeWindow::getStatusText(bool localAvail, bool remoteAvail, unsigned long localCrc, unsigned long remoteCrc) const
{
	if (localAvail && remoteAvail)
	{
		if (localCrc == remoteCrc)
			return "Synced";
		return "Out of sync";
	}
	if (localAvail)
		return "Local only";
	if (remoteAvail)
		return "Remote only";
	return "Missing";
}

// ======================================================================
