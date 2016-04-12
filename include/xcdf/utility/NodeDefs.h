
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

#include <xcdf/utility/Node.h>

#include <cmath>
#include <algorithm>
#include <set>

// Forward-declare XCDFFile to avoid circular dependency introduced
// with XCDFFieldAlias.  There should be a cleaner way to code this.
class XCDFFile;

template <typename T>
class FieldNode : public Node<T> {

  public:

    FieldNode(ConstXCDFField<T> field) : field_(field) { }

    T operator[](unsigned index) const {return field_[index];}
    unsigned GetSize() const {return field_.GetSize();}

    bool HasParent() const {return field_.HasParent();}
    const std::string& GetParentName() const {return field_.GetParentName();}
    const std::string& GetName() const {return field_.GetName();}

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

namespace {

class GetSizePolicy {
  public:
    typedef unsigned ReturnType;
    template <typename NodeType>
    ReturnType operator()(const NodeType& node) {return node.GetSize();}
};

class HasParentPolicy {
  public:
    typedef bool ReturnType;
    template <typename NodeType>
    ReturnType operator()(const NodeType& node) {return node.HasParent();}
};

class GetParentNamePolicy {
  public:
    typedef const std::string& ReturnType;
    template <typename NodeType>
    ReturnType operator()(const NodeType& node) {return node.GetParentName();}
};

class GetNamePolicy {
  public:
    typedef const std::string& ReturnType;
    template <typename NodeType>
    ReturnType operator()(const NodeType& node) {return node.GetName();}
};

}

template <typename T,
          typename U,
          typename DominantType,
          typename ReturnType,
          typename Derived>
class BinaryNode : public Node<ReturnType> {

  public:

    BinaryNode(Node<T>& n1, Node<U>& n2) : n1_(n1),
                                           n2_(n2),
                                           type_(GetRelationType(n1, n2)) { }

    ReturnType operator[](unsigned index) const {

      // Note that, as in standard C style, the burden on checking
      // that the index is in range falls on the caller.
      switch (type_) {
        case SCALAR:
        case SCALAR_FIRST: return DoEvaluation(n1_[0], n2_[index]);
        case VECTOR_VECTOR: return DoEvaluation(n1_[index], n2_[index]);
        case SCALAR_SECOND: return DoEvaluation(n1_[index], n2_[0]);
      }
    }

    GetSizePolicy::ReturnType GetSize() const {
      return ApplyToLargerNode(GetSizePolicy());
    }

    HasParentPolicy::ReturnType HasParent() const {
      return ApplyToLargerNode(HasParentPolicy());
    }

    GetParentNamePolicy::ReturnType GetParentName() const {
      return ApplyToLargerNode(GetParentNamePolicy());
    }

    GetNamePolicy::ReturnType GetName() const {
      return ApplyToLargerNode(GetNamePolicy());
    }

  private:

    Node<T>& n1_;
    Node<U>& n2_;
    NodeRelationType type_;

    // Repeat the name of the highest-dimensional vector
    // of the two nodes.
    template <typename Policy>
    typename Policy::ReturnType
    ApplyToLargerNode(Policy policy) const {
      switch (type_) {
        case SCALAR:
        case SCALAR_FIRST:
        case VECTOR_VECTOR: return policy(n2_);
        case SCALAR_SECOND: return policy(n1_);
      }
    }

    ReturnType DoEvaluation(T a, U b) const {
      return static_cast<const Derived*>(this)->Evaluate(
                 static_cast<DominantType>(a), static_cast<DominantType>(b));
    }
};

template <typename T, typename ReturnType, typename Derived>
class UnaryNode : public Node<ReturnType> {
  public:

    UnaryNode(Node<T>& node) : node_(node) { }
    ReturnType operator[](unsigned idx) const {
      return DoEvaluation(node_[idx]);
    }

    unsigned GetSize() const {return node_.GetSize();}
    bool HasParent() const {return node_.HasParent();}
    const std::string& GetParentName() const {return node_.GetParentName();}
    const std::string& GetName() const {return node_.GetName();}

  private:

    Node<T>& node_;

