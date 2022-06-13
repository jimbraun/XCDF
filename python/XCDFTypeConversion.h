/*!
 * @file XCDFTypeConversion.h
 * @author James Braun, Segev BenZvi
 * @date 6 Dec 2013
 * @brief Define functions to convert XCDF types to PyObjects.
 * @version $Id: XCDFTypeConversion.h 18498 2014-01-24 00:42:11Z sybenzvi $
 */

#ifndef XCDFTYPECONVERSION_H_INCLUDED
#define XCDFTYPECONVERSION_H_INCLUDED

#include <Python.h>

#include <xcdf/XCDFFile.h>

namespace {
/*!
 * @brief XCDF type to python type converter.
 */

template <typename T, typename Convert>
PyObject* __xcdf2python(const T& f, Convert conv)
{
  const size_t n = f.GetSize();
  PyObject* result = NULL;
  int err;

  if (n > 0) {
    if (n > 1) {
      result = PyTuple_New(n);
      for (size_t i = 0; i < n; ++i) {
        err = PyTuple_SetItem(result, i, conv(f[i]));
        if (err) {
          Py_DECREF(result);
          return NULL;
        }
      }
    }
    else
      result = conv(*f);
  }

  return result;
}

}

template <typename T>
PyObject* xcdf2python(const T& f);

template<>
PyObject* xcdf2python(const ConstXCDFUnsignedIntegerField& f) {
  return __xcdf2python(f, PyLong_FromUnsignedLong);
}

template<>
PyObject* xcdf2python(const XCDFUnsignedIntegerField& f) {
  return __xcdf2python(f, PyLong_FromUnsignedLong);
}

template<>
PyObject* xcdf2python(const ConstXCDFSignedIntegerField& f) {
  return __xcdf2python(f, PyLong_FromLong);
}

template<>
PyObject* xcdf2python(const XCDFSignedIntegerField& f) {
  return __xcdf2python(f, PyLong_FromLong);
}

template<>
PyObject* xcdf2python(const ConstXCDFFloatingPointField& f) {
  return __xcdf2python(f, PyFloat_FromDouble);
}

template<>
PyObject* xcdf2python(const XCDFFloatingPointField& f) {
  return __xcdf2python(f, PyFloat_FromDouble);
}

#endif // XCDFTYPECONVERSION_H_INCLUDED

