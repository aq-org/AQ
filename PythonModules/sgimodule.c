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

/* SGI module -- random SGI-specific things */

#include "allobjects.h"
#include "modsupport.h"
#include "ceval.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static object *
sgi_nap(self, args)
	object *self;
	object *args;
{
	long ticks;
	if (!getargs(args, "l", &ticks))
		return NULL;
	BGN_SAVE
	sginap(ticks);
	END_SAVE
	INCREF(None);
	return None;
}

extern char *_getpty(int *, int, mode_t, int);

static object *
sgi__getpty(self, args)
	object *self;
	object *args;
{
	int oflag;
	int mode;
	int nofork;
	char *name;
	int fildes;
	if (!getargs(args, "(iii)", &oflag, &mode, &nofork))
		return NULL;
	errno = 0;
	name = _getpty(&fildes, oflag, (mode_t)mode, nofork);
	if (name == NULL) {
		err_errno(IOError);
		return NULL;
	}
	return mkvalue("(si)", name, fildes);
}

static struct methodlist sgi_methods[] = {
	{"nap",		sgi_nap},
	{"_getpty",	sgi__getpty},
	{NULL,		NULL}		/* sentinel */
};


void
initsgi()
{
	initmodule("sgi", sgi_methods);
}

static int dummy; /* $%#@!& dl wants at least a byte of bss */
