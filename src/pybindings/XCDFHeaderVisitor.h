/*!
 * @file XCDFHeaderVisitor.h
 * @author Segev BenZvi
 * @date 6 Dec 2013
 * @brief An XCDF field visitor which stuffs field information into a string
 *        buffer.
 * @version $Id: XCDFHeaderVisitor.h 18150 2013-12-06 21:43:35Z sybenzvi $
 */

#ifndef XCDFHEADERVISITOR_H_INCLUDED
#define XCDFHEADERVISITOR_H_INCLUDED

#include <xcdf/XCDFFile.h>

#include <iomanip>
#include <sstream>

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

#endif // XCDFHEADERVISITOR_H_INCLUDED

