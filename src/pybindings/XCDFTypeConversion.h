/*!
 * @file XCDFTypeConversion.h
 * @author Segev BenZvi
 * @date 6 Dec 2013
 * @brief Define functions to convert XCDF types to PyObjects.
 * @version $Id: XCDFTypeConversion.h 18498 2014-01-24 00:42:11Z sybenzvi $
 */

#ifndef XCDFTYPECONVERSION_H_INCLUDED
#define XCDFTYPECONVERSION_H_INCLUDED

#include <Python.h>

#include <xcdf/XCDFFile.h>

/*!
 * @brief XCDF type to python type converter.
 */
template<typename T>
PyObject* xcdf2python(const T& value);

/*!
 * @brief Template specialization: XCDF unsigned ints to to python unsigned
 *        long.
 */
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

/*!
 * @brief Template specialization: XCDF signed ints to to python signed long.
 */
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

/*!
 * @brief Template specialization: XCDF floats to to python double-precision
 *        floats.
 */
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

#endif // XCDFTYPECONVERSION_H_INCLUDED

