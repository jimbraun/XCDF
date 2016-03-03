/*
Copyright (c) 2016, Jim Braun
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

#ifndef XCDF_UTILITY_HISTOGRAM_FILLER_H_INCLUDED
#define XCDF_UTILITY_HISTOGRAM_FILLER_H_INCLUDED

#include <xcdf/utility/NumericalExpression.h>
#include <xcdf/utility/Histogram.h>
#include <xcdf/XCDFPtr.h>

/*
 *  Objects that fill histograms.  Filling is dependent on the type
 *  of field and the relationships of the histogram field(s) and the
 *  weight field.  We could possibly combine the 1D and 2D cases with
 *  another policy class, but it is not clear that this would result
 *  in better code.
 */

class DynamicFiller1D {
  public:

    DynamicFiller1D(const NumericalExpression<double>& ne1,
                    const NumericalExpression<double>& ne2) :
                                          ne1_(ne1), ne2_(ne2) { }
    virtual ~DynamicFiller1D() { }
    virtual void Fill(Histogram1D& h) const = 0;

  protected:

    NumericalExpression<double> ne1_;
    NumericalExpression<double> ne2_;
};

class ScalarFiller1D : public DynamicFiller1D {

  public:

    ScalarFiller1D(const NumericalExpression<double>& ne1,
                   const NumericalExpression<double>& ne2) :
                                      DynamicFiller1D(ne1, ne2) { }
    void Fill(Histogram1D& h) const {
      h.Fill(ne1_.Evaluate(), ne2_.Evaluate());
    }
};

template <typename FillPolicy>
class VectorFiller1D : public DynamicFiller1D {

  public:

    VectorFiller1D(const NumericalExpression<double>& ne1,
                   const NumericalExpression<double>& ne2) :
                                      DynamicFiller1D(ne1, ne2) { }
    void Fill(Histogram1D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        FillPolicy::Fill(h, ne1_.Evaluate(i), ne2_.Evaluate());
      }
    }
};

class Vector12Filler1D : public DynamicFiller1D {

  public:

    Vector12Filler1D(const NumericalExpression<double>& ne1,
                     const NumericalExpression<double>& ne2) :
                                      DynamicFiller1D(ne1, ne2) { }
    void Fill(Histogram1D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        h.Fill(ne1_.Evaluate(i), ne2_.Evaluate(i));
      }
    }
};

class AnyFiller1D : public DynamicFiller1D {

  public:

    AnyFiller1D(const NumericalExpression<double>& ne1,
                const NumericalExpression<double>& ne2) :
                                      DynamicFiller1D(ne1, ne2) { }
    void Fill(Histogram1D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        for (unsigned j = 0; j < ne2_.GetSize(); ++j) {
          h.Fill(ne1_.Evaluate(i), ne2_.Evaluate(j));
        }
      }
    }
};

typedef XCDFPtr<DynamicFiller1D> DynamicFiller1DPtr;

struct FillXY {
  static void Fill(Histogram1D& h, double x, double y) {h.Fill(x, y);}
};

struct FillYX {
  static void Fill(Histogram1D& h, double y, double x) {h.Fill(x, y);}
};

DynamicFiller1DPtr GetFiller(NodeRelationType type,
                             const NumericalExpression<double>& ne1,
                             const NumericalExpression<double>& ne2) {

  switch (type) {

    case SCALAR:
      return DynamicFiller1DPtr(new ScalarFiller1D(ne1, ne2));

    // Second expression is vector.  Swap the order and swap back when filling
    case SCALAR_FIRST:
      return DynamicFiller1DPtr(new VectorFiller1D<FillYX>(ne2, ne1));

    case SCALAR_SECOND:
      return DynamicFiller1DPtr(new VectorFiller1D<FillXY>(ne1, ne2));

    case VECTOR_VECTOR:
      return DynamicFiller1DPtr(new Vector12Filler1D(ne1, ne2));
  }
}

class Filler1D {

  public:

    Filler1D(const std::string& xExpr,
             const std::string& wExpr) : xExpr_(xExpr), wExpr_(wExpr) { }

