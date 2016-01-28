
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

#ifndef XCDF_DATA_MANAGER_INCLUDED_H
#define XCDF_DATA_MANAGER_INCLUDED_H

#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFDefs.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFDataManager
 * @author Jim Braun
 * @brief Interface between XCDFFile and the field data.  Exists to couple
 * a field data object with a parent field.
 */

class XCDFDataManager {


  public:

    template <typename T>
    XCDFDataManager(const XCDFFieldType type,
                    const XCDFFieldDataBasePtr fieldData,
                    XCDFField<uint64_t>* parent = NULL) :
                                                     type_(type),
                                                     fieldData_(fieldData),
                                                     parent_(parent) { }

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.
     */
    void AddIntegerRepresentation(uint64_t datum) {

      fieldData_->AddIntegerRepresentation(datum);
    }

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.  Need range check in the
     *  case of file append
     */
    void AddIntegerRepresentationAppend(uint64_t datum) {

      fieldData_->AddIntegerRepresentationAppend(datum);
    }

    /*
     *  Get the #resolution units between active min and datum
     *  at the given index.
     */
    uint64_t At(const int index) const {

      return fieldData_->GetIntegerRepresentation(index);
    }

    /*
     * Align the field active min to be an integer number of resolution
     * units from zero.  This avoids field data that changes value as
     * the active min changes block-to-block.
     */
    void ZeroAlign() {fieldData_->ZeroAlign();}

    /// Set the active size of the field, as when reading back a file.
    void SetActiveSize(const uint32_t activeSize) {
      fieldData_->SetActiveSize(activeSize);
    }

    /*
     * Enter in data of known limits.
     */
    void AddUnchecked(uint64_t value) {fieldData_->AddUnchecked(value);}

    /*
     * Clear the data from the XCDFFieldData vector.
     */
    void Clear() {fieldData_->Clear();}

    /*
     * Shrink the underlying buffer in the XCDFFieldData vector.
     * This is a reallocation and invalidates iterators.
     */
    void Shrink() {fieldData_->Shrink();}

    /*
     * Reset the data from the XCDFFieldData vector (e.g. a block had
     * just been completed).
     */
    void Reset() {fieldData_->Reset();}

    /*
     * Get the compressed size of each datum in the field (in bytes)
     * for the current block
     */
    uint32_t GetActiveSize() const {return fieldData_->GetActiveSize();}

    /*
     * Get the resolution of the field.
     */
    uint64_t GetResolution() const {return fieldData_->GetResolution();}

    /*
     *  Get the number of entries in the field in the current event
     */
    uint32_t GetSize() const {return fieldData_->GetSize();}

    const std::string& GetName() const {return fieldData_->GetName();}

    /*
     * Get the minimum value of the field in the current block.
     */
    uint64_t GetActiveMin() const {return fieldData_->GetActiveMin();}


    /// Set the active min, as when reading back a file
    void SetActiveMin(uint64_t activeMin) {
      fieldData_->SetActiveMin(activeMin);
    }

    /// Check if we have a parent
    bool HasParent() const {return parent_ != NULL;}

    /// Get the parent field
    const XCDFField<uint64_t>& GetParent() const {return *parent_;}

    /// Get a pointer to the field data object
    XCDFFieldDataBasePtr GetFieldDataPtr() {return fieldData_;}


  private:

    /// Data type stored in the field.
    XCDFFieldType type_;

    /// Parent or dummy field, depending on hasParent_
    XCDFField<uint64_t>* parent_;

    /// Backing field data
    XCDFFieldDataBasePtr fieldData_;
};

#endif // XCDF_DATA_MANAGER_INCLUDED_H
