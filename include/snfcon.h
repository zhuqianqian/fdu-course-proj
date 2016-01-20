/*
* snfcon.h: This file includes the command line parser for
*	the sniffer program
*/

#include <stdio.h>

const char *appname = "snf204";
const char *version = "1.0.1.3";

void show_help(const char *app)
{
	printf( "%s: A simple network traffic sniffer.\n"
			"Usage: %s [Options] [Action]\n\n"
			"   Where options can be:\n"
			"      -h (or --help)     Show this help context.\n"
			"      --gui              Start the GUI program.\n"
			"      -n                 No standard output.\n"
			"      -o <filename>      Set output file, only invalid when the action is -t\n"
			"      --version          Show the version of this program.\n"
			"   Action can be:\n"
			"      -t                 Track the network stream\n"
			"      -l <filename>      Load a saved file.\n"
			"By default, the action is '-t', and '-n' is off\n\n"
			"Please report bugs at <http://code.google.com/p/zqqcode/issues/list>\n", app, app);
}

void show_version(const char *app, const char *version)
{
	printf( "%s %s\n\n"
			"Copyright (c) 2010, all 204 members\n"
			"You are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n", app, version);
}

unsigned int show=1;
unsigned int save = 0;
char *file;
char *errarg;

#define CMD_MUTEX	(0)
#define CMD_MUTEXEX	(1)
#define CMD_ERROR	(2)

#define CMD_HELP	(6)
#define CMD_VERSION	(7)
#define CMD_TRACK	(8)
#define CMD_LOAD	(9)
#define CMD_GUI		(10)


int get_command(int argc, char **argv)
{
	int i;
	int flag, olmut;
	int retcode = CMD_TRACK;
	if(argc < 2)
		return retcode;
	flag = olmut = 0;
	for(i = 1; i < argc; )
	{
		if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0)
		{
			return CMD_HELP;
		}
		if(strcmp(argv[i], "--version")==0)
		{
			return CMD_VERSION;
		}
		if(strcmp(argv[i], "--gui")==0)
		{
			return CMD_GUI;
		}
		if(strcmp(argv[i], "-n")==0)
		{
			show = 0;
			++i;
		}
		else if(strcmp(argv[i], "-o")==0)
		{
			if(olmut)
			{
				retcode = CMD_MUTEXEX;
				break;
			}
			olmut = 1;
			save = 1;
			++i;
			if(argv[i] && argv[i][0] != '-') /* The argument next to '-o' should be a file name */
			{
				file = argv[i];
				++i;
			}
			else
				file = NULL;
		}
		else if(strcmp(argv[i], "-t")==0)
		{
			if(flag==1) {
				retcode = CMD_MUTEX;
				break;
			}
			flag = 1;
			retcode = CMD_TRACK;
			++i;
		}
		else if(strcmp(argv[i], "-l") == 0)
		{
			if(flag)
			{
				retcode = CMD_MUTEX;
				break;
			}
			flag = 1;
			if(olmut)
			{
				retcode = CMD_MUTEXEX;
				break;
			}
			olmut = 1;
			retcode = CMD_LOAD;
			++i;
			if(argv[i] && argv[i][0] != '-')
			{
				file = argv[i];
				++i;
			}
			else file = NULL;
		}
		else
		{
			retcode = CMD_ERROR;
			errarg = argv[i];
			break;
		}
	}
	return retcode;
}

