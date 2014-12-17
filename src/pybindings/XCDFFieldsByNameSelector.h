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
#include <xcdf/XCDFField.h>
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

    FieldsByNameSelector(const std::string& names,
                         const XCDFFile& f) :
      nfields_(0)
    {
      // Parse name list assuming comma-separated values.
      size_t start = 0;
      size_t end = 0;

      std::vector<std::string> fieldNames;
      while (end != std::string::npos) {
        end = names.find(",", start);


        fieldNames.push_back(
          Trim(names.substr(
            start, end==std::string::npos ? std::string::npos : end-start))
        );

        start = ((end > (std::string::npos - 1)) ? std::string::npos :
                                                   end + 1);
      }

      nfields_ = fieldNames.size();
      for (int i = 0; i < nfields_; ++i) {

        if (f.IsUnsignedIntegerField(fieldNames[i])) {
          unsignedFields_.push_back(f.GetUnsignedIntegerField(fieldNames[i]));
          unsignedIndices_.push_back(i);
        } else if (f.IsSignedIntegerField(fieldNames[i])) {
          signedFields_.push_back(f.GetSignedIntegerField(fieldNames[i]));
          signedIndices_.push_back(i);
        } else {
          // This will throw XCDFException if the field does not exist
          floatFields_.push_back(f.GetFloatingPointField(fieldNames[i]));
          floatIndices_.push_back(i);
        }
      }
    }

    PyObject* GetTuple() const {
      PyObject* tuple = PyTuple_New(nfields_);
      int err = 0;
      try {
        for (int i = 0; i < unsignedFields_.size(); ++i) {
          err = Insert(unsignedFields_[i], unsignedIndices_[i], tuple);
        }
        for (int i = 0; i < signedFields_.size(); ++i) {
          err = Insert(signedFields_[i], signedIndices_[i], tuple);
        }
        for (int i = 0; i < floatFields_.size(); ++i) {
          err = Insert(floatFields_[i], floatIndices_[i], tuple);
        }
      } catch (const XCDFException& e) {
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_IOError, e.GetMessage().c_str());
        return NULL;
      }
      return tuple;
    }

    int GetNFields() const { return nfields_; }

  private:

    int nfields_;
    std::vector<XCDFUnsignedIntegerField> unsignedFields_;
    std::vector<XCDFSignedIntegerField> signedFields_;
    std::vector<XCDFFloatingPointField> floatFields_;
    std::vector<unsigned> unsignedIndices_;
    std::vector<unsigned> signedIndices_;
    std::vector<unsigned> floatIndices_;

    std::string Trim(const std::string& s, const std::string& ws=" \t")
    {
      const size_t begin = s.find_first_not_of(ws);
      if (begin == std::string::npos)
        return "";
     
      const size_t end = s.find_last_not_of(ws);
      const size_t range = end - begin + 1;
      return s.substr(begin, range);
    }

    template <typename T>
    int Insert(T& field, unsigned idx, PyObject* tuple) const {
      int err = 0;
      if (field.GetSize() > 0) {
        PyObject* result(xcdf2python(field));
        err = PyTuple_SetItem(tuple, idx, result);
      } else {
        Py_INCREF(Py_None);
        err = PyTuple_SetItem(tuple, idx, Py_None);
      }
      return err;
    }

};

#endif // XCDFFIELDSBYNAMESELECTOR_H_INCLUDED

