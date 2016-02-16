
/*
Copyright (c) 2014, James Braun
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

#ifndef XCDF_FIELD_DATA_BASE_INCLUDED_H
#define XCDF_FIELD_DATA_BASE_INCLUDED_H

#include <xcdf/XCDFPtr.h>
#include <xcdf/XCDFDefs.h>
#include <xcdf/XCDFBlockData.h>

#include <string>
#include <cmath>
#include <stdint.h>

/*!
 * @class XCDFFieldDataBase
 * @author Jim Braun
 * @brief Mostly virtual base class for XCDF data storage/manipulation classes.
 * Allows placing all XCDFFieldData instances into one container.
 */
class XCDFFieldDataBase {

  public:

    XCDFFieldDataBase(const XCDFFieldType type,
                      const std::string& name) : type_(type),
                                                 name_(name) { }

    virtual ~XCDFFieldDataBase() { }

    virtual void Load(XCDFBlockData& data) = 0;
    virtual void Dump(XCDFBlockData& data) = 0;
    virtual void Stash() = 0;
    virtual void Unstash() = 0;
    virtual void Clear() = 0;
    virtual uint64_t GetStashSize() const = 0;
    virtual void ZeroAlign() = 0;
    virtual void SetActiveSize(const uint32_t activeSize) = 0;
    virtual void Shrink() = 0;
    virtual void Reset() = 0;
    virtual uint32_t GetActiveSize() const = 0;
    virtual uint64_t GetRawResolution() const = 0;
    virtual unsigned GetSize() const = 0;
    virtual unsigned GetExpectedSize() const = 0;
    virtual uint64_t GetRawActiveMin() const = 0;
    virtual void SetRawActiveMin(uint64_t rawActiveMin) = 0;
    virtual uint64_t GetRawGlobalMin() const = 0;
    virtual uint64_t GetRawGlobalMax() const = 0;
    virtual uint64_t GetTotalBytes() const = 0;
    virtual void SetRawGlobalMin(uint64_t rawGlobalMin) = 0;
    virtual void SetRawGlobalMax(uint64_t rawGlobalMax) = 0;
    virtual void SetTotalBytes(uint64_t totalBytes) = 0;
    virtual void ClearBitsProcessed() = 0;
    virtual void CalculateGlobals() = 0;
    virtual bool GlobalsSet() const = 0;

    XCDFFieldType GetType() const {return type_;}

    const std::string& GetName() const {return name_;}

    virtual bool HasParent() const {return false;}

    /// Use the empty string to denote no parent.
    virtual const std::string& GetParentName() const {return NO_PARENT;}

    /// Get the parent field
    virtual const XCDFFieldDataBase* GetParent() const {
      XCDFFatal("GetParent(): Field " << GetName() << " has no parent");
      // Mandatory return that'll never be reached.
      return NULL;
    }

    /// Simple field type checks
    bool IsUnsignedIntegerField() const {
      return type_ == XCDF_UNSIGNED_INTEGER;
    }

    bool IsSignedIntegerField() const {
      return type_ == XCDF_SIGNED_INTEGER;
    }

    bool IsFloatingPointField() const {
      return type_ == XCDF_FLOATING_POINT;
    }

  protected:

    /// Data type stored in the field.
    XCDFFieldType type_;

    /// Name of the field
    std::string name_;
};

typedef XCDFPtr<XCDFFieldDataBase> XCDFFieldDataBasePtr;
typedef XCDFPtr<const XCDFFieldDataBase> XCDFFieldDataBaseConstPtr;

#endif // XCDF_FIELD_DATA_BASE_INCLUDED_H
