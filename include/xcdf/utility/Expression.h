
/*
Copyright (c) 2014, University of Maryland
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

#ifndef XCDF_UTILITY_EXPRESSION_INCLUDED_H
#define XCDF_UTILITY_EXPRESSION_INCLUDED_H

#include <xcdf/utility/Symbol.h>
#include <xcdf/XCDFDefs.h>
#include <vector>
#include <list>
#include <algorithm>

// Forward-declare XCDFFile to avoid circular dependency introduced
// with XCDFFieldAlias.  There should be a cleaner way to code this.
class XCDFFile;

class Expression {

  public:

    void Init();

    Expression(const std::string& expr,
               const XCDFFile& f) : f_(&f), expString_(expr) {

      Init();
    }

    Expression(const Expression& expr) : f_(expr.f_),
                                         expString_(expr.expString_) {

      Init();
    }

    ~Expression();

    Expression& operator=(Expression e);

    Symbol* GetHeadSymbol() {return *(parsedSymbols_.begin());}

    const std::string& GetExpressionString() const {return expString_;}
    const XCDFFile& GetFile() const {return *f_;}

  private:

    const XCDFFile* f_;
    std::string expString_;

    std::vector<Symbol*> allocatedSymbols_;
    std::list<Symbol*> parsedSymbols_;

    void ParseSymbols(const std::string& exp);
    Symbol* GetNextSymbol(const std::string& exp, size_t& pos);

    Symbol* ParseValue(const std::string& exp,
                       size_t& pos,
                       size_t operpos) const;

    Symbol* ParseOperator(const std::string& exp,
                          size_t& pos) const;

    Symbol* ParseNumerical(const std::string& numerical) const;
    Symbol* ParseValueImpl(std::string exp) const;
    Symbol* ParseOperatorImpl(std::string exp) const;
    void RecursiveParseExpression(std::list<Symbol*>::iterator& start,
                                  std::list<Symbol*>::iterator& end);

    bool ReplaceParenthesis(std::list<Symbol*>::iterator& start,
                            std::list<Symbol*>::iterator& end);

    void ReplaceFunctions(std::list<Symbol*>::iterator& start,
                          std::list<Symbol*>::iterator& end);

    void ReplaceUnary(std::list<Symbol*>::iterator& start,
                      std::list<Symbol*>::iterator& end);

    void ReplaceMultiplyDivideModulus(std::list<Symbol*>::iterator& start,
                                      std::list<Symbol*>::iterator& end);

    void ReplaceAdditionSubtraction(std::list<Symbol*>::iterator& start,
                                    std::list<Symbol*>::iterator& end);

    void ReplaceComparison(std::list<Symbol*>::iterator& start,
                           std::list<Symbol*>::iterator& end);

    void ReplaceBitwise(std::list<Symbol*>::iterator& start,
                        std::list<Symbol*>::iterator& end);

    void ReplaceLogical(std::list<Symbol*>::iterator& start,
                        std::list<Symbol*>::iterator& end);

    void ReplaceCommas(std::list<Symbol*>::iterator& start,
                       std::list<Symbol*>::iterator& end);

    std::list<Symbol*>::iterator
    ReplaceSymbols(Symbol* s,
                   std::list<Symbol*>::iterator removeStart,
                   size_t n,
                   std::list<Symbol*>::iterator& start);

    Symbol* GetUnarySymbol(std::list<Symbol*>::iterator start,
                           std::list<Symbol*>::iterator end,
                           std::list<Symbol*>::iterator it,
                           SymbolType type,
                           bool isFunction);

    Symbol* GetBinarySymbol(std::list<Symbol*>::iterator start,
                            std::list<Symbol*>::iterator end,
                            std::list<Symbol*>::iterator it,
                            SymbolType type,
                            bool isFunction);

    Symbol* GetVoidSymbol(std::list<Symbol*>::iterator start,
                          std::list<Symbol*>::iterator end,
                          std::list<Symbol*>::iterator it,
                          SymbolType type);
};

#endif // XCDF_UTILITY_EXPRESSION_INCLUDED_H
