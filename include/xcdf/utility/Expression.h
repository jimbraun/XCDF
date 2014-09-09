
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
#include <xcdf/utility/NodeDefs.h>

#include <vector>
#include <list>
#include <cassert>
#include <sstream>
#include <cctype>

class Expression {

  public:

    Expression(const std::string& exp,
               const XCDFFile& f) : f_(f) {

      expString_ = exp;

      ParseSymbols(exp);

      if (parsedSymbols_.size() == 0) {
        XCDFFatal("No evaluation expression");
      }

      std::list<Symbol*>::iterator start = parsedSymbols_.begin();
      std::list<Symbol*>::iterator end = parsedSymbols_.end();

      RecursiveParseExpression(start, end);
      if (distance(start, end) != 1) {
        XCDFFatal("Invalid expression: " << exp);
      }
    }

    ~Expression() {

      // Delete the symbols we've allocated
      for (std::vector<Symbol*>::iterator it = allocatedSymbols_.begin();
                                      it != allocatedSymbols_.end(); ++it) {
        delete *it;
      }
    }

    Symbol* GetHeadSymbol() {return *(parsedSymbols_.begin());}

    const std::string& GetExpressionString() const {return expString_;}
    const XCDFFile& GetFile() const {return f_;}

  private:

    // Disallow copy/assignment
    Expression(const Expression& exp) : f_(exp.f_) { }
    void operator=(const Expression& exp) { }

    const XCDFFile& f_;
    std::string expString_;

    std::vector<Symbol*> allocatedSymbols_;
    std::list<Symbol*> parsedSymbols_;

    void ParseSymbols(const std::string& exp) {

      size_t pos = 0;
      for (Symbol* s = GetNextSymbol(exp, pos);
                   s != NULL; s = GetNextSymbol(exp, pos)) {

        parsedSymbols_.push_back(s);
        allocatedSymbols_.push_back(s);
      }
    }

    Symbol* GetNextSymbol(const std::string& exp, size_t& pos) {

      // Advance to the next non-whitespace character
      pos = exp.find_first_not_of(" \n\r\t", pos);

      if (pos == std::string::npos) {
        // Nothing left
        return NULL;
      }

      // Get the position of next operator character
      size_t operpos = exp.find_first_of(",/*%^)(=><&|!~", pos);

      if (pos != operpos) {

        // A value, field, function name, or operators + or -
        return ParseValue(exp, pos, operpos);

      } else {

        // An operator.
        return ParseOperator(exp, pos);
      }
    }

    Symbol* ParseValue(const std::string& exp,
                       size_t& pos,
                       size_t operpos) const {

      size_t startpos = pos;
      size_t endpos = exp.find_last_not_of(" \n\r\t", operpos - 1);
      std::string valueString = exp.substr(startpos, endpos - startpos + 1);

      size_t firstpm = valueString.find_first_of("+-");
      if (firstpm != std::string::npos) {

        // Need to deal with + or - symbols.  If previous symbol is a
        // value or ')' and first character is +/-, it is an operator.
        if (parsedSymbols_.size() > 0) {
          if (firstpm == 0 &&
                      (parsedSymbols_.back()->IsNode() ||
                       parsedSymbols_.back()->GetType() == CLOSE_PARAND)) {
            return ParseOperator(exp, pos);
          }
        }

        // It is a value.  First check if first segment is the name of a field
        if (firstpm > 0) {
          size_t lastChar = valueString.find_last_not_of(" \n\r\t", firstpm - 1);
          std::string testName = valueString.substr(0, lastChar + 1);
          if (f_.HasField(testName)) {
            pos += firstpm;
            Symbol* val = ParseValueImpl(testName, false);
            assert(val);
            return val;
          }
        }

        // It is a numerical.  Parse it as the longest numerical
        // possible to catch valid cases like -3.14159e+00
        Symbol* val = ParseNumerical(valueString);
        pos = operpos;

        if (!val) {
          // This is user error.
          XCDFFatal("Cannot parse expression \"" << valueString << "\"");
        }

        return val;
      }

      // Token is a simple value
      pos = endpos + 1;
      bool requireFunctional = false;
      if (operpos != std::string::npos) {
        if (exp[operpos] == '(') {
          requireFunctional = true;
        }
      }
      Symbol* val = ParseValueImpl(valueString, requireFunctional);
      if (!val) {
        XCDFFatal("Unable to parse symbol \"" << valueString << "\"");
      }

      if (val->IsFunction()) {

        // Expect "(" next
        if (operpos == std::string::npos) {
          XCDFFatal("Missing \"(\" after " << *val);
        } else if (exp[operpos] != '(') {
          XCDFFatal("Missing \"(\" after " << *val);
        }
      }
      return val;
    }

