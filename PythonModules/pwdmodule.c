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

/* UNIX password file access module */

#include "allobjects.h"
#include "modsupport.h"

#include <sys/types.h>
#include <pwd.h>

static object *mkpwent(p)
	struct passwd *p;
{
	return mkvalue("(ssllsss)",
		       p->pw_name,
		       p->pw_passwd,
#if defined(NeXT) && defined(_POSIX_SOURCE) && defined(__LITTLE_ENDIAN__)
/* Correct a bug present on Intel machines in NextStep 3.2 and 3.3;
   for later versions you may have to remove this */
		       (long)p->pw_short_pad1, /* ugh-NeXT broke the padding */
		       (long)p->pw_short_pad2,
#else
		       (long)p->pw_uid,
		       (long)p->pw_gid,
#endif
		       p->pw_gecos,
		       p->pw_dir,
		       p->pw_shell);
}

static object *pwd_getpwuid(self, args)
	object *self, *args;
{
	int uid;
	struct passwd *p;
	if (!getintarg(args, &uid))
		return NULL;
	if ((p = getpwuid(uid)) == NULL) {
		err_setstr(KeyError, "getpwuid(): uid not found");
		return NULL;
	}
	return mkpwent(p);
}

static object *pwd_getpwnam(self, args)
	object *self, *args;
{
	char *name;
	struct passwd *p;
	if (!getstrarg(args, &name))
		return NULL;
	if ((p = getpwnam(name)) == NULL) {
		err_setstr(KeyError, "getpwnam(): name not found");
		return NULL;
	}
	return mkpwent(p);
}

static object *pwd_getpwall(self, args)
	object *self, *args;
{
	object *d;
	struct passwd *p;
	if (!getnoarg(args))
		return NULL;
	if ((d = newlistobject(0)) == NULL)
		return NULL;
	setpwent();
	while ((p = getpwent()) != NULL) {
		object *v = mkpwent(p);
		if (v == NULL || addlistitem(d, v) != 0) {
			XDECREF(v);
			DECREF(d);
			return NULL;
		}
	}
	return d;
}

static struct methodlist pwd_methods[] = {
	{"getpwuid",	pwd_getpwuid},
	{"getpwnam",	pwd_getpwnam},
	{"getpwall",	pwd_getpwall},
	{NULL,		NULL}		/* sentinel */
};

void
initpwd()
{
	initmodule("pwd", pwd_methods);
}
