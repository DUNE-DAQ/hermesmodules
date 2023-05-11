/**
 * @file renameme.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include <sstream>

#include "hermesmodules/HermesCoreController.hpp"

namespace py = pybind11;

namespace dunedaq::hermesmodules::python {

void
register_hermescorecontroller(py::module& m)
{
    py::class_<HermesCoreController>(m, "HermesCoreController")
    .def(py::init<uhal::HwInterface>())
    .def("load_hw_info", &HermesCoreController::load_hw_info)

    // .def("get_attribute",
    //      py::overload_cast<const std::string&>
    //      (&HDF5RawDataFile::get_attribute<std::string>),
    //      "Get attribute")
    ;
}

} // namespace dunedaq::hermesmodules::python