    ReturnType DoEvaluation(T a) const {
      return static_cast<const Derived*>(this)->Evaluate(a);
    }
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
class LogicalNOTNode : public UnaryNode<T, uint64_t, LogicalNOTNode<T> > {

  public:

  LogicalNOTNode(Node<T>& n1) :
              UnaryNode<T, uint64_t, LogicalNOTNode<T> >(n1) { }
  uint64_t Evaluate(T a) const {return !a;}
};

template <typename T>
class BitwiseNOTNode : public UnaryNode<T, T, BitwiseNOTNode<T> > {

  public:

  BitwiseNOTNode(Node<T>& n1) :
      UnaryNode<T, T, BitwiseNOTNode<T> >(n1) { }
  T Evaluate(T a) const {return ~a;}
};

template <>
class BitwiseNOTNode<double> :
            public UnaryNode<double, double, BitwiseNOTNode<double> > {

  public:

    BitwiseNOTNode(Node<double>& n1) :
               UnaryNode<double, double, BitwiseNOTNode<double> >(n1) { }
    double Evaluate(double a) const {
      XCDFFatal("Bitwise NOT requested for floating point data");
      return 0.;
  }
};

template <typename T>
class InNode : public UnaryNode<T, uint64_t, InNode<T> > {

  public:

    InNode(Node<T>& node, const std::vector<T>& data) :
                     UnaryNode<T, uint64_t, InNode<T> >(node) {
      for (typename std::vector<T>::const_iterator it = data.begin();
                                                   it != data.end(); ++it) {
        data_.insert(static_cast<T>(*it));
      }
    }
    uint64_t Evaluate(T a) const {return data_.find(a) != data_.end();}

  private:
    std::set<T> data_;
};

template <typename T, typename U>
class CastNode : public UnaryNode<U, T, CastNode<T, U> > {

  public:

  CastNode(Node<U>& n1) : UnaryNode<U, T, CastNode<T, U> >(n1) { }
  T Evaluate(U a) const {return static_cast<T>(a);}
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
class AnyNode : public Node<uint64_t> {

  public:

    AnyNode(Node<T>& node) : node_(node) { }
    uint64_t operator[](unsigned idx) const {

      for (unsigned i = 0; i < node_.GetSize(); ++i) {
        // Note that this is node_[i] != 0, as defined by the C++ spec
        if (node_[i]) {
          return true;
        }
      }
      return false;
    }
    unsigned GetSize() const {return 1;}

  private:

    Node<T>& node_;
};

template <typename T>
class AllNode : public Node<uint64_t> {

  public:

    AllNode(Node<T>& node) : node_(node) { }
    uint64_t operator[](unsigned idx) const {

      // Need to explicitly check size and return false if size is zero
      if (node_.GetSize() == 0) {
        return false;
      }

      for (unsigned i = 0; i < node_.GetSize(); ++i) {
        // Note that this is node_[i] == 0, as defined by the C++ spec
        if (!node_[i]) {
          return false;
        }
      }
      return true;
    }
    unsigned GetSize() const {return 1;}

  private:

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
class SinNode : public UnaryNode<T, double, SinNode<T> > {

  public:

    SinNode(Node<T>& node) : UnaryNode<T, double, SinNode<T> >(node) { }
    double Evaluate(T a) const {return sin(a);}
};

template <typename T>
class CosNode : public UnaryNode<T, double, CosNode<T> > {

  public:

    CosNode(Node<T>& node) : UnaryNode<T, double, CosNode<T> >(node) { }
    double Evaluate(T a) const {return cos(a);}
};

template <typename T>
class TanNode : public UnaryNode<T, double, TanNode<T> > {

  public:

    TanNode(Node<T>& node) : UnaryNode<T, double, TanNode<T> >(node) { }
    double Evaluate(T a) const {return tan(a);}
};

template <typename T>
class AsinNode : public UnaryNode<T, double, AsinNode<T> > {

  public:

    AsinNode(Node<T>& node) : UnaryNode<T, double, AsinNode<T> >(node) { }
    double Evaluate(T a) const {return asin(a);}
};

template <typename T>
class AcosNode : public UnaryNode<T, double, AcosNode<T> > {

  public:

