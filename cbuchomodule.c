#include <Python.h>

/* module Functions */
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


static PyObject *
cbucho_show(PyObject *self)
{
    return PyString_FromString("show\n");
}


static PyMethodDef cbucho_methods[] = {
    {"system", cbucho_system, METH_VARARGS,
     "execute a shell command"},
    {"show", cbucho_show, METH_NOARGS,
     "show"},
    {NULL, NULL},
};



void
initcbucho(void)
{
    Py_InitModule("cbucho", cbucho_methods);
}

