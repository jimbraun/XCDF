
/*
 * Minimal shared ptr implementation, to avoid c++11 and boost dependencies
 */

#ifndef XCDF_PTR_INCLUDED_H
#define XCDF_PTR_INCLUDED_H

class ReferenceCount {

  public:

	  ReferenceCount() : referenceCnt_(1) { }

	  unsigned AddReference() {return ++referenceCnt_;}
	  unsigned RemoveReference() {return --referenceCnt_;}
	  unsigned GetCount() const {return referenceCnt_;}

  private:

	  unsigned referenceCnt_;
};

template <typename T>
class XCDFPtr {

  public:

    XCDFPtr() : t_(NULL), rc_(NULL) { }
    XCDFPtr(T* t) : t_(t), rc_(new ReferenceCount()) { }
	  XCDFPtr(const XCDFPtr<T>& p) : t_(p.t_), rc_(p.rc_) {AddReference();}

	  XCDFPtr<T>& operator=(const XCDFPtr<T>& p) {

      // Skip if self-assignment
      if (this == &p) {
        return *this;
      }

      // Check reference count for possible deallocation
      DecrementReferenceCount();

      // Copy pointers
      t_ = p.t_;
      rc_ = p.rc_;
      AddReference();

      return *this;
	  }

	  ~XCDFPtr() {
	    DecrementReferenceCount();
	  }

	  T& operator*() const {return *t_;}
    T* operator->() const {return t_;}

    bool IsNull() const {return t_ == NULL;}
    unsigned GetReferenceCount() const {
      return rc_ == NULL ? 0 : rc_->GetCount();
    }

  private:

	  T* t_;
	  ReferenceCount* rc_;

	  void DecrementReferenceCount() {
	    if (rc_) {
	      // If rc_ is non-null, we are managing memory for rc_ and possibly t_
  	    if (rc_->RemoveReference() == 0) {
	        delete t_;
	        delete rc_;
	        t_ = NULL;
	        rc_ = NULL;
	      }
	    }
	  }

	  void AddReference() {
	    // Need to wrap in case we don't have memory to manage
	    if (rc_) {
	      rc_->AddReference();
	    }
	  }
};

template <typename T>
XCDFPtr<T> xcdf_shared(T* t) {return XCDFPtr<T>(t);}

#endif // XCDF_PTR_INCLUDED_H