    AcosNode(Node<T>& node) : UnaryNode<T, double, AcosNode<T> >(node) { }
    double Evaluate(T a) const {return acos(a);}
};

template <typename T>
class AtanNode : public UnaryNode<T, double, AtanNode<T> > {

  public:

    AtanNode(Node<T>& node) : UnaryNode<T, double, AtanNode<T> >(node) { }
    double Evaluate(T a) const {return atan(a);}
};

template <typename T>
class LogNode : public UnaryNode<T, double, LogNode<T> > {

  public:

    LogNode(Node<T>& node) : UnaryNode<T, double, LogNode<T> >(node) { }
    double Evaluate(T a) const {return log(a);}
};

template <typename T>
class Log10Node : public UnaryNode<T, double, Log10Node<T> > {

  public:

    Log10Node(Node<T>& node) : UnaryNode<T, double, Log10Node<T> >(node) { }
    double Evaluate(T a) const {return log10(a);}
};

template <typename T>
class ExpNode : public UnaryNode<T, double, ExpNode<T> > {

  public:

    ExpNode(Node<T>& node) : UnaryNode<T, double, ExpNode<T> >(node) { }
    double Evaluate(T a) const {return exp(a);}
};

template <typename T>
class AbsNode : public UnaryNode<T, double, AbsNode<T> > {

  public:

    AbsNode(Node<T>& node) : UnaryNode<T, double, AbsNode<T> >(node) { }
    double Evaluate(T a) const {return fabs(static_cast<double>(a));}
};

// Specialize for uint64_t to avoid snarky warnings from Clang
template <>
class AbsNode<uint64_t> :
           public UnaryNode<uint64_t, double, AbsNode<uint64_t> > {

  public:

      AbsNode(Node<uint64_t>& node) :
            UnaryNode<uint64_t, double, AbsNode<uint64_t> >(node) { }
      double Evaluate(uint64_t a) const {return static_cast<double>(a);}
};

template <typename T>
class SqrtNode : public UnaryNode<T, double, SqrtNode<T> > {

  public:

    SqrtNode(Node<T>& node) : UnaryNode<T, double, SqrtNode<T> >(node) { }
    double Evaluate(T a) const {return sqrt(a);}
};

template <typename T>
class CeilNode : public UnaryNode<T, double, CeilNode<T> > {

  public:

    CeilNode(Node<T>& node) : UnaryNode<T, double, CeilNode<T> >(node) { }
    double Evaluate(T a) const {return ceil(a);}
};

template <typename T>
class FloorNode : public UnaryNode<T, double, FloorNode<T> > {

  public:

    FloorNode(Node<T>& node) : UnaryNode<T, double, FloorNode<T> >(node) { }
    double Evaluate(T a) const {return floor(a);}
};

template <typename T>
class IsNaNNode : public UnaryNode<T, uint64_t, IsNaNNode<T> > {

  public:

    IsNaNNode(Node<T>& node) : UnaryNode<T, uint64_t, IsNaNNode<T> >(node) { }
    uint64_t Evaluate(T a) const {return std::isnan(a);}
};

template <typename T>
class IsInfNode : public UnaryNode<T, uint64_t, IsInfNode<T> > {

  public:

    IsInfNode(Node<T>& node) : UnaryNode<T, uint64_t, IsInfNode<T> >(node) { }
    uint64_t Evaluate(T a) const {return std::isinf(a);}
};

template <typename T>
class SinhNode : public UnaryNode<T, double, SinhNode<T> > {

  public:

    SinhNode(Node<T>& node) : UnaryNode<T, double, SinhNode<T> >(node) { }
    double Evaluate(T a) const {return sinh(a);}
};

template <typename T>
class CoshNode : public UnaryNode<T, double, CoshNode<T> > {

  public:

    CoshNode(Node<T>& node) : UnaryNode<T, double, CoshNode<T> >(node) { }
    double Evaluate(T a) const {return cosh(a);}
};

template <typename T>
class TanhNode : public UnaryNode<T, double, TanhNode<T> > {

  public:

    TanhNode(Node<T>& node) : UnaryNode<T, double, TanhNode<T> >(node) { }
    double Evaluate(T a) const {return tanh(a);}
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
