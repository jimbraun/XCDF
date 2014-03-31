/*!
 * @file PyXCDF.cc
 * @author Segev BenZvi
 * @date 30 May 2013
 * @brief Define python bindings to XCDF functions using the python C API.
 * @version $Id: PyXCDF.cc 17994 2013-11-23 02:27:32Z sybenzvi $
 */

#include <xcdf/XCDFFile.h>

#include <Python.h>
#include <structmember.h>

#include <iomanip>
#include <sstream>

// _____________________________________________
// Functions to convert XCDF types to PyObjects \_______________________________
template<typename T>
PyObject* xcdf2python(const T& value);

template<>
PyObject* xcdf2python(const XCDFUnsignedIntegerField& f)
{
  const size_t n = f.GetSize();
  PyObject* result = NULL;
  int err;

  if (n > 0) {
    if (n > 1) {
      result = PyTuple_New(n);
      for (size_t i = 0; i < n; ++i) {
        err = PyTuple_SetItem(result, i, PyLong_FromUnsignedLong(f[i]));
        if (err) {
          Py_DECREF(result);
          return NULL;
        }
      }
    }
    else
      result = PyLong_FromUnsignedLong(*f);
  }
  return result;
}

template<>
PyObject* xcdf2python(const XCDFSignedIntegerField& f)
{
  const size_t n = f.GetSize();
  PyObject* result = NULL;
  int err;

  if (n > 0) {
    if (n > 1) {
      result = PyTuple_New(n);
      for (size_t i = 0; i < n; ++i) {
        err = PyTuple_SetItem(result, i, PyLong_FromLong(f[i]));
        if (err) {
          Py_DECREF(result);
          return NULL;
        }
      }
    }
    else
      result = PyLong_FromLong(*f);
  }
  return result;
}

template<>
PyObject* xcdf2python(const XCDFFloatingPointField& f)
{
  const size_t n = f.GetSize();
  PyObject* result = NULL;
  int err;

  if (n > 0) {
    if (n > 1) {
      result = PyTuple_New(n);
      for (size_t i = 0; i < n; ++i) {
        err = PyTuple_SetItem(result, i, PyFloat_FromDouble(f[i]));
        if (err) {
          Py_DECREF(result);
          return NULL;
        }
      }
    }
    else
      result = PyFloat_FromDouble(*f);
  }
  return result;
}

/*!
 * @class TupleSetter
 * @brief A field visitor which stuffs XCDF data into a python tuple
 */
class TupleSetter {

  public:

    TupleSetter(const int nfields) :
      nfields_(nfields),
      ifield_(0),
      tuple_(PyTuple_New(nfields))
    { }

    template<typename T>
    void operator()(const XCDFField<T>& field) {
      size_t n = field.GetSize();
      int err = 0;
      if (n > 0) {
        PyObject* result(xcdf2python(field));
        err = PyTuple_SetItem(tuple_, (ifield_++ % nfields_), result);
      }
    }

    PyObject* GetTuple() const { return tuple_; }

    int GetNFields() const { return nfields_; }

  private:

    int nfields_;
    int ifield_;
    PyObject* tuple_;

};

/*!
 * @class HeaderVisitor
 * @brief A field visitor which stores header information into a string buffer
 */
class HeaderVisitor {

  public:

    HeaderVisitor(const XCDFFile& f, std::stringstream& ostr) :
      file_(f),
      isFirst_(true),
      ostr_(ostr)
    { }

    template<typename T>
    void operator()(const XCDFField<T>& field) {
      if (isFirst_) {
        ostr_ << std::left << std::setw(28) << "Field" << " "
              << std::setw(20) << "Type"
              << std::right << std::setw(10) << "Resolution" << "   "
              << "Parent" << std::endl;
        ostr_ << std::left << std::setw(28) << "-----" << " "
              << std::setw(20) << "----"
              << std::right << std::setw(10) << "----------" << "   "
              << "------" << std::endl;
        isFirst_ = false;
      }

      ostr_ << std::left << std::setw(28) << field.GetName() << " ";

      if (file_.IsUnsignedIntegerField(field.GetName()))
        ostr_ << std::setw(20) << "Unsigned Integer";
      else if (file_.IsSignedIntegerField(field.GetName()))
        ostr_ << std::setw(20) << "Signed Integer";
      else
        ostr_ << std::setw(20) << "Floating Point";

      ostr_ << std::right << std::setw(10) << field.GetResolution();

      if (file_.IsVectorField(field.GetName()))
        ostr_ << "   " << file_.GetFieldParentName(field.GetName());
      ostr_ << std::endl;
    }
    
