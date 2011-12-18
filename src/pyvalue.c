//The 
#include "pyvalue.h"

//Type define here
PyTypeObject pyvpi_value_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyvpi._Value",             /*tp_name*/
    sizeof(s_pyvpi_value),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)pyvpi_value_Dealloc, /*tp_dealloc*/    
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "pyvpi value objects",     /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    pyvpi_value_methods,       /* tp_methods */
    pyvpi_value_members,       /* tp_members */
    pyvpi_value_getsets,       /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc) pyvpi_value_Init, /* tp_init */
    0,                         /* tp_alloc */
    pyvpi_value_New,           /* tp_new */
};

void pyvpi_value_Dealloc(p_pyvpi_value self)
{
    //Free self.
#ifdef PYVPI_DEBUG
    vpi_printf("[PYVPI_DEBUG] pyvpi._Value is release,ptr is 0x%x.\n",self);
#endif
    if(self->obj != NULL) Py_XDECREF(self->obj);
    self->ob_type->tp_free((PyObject*)self);
}

int  pyvpi_value_Init(s_pyvpi_value *self, PyObject *args, PyObject *kwds)
{    
    static char *kwlist[] = {"format", NULL};
    
    self->_vpi_value.format = vpiHexStrVal; //Default value.
    Py_DECREF(self->obj);
    self->obj    = PyString_FromString("");
    self->_vpi_value.value.str = PyString_AsString(self->obj);
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
                                      &self->_vpi_value.format))
        return -1;
    Py_DECREF(self->obj);
#ifdef PYVPI_DEBUG
    vpi_printf("[PYVPI_DEBUG] pyvpi._Value is Initial,format is 0x%x.\n",self->_vpi_value.format);
#endif
    return update_format(self,self->_vpi_value.format,NULL);
}

PyObject * pyvpi_value_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    p_pyvpi_value   self;
    int i = 0;
    self = (p_pyvpi_value)type->tp_alloc(type, 0);
    if(self == NULL) 
        return NULL;
    Py_INCREF(Py_None);
    self-> obj = Py_None;
#ifdef PYVPI_DEBUG
    vpi_printf("[PYVPI_DEBUG] pyvpi._Value is allocate,ptr is <0x%x>, type ptr is <0x%x>.\n",self,type);
#endif 
    return (PyObject *) self;
}

//Get/Set
PyObject * s_pyvpi_value_getvalue(s_pyvpi_value *self, void *closure)
{
    Py_INCREF(self->obj);
    return self->obj;
}
int        s_pyvpi_value_setvalue(s_pyvpi_value *self, PyObject *value, void *closure)
{
    PyObject * tmp;
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot set index to NULL.");
        return -1;
    }
    Py_INCREF(self->obj);
    tmp = self->obj;
    Py_INCREF(value);
    Py_DECREF(self->obj);
    if(update_format(self,self->_vpi_value.format,value) == -1){
        Py_DECREF(value);
        self->obj = tmp;
        return -1;
    }
    Py_DECREF(tmp);
    return 0;
}

/* Update vpi value by vpi value structure.
 * If this format if vpiVectorVal, use must support the vector size.
 * This function must deal with the reference, this is different with
 * update_format.
 */
