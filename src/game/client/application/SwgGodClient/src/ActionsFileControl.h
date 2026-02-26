// ======================================================================
//
// ActionsFileControl.h
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsFileControl_H
#define INCLUDED_ActionsFileControl_H

// ======================================================================

#include "Singleton/Singleton.h"

#include <string>
#include <vector>

// ======================================================================

class ActionHack;

// ======================================================================

class ActionsFileControl : public QObject, public Singleton<ActionsFileControl>
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	//lint -save
	//lint -e1925
	ActionHack * m_sendAsset;
	ActionHack * m_reloadAsset;
	ActionHack * m_retrieveAsset;
	ActionHack * m_broadcastUpdate;
	ActionHack * m_updateDbTemplates;
	ActionHack * m_verifyAsset;
	ActionHack * m_flush;
	ActionHack * m_openFileServerTree;
	ActionHack * m_openTemplateEditor;
	ActionHack * m_openDatatableEditor;
	ActionHack * m_openBuildoutEditor;
	//lint -restore

public:
	ActionsFileControl();
	~ActionsFileControl();

	bool isConnected() const;

	enum LifecycleStage
	{
		LS_IDLE,
		LS_UPLOADING,
		LS_STORED,
		LS_REBUILDING,
		LS_RELOADING,
		LS_DISTRIBUTING,
		LS_CONFIRMED,
		LS_FAILED
	};

	LifecycleStage getLifecycleStage() const;
	const char *   getLifecycleStageText() const;

public slots:
	void onSendAsset();
	void onReloadAsset();
	void onRetrieveAsset();
	void onBroadcastUpdate();
	void onUpdateDbTemplates();
	void onVerifyAsset();
	void onFlush();
	void onOpenFileServerTree();
	void onOpenTemplateEditor();
	void onOpenDatatableEditor();
	void onOpenBuildoutEditor();

	void onSelectedPathChanged(const std::string & path);

signals:
	void fileControlStatusMessage(const char * msg);

private:
	ActionsFileControl(const ActionsFileControl &);
	ActionsFileControl & operator=(const ActionsFileControl &);

	void setLifecycleStage(LifecycleStage stage);
	void logLifecycle(const char * action, const std::string & path, bool success);

	static std::string convertCompiledPathToSource(const std::string & compiledPath);

	std::string    m_selectedPath;
	bool           m_connected;
	LifecycleStage m_lifecycleStage;
};

// ======================================================================

inline bool ActionsFileControl::isConnected() const
{
	return m_connected;
}

inline ActionsFileControl::LifecycleStage ActionsFileControl::getLifecycleStage() const
{
	return m_lifecycleStage;
}

// ======================================================================

#endif
