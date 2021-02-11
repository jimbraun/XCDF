
XCDF: The eXplicitly Compacted Data Format
--------------------------------------------------------------------------------

I.   Overview
II.  Description
  A. Bit Packing
  B. XCDF

--------------------------------------------------------------------------------

I. Overview

  The eXplicitly Compacted Data Format (XCDF) is written and maintained by Jim
  Braun at the University of Maryland.  XCDF is a binary data format designed
  to store data fields with a user-specified accuracy.  The library uses
  bit-packing to store the field at the given accuracy for a given set of
  values, and therefore provides substantial compression.

  Python bindings for XCDF are written and maintained by Segev BenZvi at the
  University of Wisconsin-Madison.

II. Description

  A. Bit Packing

  Bit-packing is the process of compressing data by reducing the number of bits
  needed to represent data values, then packing those bits serially for each
  datum, ignoring word boundaries. This technique is particularly effective for
  floating-point data, as precision in the mantissa beyond the needed
  resolution has high entropy and is poorly compressed by generic tools (e.g.
  GZip). As an example, suppose we wish to record zenith and azimuth angles of
  cosmic ray events from an experiment with an angular resolution of one
  degree. Since the resolution is one degree, the data can be stored in bins of
  0.1 degree without significant degradation. Therefore, we create 1800 bins in
  zenith angle, representing 0-180 degrees, and we create 3600 bins in azimuth
  angle, representing 0-360 degrees. We need a minimum of 11 bits to store
  zenith angle (2^11 = 2048) and 12 bits to store azimuth angle (2^12 = 4096).
  Successive zenith/azimuth angle pairs will be written consecutively in the
  file zenith1,azimuth1,zenith2,azimuth2,zenith3, etc., requiring 23 total bits
  for each pair. This is nearly a factor of 3 less space than needed by writing
  32-bit floating-point values directly into the file, which would require 64
  bits for each zenith,azimuth pair.

  This scheme has two major drawbacks, however:

  1. Changes to the data format require significant updates to data readers, as
     well as a data versioning scheme. Examples:
    a) Suppose one later improves the experiment and needs to write data in
       0.05 degree bins. This requires adding one bit to both the azimuth and
       zenith fields.
    b) Suppose a new field (e.g. energy) is added. Now three fields are written
       sequentially for each event (zenith,azimuth,energy).

  2. The maximum and minimum values of each data field must be specified.

  XCDF solves these problems by providing a layer of abstraction between the
  data and data readers, and by dynamically calculating maximum and minimum
  field values as data are written.

  B. XCDF

  XCDF stores groups of fields that have a common index (nTuples). Each index
  is referred to as an event. Data fields are added to the file by specifying a
  name, resolution, and field type (unsigned/signed integer, or
  floating-point). Vector fields (fields with multiple entries per event) are
  also supported, provided that the length of the vector is the value of one of
  the fields present in the event. The data is written into each field, and
  then the event is written into the file.

  The file is written as blocks of events. The field data is stored in a
  temporary buffer until either the set number of events in the block is
  reached (default: 1000 events), or the set maximum block size is reached
  (default: 100 MB). Once the block is filled, the minimum and maximum values
  of each field are calculated for the block and written to the file. For each
  event, each field is bit-packed using the calculated minimum and maximum
  field values and the specified field resolution, and is then written
  sequentially into the file.

  The above procedure is reversed when reading the file back. When a data block
  is read, the field minimum and maximum are recovered. The data fields are
  filled each time an event is read, converting the bit-packed datum into the
  original field value (accurate to the specified field resolution).

