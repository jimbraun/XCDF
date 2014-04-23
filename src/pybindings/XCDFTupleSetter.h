/*!
 * @file XCDFTupleSetter.h
 * @author Segev BenZvi
 * @date 6 Dec 2013
 * @brief A field visitor which inserts XCDF data into a python tuple.
 * @version $Id: XCDFTupleSetter.h 18498 2014-01-24 00:42:11Z sybenzvi $
 */

#ifndef XCDFTUPLESETTER_H_INCLUDED
#define XCDFTUPLESETTER_H_INCLUDED

#include <xcdf/XCDFFile.h>
#include <XCDFTypeConversion.h>

#include <Python.h>

/*!
 * @class TupleSetter
 * @brief A field visitor which stuffs XCDF data into a python tuple.
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
      else {
        Py_INCREF(Py_None);
        err = PyTuple_SetItem(tuple_, (ifield_++ % nfields_), Py_None);
      }
    }

    PyObject* GetTuple() const { return tuple_; }

    int GetNFields() const { return nfields_; }

  private:

    int nfields_;
    int ifield_;
    PyObject* tuple_;

};

#endif // XCDFTUPLESETTER_H_INCLUDED

