.. _userguide:

User guide
==========

C++ interface
-------------

Help about the C++ interface and its options can be obtained with the command ``xcdf -h``,

.. code-block::

    Usage: xcdf [verb] {infiles}

        verb:    Description
        ----     -----------

        version  Print XCDF version information and exit.

        info     Print descriptions of each field in the file.

        dump     Output data event-by-event in a human-readable format.

        count    {-e expression} Count the number of events in the file.
                If optional expression is supplied, count only the
                events that satisfy the expression.

        csv      Output data into comma-separated-value format.

        check    Check if input is a valid XCDF file and check internal
                data checksums

        select-fields "field1, field2, ..." {-o outfile} {infiles}:

                        Copy the given fields and write the result to a
                        new XCDF file at the path specified by
                        {-o outfile}, or stdout if outfile is unspecified.
                        A wildcard '*' character is allowed in matching
                        field names.

        select "boolean expression" {-o outfile} {infiles}:

                        Copy events satisfying the given boolean
                        expression into a new XCDF file at the path
                        specified by {-o outfile}, or stdout if outfile
                        is unspecified. The expression is of the form
                        e.g.: "field1 == 0" to select all events
                        where the value of field1 is zero.  The variable
                        "currentEventNumber" refers to the current
                        event in the file.

        paste {-d delimeter} {-c existingfile} {-o outfile} {infile}:

                        Copy events in CSV format from infile (or stdin,
                        if unspecified) into outfile (or stdout if unspecified).
                        If an existing XCDF file is specified with -c, the
                        fields are added to the existing file. A delimeters can be 
                        specified but will default to commas if unspecified. 

        recover {-o outfile} {infiles} Recover a corrupt XCDF file.

        add-alias name "expression" {-o outfile} {infiles}:

                        Add an alias to "infile" consisting of a numerical                    expression.  The expression may contain fields, e.g.                    'xcdf add-alias myAlias "abs(field1)" myFile.xcd                    creates the alias 'myAlias' that contains the absolute                    value of XCDF field 'field1'. 'myAlias' may then be                    selections and other expressions.  If an output file is not                    specified, the alias is added in-place to the existing                    file if possible.  If an output file is specified,                    adding an alias with the same name as an existing alias                    replaces that alias with the new expression.  Note that                    aliases added in-place are not available when reading an                    XCDF file using a stream or pipe.

        remove-alias name {-o outfile} {infiles}:

                        Remove an alias from "infile".  If an output file is                    not specified, the removal is done in-place if possible.                    Only aliases added in-place may be removed in-place.    histogram "histogram expression" {infiles}:

                        Create a histogram from the selected files according to
                        the specified expression.  Valid expressions are of the form
                        "nbins, min, max, expr" or "nbins, expr", dynamically
                        determining the min and max.
                        "expr" is of the form e.g. "fieldName*3.14159".
                        An optional expression may be appended
                        to weight the entry, e.g. "100, 0, 1, field1, field2" would
                        create a histogram of field1 with 100 bins from 0 to 1,
                        weighting each entry by the value of field2.

        histogram2d "histogram expression" {infiles}:

                        Create a 2D histogram from the selected files according to
                        the specified expression.  Valid expressions are of the form
                        "nbinsX, minX, maxX, exprX, nbinsY, minY, maxY, exprY" or
                        "nbinsX, exprX, nbinsY, exprY", dynamically determining min
                        and max.  An optional expression may be appended to weight the
                        entry.

        comments {infiles} Display all comments from an XCDF file

        add-comment "comment" {-o outfile} {infiles} Add comment to an XCDF file

        remove-comments {-o outfile} {infiles} Remove all comments from an XCDF file

        compare file1 file2 Compare the contents of file1 and file2



    Note: if input/output file(s) are not specified, they are
    read/written from/to stdin/stdout.

    Multiple input files are allowed.

Python interface
----------------

.. important::

    After XCDF v3.00.03 support for Python2 has been dropped.

This is a simple tutorial on how to write an XCDF file and
read its contents in Python3 using numpy data structures.

Write an XCDF file
^^^^^^^^^^^^^^^^^^

First we will write a file containing two events (rows),
each defined by two fields, ``A`` and ``B``.
The latter is a child of the former, which means that
for each event the value of ``A`` dictates
how many elements should be contained in ``B``.

In particular we declare ``A`` as a integer-type field
and ``B`` a float-type field with a resolution of 0.1.

We will also write a small comment to the file header
at the end.

.. exec_code::
   :filename: py3_bindings_write.py

Read an XCDF file
^^^^^^^^^^^^^^^^^

Now let's re-open the same file and read back what we wrote in it.

Notice that, when read back, the data associated to the ``B`` field
of the first event is the array ``[0. , 0.3, 0.5, 0.8]``
and not what we injected at writing time, ``[0.  , 0.25, 0.5 , 0.75])``,
because we declared (i.e. allocated) the ``B`` field with a resolution
of ``0.1``.

.. exec_code::
   :filename: py3_bindings_read.py

Append to an XCDF file
^^^^^^^^^^^^^^^^^^^^^^

Now let's add some more data to this existing file.

.. exec_code::
   :filename: py3_bindings_append.py