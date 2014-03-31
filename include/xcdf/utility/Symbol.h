
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

#ifndef XCDF_UTILITY_SYMBOL_H_INCLUDED
#define XCDF_UTILITY_SYMBOL_H_INCLUDED

enum SymbolType {

    VOID,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULUS,
    POWER,
    OPEN_PARAND,
    CLOSE_PARAND,
    EQUALITY,
    INEQUALITY,
    GREATER_THAN,
    LESS_THAN,
    GREATER_THAN_EQUAL,
    LESS_THAN_EQUAL,
    LOGICAL_OR,
    LOGICAL_AND,
    BITWISE_OR,
    BITWISE_AND,
    LOGICAL_NOT,
    BITWISE_NOT,
    COMMA,
    SIGNED_NODE,
    UNSIGNED_NODE,
    FLOATING_POINT_NODE,
    UNIQUE,
    SIN,
    COS,
    TAN,
    ASIN,
    ACOS,
    ATAN,
    LOG,
    LOG10,
    EXP,
    ABS,
    SQRT,
    CEIL,
    FLOOR,
    ISNAN,
    ISINF,
    SINH,
    COSH,
    TANH,
    RAND,
    FMOD,
    POW,
    ATAN2,
};

class Symbol {

  public:

    Symbol() : type_(VOID) { }

    Symbol(SymbolType type) : type_(type) { }

    SymbolType GetType() const {return type_;}

    bool IsNode() const {
      return (type_ == SIGNED_NODE ||
              type_ == UNSIGNED_NODE ||
              type_ == FLOATING_POINT_NODE);
    }

    bool IsFunction() const {

      return (IsUnaryFunction() ||
              IsVoidFunction()  ||
              IsBinaryFunction());
    }

    bool IsUnaryFunction() const {
      return (type_ == UNIQUE ||
              type_ == SIN    ||
              type_ == COS    ||
              type_ == TAN    ||
              type_ == ASIN   ||
              type_ == ACOS   ||
              type_ == ATAN   ||
              type_ == LOG    ||
              type_ == LOG10  ||
              type_ == EXP    ||
              type_ == ABS    ||
              type_ == SQRT   ||
              type_ == CEIL   ||
              type_ == FLOOR  ||
              type_ == ISNAN  ||
              type_ == ISINF  ||
              type_ == SINH   ||
              type_ == COSH   ||
              type_ == TANH);
    }

    bool IsVoidFunction() const {
      return type_ == RAND;
    }

    bool IsBinaryFunction() const {
      return (type_ == FMOD ||
              type_ == POW  ||
              type_ == ATAN2);
    }

    bool IsComparison() const {
      return (type_ == GREATER_THAN ||
              type_ == LESS_THAN ||
              type_ == GREATER_THAN_EQUAL ||
              type_ == LESS_THAN_EQUAL);
    }

    bool IsEquality() const {
      return (type_ == EQUALITY ||
              type_ == INEQUALITY);
    }

    virtual ~Symbol() { }

  protected:

    SymbolType type_;
};

std::ostream& operator<<(std::ostream& os, const Symbol& s) {

  switch (s.GetType()) {
    case VOID:                os << "VOID"; break;
    case ADDITION:            os << "operator +"; break;
    case SUBTRACTION:         os << "operator -"; break;
    case MULTIPLICATION:      os << "operator *"; break;
    case DIVISION:            os << "operator /"; break;
    case MODULUS:             os << "operator %"; break;
    case POWER:               os << "operator ^"; break;
    case OPEN_PARAND:         os << "operator ("; break;
    case CLOSE_PARAND:        os << "operator )"; break;
    case EQUALITY:            os << "operator =="; break;
    case INEQUALITY:          os << "operator !="; break;
    case GREATER_THAN:        os << "operator >"; break;
    case LESS_THAN:           os << "operator <"; break;
    case GREATER_THAN_EQUAL:  os << "operator >="; break;
    case LESS_THAN_EQUAL:     os << "operator <="; break;
    case LOGICAL_OR:          os << "operator ||"; break;
    case LOGICAL_AND:         os << "operator &&"; break;
    case BITWISE_OR:          os << "operator |"; break;
    case BITWISE_AND:         os << "operator &"; break;
    case LOGICAL_NOT:         os << "operator !"; break;
    case BITWISE_NOT:         os << "operator ~"; break;
    case COMMA:               os << "operator ,"; break;
    case SIGNED_NODE:         os << "SIGNED_NODE"; break;
    case UNSIGNED_NODE:       os << "UNSIGNED_NODE"; break;
    case FLOATING_POINT_NODE: os << "FLOATING_POINT_NODE"; break;
    case UNIQUE:              os << "unique"; break;
    case SIN:                 os << "sin"; break;
    case COS:                 os << "cos"; break;
    case TAN:                 os << "tan"; break;
    case ASIN:                os << "asin"; break;
    case ACOS:                os << "acos"; break;
    case ATAN:                os << "atan"; break;
    case LOG:                 os << "log"; break;
    case LOG10:               os << "log10"; break;
    case EXP:                 os << "exp"; break;
    case ABS:                 os << "abs"; break;
    case SQRT:                os << "sqrt"; break;
    case CEIL:                os << "ceil"; break;
    case FLOOR:               os << "floor"; break;
    case ISNAN:               os << "isnan"; break;
    case ISINF:               os << "isinf"; break;
    case SINH:                os << "sinh"; break;
    case COSH:                os << "cosh"; break;
    case TANH:                os << "tanh"; break;
    case RAND:                os << "rand"; break;
    case FMOD:                os << "fmod"; break;
    case POW:                 os << "pow"; break;
    case ATAN2:               os << "atan2"; break;
  }
  return os;
}

#endif // XCDF_UTILITY_SYMBOL_H_INCLUDED
