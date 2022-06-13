#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>

#include "xcdf/XCDFFile.h"

namespace py = pybind11;

std::string getVersion() {
    std::stringstream ss;
    ss << XCDF_MAJOR_VERSION
        << "." << XCDF_MINOR_VERSION
        << "." << XCDF_PATCH_VERSION;
    return ss.str();
}

PYBIND11_MODULE(xcdf, m) {
    m.doc() = "Python Bindings for XCDF";
    m.attr("__version__") = getVersion();

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
        .def("__iter__", [](XCDFFile& self){ return &self;})
        .def("__next__", [](XCDFFile& self){
            int ret = self.Read();
            if (ret == 0) {
                throw pybind11::stop_iteration();
            }
        })
        ;
}