  private:

    const XCDFFile& file_;
    bool isFirst_;
    std::stringstream& ostr_;

};

// ___________________________________
// Expose parts of XCDFFile to python \_________________________________________
typedef struct {
  PyObject_HEAD
  PyObject* filename_;    // XCDF filename (a python string)
  XCDFFile* file_;        // an XCDF file instance
} pyxcdf_XCDFFile;


// Python XCDFFile allocator
static PyObject*
XCDFFile_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
  pyxcdf_XCDFFile* self = (pyxcdf_XCDFFile*)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->filename_ = PyString_FromString("");
    if (self->filename_ == NULL) {
      Py_DECREF(self);
      return NULL;
    }
    self->file_ = NULL;
  }
  return (PyObject*)self;
}

// Python XCDFFile initializer (from filename argument)
static int
XCDFFile_init(pyxcdf_XCDFFile* self, PyObject* args)
{
  PyObject* filename = NULL;
  PyObject* tmp;

  static const char* format = "|S";

  if (!PyArg_ParseTuple(args, format, &filename))
    return -1;

  if (filename) {
    tmp = self->filename_;
    Py_INCREF(filename);
    self->filename_ = filename;
    Py_XDECREF(tmp);

    self->file_ = new XCDFFile(PyString_AsString(filename), "R");
  }

  return 0;
}

// Python XCDFFile deallocator
static void
XCDFFile_dealloc(pyxcdf_XCDFFile* self)
{
  Py_XDECREF(self->filename_);
  if (self->file_)
    delete self->file_;
  self->ob_type->tp_free((PyObject*)self);
}

// Member definitions for XCDFFile object
static PyMemberDef XCDFFile_members[] =
{
  { const_cast<char*>("filename"), T_OBJECT_EX,
    offsetof(pyxcdf_XCDFFile, filename_), 0,
    const_cast<char*>("XCDF file name") },

  { NULL }
};

// _______________________
// XCDFFile data iterator \_____________________________________________________
typedef std::vector<XCDFFieldDescriptor>::const_iterator XCDFDescriptorIterator;

typedef struct {
  PyObject_HEAD
  XCDFFile* file_;                      // pointer to current open XCDF file
  int iCurrent_;                        // current record being read
  int iTotal_;                          // total number of records in file
} XCDFRecordIterator;

// Define __iter__()
PyObject*
XCDFRecordIterator_iter(PyObject* self)
{
  Py_INCREF(self);
  return self;
}

// Define next() for iteration over XCDF records
PyObject*
XCDFRecordIterator_iternext(PyObject* self)
{
  XCDFRecordIterator* p = (XCDFRecordIterator*)self;

  // If we have not reached the end of the file:
  if (p->iCurrent_ < p->iTotal_ && p->file_->Read() > 0)
  {
    p->iCurrent_ = p->file_->GetCurrentEventNumber();
    if (p->iCurrent_ < 0) {
      PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }

    // Create a field visitor to stuff data into a tuple
    TupleSetter tsetter(p->file_->GetNFields());
    p->file_->ApplyFieldVisitor(tsetter);

    PyObject* result(tsetter.GetTuple());
    return result;
  }
  // When reaching EOF, rewind the XCDF file and stop the iterator
  else {
    p->file_->Rewind();
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}

// Definition of the record iterator type for python
static PyTypeObject
XCDFRecordIteratorType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyxcdf._iter",                             // tp_name
    sizeof(XCDFRecordIterator),                 // tp_basicsize
    0,                                          // tp_itemsize
    0,                                          // tp_dealloc
    0,                                          // tp_print
    0,                                          // tp_getattr
    0,                                          // tp_setattr
    0,                                          // tp_reserved
    0,                                          // tp_repr
    0,                                          // tp_as_number
    0,                                          // tp_as_sequence
    0,                                          // tp_as_mapping
    0,                                          // tp_hash
    0,                                          // tp_call
    0,                                          // tp_str
    0,                                          // tp_getattro
    0,                                          // tp_setattro
    0,                                          // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  // use tp_iter and tp_iternext
    "Internal record iterator object",          // tp_doc
    0,                                          // tp_traverse
    0,                                          // tp_clear
    0,                                          // tp_richcompare
    0,                                          // tp_weaklistoffset
    (getiterfunc)XCDFRecordIterator_iter,       // tp_iter
    (iternextfunc)XCDFRecordIterator_iternext,  // tp_iternext
};

