//
// Created by root on 03.11.18.
//
#include <string>
#include <sstream>
#include <vector>


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
#include <reverseengine/shape.hh>
#include <reverseengine/nearest_neighbors.hh>
#include <reverseengine/external.hh>
#include <reverseengine/core.hh>
//
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace RE {
    // static PyObject *
    // addressof(PyObject *self, PyObject *obj)
    // {
    //     if (CDataObject_Check(obj))
    //         return PyLong_FromVoidPtr(((CDataObject *)obj)->b_ptr);
    //     PyErr_SetString(PyExc_TypeError,
    //                     "invalid type");
    //     return NULL;
    // }

    py::object addressof(py::object& o)
    {
        py::object addr = o.attr("_address_")();
        return addr;
    }
}

PYBIND11_MODULE(master, m) {
    m.doc() = "pybind11 wrapper for Reverse Engine";
    m.def("addressof", &RE::addressof, "gets the address where the function pointer is stored");

    ///*
    // TODO[high]: split to modules to speedup build time
    // reverseengine/shape.hh
    py::class_<RE::Shape>(m, "Shape")
            .def_readwrite("x", &RE::Shape::x)
            .def_readwrite("y", &RE::Shape::y)
            .def("move", &RE::Shape::move)
            .def("area", &RE::Shape::area)
            .def("perimeter", &RE::Shape::perimeter)
            .def_readwrite_static("nshapes", &RE::Shape::nshapes)
            ;
    py::class_<RE::Circle, RE::Shape>(m, "Circle")
            .def(py::init<double>())
            ;
    py::class_<RE::Square, RE::Shape>(m, "Square")
            .def(py::init<double>())
            ;

    // reverseengine/nearest_neighbors.hh
    py::class_<RE::Point>(m, "Point")
            .def(py::init<>())
            .def(py::init<double, double>())
            .def_readwrite("x", &RE::Point::x)
            .def_readwrite("y", &RE::Point::y)
            ;
    py::class_<RE::NearestNeighbors>(m, "NearestNeighbors")
            .def(py::init<>())
            .def_readwrite("points", &RE::NearestNeighbors::points)
            .def("nearest", &RE::NearestNeighbors::nearest)
            ;

    // reverseengine/external.hh
    m.def("execute", &RE::execute);
    py::class_<RE::CProcess>(m, "CProcess")
            .def(py::init<const std::string &, const std::string &, const std::string &>())
            .def("__repr__", &RE::CProcess::str)
            ;
    m.def("getProcesses", &RE::getProcesses);
    m.def("get_mem_total", &RE::get_mem_total, "list(MemTotal, MemFree, MemAvailable, Cached)[i]", py::arg("i") = 0);
    m.def("get_mem_free", &RE::get_mem_free);

    // reverseengine/core.hh
    py::class_<RE::Handle>(m, "Handle")
            .def(py::init<>())
            .def(py::init<pid_t>())
            .def(py::init<const std::string &>())
            .def_readwrite("pid", &RE::Handle::pid)
            .def_readwrite("title", &RE::Handle::title)
            .def_readwrite("regions_ignored", &RE::Handle::regions_ignored)
            .def_readwrite("regions", &RE::Handle::regions)
            .def("attach", (void (RE::Handle::*)(pid_t)) &RE::Handle::attach, "Attach by PID")
            .def("attach", (void (RE::Handle::*)(const std::string &)) &RE::Handle::attach, "Attach by title")
            .def("get_path", &RE::Handle::get_path, "Get executable path")
            .def("get_working_directory", &RE::Handle::get_working_directory, "Get executable working directory")
            .def("is_valid", &RE::Handle::is_valid, "Is handle valid")
            .def("is_running", &RE::Handle::is_running, "Is handle running")
            .def("is_good", &RE::Handle::is_running, "Is handle valid and running")
            .def("read", [](RE::Handle& self, uintptr_t& address, uintptr_t& out, size_t& size) -> auto {
                return self.read(address, reinterpret_cast<void *>(out), size);
            }, "Read from handle", py::arg("address"), py::arg("out"), py::arg("size"))
            .def("read", [](RE::Handle& self, uintptr_t& address, size_t& size) -> auto {
                // fixme[critical]: !! memory leak !!
                uint8_t *b = new uint8_t[size];
                size_t s = self.read(address, b, size);
                if (s == RE::Handle::npos)
                    s = 0;
                return py::bytes(reinterpret_cast<char *>(b), s);
            }, "Read from handle", py::arg("address"), py::arg("size"))
            .def("write", &RE::Handle::write, "Write to handle", py::arg("address"), py::arg("in"), py::arg("size"))
            .def("read_cached", &RE::Handle::read_cached, "Read from handle", py::arg("address"), py::arg("out"), py::arg("size"))
            .def("update_regions", &RE::Handle::update_regions, "Update maps")
            .def("get_region_by_name", &RE::Handle::get_region_by_name)
            .def("get_region_of_address", &RE::Handle::get_region_of_address)
            .def("find_pattern", &RE::Handle::find_pattern)
            .def("get_call_address", &RE::Handle::get_call_address)
            .def("get_absolute_address", &RE::Handle::get_absolute_address)
            ;
//*/

    // reverseengine/value.hh
    // todo[high] add other classes
    pybind11::class_<RE::mem64_t> (m, "mem64_t")
            .def(pybind11::init<>())
            .def_property("i8",  [](RE::mem64_t& self) -> auto { return self.i8;  }, [](RE::mem64_t& self, typeof(self.i8 ) set) { self.i8  = set; })
            .def_property("u8",  [](RE::mem64_t& self) -> auto { return self.u8;  }, [](RE::mem64_t& self, typeof(self.u8 ) set) { self.u8  = set; })
            .def_property("i16", [](RE::mem64_t& self) -> auto { return self.i16; }, [](RE::mem64_t& self, typeof(self.i16) set) { self.i16 = set; })
            .def_property("u16", [](RE::mem64_t& self) -> auto { return self.u16; }, [](RE::mem64_t& self, typeof(self.u16) set) { self.u16 = set; })
            .def_property("i32", [](RE::mem64_t& self) -> auto { return self.i32; }, [](RE::mem64_t& self, typeof(self.i32) set) { self.i32 = set; })
            .def_property("u32", [](RE::mem64_t& self) -> auto { return self.u32; }, [](RE::mem64_t& self, typeof(self.u32) set) { self.u32 = set; })
            .def_property("i64", [](RE::mem64_t& self) -> auto { return self.i64; }, [](RE::mem64_t& self, typeof(self.i64) set) { self.i64 = set; })
            .def_property("u64", [](RE::mem64_t& self) -> auto { return self.u64; }, [](RE::mem64_t& self, typeof(self.u64) set) { self.u64 = set; })
            .def_property("f32", [](RE::mem64_t& self) -> auto { return self.f32; }, [](RE::mem64_t& self, typeof(self.f32) set) { self.f32 = set; })
            .def_property("f64", [](RE::mem64_t& self) -> auto { return self.f64; }, [](RE::mem64_t& self, typeof(self.f64) set) { self.f64 = set; })
            //todo[high] add array support (bytes and chars)
            .def("__bytes__", [](RE::mem64_t& self) -> auto {
                return py::bytes(self.chars, sizeof(self.chars));
            })
            .def("__str__", [](RE::mem64_t& self) -> auto {
                using namespace std::string_literals;
                std::ostringstream bytes;
                bytes<<std::hex<<std::noshowbase<<std::setfill('0');
                for(auto *cur = self.bytes; cur < self.bytes + sizeof(self.bytes); cur++) {
                    bytes<<"\\x"<<std::setw(2)<<static_cast<unsigned>(*cur);
                }
                std::string result = "{\n"s
                                   + "    i8 : "+std::to_string(self.i8 )+"\n"
                                   + "    u8 : "+std::to_string(self.u8 )+"\n"
                                   + "    i16: "+std::to_string(self.i16)+"\n"
                                   + "    u16: "+std::to_string(self.u16)+"\n"
                                   + "    i32: "+std::to_string(self.i32)+"\n"
                                   + "    u32: "+std::to_string(self.u32)+"\n"
                                   + "    i64: "+std::to_string(self.i64)+"\n"
                                   + "    u64: "+std::to_string(self.u64)+"\n"
                                   + "    f32: "+std::to_string(self.f32)+"\n"
                                   + "    f64: "+std::to_string(self.f64)+"\n"
                                   + "    bytes: "+bytes.str()+"\n"
                                   + "}";
                // std::clog<<"returning "<<result<<std::endl;
                return result;
            })
            .def("_address_", [](RE::mem64_t& self) -> uintptr_t { return reinterpret_cast<uintptr_t>(&self); }, "the base object")
            ;

    //todo[high] add other RE's headers
}
