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

/* Use this file as a template to start implementing a module that
   also declares objects types. All occurrences of 'Xxo' should be changed
   to something reasonable for your objects. After that, all other
   occurrences of 'xx' should be changed to something reasonable for your
   module. If your module is named foo your sourcefile should be named
   foomodule.c.
   
   You will probably want to delete all references to 'x_attr' and add
   your own types of attributes instead.  Maybe you want to name your
   local variables other than 'self'.  If your object type is needed in
   other files, you'll have to create a file "foobarobject.h"; see
   intobject.h for an example. */

/* Xxo objects */

#include "Python.h"

static PyObject *ErrorObject;

typedef struct {
	PyObject_HEAD
	PyObject	*x_attr;	/* Attributes dictionary */
} XxoObject;

staticforward PyTypeObject Xxo_Type;

#define XxoObject_Check(v)	((v)->ob_type == &Xxo_Type)

static XxoObject *
newXxoObject(arg)
	PyObject *arg;
{
	XxoObject *self;
	self = PyObject_NEW(XxoObject, &Xxo_Type);
	if (self == NULL)
		return NULL;
	self->x_attr = NULL;
	return self;
}

/* Xxo methods */

static void
Xxo_dealloc(self)
	XxoObject *self;
{
	Py_XDECREF(self->x_attr);
	PyMem_DEL(self);
}

static PyObject *
Xxo_demo(self, args)
	XxoObject *self;
	PyObject *args;
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef Xxo_methods[] = {
	{"demo",	(PyCFunction)Xxo_demo,	1},
	{NULL,		NULL}		/* sentinel */
};

static PyObject *
Xxo_getattr(self, name)
	XxoObject *self;
	char *name;
{
	if (self->x_attr != NULL) {
		PyObject *v = PyDict_GetItemString(self->x_attr, name);
		if (v != NULL) {
			Py_INCREF(v);
			return v;
		}
	}
	return Py_FindMethod(Xxo_methods, (PyObject *)self, name);
}

static int
Xxo_setattr(self, name, v)
	XxoObject *self;
	char *name;
	PyObject *v;
{
	if (self->x_attr == NULL) {
		self->x_attr = PyDict_New();
		if (self->x_attr == NULL)
			return -1;
	}
	if (v == NULL) {
		int rv = PyDict_DelItemString(self->x_attr, name);
		if (rv < 0)
			PyErr_SetString(PyExc_AttributeError,
			        "delete non-existing Xxo attribute");
		return rv;
	}
	else
		return PyDict_SetItemString(self->x_attr, name, v);
}

staticforward PyTypeObject Xxo_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,			/*ob_size*/
	"Xxo",			/*tp_name*/
	sizeof(XxoObject),	/*tp_basicsize*/
	0,			/*tp_itemsize*/
	/* methods */
	(destructor)Xxo_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	(getattrfunc)Xxo_getattr, /*tp_getattr*/
	(setattrfunc)Xxo_setattr, /*tp_setattr*/
	0,			/*tp_compare*/
	0,			/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
};
/* --------------------------------------------------------------------- */

/* Function of two integers returning integer */

static PyObject *
xx_foo(self, args)
	PyObject *self; /* Not used */
	PyObject *args;
{
	long i, j;
	long res;
	if (!PyArg_ParseTuple(args, "ll", &i, &j))
		return NULL;
	res = i+j; /* XXX Do something here */
	return PyInt_FromLong(res);
}


/* Function of no arguments returning new Xxo object */

static PyObject *
xx_new(self, args)
	PyObject *self; /* Not used */
	PyObject *args;
{
	int i, j;
	XxoObject *rv;
	
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	rv = newXxoObject(args);
	if ( rv == NULL )
	    return NULL;
	return (PyObject *)rv;
}


/* List of functions defined in the module */

static PyMethodDef xx_methods[] = {
	{"foo",		xx_foo,		1},
	{"new",		xx_new,		1},
	{NULL,		NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initxx) */

void
initxx()
{
	PyObject *m, *d;

	/* Create the module and add the functions */
	m = Py_InitModule("xx", xx_methods);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);
	ErrorObject = PyString_FromString("xx.error");
	PyDict_SetItemString(d, "error", ErrorObject);

	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module xx");
}