    void Fill(Histogram1D& h, XCDFFile& f) {

      NumericalExpression<double> xne(xExpr_, f);
      NumericalExpression<double> wne(wExpr_, f);

      // Get the filler appropriate to the relation between the two nodes
      DynamicFiller1DPtr filler;
      try {
        filler = GetFiller(xne.GetNodeRelationType(wne), xne, wne);
      } catch (XCDFException& e) {
        // We can still make the histogram by drawing all against all
        filler = DynamicFiller1DPtr(new AnyFiller1D(xne, wne));
      }

      while (f.Read()) {
        filler->Fill(h);
      }
    }

  private:

    std::string xExpr_;
    std::string wExpr_;
};

class DynamicFiller2D {
  public:

    DynamicFiller2D(const NumericalExpression<double>& ne1,
                    const NumericalExpression<double>& ne2,
                    const NumericalExpression<double>& ne3) :
                                    ne1_(ne1), ne2_(ne2), ne3_(ne3) { }
    virtual ~DynamicFiller2D() { }
    virtual void Fill(Histogram2D& h) const = 0;

  protected:

    NumericalExpression<double> ne1_;
    NumericalExpression<double> ne2_;
    NumericalExpression<double> ne3_;
};

class ScalarFiller2D : public DynamicFiller2D {

  public:

    ScalarFiller2D(const NumericalExpression<double>& ne1,
                   const NumericalExpression<double>& ne2,
                   const NumericalExpression<double>& ne3) :
                                 DynamicFiller2D(ne1, ne2, ne3) { }

    void Fill(Histogram2D& h) const {
      h.Fill(ne1_.Evaluate(), ne2_.Evaluate(), ne3_.Evaluate());
    }
};

template <typename FillPolicy>
class VectorFiller2D : public DynamicFiller2D {

  public:

    VectorFiller2D(const NumericalExpression<double>& ne1,
                   const NumericalExpression<double>& ne2,
                   const NumericalExpression<double>& ne3) :
                                 DynamicFiller2D(ne1, ne2, ne3) { }

    void Fill(Histogram2D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        FillPolicy::Fill(h, ne1_.Evaluate(i),
                         ne2_.Evaluate(), ne3_.Evaluate());
      }
    }
};

template <typename FillPolicy>
class Vector12Filler2D : public DynamicFiller2D {

  public:

    Vector12Filler2D(const NumericalExpression<double>& ne1,
                     const NumericalExpression<double>& ne2,
                     const NumericalExpression<double>& ne3) :
                                 DynamicFiller2D(ne1, ne2, ne3) { }

    void Fill(Histogram2D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        FillPolicy::Fill(h, ne1_.Evaluate(i),
                         ne2_.Evaluate(i), ne3_.Evaluate());
      }
    }
};

class Vector123Filler2D : public DynamicFiller2D {

  public:

    Vector123Filler2D(const NumericalExpression<double>& ne1,
                      const NumericalExpression<double>& ne2,
                      const NumericalExpression<double>& ne3) :
                                 DynamicFiller2D(ne1, ne2, ne3) { }

    void Fill(Histogram2D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        h.Fill(ne1_.Evaluate(i), ne2_.Evaluate(i), ne3_.Evaluate(i));
      }
    }
};

class AnyFiller2D : public DynamicFiller2D {

  public:

    AnyFiller2D(const NumericalExpression<double>& ne1,
                const NumericalExpression<double>& ne2,
                const NumericalExpression<double>& ne3) :
                             DynamicFiller2D(ne1, ne2, ne3) { }

    void Fill(Histogram2D& h) const {
      for (unsigned i = 0; i < ne1_.GetSize(); ++i) {
        for (unsigned j = 0; j < ne2_.GetSize(); ++j) {
          for (unsigned k = 0; k < ne3_.GetSize(); ++k) {
            h.Fill(ne1_.Evaluate(i), ne2_.Evaluate(j), ne3_.Evaluate(k));
          }
        }
      }
    }
};

