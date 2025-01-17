//////////////////////////////////////////////////////////////////////////
//
// pgAgent - PostgreSQL Tools
//
// Copyright (C) 2002 - 2015, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// unix.cpp - pgAgent unix specific functions
//
//////////////////////////////////////////////////////////////////////////

#include "pgAgent.h"

// *nix only!!
#ifndef WIN32

#include <wx/filename.h>
#include <wx/ffile.h>
#include <fcntl.h>
void printVersion();

void usage(const wxString &executable)
{
	wxFileName *fn = new wxFileName(executable);
	printVersion();

	wxPrintf(_("Usage:\n"));
	wxPrintf(fn->GetName() + _(" [options] <connect-string>\n"));
	wxPrintf(_("options:\n"));
	wxPrintf(_("-v (display version info and then exit)\n"));
	wxPrintf(_("-f run in the foreground (do not detach from the terminal)\n"));
	wxPrintf(_("-t <poll time interval in seconds (default 10)>\n"));
	wxPrintf(_("-r <retry period after connection abort in seconds (>=10, default 30)>\n"));
	wxPrintf(_("-s <log file (messages are logged to STDOUT if not specified>\n"));
	wxPrintf(_("-l <logging verbosity (ERROR=0, WARNING=1, DEBUG=2, default 0)>\n"));
}

void LogMessage(wxString msg, int level)
{
	wxFFile file;
	if (logFile.IsEmpty())
	{
		file.Attach(stdout);
	}
	else
	{
		file.Open(logFile.c_str(), wxT("a"));
	}

	if (!file.IsOpened())
	{
		wxFprintf(stderr, _("Can not open the logfile!"));
		return;
	}

	wxDateTime logTime = wxDateTime::Now();
	wxString logTimeString = logTime.Format() + wxT(" : ");

	switch (level)
	{
		case LOG_DEBUG:
			if (minLogLevel >= LOG_DEBUG)
				file.Write(logTimeString + _("DEBUG: ") + msg + wxT("\n"));
			break;
		case LOG_WARNING:
			if (minLogLevel >= LOG_WARNING)
				file.Write(logTimeString + _("WARNING: ") + msg + wxT("\n"));
			break;
		case LOG_ERROR:
			file.Write(logTimeString + _("ERROR: ") + msg + wxT("\n"));
			exit(1);
			break;
		case LOG_STARTUP:
			file.Write(logTimeString + _("WARNING: ") + msg + wxT("\n"));
			break;
	}

	if (logFile.IsEmpty())
	{
		file.Detach();
	}
	else
	{
		file.Close();
	}
}

// Shamelessly lifted from pg_autovacuum...
static void daemonize(void)
{
	pid_t pid;

	pid = fork();
	if (pid == (pid_t) - 1)
	{
		LogMessage(_("Cannot disassociate from controlling TTY"), LOG_ERROR);
		exit(1);
	}
	else if (pid)
		exit(0);

#ifdef HAVE_SETSID
	if (setsid() < 0)
	{
		LogMessage(_("Cannot disassociate from controlling TTY"), LOG_ERROR);
		exit(1);
	}
#endif

}

int main(int argc, char **argv)
{
	// Statup wx
	wxInitialize();

	wxString executable;
	executable = wxString::FromAscii(argv[0]);

	if (argc < 2)
	{
		usage(executable);
		return 1;
	}

	argc--;
	argv++;

	setOptions(argc, argv, executable);

	if (!runInForeground)
		daemonize();

	MainLoop();

	return 0;
}

#endif // !WIN32
