/*!
 * @file XCDFFieldsByNameSelector.h
 * @author Segev BenZvi
 * @date 6 Dec 2013
 * @brief A field visitor which inserts XCDF data into a python tuple using a
 *        comma-separated list of field names.
 * @version $Id: XCDFFieldsByNameSelector.h 18675 2014-02-03 01:37:10Z sybenzvi $
 */

#ifndef XCDFFIELDSBYNAMESELECTOR_H_INCLUDED
#define XCDFFIELDSBYNAMESELECTOR_H_INCLUDED

#include <xcdf/XCDFFile.h>
#include <XCDFTypeConversion.h>

#include <Python.h>

#include <algorithm>
#include <string>
#include <vector>

/*!
 * @class FieldsByNameSelector
 * @brief A field visitor which inserts XCDF data into a python tuple using a
 *        comma-separated list of field names.
 */
class FieldsByNameSelector {

  public:

    FieldsByNameSelector(const std::string& names) :
      nfields_(0)
    {
      // Parse name list assuming comma-separated values.
      size_t start = 0;
      size_t end = 0;

      while (end != std::string::npos) {
        end = names.find(",", start);

        fieldNames_.push_back(
          Trim(names.substr(
            start, end==std::string::npos ? std::string::npos : end-start))
        );

        start = ((end > (std::string::npos - 1)) ? std::string::npos :
                                                   end + 1);
      }

      nfields_ = fieldNames_.size();
      tuple_ = PyTuple_New(nfields_);
    }

    template<typename T>
    void operator()(const XCDFField<T>& field) {
      // Skip over fields not listed in the field list
      std::vector<std::string>::iterator i =
        std::find(fieldNames_.begin(), fieldNames_.end(), field.GetName());
      if (i == fieldNames_.end())
        return;

      int idx = std::distance(fieldNames_.begin(), i);

      size_t n = field.GetSize();
      int err = 0;
      if (n > 0) {
        PyObject* result(xcdf2python(field));
        err = PyTuple_SetItem(tuple_, idx, result);
      }
      else {
        Py_INCREF(Py_None);
        err = PyTuple_SetItem(tuple_, idx, Py_None);
      }
    }

    PyObject* GetTuple() const {
      for (int i = 0; i < nfields_; ++i) {
        if (PyTuple_GetItem(tuple_, i) == NULL) {
          Py_INCREF(Py_None);
          PyTuple_SetItem(tuple_, i, Py_None);
        }
      }
      return tuple_;
    }

    int GetNFields() const { return nfields_; }

  private:

    int nfields_;
    PyObject* tuple_;

    std::vector<std::string> fieldNames_;

    std::string Trim(const std::string& s, const std::string& ws=" \t")
    {
      const size_t begin = s.find_first_not_of(ws);
      if (begin == std::string::npos)
        return "";
     
      const size_t end = s.find_last_not_of(ws);
      const size_t range = end - begin + 1;
      return s.substr(begin, range);
    }

};

#endif // XCDFFIELDSBYNAMESELECTOR_H_INCLUDED