struct FillXYZ {
  static void Fill(Histogram2D& h, double x, double y, double z) {
    h.Fill(x, y, z);
  }
};

struct FillXZY {
  static void Fill(Histogram2D& h, double x, double z, double y) {
    h.Fill(x, y, z);
  }
};

struct FillYXZ {
  static void Fill(Histogram2D& h, double y, double x, double z) {
    h.Fill(x, y, z);
  }
};

struct FillYZX {
  static void Fill(Histogram2D& h, double y, double z, double x) {
    h.Fill(x, y, z);
  }
};

struct FillZXY {
  static void Fill(Histogram2D& h, double z, double x, double y) {
    h.Fill(x, y, z);
  }
};

struct FillZYX {
  static void Fill(Histogram2D& h, double z, double y, double x) {
    h.Fill(x, y, z);
  }
};

typedef XCDFPtr<DynamicFiller2D> DynamicFiller2DPtr;

DynamicFiller2DPtr GetFiller(NodeRelationType type12,
                             NodeRelationType type13,
                             NodeRelationType type23,
                             const NumericalExpression<double>& ne1,
                             const NumericalExpression<double>& ne2,
                             const NumericalExpression<double>& ne3) {

  switch (type12) {

    case SCALAR: {
      if (type13 == SCALAR) {
        return DynamicFiller2DPtr(new ScalarFiller2D(ne1, ne2, ne3));
      } else {
        // type13 == SCALAR_FIRST
        return DynamicFiller2DPtr(new VectorFiller2D<FillZXY>(ne3, ne1, ne2));
      }
    }

    case SCALAR_FIRST: {
      if (type13 == SCALAR) {
        return DynamicFiller2DPtr(new VectorFiller2D<FillYXZ>(ne2, ne1, ne3));
        // type13 == SCALAR_FIRST.  Field3 is a vector.
      } else {
        return DynamicFiller2DPtr(new Vector12Filler2D<FillYZX>(ne2, ne3, ne1));
      }
    }

    case SCALAR_SECOND: {
      if (type23 == SCALAR) {
        return DynamicFiller2DPtr(new VectorFiller2D<FillXYZ>(ne1, ne2, ne3));
        // type23 == SCALAR_FIRST.  Field3 is a vector.
      } else {
        return DynamicFiller2DPtr(new Vector12Filler2D<FillXZY>(ne1, ne3, ne2));
      }
    }

    case VECTOR_VECTOR: {
      if (type23 == SCALAR_SECOND) {
        return DynamicFiller2DPtr(new Vector12Filler2D<FillXYZ>(ne1, ne2, ne3));
      } else {
        return DynamicFiller2DPtr(new Vector123Filler2D(ne1, ne2, ne3));
      }
    }
  }
}

class Filler2D {

  public:

    Filler2D(const std::string& xExpr,
             const std::string& yExpr,
             const std::string& wExpr) : xExpr_(xExpr),
                                         yExpr_(yExpr),
                                         wExpr_(wExpr) { }

    void Fill(Histogram2D& h, XCDFFile& f) {

      NumericalExpression<double> xne(xExpr_, f);
      NumericalExpression<double> yne(yExpr_, f);
      NumericalExpression<double> wne(wExpr_, f);

      // Check the relation between two axis nodes and the weight node
      // Important to get all 3 relations to ensure the fields can all
      // be compared.
      DynamicFiller2DPtr filler;

      try {
        filler = GetFiller(xne.GetNodeRelationType(yne),
                           xne.GetNodeRelationType(wne),
                           yne.GetNodeRelationType(wne), xne, yne, wne);
      } catch (XCDFException& e) {
        // We can still make the histogram by drawing all,all against all
        filler = DynamicFiller2DPtr(new AnyFiller2D(xne, yne, wne));
      }

      while (f.Read()) {
        filler->Fill(h);
      }
    }

  private:

    std::string xExpr_;
    std::string yExpr_;
    std::string wExpr_;
};

#endif // XCDF_UTILITY_HISTOGRAM_FILLER_H_INCLUDED