    Symbol* ParseOperator(const std::string& exp,
                          size_t& pos) const {

      // An operator.
      size_t startpos = pos;
      size_t endpos = startpos;

      // Parenthesis must be treated alone
      if (exp[startpos] != '(' && exp[startpos] != ')') {

        // Find the end of the operator string
        while (exp.find_first_of(",/*%^=><&|!~", endpos + 1) == endpos + 1) {
          ++endpos;
        }
      }
      pos = endpos + 1;
      Symbol* op = ParseOperatorImpl(exp.substr(startpos, pos - startpos));
      if (!op) {
        XCDFFatal("Unable to parse operator: \"" <<
                        exp.substr(startpos, pos - startpos) << "\"");
      }
      return op;
    }

    template <typename T, typename M>
    Symbol* DoConstNode(const std::string& numerical, M manip) const {
      std::stringstream ss(numerical);
      ss << manip;
      T out;
      ss >> out;
      if (ss.fail()) {
        return NULL;
      }
      // Make sure there are no unconverted characters
      std::string s;
      ss >> s;
      if (s.size() > 0) {
        return NULL;
      }
      return new ConstNode<T>(out);
    }

    Symbol* ParseNumerical(const std::string& numerical) const {

      Symbol* out = DoConstNode<uint64_t>(numerical, std::hex);
      if (!out) {
        out = DoConstNode<uint64_t>(numerical, std::dec);
      }
      if (!out) {
        out = DoConstNode<int64_t>(numerical, std::dec);
      }
      if (!out) {
        out = DoConstNode<double>(numerical, std::dec);
      }
      return out;
    }

    Symbol* ParseValueImpl(std::string exp, bool requireFunctional) const {

      if (!requireFunctional) {
        if (f_.HasField(exp)) {

          // An XCDF field
          if (f_.IsUnsignedIntegerField(exp)) {
            return new FieldNode<uint64_t>(f_.GetUnsignedIntegerField(exp));
          }

          if (f_.IsSignedIntegerField(exp)) {
            return new FieldNode<int64_t>(f_.GetSignedIntegerField(exp));
          }

          return new FieldNode<double>(f_.GetFloatingPointField(exp));
        }
      }

      // "currentEventNumber" refers to the event count and is reserved
      if (!exp.compare("currentEventNumber")) {
        return new CounterNode(f_);
      }

      // Try to parse as a numerical value
      Symbol* numerical = ParseNumerical(exp);
      if (numerical) {
        return numerical;
      }

      // Is this the name of a supported function?
      if (!exp.compare("unique")) {
        return new Symbol(UNIQUE);
      }

      if (!exp.compare("sin")) {
        return new Symbol(SIN);
      }

      if (!exp.compare("cos")) {
        return new Symbol(COS);
      }

      if (!exp.compare("tan")) {
        return new Symbol(TAN);
      }

      if (!exp.compare("asin")) {
        return new Symbol(ASIN);
      }

      if (!exp.compare("acos")) {
        return new Symbol(ACOS);
      }

      if (!exp.compare("atan")) {
        return new Symbol(ATAN);
      }

      if (!exp.compare("log")) {
        return new Symbol(LOG);
      }

      if (!exp.compare("log10")) {
        return new Symbol(LOG10);
      }

      if (!exp.compare("exp")) {
        return new Symbol(EXP);
      }

      if (!exp.compare("abs")) {
        return new Symbol(ABS);
      }

      if (!exp.compare("fabs")) {
        return new Symbol(ABS);
      }

      if (!exp.compare("sqrt")) {
        return new Symbol(SQRT);
      }

      if (!exp.compare("ceil")) {
        return new Symbol(CEIL);
      }

      if (!exp.compare("floor")) {
        return new Symbol(FLOOR);
      }

      if (!exp.compare("isnan")) {
        return new Symbol(ISNAN);
      }

      if (!exp.compare("isinf")) {
        return new Symbol(ISINF);
      }

      if (!exp.compare("sinh")) {
        return new Symbol(SINH);
      }

      if (!exp.compare("cosh")) {
        return new Symbol(COSH);
      }

      if (!exp.compare("tanh")) {
        return new Symbol(TANH);
      }

      if (!exp.compare("rand")) {
        return new Symbol(RAND);
      }

      if (!exp.compare("fmod")) {
        return new Symbol(FMOD);
      }

      if (!exp.compare("pow")) {
        return new Symbol(POW);
      }

      if (!exp.compare("atan2")) {
        return new Symbol(ATAN2);
      }

      if (!exp.compare("true")) {
        return new ConstNode<uint64_t>(1);
      }

      if (!exp.compare("false")) {
        return new ConstNode<uint64_t>(0);
      }

      return NULL;
    }