// ________________________
// XCDFFile field iterator \____________________________________________________
typedef struct {
  PyObject_HEAD
  XCDFFile* file_;                      // pointer to current open XCDF file
  int iCurrent_;                        // current record being read
  int iTotal_;                          // total number of records in file
  std::string fieldName_;               // name of the field to extract
} XCDFFieldIterator;

// Define __iter__()
PyObject*
XCDFFieldIterator_iter(PyObject* self)
{
  Py_INCREF(self);
  return self;
}

// Define next() for iteration over XCDF field inside XCDF records
PyObject*
XCDFFieldIterator_iternext(PyObject* self)
{
  XCDFFieldIterator* p = (XCDFFieldIterator*)self;

  // If we have not reached the end of the file:
  if (p->iCurrent_ < p->iTotal_ && p->file_->Read() > 0)
  {
    p->iCurrent_ = p->file_->GetCurrentEventNumber();
    if (p->iCurrent_ < 0) {
      PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }

    // If field name is not correct
    if (!p->file_->HasField(p->fieldName_)) {
      std::stringstream errMsg;
      errMsg << "Field \"" << p->fieldName_.c_str() << "\" not found."
             << std::endl;
      PyErr_SetString(PyExc_LookupError, errMsg.str().c_str());
      return NULL;
    }

    // Get the field descriptor
    XCDFDescriptorIterator it;
    for (it = p->file_->FieldDescriptorsBegin();
         it != p->file_->FieldDescriptorsEnd(); ++it)
    {
      if (it->name_ == p->fieldName_)
        break;
    }

    // Pack the data into the result
    PyObject* result = NULL;

    switch (it->type_) {
      case XCDF_UNSIGNED_INTEGER:
        // Pack vector fields into a tuple
        if (p->file_->IsVectorField(it->name_)) {
          XCDFUnsignedIntegerField f = 
            p->file_->GetUnsignedIntegerField(it->name_);
          uint32_t n = f.GetSize();
          result = PyTuple_New(n);
          for (uint32_t j = 0; j < n; ++j)
            PyTuple_SetItem(result, j, PyInt_FromLong(f[j]));
        }
        // Else just return a single integer
        else {
          result =
            PyInt_FromLong(*(p->file_->GetUnsignedIntegerField(it->name_)));
        }
        break;
      case XCDF_SIGNED_INTEGER:
        // Pack vector fields into a tuple
        if (p->file_->IsVectorField(it->name_)) {
          XCDFSignedIntegerField f = 
            p->file_->GetSignedIntegerField(it->name_);
          uint32_t n = f.GetSize();
          result = PyTuple_New(n);
          for (uint32_t j = 0; j < n; ++j)
            PyTuple_SetItem(result, j, PyInt_FromLong(f[j]));
        }
        // Else just return a single integer
        else {
          result =
            PyInt_FromLong(*(p->file_->GetSignedIntegerField(it->name_)));
        }
        break;
      case XCDF_FLOATING_POINT:
        // Pack vector fields into a tuple
        if (p->file_->IsVectorField(it->name_)) {
          XCDFSignedIntegerField f = 
            p->file_->GetSignedIntegerField(it->name_);
          uint32_t n = f.GetSize();
          result = PyTuple_New(n);
          for (uint32_t j = 0; j < n; ++j)
            PyTuple_SetItem(result, j, PyInt_FromLong(f[j]));
        }
        // Else just return a single integer
        else {
          result =
            PyFloat_FromDouble(*(p->file_->GetFloatingPointField(it->name_)));
        }
        break;
      default:
        break;
    }

    return result;
  }
  // When reaching EOF, rewind the XCDF file and stop the iterator
  else {
    p->file_->Rewind();
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}

// Definition of the field iterator type for python
static PyTypeObject
XCDFFieldIteratorType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyxcdf._iter",                             // tp_name
    sizeof(XCDFFieldIterator),                  // tp_basicsize
    0,                                          // tp_itemsize
    0,                                          // tp_dealloc
    0,                                          // tp_print
    0,                                          // tp_getattr
    0,                                          // tp_setattr
    0,                                          // tp_reserved
    0,                                          // tp_repr
    0,                                          // tp_as_number
    0,                                          // tp_as_sequence
    0,                                          // tp_as_mapping
    0,                                          // tp_hash
    0,                                          // tp_call
    0,                                          // tp_str
    0,                                          // tp_getattro
    0,                                          // tp_setattro
    0,                                          // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,  // use tp_iter and tp_iternext
    "Internal field iterator object",           // tp_doc
    0,                                          // tp_traverse
    0,                                          // tp_clear
    0,                                          // tp_richcompare
    0,                                          // tp_weaklistoffset
    (getiterfunc)XCDFFieldIterator_iter,        // tp_iter
    (iternextfunc)XCDFFieldIterator_iternext,   // tp_iternext
};

