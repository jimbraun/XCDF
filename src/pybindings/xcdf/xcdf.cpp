#include <sstream>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "xcdf/XCDFFieldDescriptor.h"
#include "xcdf/XCDFFile.h"

namespace py = pybind11;

std::string getVersion() {
    std::stringstream ss;
    ss << XCDF_MAJOR_VERSION
        << "." << XCDF_MINOR_VERSION
        << "." << XCDF_PATCH_VERSION;
    return ss.str();
}


struct DictBuilder {
    py::dict data;

    template<typename T>
    void operator()(const XCDFField<T>& field) {

        py::str key = field.GetName();
        // return vector fields as numpy arrays
        if (field.HasParent()) {
            py::array_t<T> array(field.GetSize());
            py::buffer_info buffer = array.request();
            T* ptr = static_cast<T*>(buffer.ptr);

            for (size_t i=0; i < field.GetSize(); i++) {
                ptr[i] = field[i];
            }

            data[key] = array;
        // scalar fields as python objects (None if not set)
        } else {
            if (field.GetSize() == 0) {
                data[key] = py::none();
            } else {
                data[key] = field[0];
            }
        }
    }
};

PYBIND11_MODULE(xcdf, m) {
    m.doc() = "Python Bindings for XCDF";
    m.attr("__version__") = getVersion();


    py::class_<XCDFFieldDescriptor>(m, "FieldDescriptor")
        .def("__repr__", [](XCDFFieldDescriptor& self) {
            std::stringstream ss;
            ss << "FieldDescriptor(name=" << self.name_
                << ", type=" << static_cast<int>(self.type_)
                << ", parent=" << self.parentName_
                << ", raw_resolution=" << self.rawResolution_
                << ")";

            return ss.str();
        })
        ;


    py::class_<XCDFFile>(m, "File")
        .def(py::init<const char*, const char*>())
        .def("__len__", &XCDFFile::GetEventCount)
        .def("__in__", &XCDFFile::HasField)
        .def_property_readonly("version", &XCDFFile::GetVersion)
        .def_property_readonly("n_fields", &XCDFFile::GetNFields)
        .def_property_readonly("is_simple", &XCDFFile::IsSimple)
        .def_property_readonly("comments", [](XCDFFile& self) {
            std::vector<std::string> comments;
            comments.reserve(self.GetNComments());
            for (auto it = self.CommentsBegin(); it != self.CommentsEnd(); it++) {
                comments.push_back(*it);
            }
            return comments;
        })
        .def_property_readonly("fields", [](XCDFFile& self) {
            std::vector<XCDFFieldDescriptor> fields;
            fields.reserve(self.GetNFields());
            for (auto it = self.FieldDescriptorsBegin(); it != self.FieldDescriptorsEnd(); it++) {
                fields.push_back(*it);
            }
            return fields;
        })
        .def_property_readonly("field_names", [](XCDFFile& self) {
            std::vector<std::string> names;
            names.reserve(self.GetNFields());
            for (auto field = self.FieldDescriptorsBegin(); field != self.FieldDescriptorsEnd(); field++) {
                names.push_back(field->name_);
            }
            return names;
        })
        .def("__iter__", [](XCDFFile& self){ return &self;})
        .def("__next__", [](XCDFFile& self){
            int ret = self.Read();
            if (ret == 0) {
                throw pybind11::stop_iteration();
            }
            DictBuilder builder;
            self.ApplyFieldVisitor(builder);
            return builder.data;
        })
        ;
}
