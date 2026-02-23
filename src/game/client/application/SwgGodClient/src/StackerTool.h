// ======================================================================
//
// StackerTool.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StackerTool_H
#define INCLUDED_StackerTool_H

// ======================================================================

#include "BaseStackerTool.h"

// ======================================================================

class StackerTool : public BaseStackerTool
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	explicit StackerTool(QWidget *parent=0, const char *name=0);
	virtual ~StackerTool();

private:
	//disabled
	StackerTool(const StackerTool & rhs);
	StackerTool & operator=(const StackerTool & rhs);

private slots:
	void onStack();
};

// ======================================================================

#endif
