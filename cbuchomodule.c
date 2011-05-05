#include <Python.h>

/* Functions */

static PyObject *
cbucho_system(PyObject *self, PyObject *args) 
{
	const char *command;
	int sts;

	if (!PyArg_ParseTuple(args, "s", &command))
		return NULL;

	sts = system(command);
	return Py_BuildValue("i", sts);
}


static PyMethodDef cbucho_methods[] = {
	{"system", cbucho_system, METH_VARARGS,
	 "execute a shell command"},
	{NULL, NULL, 0, NULL}
};



void
initcbucho(void)
{
    PyImport_AddModule("xyzzy");
    Py_InitModule("cbucho", cbucho_methods);
}

