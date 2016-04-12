
/*
Copyright (c) 2016, James Braun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XCDF_FIELD_ALIAS_BASE_INCLUDED_H
#define XCDF_FIELD_ALIAS_BASE_INCLUDED_H

#include <xcdf/XCDFDefs.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFFieldAliasBase
 * @author Jim Braun
 * @brief Base class for XCDFField class aliases
 */

class XCDFFieldAliasBase {

  public:

    XCDFFieldAliasBase(const std::string& name,
                       const std::string& expression) :
                                              name_(name),
                                              expression_(expression) { }

    virtual ~XCDFFieldAliasBase() { }

    const std::string& GetName() const {return name_;}
    const std::string& GetExpression() const {return expression_;}
    virtual XCDFFieldType GetType() const = 0;

  private:

    std::string& name_;
    std::string& expression_;
};

typedef XCDFPtr<XCDFFieldAliasBase> XCDFFieldAliasBasePtr;
typedef XCDFPtr<const XCDFFieldAliasBase> XCDFFieldAliasBaseConstPtr;

#endif // XCDF_FIELD_ALIAS_BASE_INCLUDED_H
