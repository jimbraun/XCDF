
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

#ifndef XCDF_ALIAS_DESCRIPTOR_INCLUDED_H
#define XCDF_ALIAS_DESCRIPTOR_INCLUDED_H

#include <string>
#include <stdint.h>


class XCDFAliasDescriptor {

  public:

    XCDFAliasDescriptor() : name_(""),
                            expression_("") { }

    XCDFAliasDescriptor(const std::string& name,
                        const std::string& expression) :
                                             name_(name),
                                             expression_(expression) { }

    ~XCDFAliasDescriptor() { }

    const std::string& GetName() const {return name_;}
    const std::string& GetExpression() const {return expression_;}

    bool operator==(const XCDFAliasDescriptor& ad) const {
      return name_ == ad.name_ &&
             expression_ == ad.expression_;
    }

    bool operator!=(const XCDFAliasDescriptor& ad) const {
      return !(*this == ad);
    }

  private:

    std::string name_;
    std::string expression_;
};

#endif // XCDF_ALIAS_DESCRIPTOR_INCLUDED_H