    Symbol* ParseOperatorImpl(std::string exp) const {

      if (!exp.compare("+")) {
        return new Symbol(ADDITION);
      } else if (!exp.compare("-")) {
        return new Symbol(SUBTRACTION);
      } else if (!exp.compare("*")) {
        return new Symbol(MULTIPLICATION);
      } else if (!exp.compare("/")) {
        return new Symbol(DIVISION);
      } else if (!exp.compare("%")) {
        return new Symbol(MODULUS);
      } else if (!exp.compare("^")) {
        return new Symbol(POWER);
      } else if (!exp.compare("(")) {
        return new Symbol(OPEN_PARAND);
      } else if (!exp.compare(")")) {
        return new Symbol(CLOSE_PARAND);
      } else if (!exp.compare("==")) {
        return new Symbol(EQUALITY);
      } else if (!exp.compare("!=")) {
        return new Symbol(INEQUALITY);
      } else if (!exp.compare(">")) {
        return new Symbol(GREATER_THAN);
      } else if (!exp.compare("<")) {
        return new Symbol(LESS_THAN);
      } else if (!exp.compare(">=")) {
        return new Symbol(GREATER_THAN_EQUAL);
      } else if (!exp.compare("<=")) {
        return new Symbol(LESS_THAN_EQUAL);
      } else if (!exp.compare("||")) {
        return new Symbol(LOGICAL_OR);
      } else if (!exp.compare("&&")) {
        return new Symbol(LOGICAL_AND);
      } else if (!exp.compare("|")) {
        return new Symbol(BITWISE_OR);
      } else if (!exp.compare("&")) {
        return new Symbol(BITWISE_AND);
      } else if (!exp.compare("!")) {
        return new Symbol(LOGICAL_NOT);
      } else if (!exp.compare("~")) {
        return new Symbol(BITWISE_NOT);
      } else if (!exp.compare(",")) {
        return new Symbol(COMMA);
      } else {
        return NULL;
      }
    }

    void RecursiveParseExpression(std::list<Symbol*>::iterator& start,
                                  std::list<Symbol*>::iterator& end) {

      // Is anything here?
      if (start == end) {
        return;
      }

      // Scan for parenthesis
      while (ReplaceParenthesis(start, end)) { }

      // No more parenthesis -- just apply the operators
      // left to right in order of precedence
      ReplaceFunctions(start, end);
      ReplaceUnary(start, end);
      ReplaceMultiplyDivideModulus(start, end);
      ReplaceAdditionSubtraction(start, end);
      ReplaceComparison(start, end);
      ReplaceBitwise(start, end);
      ReplaceLogical(start, end);
      ReplaceCommas(start, end);
    }

    bool ReplaceParenthesis(std::list<Symbol*>::iterator& start,
                            std::list<Symbol*>::iterator& end) {

      // Scan for parenthesis
      std::list<Symbol*>::iterator firstOpenParand = end;
      std::list<Symbol*>::iterator closeParand = end;

      int nOpen = 0;
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {
        if ((*it)->GetType() == OPEN_PARAND) {
          nOpen++;
          if (firstOpenParand == end) {
            firstOpenParand = it;
          }
        } else if ((*it)->GetType() == CLOSE_PARAND) {
          nOpen--;
          if (nOpen == 0) {
            closeParand = it;
            break;

          } else if (nOpen < 0) {
            XCDFFatal("Found unpaired \")\"");
          }
        }
      }

      // Check sanity
      if (nOpen > 0) {
        XCDFFatal("Found unpaired \"(\"");
      }

      // Do we have parenthesis?
      if (firstOpenParand == end) {
        return false;
      }

      // Parse what is inside the parenthesis
      ++firstOpenParand;
      RecursiveParseExpression(firstOpenParand, closeParand);


      // Remove the parenthesis
      ReplaceSymbols(NULL, --firstOpenParand, 1, start);
      ReplaceSymbols(NULL, closeParand, 1, start);
      return true;
    }