void update_value(s_pyvpi_value *self, s_vpi_value *ovalp, PLI_INT32 blen)
{
    p_pyvpi_vector pvector;
    p_pyvpi_time   ptime;
    p_pyvpi_strengthval   pstrength;
    PLI_INT32   numvals,i;
    PyObject*   tmpobj;
    PyObject*   dictobj;
    Py_INCREF(self->obj);
    tmpobj = self->obj;
    Py_DECREF(self->obj);
    self->obj = NULL;
    self->_vpi_value.format    = ovalp->format;
    switch (ovalp->format) {
    /* all string values */
    case vpiBinStrVal:
    case vpiOctStrVal:
    case vpiDecStrVal:
    case vpiHexStrVal:
    case vpiStringVal:
        if(self->obj == NULL)
            self->obj    = PyString_FromString(ovalp->value.str);
        self->_vpi_value.value.str = PyString_AsString(self->obj);
        break;
    case vpiScalarVal:
        self->obj    = PyInt_FromLong(ovalp->value.scalar);
        self->_vpi_value.value.scalar = PyInt_AsLong(self->obj);
        break;
    case vpiIntVal:
        self->obj    = PyInt_FromLong(ovalp->value.integer);
        self->_vpi_value.value.integer = PyInt_AsLong(self->obj);
        break;
    case vpiRealVal:
        self->obj    = PyFloat_FromDouble(ovalp->value.real);
        self->_vpi_value.value.real = PyFloat_AsDouble(self->obj);
        break;
    case vpiVectorVal:
        self->obj    = pyvpi_vector_New(&pyvpi_vector_Type,PyTuple_New(0),PyDict_New());
        dictobj = PyDict_New();
        PyDict_SetItem(dictobj,PyString_FromString("size"),PyInt_FromLong(blen));
        pyvpi_vector_Init((p_pyvpi_vector)self->obj,PyTuple_New(0),dictobj);    //TBD check error.
        pvector = (p_pyvpi_vector) self->obj;
        self->_vpi_value.value.vector = pvector->_vpi_vector;
        numvals = (blen + 31) >> 5;
        for(i=0; i <numvals; i++)
        {
            pvector->_vpi_vector[i].aval = ovalp->value.vector[i].aval;
            pvector->_vpi_vector[i].bval = ovalp->value.vector[i].bval;
        }
        break;
    case vpiStrengthVal:
        self->obj    = pyvpi_strengthval_New(&pyvpi_strengthval_Type,PyTuple_New(0),PyDict_New());
        pyvpi_strengthval_Init((p_pyvpi_strengthval)self->obj,PyTuple_New(0),PyDict_New());        
        pstrength = (p_pyvpi_strengthval) self->obj;
        self->_vpi_value.value.strength = &pstrength->_vpi_strengthval;
        
        pstrength->_vpi_strengthval.logic = ovalp->value.strength->logic;
        pstrength->_vpi_strengthval.s0 = ovalp->value.strength->s0;
        pstrength->_vpi_strengthval.s1 = ovalp->value.strength->s1;
        break;
    case vpiTimeVal:
        self->obj    = pyvpi_time_New(&pyvpi_time_Type,PyTuple_New(0),PyDict_New());
        pyvpi_time_Init((p_pyvpi_time)self->obj,PyTuple_New(0),PyDict_New());
        ptime = (p_pyvpi_time) self->obj;
        self->_vpi_value.value.time = &ptime->_vpi_time;
        
        ptime->_vpi_time.type = ovalp->value.time->type;
        ptime->_vpi_time.high = ovalp->value.time->high;
        ptime->_vpi_time.low = ovalp->value.time->low;
        ptime->_vpi_time.real = ovalp->value.time->real;
        break;
        /* not sure what to do here? */
    case vpiObjTypeVal: case vpiSuppressVal:
        break;
    }
    Py_DECREF(tmpobj);
    return ;
}

/* Update format 
 * User can update format and object here, before call this function, all reference must
 * be care processed.
 */
