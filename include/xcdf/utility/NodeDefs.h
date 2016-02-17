
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

#ifndef XCDF_UTILITY_NODE_DEFS_H_INCLUDED
#define XCDF_UTILITY_NODE_DEFS_H_INCLUDED

#include <xcdf/XCDF.h>
#include <xcdf/utility/Node.h>

#include <cmath>
#include <algorithm>
#include <set>

template <typename T>
class FieldNode : public Node<T> {

  public:

    FieldNode(ConstXCDFField<T> field) : field_(field) { }

    T operator[](unsigned index) const {return field_[index];}
    unsigned GetSize() const {return field_.GetSize();}

  private:

    ConstXCDFField<T> field_;
};

template <typename T>
class ConstNode : public Node<T> {

  public:

    ConstNode(T datum) : datum_(datum) { }

    T operator[](unsigned index) const {return datum_;}
    unsigned GetSize() const {return 1;}

  private:

    T datum_;
};

class CounterNode : public Node<uint64_t> {

  public:

    CounterNode(const XCDFFile& f) : f_(f) { }

    uint64_t operator[](unsigned index) const {
      return f_.GetCurrentEventNumber();
    }
    unsigned GetSize() const {return 1;}

  private:

    const XCDFFile& f_;
};

template <typename T,
          typename U,
          typename DominantType,
          typename ReturnType,
          typename Derived>
class BinaryNode : public Node<ReturnType> {

  public:

    BinaryNode(Node<T>& n1, Node<U>& n2) : n1_(n1),
                                           n2_(n2) { }

    ReturnType operator[](unsigned index) const {

      // Get sizes of underlying nodes.  Note:  This is very
      // inefficient!
      unsigned size1 = n1_.GetSize();
      unsigned size2 = n2_.GetSize();

      if (size1 == 1) {
        if (index >= size2) {
          XCDFFatal("Array index out of bounds");
        }
        return DoEvaluation(n1_[0], n2_[index]);
      }

      if (size2 == 1) {
        if (index >= size1) {
          XCDFFatal("Array index out of bounds");
        }
        return DoEvaluation(n1_[index], n2_[0]);
      }

      // 2 vector nodes; this is rare. Verify that vector sizes are equal to
      // avoid dumb errors.
      if (size1 != size2) {
        XCDFFatal("Unable to evaluate: Vector nodes have different lengths!");
      }
      if (index >= size2) {
        XCDFFatal("Array index out of bounds");
      }
      return DoEvaluation(n1_[index], n2_[index]);
    }

    // Support the (scalar + vector) case.
    unsigned GetSize() const {

      unsigned size1 = n1_.GetSize();
      unsigned size2 = n2_.GetSize();

      if (size1 == 1) {
        return size2;
      }

      if (size2 == 1) {
        return size1;
      }

      return std::min(size1, size2);
    }

  private:

    ReturnType DoEvaluation(T a, U b) const {
      return static_cast<const Derived*>(this)->Evaluate(
               static_cast<DominantType>(a), static_cast<DominantType>(b));
    }

    Node<T>& n1_;
    Node<U>& n2_;

};

template <typename T, typename U, typename DominantType>
class AdditionNode : public BinaryNode<T, U, DominantType, DominantType,
                                       AdditionNode<T, U, DominantType> > {

  public:

    AdditionNode(Node<T>& n1, Node<U>& n2) :
      BinaryNode<T, U, DominantType, DominantType,
                  AdditionNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a + b;}
};

template <typename T, typename U, typename DominantType>
class SubtractionNode :
   public BinaryNode<T, U, DominantType, DominantType,
                        SubtractionNode<T, U, DominantType> > {

  public:

    SubtractionNode(Node<T>& n1, Node<U>& n2) :
          BinaryNode<T, U, DominantType, DominantType,
                      SubtractionNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a - b;}
};


template <typename T, typename U, typename DominantType>
class MultiplicationNode :
   public BinaryNode<T, U, DominantType,
                DominantType, MultiplicationNode<T, U, DominantType> > {

  public:

    MultiplicationNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, DominantType,
                      MultiplicationNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a * b;}
};

template <typename T, typename U, typename DominantType>
class DivisionNode :
   public BinaryNode<T, U, DominantType,
                      DominantType, DivisionNode<T, U, DominantType> > {

  public:

    DivisionNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, DominantType,
                         DivisionNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a / b;}
};

template <typename T, typename U>
class ModulusNode :
   public BinaryNode<T, U, uint64_t, uint64_t, ModulusNode<T, U> > {

  public:

    ModulusNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, uint64_t,
                        uint64_t, ModulusNode<T, U> >(n1, n2) { }

    uint64_t Evaluate(uint64_t a, uint64_t b) const {return a % b;}
};

template <typename T, typename U>
class PowerNode : public BinaryNode<T, U, double, double, PowerNode<T, U> > {

  public:

    PowerNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, double, double, PowerNode<T, U> >(n1, n2) { }

