#include <sstream>
#include <string>
#include <cstdio>
#include <limits>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <xcdf/XCDF.h>
#include "xcdf/XCDFFieldDescriptor.h"
#include "xcdf/XCDFFile.h"

namespace py = pybind11;

std::string getVersion()
{
    std::stringstream ss;
    ss << XCDF_MAJOR_VERSION
       << "." << XCDF_MINOR_VERSION
       << "." << XCDF_PATCH_VERSION;
    return ss.str();
}

int write_test_file()
{
    XCDFFile f("test_file_for_pybindings.xcd", "w");

    XCDFUnsignedIntegerField field1 =
        f.AllocateUnsignedIntegerField("field1", 1);
    XCDFUnsignedIntegerField field2 =
        f.AllocateUnsignedIntegerField("field2", 1, "field1");
    XCDFFloatingPointField field3 =
        f.AllocateFloatingPointField("field3", 0.1);
    XCDFFloatingPointField field4 =
        f.AllocateFloatingPointField("field4", 0.1);
    XCDFFloatingPointField field5 =
        f.AllocateFloatingPointField("field5", 0.1);
    XCDFUnsignedIntegerField field6 =
        f.AllocateUnsignedIntegerField("field6", 1);
    XCDFFloatingPointField field7 =
        f.AllocateFloatingPointField("field7", 0.);
    // 2D vector
    XCDFUnsignedIntegerField field8 =
        f.AllocateUnsignedIntegerField("field8", 1, "field2");
    // 3D vector
    XCDFFloatingPointField field9 =
        f.AllocateFloatingPointField("field9", 0.5, "field8");
    // Header alias
    f.CreateAlias("testAlias", "field1 + 1");

    field1 << 2;
    field2 << 1 << 1;
    field3 << 0.1;
    field4 << 5.;
    field5 << 5.;
    field6 << 0xDEADBEEFDEADBEEFULL;
    field7 << 0.12;
    field8 << 2 << 1;
    field9 << 1. << 2. << 3.;

    std::cout << f.Write() << std::endl;

    field1 << 2;
    field2 << 1 << 3;
    field3 << 0.3;

    // Write a NaN into field4
    field4 << std::numeric_limits<double>::signaling_NaN();

    // Write an inf into field5
    field5 << std::numeric_limits<double>::infinity();

    field6 << 0xDEADBEEFDEADBEEFULL;

    field7 << 0.12;

    field8 << 2 << 2 << 1 << 1;
    field9 << 1. << 2. << 3. << 4. << 5. << 6.;

    std::cout << f.Write() << std::endl;

    for (int k = 0; k < 1000; k++)
    {
        field1 << 2;
        field2 << 1 << 3;
        field3 << 0.3;
        field4 << std::numeric_limits<double>::signaling_NaN();
        field5 << std::numeric_limits<double>::infinity();
        field6 << 0xDEADBEEFDEADBEEFULL;
        field7 << 0.12;
        field8 << 2 << 2 << 1 << 1;
        field9 << 1. << 2. << 3. << 4. << 5. << 6.;
        f.Write();
    }

    f.AddComment("test file");

    // Trailer alias
    f.CreateAlias("testTrailerAlias", "double(testAlias + 2)");

    f.Close();

    return 0;
}

struct DictBuilder
{
    py::dict data;

    template <typename T>
    void operator()(const XCDFField<T> &field)
    {

        py::str key = field.GetName();
        // return vector fields as numpy arrays
        if (field.HasParent())
        {
            py::array_t<T> array(field.GetSize());
            py::buffer_info buffer = array.request();
            T *ptr = static_cast<T *>(buffer.ptr);

            for (size_t i = 0; i < field.GetSize(); i++)
            {
                ptr[i] = field[i];
            }

            data[key] = array;
            // scalar fields as python objects (None if not set)
        }
        else
        {
            if (field.GetSize() == 0)
            {
                data[key] = py::none();
            }
            else
            {
                data[key] = field[0];
            }
        }
    }
};

PYBIND11_MODULE(xcdf, m)
{
    m.doc() = "Python Bindings for XCDF";
    m.attr("__version__") = getVersion();

    m.def("write_test_file", &write_test_file, "Write an XCDF test file in C++");

    py::class_<XCDFFieldDescriptor>(m, "FieldDescriptor")
        .def("__repr__", [](XCDFFieldDescriptor &self)
             {
            std::stringstream ss;
            ss << "FieldDescriptor(name=" << self.name_
                << ", type=" << static_cast<int>(self.type_)
                << ", parent=" << self.parentName_
                << ", raw_resolution=" << self.rawResolution_
                << ")";

            return ss.str(); })
        .def_property_readonly("name", [](XCDFFieldDescriptor &self)
                               { return self.name_; })
        .def_property_readonly("type", [](XCDFFieldDescriptor &self)
                               { return self.type_; })
        .def_property_readonly("parent", [](XCDFFieldDescriptor &self)
                               { return self.parentName_; })
        .def_property_readonly("raw_resolution", [](XCDFFieldDescriptor &self)
                               { return self.rawResolution_; });

    py::class_<XCDFFile>(m, "File")
        .def(py::init<const char *, const char *>())
        .def("close", &XCDFFile::Close)
        .def("seek", &XCDFFile::Seek)
        .def("rewind", &XCDFFile::Rewind)
        .def("__len__", &XCDFFile::GetEventCount)
        .def("__in__", &XCDFFile::HasField)
        .def_property_readonly("version", &XCDFFile::GetVersion)
        .def_property_readonly("n_fields", &XCDFFile::GetNFields)
        .def_property_readonly("is_simple", &XCDFFile::IsSimple)
        .def_property_readonly("is_open", &XCDFFile::IsOpen)
        .def_property_readonly("comments", [](XCDFFile &self)
                               {
            std::vector<std::string> comments;
            comments.reserve(self.GetNComments());
            for (auto it = self.CommentsBegin(); it != self.CommentsEnd(); it++) {
                comments.push_back(*it);
            }
            return comments; })
        .def_property_readonly("fields", [](XCDFFile &self)
                               {
            std::vector<XCDFFieldDescriptor> fields;
            fields.reserve(self.GetNFields());
            for (auto it = self.FieldDescriptorsBegin(); it != self.FieldDescriptorsEnd(); it++) {
                fields.push_back(*it);
            }
            return fields; })
        .def_property_readonly("field_names", [](XCDFFile &self)
                               {
            std::vector<std::string> names;
            names.reserve(self.GetNFields());
            for (auto field = self.FieldDescriptorsBegin(); field != self.FieldDescriptorsEnd(); field++) {
                names.push_back(field->name_);
            }
            return names; })
        .def("__iter__", [](XCDFFile &self)
             { return &self; })
        .def("__next__", [](XCDFFile &self)
             {
            int ret = self.Read();
            if (ret == 0) {
                throw pybind11::stop_iteration();
            }
            DictBuilder builder;
            self.ApplyFieldVisitor(builder);
            return builder.data; })
        .def("__enter__", [](XCDFFile &self)
             { return &self; })
        .def("__exit__", [&](XCDFFile &r, void *exc_type, void *exc_value, void *traceback)
             { r.Close();  });
}