static PLI_INT32 update_format(p_pyvpi_value self, PLI_INT32 nformat, PyObject* nobj)
{
    p_pyvpi_vector pvector;
    p_pyvpi_time   ptime;
    p_pyvpi_strengthval   pstrength;
    self->obj = nobj;
    self->_vpi_value.format    = nformat;
    switch (nformat) {
    /* all string values */
    case vpiBinStrVal:
    case vpiOctStrVal:
    case vpiDecStrVal:
    case vpiHexStrVal:
    case vpiStringVal:
        if(self->obj == NULL)
            self->obj    = PyString_FromString("");
        if (! PyString_Check(self->obj)) {
                PyErr_SetString(PyExc_TypeError,
                                "The value must be a string<Current format is string>.");
                return -1;
            }
        self->_vpi_value.value.str = PyString_AsString(self->obj);
        break;
    case vpiScalarVal:
        if(self->obj == NULL)
            self->obj    = PyInt_FromLong(vpi0);
        if (! PyInt_Check(self->obj) || PyInt_AS_LONG(self->obj) < 0 || PyInt_AS_LONG(self->obj) > 3) {
            PyErr_SetString(PyExc_TypeError,
                            "The value must be vpi[0,1,X,Z]<Current format is vpiScalarVal>.");
            return -1;
        }
        self->_vpi_value.value.scalar = PyInt_AsLong(self->obj);
        break;
    case vpiIntVal:
        if(self->obj == NULL)
            self->obj    = PyInt_FromLong(0);
        if (! PyInt_Check(self->obj)) {
                PyErr_SetString(PyExc_TypeError,
                                "The value must be an int<Current format is vpiIntVal>.");
                return -1;
            }
        self->_vpi_value.value.integer = PyInt_AsLong(self->obj);
        break;
    case vpiRealVal:
        if(self->obj == NULL)
            self->obj    = PyFloat_FromDouble(0.0);
        if (! PyFloat_Check(self->obj)) {
            PyErr_SetString(PyExc_TypeError,
                            "The value must be an float<Current format is vpiRealVal>.");
            return -1;
        }
        self->_vpi_value.value.real = PyFloat_AsDouble(self->obj);
        break;
    case vpiVectorVal:
        if(self->obj == NULL) {
            self->obj    = pyvpi_vector_New(&pyvpi_vector_Type,PyTuple_New(0),PyDict_New());
            pyvpi_vector_Init((p_pyvpi_vector)self->obj,PyTuple_New(0),PyDict_New());   //TBD check error
        }
        if (! PyObject_TypeCheck(self->obj,&pyvpi_vector_Type)) {
            PyErr_SetString(PyExc_TypeError,
                            "The value object be a pyvpi._Vector<Current format is vpiVectorVal>.");
            return -1;
        }
        pvector = (p_pyvpi_vector) self->obj;
        self->_vpi_value.value.vector = pvector->_vpi_vector;
        break;
    case vpiStrengthVal:
        if(self->obj == NULL) {
            self->obj    = pyvpi_strengthval_New(&pyvpi_strengthval_Type,PyTuple_New(0),PyDict_New());
            pyvpi_strengthval_Init((p_pyvpi_strengthval)self->obj,PyTuple_New(0),PyDict_New());
        }
        if (! PyObject_TypeCheck(self->obj,&pyvpi_strengthval_Type)) {
            PyErr_SetString(PyExc_TypeError,
                            "The value object be a pyvpi._Strength<Current format is vpiStrengthVal>.");
            return -1;
        }
        pstrength = (p_pyvpi_strengthval) self->obj;
        self->_vpi_value.value.strength = &pstrength->_vpi_strengthval;
        break;
    case vpiTimeVal:
        if(self->obj == NULL) {
            self->obj    = pyvpi_time_New(&pyvpi_time_Type,PyTuple_New(0),PyDict_New());
            pyvpi_time_Init((p_pyvpi_time)self->obj,PyTuple_New(0),PyDict_New());
        }
        if (! PyObject_TypeCheck(self->obj,&pyvpi_time_Type)) {
            PyErr_SetString(PyExc_TypeError,
                            "The value object be a pyvpi._Time<Current format is vpiTimeVal>.");
            return -1;
        }
        ptime = (p_pyvpi_time) self->obj;
        self->_vpi_value.value.time = &ptime->_vpi_time;
        break;
        /* not sure what to do here? */
    case vpiObjTypeVal: case vpiSuppressVal:
        break;
    }
    return 0;
}

