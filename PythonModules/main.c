/***********************************************************
Copyright 1991-1995 by Stichting Mathematisch Centrum, Amsterdam,
The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* Python interpreter main program */

#include "Python.h"


/* Interface to getopt(): */
extern int optind;
extern char *optarg;
extern int getopt(); /* PROTO((int, char **, char *)); -- not standardized */


/* Subroutines that live in their own file */
extern char *Py_GetVersion();
extern char *Py_GetCopyright();


/* For Py_GetProgramName(); set by main() */
static char *argv0;

/* For Py_GetArgcArgv(); set by main() */
static char **orig_argv;
static int  orig_argc;


/* Short usage message (with %s for argv0) */
static char *usage_line =
"usage: %s [-d] [-i] [-s] [-u ] [-v] [-c cmd | file | -] [arg] ...\n";

/* Long usage message, split into parts < 512 bytes */
static char *usage_top = "\n\
Options and arguments (and corresponding environment variables):\n\
-d     : debug output from parser (also PYTHONDEBUG=x)\n\
-i     : inspect interactively after running script (also PYTHONINSPECT=x)\n\
-s     : suppress printing of top level expressions (also PYTHONSUPPRESS=x)\n\
-u     : unbuffered stdout and stderr (also PYTHONUNBUFFERED=x)\n\
-v     : verbose (trace import statements) (also PYTHONVERBOSE=x)\n\
-c cmd : program passed in as string (terminates option list)\n\
";
static char *usage_bot = "\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
\n\
Other environment variables:\n\
PYTHONSTARTUP: file executed on interactive startup (no default)\n\
PYTHONPATH   : colon-separated list of directories prefixed to the\n\
               default module search path.  The result is sys.path.\n\
";


/* Main program */

int
main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	int sts;
	char *command = NULL;
	char *filename = NULL;
	FILE *fp = stdin;
	char *p;
	int inspect = 0;
	int unbuffered = 0;

	orig_argc = argc;	/* For Py_GetArgcArgv() */
	orig_argv = argv;
	argv0 = argv[0];	/* For Py_GetProgramName() */

	if ((p = getenv("PYTHONDEBUG")) && *p != '\0')
		Py_DebugFlag = 1;
	if ((p = getenv("PYTHONSUPPRESS")) && *p != '\0')
		Py_SuppressPrintingFlag = 1;
	if ((p = getenv("PYTHONVERBOSE")) && *p != '\0')
		Py_VerboseFlag = 1;
	if ((p = getenv("PYTHONINSPECT")) && *p != '\0')
		inspect = 1;
	if ((p = getenv("PYTHONUNBUFFERED")) && *p != '\0')
		unbuffered = 1;

	while ((c = getopt(argc, argv, "c:disuv")) != EOF) {
		if (c == 'c') {
			/* -c is the last option; following arguments
			   that look like options are left for the
			   the command to interpret. */
			command = malloc(strlen(optarg) + 2);
			if (command == NULL)
				Py_FatalError(
				   "not enough memory to copy -c argument");
			strcpy(command, optarg);
			strcat(command, "\n");
			break;
		}
		
		switch (c) {

		case 'd':
			Py_DebugFlag++;
			break;

		case 'i':
			inspect++;
			break;

		case 's':
			Py_SuppressPrintingFlag++;
			break;

		case 'u':
			unbuffered++;
			break;

		case 'v':
			Py_VerboseFlag++;
			break;

		/* This space reserved for other options */

		default:
			fprintf(stderr, usage_line, argv[0]);
			fprintf(stderr, usage_top);
			fprintf(stderr, usage_bot);
			exit(2);
			/*NOTREACHED*/

		}
	}

	if (unbuffered) {
#ifndef MPW
		setbuf(stdout, (char *)NULL);
		setbuf(stderr, (char *)NULL);
#else
		/* On MPW (3.2) unbuffered seems to hang */
		setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZ);
		setvbuf(stderr, (char *)NULL, _IOLBF, BUFSIZ);
#endif
	}

	if (command == NULL && optind < argc &&
	    strcmp(argv[optind], "-") != 0)
		filename = argv[optind];

	if (Py_VerboseFlag ||
	    command == NULL && filename == NULL && isatty((int)fileno(fp)))
		fprintf(stderr, "Python %s\n%s\n",
			Py_GetVersion(), Py_GetCopyright());
	
	if (filename != NULL) {
		if ((fp = fopen(filename, "r")) == NULL) {
			fprintf(stderr, "%s: can't open file '%s'\n",
				argv[0], filename);
			exit(2);
		}
	}
	
	Py_Initialize();
	
	if (command != NULL) {
		/* Backup optind and force sys.argv[0] = '-c' */
		optind--;
		argv[optind] = "-c";
	}

	PySys_SetArgv(argc-optind, argv+optind);

	if (command) {
		sts = PyRun_SimpleString(command) != 0;
	}
	else {
		if (filename == NULL && isatty((int)fileno(fp))) {
			char *startup = getenv("PYTHONSTARTUP");
			if (startup != NULL && startup[0] != '\0') {
				FILE *fp = fopen(startup, "r");
				if (fp != NULL) {
					(void) PyRun_SimpleFile(fp, startup);
					PyErr_Clear();
					fclose(fp);
				}
			}
		}
		sts = PyRun_AnyFile(
			fp, filename == NULL ? "<stdin>" : filename) != 0;
		if (filename != NULL)
			fclose(fp);
	}

	if (inspect && isatty((int)fileno(stdin)) &&
	    (filename != NULL || command != NULL))
		sts = PyRun_AnyFile(stdin, "<stdin>") != 0;

	Py_Exit(sts);
	/*NOTREACHED*/
}


/* Return the program name -- some code out there needs this
   (currently _tkinter.c and importdl.c). */

char *
Py_GetProgramName()
{
	return argv0;
}


/* Make the *original* argc/argv available to other modules.
   This is rare, but it is needed by the secureware extension. */

void
Py_GetArgcArgv(argc, argv)
	int *argc;
	char ***argv;
{
	*argc = orig_argc;
	*argv = orig_argv;
}