// __________________________
// XCDFFile member functions \__________________________________________________

// Function to print header information from the file
static PyObject*
XCDFFile_header(pyxcdf_XCDFFile* self)
{
  // Make sure the XCDF file is valid
  if (self->file_ == NULL) {
    PyErr_SetString(PyExc_AttributeError, "file: not open");
    return NULL;
  }

  try {
    // Create a field visitor to stuff header data into a string buffer
    std::stringstream ostr;

    HeaderVisitor hvisitor(*(self->file_), ostr);
    self->file_->ApplyFieldVisitor(hvisitor);

    PyObject* result = PyString_FromString(ostr.str().c_str());
    return result;
  }
  catch (const XCDFException& e) {
    PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
    return NULL;
  }
}

// Function to set up access to the record iterator
static PyObject*
XCDFRecord_iterator(pyxcdf_XCDFFile* self)
{
  try {
    XCDFRecordIterator* it =
      PyObject_New(XCDFRecordIterator, &XCDFRecordIteratorType);

    if (!it) {
      std::cerr << "Record iterator is NULL" << std::endl;
      return NULL;
    }

    if (!PyObject_Init((PyObject*)it, &XCDFRecordIteratorType)) {
      std::cerr << "Record iterator could not be initialized" << std::endl;
      Py_DECREF(it);
      return NULL;
    }

    // Don't forget to update the reference count for the iterator
    Py_INCREF(it);

    it->file_ = self->file_;
    it->iCurrent_ = 0;
    it->iTotal_ = self->file_->GetEventCount();

    return (PyObject*)it;
  }
  catch (const XCDFException& e) {
    PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
    return NULL;
  }
}

// Function to set up access to field iterator
static PyObject*
XCDFField_iterator(pyxcdf_XCDFFile* self, PyObject* fieldName)
{
  try {
    XCDFFieldIterator* it =
      PyObject_New(XCDFFieldIterator, &XCDFFieldIteratorType);

    if (!it) {
      std::cerr << "Field iterator is NULL" << std::endl;
      return NULL;
    }

    if (!PyObject_Init((PyObject*)it, &XCDFFieldIteratorType)) {
      std::cerr << "Field iterator could not be initialized" << std::endl;
      Py_DECREF(it);
      return NULL;
    }

    // Don't forget to update the reference count for the iterator
    Py_INCREF(it);

    it->file_ = self->file_;
    it->iCurrent_ = 0;
    it->iTotal_ = self->file_->GetEventCount();
    it->fieldName_ = PyString_AsString(fieldName);

    return (PyObject*)it;
  }
  catch (const XCDFException& e) {
    PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
    return NULL;
  }
}

// Function to provide random access to a record in the file
static PyObject*
XCDFFile_getRecord(pyxcdf_XCDFFile* self, PyObject* recordId)
{
  try {
    // Seek to a given record ID in the file
    uint64_t id = PyInt_AsUnsignedLongLongMask(recordId);
    PyObject* result = NULL;

    if (self->file_->Seek(id)) {

      // Create a field visitor to stuff data into a tuple
      TupleSetter tsetter(self->file_->GetNFields());
      self->file_->ApplyFieldVisitor(tsetter);
      result = tsetter.GetTuple();

      self->file_->Rewind();
    }
    else {
      std::stringstream errMsg;
      errMsg << "Invalid event number " << id;
      PyErr_SetString(PyExc_LookupError, errMsg.str().c_str());
    }

    return result;
  }
  catch (const XCDFException& e) {
    PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
    return NULL;
  }
}

