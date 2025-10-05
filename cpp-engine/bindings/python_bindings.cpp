#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "../include/BinaryFuseWrapper.hpp"
#include "../include/MortonFilterWrapper.hpp"
#include "../include/numa_optimized_filter.hpp"

namespace py = pybind11;

// CRITICAL: This must match the filename without extension
PYBIND11_MODULE(llamashield_py, m) {
    m.doc() = "LlamaShield high-performance URL filtering engine";
    
    // BinaryFuseWrapper binding
    py::class_<BinaryFuseWrapper>(m, "BinaryFuseWrapper")
        .def(py::init<>())
        .def("build_from_keys", &BinaryFuseWrapper::build_from_keys)
        .def("contains", &BinaryFuseWrapper::contains)
        .def("save_to_file", &BinaryFuseWrapper::save_to_file)
        .def("load_from_file", &BinaryFuseWrapper::load_from_file)
        .def_static("hash_url", &BinaryFuseWrapper::hash_url);
    
    // MortonFilterWrapper binding  
    py::class_<MortonFilterWrapper>(m, "MortonFilterWrapper")
        .def(py::init<>())
        .def("initialize", &MortonFilterWrapper::initialize)
        .def("insert", &MortonFilterWrapper::insert)
        .def("contains", &MortonFilterWrapper::contains)
        .def("insert_batch", &MortonFilterWrapper::insert_batch)
        .def("contains_batch", &MortonFilterWrapper::contains_batch)
        .def("get_count", &MortonFilterWrapper::get_count)
        .def("get_memory_usage", &MortonFilterWrapper::get_memory_usage)
        .def("save_to_file", &MortonFilterWrapper::save_to_file)
        .def("load_from_file", &MortonFilterWrapper::load_from_file);
    
    // NUMAOptimizedFilter binding
    py::class_<NUMAOptimizedFilter>(m, "NUMAOptimizedFilter")
        .def(py::init<>())
        .def("initialize", &NUMAOptimizedFilter::initialize)
        .def("contains", &NUMAOptimizedFilter::contains)
        .def("check_url", &NUMAOptimizedFilter::check_url)
        .def("insert", &NUMAOptimizedFilter::insert)
        .def("insert_batch", &NUMAOptimizedFilter::insert_batch)
        .def("print_stats", &NUMAOptimizedFilter::print_stats);
}