    double Evaluate(double a, double b) const {return pow(a, b);}
};

template <typename T, typename U, typename DominantType>
class EqualityNode :
   public BinaryNode<T, U, DominantType,
                      uint64_t, EqualityNode<T, U, DominantType> > {

  public:

    EqualityNode(Node<T>& n1, Node<U>& n2) :
        BinaryNode<T, U, DominantType, uint64_t,
                    EqualityNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a == b;}
};

template <typename T, typename U, typename DominantType>
class InequalityNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, InequalityNode<T, U, DominantType> > {

  public:

    InequalityNode(Node<T>& n1, Node<U>& n2) :
        BinaryNode<T, U, DominantType, uint64_t,
                      InequalityNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a != b;}
};

template <typename T, typename U, typename DominantType>
class GreaterThanNode :
   public BinaryNode<T, U, DominantType,
                        uint64_t, GreaterThanNode<T, U, DominantType> > {

  public:

    GreaterThanNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, uint64_t,
                         GreaterThanNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a > b;}
};

template <typename T, typename U, typename DominantType>
class LessThanNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, LessThanNode<T, U, DominantType> > {

  public:

     LessThanNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, uint64_t,
                       LessThanNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a < b;}
};

template <typename T, typename U, typename DominantType>
class GreaterThanEqualNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, GreaterThanEqualNode<T, U, DominantType> > {

  public:

    GreaterThanEqualNode(Node<T>& n1, Node<U>& n2) :
       BinaryNode<T, U, DominantType, uint64_t,
                   GreaterThanEqualNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a >= b;}
};

template <typename T, typename U, typename DominantType>
class LessThanEqualNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, LessThanEqualNode<T, U, DominantType> > {

  public:

    LessThanEqualNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, uint64_t,
                       LessThanEqualNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a <= b;}
};

template <typename T, typename U, typename DominantType>
class LogicalANDNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, LogicalANDNode<T, U, DominantType> > {

  public:

    LogicalANDNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, uint64_t,
                         LogicalANDNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a && b;}
};

template <typename T, typename U, typename DominantType>
class LogicalORNode :
   public BinaryNode<T, U, DominantType,
                       uint64_t, LogicalORNode<T, U, DominantType> > {

  public:

    LogicalORNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, uint64_t,
                         LogicalORNode<T, U, DominantType> >(n1, n2) { }

    uint64_t Evaluate(DominantType a, DominantType b) const {return a || b;}
};

template <typename T, typename U, typename DominantType>
class BitwiseORNode :
  public BinaryNode<T, U, DominantType,
              DominantType, BitwiseORNode<T, U, DominantType> > {

  public:

    BitwiseORNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, DominantType,
                       BitwiseORNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a | b;}
};

template <typename T, typename U>
class BitwiseORNode<T, U, double> :
  public BinaryNode<T, U, double,
              double, BitwiseORNode<T, U, double> > {

  public:

    BitwiseORNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, double, double,
                       BitwiseORNode<T, U, double> >(n1, n2) { }

    double Evaluate(double a, double b) const {
      XCDFFatal("Bitwise OR requested for floating point data");
      return 0.; // Unreachable
    }
};

template <typename T, typename U, typename DominantType>
class BitwiseANDNode :
  public BinaryNode<T, U, DominantType,
            DominantType, BitwiseANDNode<T, U, DominantType> > {

  public:

    BitwiseANDNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, DominantType, DominantType,
                      BitwiseANDNode<T, U, DominantType> >(n1, n2) { }

    DominantType Evaluate(DominantType a, DominantType b) const {return a & b;}
};

template <typename T, typename U>
class BitwiseANDNode<T, U, double> :
  public BinaryNode<T, U, double,
              double, BitwiseANDNode<T, U, double> > {

  public:

    BitwiseANDNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, double, double,
                       BitwiseANDNode<T, U, double> >(n1, n2) { }

    double Evaluate(double a, double b) const {
      XCDFFatal("Bitwise AND requested for floating point data");
      return 0.; // Unreachable

    }
};

template <typename T>
class LogicalNOTNode : public Node<uint64_t> {

  public:

  LogicalNOTNode(Node<T>& n1) : n1_(n1) { }

  uint64_t operator[](unsigned idx) const {return !(n1_[idx]);}
  unsigned GetSize() const {return n1_.GetSize();}

  private:

    Node<T>& n1_;
};

template <typename T>
class BitwiseNOTNode : public Node<T> {

  public:

  BitwiseNOTNode(Node<T>& n1) : n1_(n1) { }

  T operator[](unsigned idx) const {return ~(n1_[idx]);}
  unsigned GetSize() const {return n1_.GetSize();}

  private:

    Node<T>& n1_;
};

template <>
class BitwiseNOTNode<double> : public Node<double> {

  public:

  BitwiseNOTNode(Node<double>& n1) : n1_(n1) { }

  double operator[](unsigned idx) const {
    XCDFFatal("Bitwise NOT requested for floating point data");
    return 0.; // Unreachable
  }
  unsigned GetSize() const {return n1_.GetSize();}