// Method definitions for XCDFFile object
static PyMethodDef XCDFFile_methods[] =
{
  { const_cast<char*>("header"), (PyCFunction)XCDFFile_header, METH_NOARGS,
    const_cast<char*>("Print XCDF file field data") },

  { const_cast<char*>("getRecord"), (PyCFunction)XCDFFile_getRecord, METH_O,
    const_cast<char*>("Get a record by number from the file") },

  { const_cast<char*>("records"), (PyCFunction)XCDFRecord_iterator, METH_NOARGS,
    const_cast<char*>("Iterator over XCDF records") },

  { const_cast<char*>("field"), (PyCFunction)XCDFField_iterator, METH_O,
    const_cast<char*>("Iterator over an XCDF field") },

  { NULL }
};

// _____________________
// XCDFFile get/setters \_______________________________________________________
static PyObject*
XCDFFile_getfilename(pyxcdf_XCDFFile* self, void* closure)
{
  Py_INCREF(self->filename_);
  return self->filename_;
}

static PyObject*
XCDFFile_getcount(pyxcdf_XCDFFile* self, void* closure)
{
  try {
    PyObject* tmp = Py_BuildValue("l", self->file_->GetEventCount());
    return tmp;
  }
  catch (const XCDFException& e) {
    PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
    return NULL;
  }
}

static PyObject*
XCDFFile_getnfields(pyxcdf_XCDFFile* self, void* closure)
{
  PyObject* tmp = Py_BuildValue("l", self->file_->GetNFields());
  Py_INCREF(tmp);
  return tmp;
}

static PyGetSetDef XCDFFile_getseters[] =
{
  { const_cast<char*>("filename"), (getter)XCDFFile_getfilename, NULL,
    const_cast<char*>("XCDF file name"),
    NULL },

  { const_cast<char*>("count"), (getter)XCDFFile_getcount, NULL,
    const_cast<char*>("XCDF record count"),
    NULL },

  { const_cast<char*>("nfields"), (getter)XCDFFile_getnfields, NULL,
    const_cast<char*>("Number of fields per record"),
    NULL },

  { NULL }
};

// ________________________________
// XCDFFile type object definition \____________________________________________
static PyTypeObject
pyxcdf_XCDFFileType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyxcdf.XCDFFile",                        // tp_name
    sizeof(pyxcdf_XCDFFile),                  // tp_basicsize
    0,                                        // tp_itemsize
    (destructor)XCDFFile_dealloc,             // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping
    0,                                        // tp_hash
    0,                                        // tp_call
    0,                                        // tp_str
    0,                                        // tp_getattro
    0,                                        // tp_setattro
    0,                                        // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "XCDFFile object",                        // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    XCDFFile_methods,                         // tp_methods
    XCDFFile_members,                         // tp_members
    XCDFFile_getseters,                       // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    (initproc)XCDFFile_init,                  // tp_init
    0,                                        // tp_alloc
    XCDFFile_new,                             // tp_new
};

static PyMethodDef pyxcdf_methods[] =
{
  { NULL }
};

// _________________________
// Python module definition \___________________________________________________
#if PY_MAJOR_VERSION >= 3
  static struct PyModuleDef pyxdfModule = {
    PyModuleDef_HEAD_INIT,
    "pyxcdf",                 // m_name
    modDoc,                   // m_doc
    -1,                       // m_size
  };

  PyMODINIT_FUNC
  PyInit_pyxcdf(void)
  {
    PyObject* module = PyModule_Create(&pyxcdfModule);
    if (!module)
      return NULL;
  }
#else
  PyMODINIT_FUNC
  initpyxcdf(void)
  {
    // Enable creation of new XCDFFile objects
    pyxcdf_XCDFFileType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyxcdf_XCDFFileType) < 0)
      return;

    PyObject* module = Py_InitModule3("pyxcdf", pyxcdf_methods,
                                      "Python bindings to XCDF library.");

    // Add XCDFFile type to the module dictionary
    Py_INCREF(&pyxcdf_XCDFFileType);
    PyModule_AddObject(module, "XCDFFile", (PyObject*)&pyxcdf_XCDFFileType);
  }
#endif

