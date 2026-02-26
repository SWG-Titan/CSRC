// ======================================================================
//
// SwgFileControl.h
// copyright 2024 Sony Online Entertainment
//
// Main application class for SwgFileControl.
// One application, two modalities: server and client.
//
// Windows: SwgFileControl_[r/d].exe
// Linux:   SwgFileControl
//
// ======================================================================

#ifndef INCLUDED_SwgFileControl_H
#define INCLUDED_SwgFileControl_H

// ======================================================================

#include <string>

// ======================================================================

class SwgFileControl
{
public:

	static void run();
	static void usage();

private:

	SwgFileControl();
	SwgFileControl(const SwgFileControl &);
	SwgFileControl & operator=(const SwgFileControl &);

	static void runServer();
	static void runClient();

	static std::string ms_updateFile;
	static std::string ms_downloadFile;
	static std::string ms_compareFile;
	static std::string ms_testRunFile;
	static std::string ms_fileServerKey;
};

// ======================================================================

#endif
