#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logrecord.hxx"



int Formatter_init(Formatter *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = Py_None, *dateFmt = Py_None;
    char style = '%';
    bool validate = true;
    static char *kwlist[] = {"fmt", "datefmt", "style", "validate", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOCp", kwlist, &fmt, &dateFmt, &style, &validate))
        return -1;

    PyObject* styleType = nullptr;

    switch (style){
        case '%':
            /* Call the class object. */
            styleType = (PyObject*)&PercentStyleType;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unsupported style");
            return -1;
    }

    PyObject * styleCls = PyObject_CallFunctionObjArgs(styleType, fmt, NULL);
    if (PyErr_Occurred()){ // Got exception in PercentStyle.__init__()
        return -1;
    }
    if (styleCls == nullptr){
        PyErr_Format(PyExc_ValueError, "Could not initialize Style formatter class.");
        return -1;
    }

    self->style = styleCls;
    Py_INCREF(self->style);

    self->fmt = ((PercentStyle*)(self->style))->fmt;
    Py_INCREF(fmt);

    self->dateFmt = dateFmt;
    Py_INCREF(dateFmt);

    if (validate){
        if (PyObject_CallMethod(self->style, "validate", NULL) == nullptr){
            Py_DECREF(self->style);
            Py_DECREF(fmt);
            Py_DECREF(dateFmt);
            return -1;
        }
    }

    return 0;
}

PyObject* Formatter_format(Formatter *self, PyObject *record){
    if (LogRecord_CheckExact(record)){
        LogRecord* logRecord = (LogRecord*)record;
        LogRecord_getMessage(logRecord);
        if (PercentStyle_CheckExact(self->style)){
            return PercentStyle_format((PercentStyle*)self->style, record);
        } else {
            return PyObject_CallMethodObjArgs(self->style, PyUnicode_FromString("format"), record, NULL);
            // TODO : format exc_info, exc_text and stack_info.
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be a LogRecord");
        return nullptr;
    }
    return Py_None; // TODO: Implement.
}

PyObject* Formatter_dealloc(Formatter *self) {
    Py_XDECREF(self->style);
    Py_XDECREF(self->fmt);
    Py_XDECREF(self->dateFmt);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

static PyMethodDef Formatter_methods[] = {
    {"format", (PyCFunction)Formatter_format, METH_O, "Format record into log event string"},
    {NULL}
};

static PyMemberDef Formatter_members[] = {
    {"_fmt", T_OBJECT_EX, offsetof(Formatter, fmt), 0, "Format string"},
    {"_style", T_OBJECT_EX, offsetof(Formatter, style), 0, "String style formatter"},
    {"datefmt", T_OBJECT_EX, offsetof(Formatter, dateFmt), 0, "Date format string"},
    {NULL}
};

PyTypeObject FormatterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "picologging.Formatter",
    .tp_basicsize = sizeof(Formatter),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)Formatter_dealloc,
    .tp_repr = PyObject_Repr,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PyDoc_STR("Formatter for log records."),
    .tp_methods = Formatter_methods,
    .tp_members = Formatter_members,
    .tp_init = (initproc)Formatter_init,
    .tp_new = PyType_GenericNew,
};