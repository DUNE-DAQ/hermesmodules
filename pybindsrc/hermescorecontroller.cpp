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
#include "module.hpp"

namespace py = pybind11;
using namespace pybind11::literals; 


namespace dunedaq::hermesmodules::python {

void
register_hermescorecontroller(py::module& m)
{
    py::class_<HermesCoreController>(m, "HermesCoreController")
    .def(py::init<uhal::HwInterface>())
    // .def("load_hw_info", &HermesCoreController::load_hw_info)
    .def("sel_tx_mux", &HermesCoreController::sel_tx_mux)
    .def("sel_tx_mux_buf", &HermesCoreController::sel_tx_mux_buf)
    .def("reset", &HermesCoreController::reset)
    .def("is_link_in_error", &HermesCoreController::is_link_in_error, "link"_a, "do_throw"_a = false)
    .def("enable", &HermesCoreController::enable)
    .def("config_mux", &HermesCoreController::config_mux)
    .def("config_udp", &HermesCoreController::config_udp)
    .def("config_fake_src", &HermesCoreController::config_fake_src)
    .def("read_link_stats", &HermesCoreController::read_link_stats)

    // .def("get_attribute",
    //      py::overload_cast<const std::string&>
    //      (&HDF5RawDataFile::get_attribute<std::string>),
    //      "Get attribute")
    ;
}

} // namespace dunedaq::hermesmodules::python