  private:

    Node<double>& n1_;
};

template <typename T>
class IsTrueNode : public Node<uint64_t> {

  public:

  IsTrueNode(Node<T>& n1) : n1_(n1) { }

  uint64_t operator[](unsigned idx) const {return n1_[idx] != 0;}
  unsigned GetSize() const {return n1_.GetSize();}

  private:

    Node<T>& n1_;
};

template <typename T, typename U>
class CastNode : public Node<T> {

  public:

  CastNode(Node<U>& n1) : n1_(n1) { }

  T operator[](unsigned idx) const {return static_cast<T>(n1_[idx]);}
  unsigned GetSize() const {return n1_.GetSize();}

  private:

    Node<U>& n1_;
};

template <typename T>
class UniqueNode : public Node<uint64_t> {

  public:

    UniqueNode(Node<T>& node) : node_(node) { }
    uint64_t operator[](unsigned idx) const {

      data_.clear();
      for (unsigned i = 0; i < node_.GetSize(); ++i) {
        data_.insert(node_[i]);
      }
      return data_.size();
    }
    unsigned GetSize() const {return 1;}

  private:

    mutable std::set<T> data_;
    Node<T>& node_;
};

template <typename T>
class SumNode : public Node<T> {

  public:

    SumNode(Node<T>& node) : node_(node) { }
    T operator[](unsigned idx) const {

      T sum = 0;
      for (unsigned i = 0; i < node_.GetSize(); ++i) {
        sum += node_[i];
      }
      return sum;
    }
    unsigned GetSize() const {return 1;}

  private:
    Node<T>& node_;
};

template <typename T>
class SinNode : public Node<double> {

  public:

    SinNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return sin(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class CosNode : public Node<double> {

  public:

    CosNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return cos(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class TanNode : public Node<double> {

  public:

    TanNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return tan(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class AsinNode : public Node<double> {

  public:

    AsinNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return asin(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class AcosNode : public Node<double> {

  public:

    AcosNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return acos(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class AtanNode : public Node<double> {

  public:

    AtanNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return atan(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class LogNode : public Node<double> {

  public:

    LogNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return log(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class Log10Node : public Node<double> {

  public:

    Log10Node(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return log10(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class ExpNode : public Node<double> {

  public:

    ExpNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return exp(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class AbsNode : public Node<double> {

  public:

    AbsNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {
      return fabs(static_cast<double>(node_[idx]));
    }
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

// Specialize for uint64_t to avoid snarky warnings from Clang
template <>
class AbsNode<uint64_t> : public Node<double> {

  public:

      AbsNode(Node<uint64_t>& node) : node_(node) { }
      double operator[](unsigned idx) const {return static_cast<double>(node_[idx]);}
      unsigned GetSize() const {return node_.GetSize();}

    private:

      Node<uint64_t>& node_;
};

template <typename T>
class SqrtNode : public Node<double> {

  public:

    SqrtNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return sqrt(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class CeilNode : public Node<double> {

  public:

    CeilNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return ceil(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class FloorNode : public Node<double> {

  public:

    FloorNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return floor(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class IsNaNNode : public Node<uint64_t> {

  public:

    IsNaNNode(Node<T>& node) : node_(node) { }
    uint64_t operator[](unsigned idx) const {return std::isnan(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class IsInfNode : public Node<uint64_t> {

  public:

    IsInfNode(Node<T>& node) : node_(node) { }
    uint64_t operator[](unsigned idx) const {return std::isinf(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class SinhNode : public Node<double> {

  public:

    SinhNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return sinh(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class CoshNode : public Node<double> {

  public:

    CoshNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return cosh(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

template <typename T>
class TanhNode : public Node<double> {

  public:

    TanhNode(Node<T>& node) : node_(node) { }
    double operator[](unsigned idx) const {return tanh(node_[idx]);}
    unsigned GetSize() const {return node_.GetSize();}

  private:

    Node<T>& node_;
};

class RandNode : public Node<uint64_t> {

  public:

    RandNode() { }
    uint64_t operator[](unsigned idx) const {return rand();}
    unsigned GetSize() const {return 1;}
};

template <typename T, typename U>
class FmodNode :
   public BinaryNode<T, U, double, double, FmodNode<T, U> > {

  public:

     FmodNode(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, double,
                        double, FmodNode<T, U> >(n1, n2) { }

    double Evaluate(double a, double b) const {return fmod(a, b);}
};

template <typename T, typename U>
class Atan2Node :
   public BinaryNode<T, U, double, double, Atan2Node<T, U> > {

  public:

    Atan2Node(Node<T>& n1, Node<U>& n2) :
           BinaryNode<T, U, double,
                        double, Atan2Node<T, U> >(n1, n2) { }

    double Evaluate(double a, double b) const {return atan2(a, b);}
};

#endif // XCDF_UTILITY_NODE_DEFS_H_INCLUDED