    void ReplaceFunctions(std::list<Symbol*>::iterator& start,
                          std::list<Symbol*>::iterator& end) {

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->IsUnaryFunction()) {
          Symbol* s = GetUnarySymbol(start, end, it, (*it)->GetType(), true);
          it = ReplaceSymbols(s, it, 2, start);
        }

        if ((*it)->IsVoidFunction()) {
          Symbol* s = GetVoidSymbol(start, end, it, (*it)->GetType());
          it = ReplaceSymbols(s, it, 1, start);
        }


        if ((*it)->IsBinaryFunction()) {
          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), true);
          it = ReplaceSymbols(s, it, 3, start);
        }

        if ((*it)->GetType() == POWER) {
          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceUnary(std::list<Symbol*>::iterator& start,
                      std::list<Symbol*>::iterator& end) {

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == LOGICAL_NOT ||
            (*it)->GetType() == BITWISE_NOT) {
          Symbol* s = GetUnarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, it, 2, start);
        }
      }
    }

    void ReplaceMultiplyDivideModulus(std::list<Symbol*>::iterator& start,
                                      std::list<Symbol*>::iterator& end) {

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == MULTIPLICATION ||
            (*it)->GetType() == DIVISION ||
            (*it)->GetType() == MODULUS) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceAdditionSubtraction(std::list<Symbol*>::iterator& start,
                                    std::list<Symbol*>::iterator& end) {

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == ADDITION ||
            (*it)->GetType() == SUBTRACTION) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceComparison(std::list<Symbol*>::iterator& start,
                           std::list<Symbol*>::iterator& end) {

      // Start with comparison
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->IsComparison()) {
          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }

      // Now equality
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->IsEquality()) {
          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceBitwise(std::list<Symbol*>::iterator& start,
                        std::list<Symbol*>::iterator& end) {

      // Replace AND first
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == BITWISE_AND) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }

      // Last is OR
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == BITWISE_OR) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceLogical(std::list<Symbol*>::iterator& start,
                        std::list<Symbol*>::iterator& end) {

      // Replace AND first
      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == LOGICAL_AND) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == LOGICAL_OR) {

          Symbol* s = GetBinarySymbol(start, end, it, (*it)->GetType(), false);
          it = ReplaceSymbols(s, --it, 3, start);
        }
      }
    }

    void ReplaceCommas(std::list<Symbol*>::iterator& start,
                       std::list<Symbol*>::iterator& end) {

      for (std::list<Symbol*>::iterator it = start; it != end; ++it) {

        if ((*it)->GetType() == COMMA) {
          it = ReplaceSymbols(NULL, it, 1, start);
        }
      }
    }

    std::list<Symbol*>::iterator
    ReplaceSymbols(Symbol* s,
                   std::list<Symbol*>::iterator removeStart,
                   size_t n,
                   std::list<Symbol*>::iterator& start) {

      std::list<Symbol*>::iterator removeEnd = removeStart;
      for (unsigned i = 0; i < n; i++) {
        removeEnd++;
      }

      std::list<Symbol*>::iterator pos = removeEnd;
      if (s) {
        // Push new symbol after the symbols it is replacing
        pos = parsedSymbols_.insert(removeEnd, s);
      }

      // Need to change start if we're removing from the beginning
      if (removeStart == start) {
        start = pos;
      }

      parsedSymbols_.erase(removeStart, pos);
      return pos;
    }

    Symbol* GetUnarySymbol(std::list<Symbol*>::iterator start,
                           std::list<Symbol*>::iterator end,
                           std::list<Symbol*>::iterator it,
                           SymbolType type,
                           bool isFunction) {

      Symbol* func = *it;
      if (distance(it, end) < 2) {
        XCDFFatal("Cannot evaluate expression: " <<
                            "Missing unary operand in " << *func);
      }

      ++it;
      Symbol* n1 = *it;

      if (!(n1->IsNode())) {
        XCDFFatal("Cannot evaluate expression: " <<
                            "Missing unary operand in " << *func);
      }

      ++it;
      if (it != end) {
        if ((*it)->IsNode() && isFunction) {
          XCDFFatal("Too many arguments to unary function " << *func);
        }
      }

      switch (n1->GetType()) {

        default:
        case FLOATING_POINT_NODE:
          return GetNode(static_cast<Node<double>* >(n1), type);
        case SIGNED_NODE:
          return GetNode(static_cast<Node<int64_t>* >(n1), type);
        case UNSIGNED_NODE:
          return GetNode(static_cast<Node<uint64_t>* >(n1), type);
      }
    }

    Symbol* GetBinarySymbol(std::list<Symbol*>::iterator start,
                            std::list<Symbol*>::iterator end,
                            std::list<Symbol*>::iterator it,
                            SymbolType type,
                            bool isFunction) {

      Symbol* func = *it;

      Symbol* n1;
      Symbol* n2;

      if (isFunction) {

        if (distance(it, end) < 3) {
          XCDFFatal("Cannot evaluate expression: " <<
                             "Missing binary operand in " << *func);
        }

        ++it;
        n1 = *it;
        ++it;
        n2 = *it;

        ++it;
        if (it != end) {
          if ((*it)->IsNode()) {
            XCDFFatal("Too many arguments to binary function " << *func);
          }
        }

      } else {

        if (it == start || distance(it, end) < 2) {
          XCDFFatal("Cannot evaluate expression: " <<
                             "Missing binary operand in " << *func);
        }

        --it;
        n1 = *it;

        ++it;
        ++it;
        n2 = *it;
      }

      if (!(n2->IsNode()) || !(n1->IsNode())) {
        XCDFFatal("Cannot evaluate expression: " <<
                                "Missing binary operand in " << *func);
      }

      switch (n1->GetType()) {

        case FLOATING_POINT_NODE:

          switch (n2->GetType()) {

            default:
            case FLOATING_POINT_NODE:
              return GetNode<double, double, double>(
                             static_cast<Node<double>* >(n1),
                             static_cast<Node<double>* >(n2), type);
            case SIGNED_NODE:
              return GetNode<double, int64_t, double>(
                             static_cast<Node<double>* >(n1),
                             static_cast<Node<int64_t>* >(n2), type);
            case UNSIGNED_NODE:
              return GetNode<double, uint64_t, double>(
                             static_cast<Node<double>* >(n1),
                             static_cast<Node<uint64_t>* >(n2), type);
          }

        case SIGNED_NODE:

          switch (n2->GetType()) {

            default:
            case FLOATING_POINT_NODE:
              return GetNode<int64_t, double, double>(
                             static_cast<Node<int64_t>* >(n1),
                             static_cast<Node<double>* >(n2), type);
            case SIGNED_NODE:
              return GetNode<int64_t, int64_t, int64_t>(
                             static_cast<Node<int64_t>* >(n1),
                             static_cast<Node<int64_t>* >(n2), type);
            case UNSIGNED_NODE:
              return GetNode<int64_t, uint64_t, int64_t>(
                             static_cast<Node<int64_t>* >(n1),
                             static_cast<Node<uint64_t>* >(n2), type);
          }

        case UNSIGNED_NODE:

          switch (n2->GetType()) {

            default:
            case FLOATING_POINT_NODE:
              return GetNode<uint64_t, double, double>(
                             static_cast<Node<uint64_t>* >(n1),
                             static_cast<Node<double>* >(n2), type);
            case SIGNED_NODE:
              return GetNode<uint64_t, int64_t, int64_t>(
                             static_cast<Node<uint64_t>* >(n1),
                             static_cast<Node<int64_t>* >(n2), type);
            case UNSIGNED_NODE:
              return GetNode<uint64_t, uint64_t, uint64_t>(
                             static_cast<Node<uint64_t>* >(n1),
                             static_cast<Node<uint64_t>* >(n2), type);
          }

        default:

          return new Symbol();
      }
    }

    template <typename T>
    Symbol* GetNode(Node<T>* n1, SymbolType type) {
      Symbol* symbol = GetNodeImpl<T>(n1, type);
      allocatedSymbols_.push_back(symbol);
      return symbol;
    }

    template <typename T>
    Symbol* GetNodeImpl(Node<T>* n1, SymbolType type) {
      switch (type) {

        case LOGICAL_NOT:
          return new LogicalNOTNode<T>(*n1);
        case BITWISE_NOT:
          return new BitwiseNOTNode<T>(*n1);
        case UNIQUE:
          return new UniqueNode<T>(*n1);
        case SIN:
          return new SinNode<T>(*n1);
        case COS:
          return new CosNode<T>(*n1);
        case TAN:
          return new TanNode<T>(*n1);
        case ASIN:
          return new AsinNode<T>(*n1);
        case ACOS:
          return new AcosNode<T>(*n1);
        case ATAN:
          return new AtanNode<T>(*n1);
        case LOG:
          return new LogNode<T>(*n1);
        case LOG10:
          return new Log10Node<T>(*n1);
        case EXP:
          return new ExpNode<T>(*n1);
        case ABS:
          return new AbsNode<T>(*n1);
        case SQRT:
          return new SqrtNode<T>(*n1);
        case CEIL:
          return new CeilNode<T>(*n1);
        case FLOOR:
          return new FloorNode<T>(*n1);
        case ISNAN:
          return new IsNaNNode<T>(*n1);
        case ISINF:
          return new IsInfNode<T>(*n1);
        case SINH:
          return new SinhNode<T>(*n1);
        case COSH:
          return new CoshNode<T>(*n1);
        case TANH:
          return new TanhNode<T>(*n1);
        default:
          return new Symbol();
      }

      assert(false);
      return new Symbol();
    }

    template <typename T, typename U, typename DominantType>
    Symbol* GetNode(Node<T>* n1, Node<U>* n2, SymbolType type) {
      Symbol* symbol = GetNodeImpl<T, U, DominantType>(n1, n2, type);
      allocatedSymbols_.push_back(symbol);
      return symbol;
    }

    template <typename T, typename U, typename DominantType>
    Symbol* GetNodeImpl(Node<T>* n1, Node<U>* n2, SymbolType type) {
      switch (type) {

        case EQUALITY:
          return new EqualityNode<T, U, DominantType>(*n1, *n2);
        case INEQUALITY:
          return new InequalityNode<T, U, DominantType>(*n1, *n2);
        case GREATER_THAN:
          return new GreaterThanNode<T, U, DominantType>(*n1, *n2);
        case LESS_THAN:
          return new LessThanNode<T, U, DominantType>(*n1, *n2);
        case GREATER_THAN_EQUAL:
          return new GreaterThanEqualNode<T, U, DominantType>(*n1, *n2);
        case LESS_THAN_EQUAL:
          return new LessThanEqualNode<T, U, DominantType>(*n1, *n2);
        case LOGICAL_OR:
          return new LogicalORNode<T, U, DominantType>(*n1, *n2);
        case LOGICAL_AND:
          return new LogicalANDNode<T, U, DominantType>(*n1, *n2);
        case BITWISE_OR:
          return new BitwiseORNode<T, U, DominantType>(*n1, *n2);
        case BITWISE_AND:
          return new BitwiseANDNode<T, U, DominantType>(*n1, *n2);
        case ADDITION:
          return new AdditionNode<T, U, DominantType>(*n1, *n2);
        case SUBTRACTION:
          return new SubtractionNode<T, U, DominantType>(*n1, *n2);
        case MULTIPLICATION:
          return new MultiplicationNode<T, U, DominantType>(*n1, *n2);
        case DIVISION:
          return new DivisionNode<T, U, DominantType>(*n1, *n2);
        case MODULUS:
          return new ModulusNode<T, U>(*n1, *n2);
        case POWER:
          return new PowerNode<T, U>(*n1, *n2);
        case FMOD:
          return new FmodNode<T, U>(*n1, *n2);
        case ATAN2:
          return new Atan2Node<T, U>(*n1, *n2);
        case POW:
          return new PowerNode<T, U>(*n1, *n2);

        default:
          return new Symbol();
      }
    }

    Symbol* GetVoidSymbol(std::list<Symbol*>::iterator start,
                          std::list<Symbol*>::iterator end,
                          std::list<Symbol*>::iterator it,
                          SymbolType type) {

      Symbol* func = *it;

      ++it;
      if (it != end) {
        if ((*it)->IsNode()) {
          XCDFFatal("Too many arguments to function " << *func);
        }
      }

      return GetNode(type);
    }

    Symbol* GetNode(SymbolType type) {
      Symbol* symbol = GetNodeImpl(type);
      allocatedSymbols_.push_back(symbol);
      return symbol;
    }

    Symbol* GetNodeImpl(SymbolType type) {
      switch(type) {

        case RAND:
          return new RandNode();
        default:
          return new Symbol();
      }
    }
};

#endif // XCDF_UTILITY_EXPRESSION_INCLUDED_H
