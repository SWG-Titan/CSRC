// ======================================================================
//
// DatatableEditorWindow.h
// copyright 2024 Sony Online Entertainment
//
// Qt spreadsheet window for creating/editing datatables (.tab) and
// compiling with DataTableTool. Features pagination for data rows,
// with column name and type rows frozen at the top (not paginated).
//
// ======================================================================

#ifndef INCLUDED_DatatableEditorWindow_H
#define INCLUDED_DatatableEditorWindow_H

// ======================================================================

#include <qdialog.h>
#include <string>
#include <vector>

class QTable;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;

// ======================================================================

class DatatableEditorWindow : public QDialog
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	explicit DatatableEditorWindow(QWidget * parent = 0, const char * name = 0);
	virtual ~DatatableEditorWindow();

	void loadDatatable(const std::string & tabPath);
	void newDatatable();

signals:
	void statusMessage(const char * msg);

public slots:
	void onSave();
	void onCompile();
	void onBrowse();
	void onPageFirst();
	void onPagePrev();
	void onPageNext();
	void onPageLast();
	void onAddRow();
	void onDeleteRow();
	void onAddColumn();
	void onDeleteColumn();

private:
	DatatableEditorWindow(const DatatableEditorWindow &);
	DatatableEditorWindow & operator=(const DatatableEditorWindow &);

	void refreshTableView();
	void syncHeaderFromTable();
	void syncDataFromTable();
	bool parseTabFile(const std::string & path);
	bool writeTabFile(const std::string & path);
	bool runDataTableTool(const std::string & tabPath);

	static const int ROWS_PER_PAGE = 50;

	QTable *      m_headerTable;
	QTable *      m_table;
	QPushButton * m_saveButton;
	QPushButton * m_compileButton;
	QPushButton * m_browseButton;
	QPushButton * m_pageFirstButton;
	QPushButton * m_pagePrevButton;
	QPushButton * m_pageNextButton;
	QPushButton * m_pageLastButton;
	QPushButton * m_addRowButton;
	QPushButton * m_deleteRowButton;
	QPushButton * m_addColumnButton;
	QPushButton * m_deleteColumnButton;
	QLineEdit *   m_filePathEdit;
	QLabel *      m_pageLabel;
	QLabel *      m_statusLabel;

	std::string   m_currentPath;
	int           m_currentPage;
	int           m_totalPages;

	std::vector<std::string> m_columnNames;
	std::vector<std::string> m_columnTypes;
	std::vector<std::vector<std::string> > m_rowData;
};

// ======================================================================

#endif
