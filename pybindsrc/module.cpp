/**
 * @file module.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "module.hpp"
#include "hermesmodules/HermesCoreController.hpp"


namespace py = pybind11;

namespace dunedaq {
namespace hermesmodules {
namespace python {

extern void register_hermescorecontroller(py::module&);
extern void load_hw_info(py::module& m);

PYBIND11_MODULE(_daq_hermesmodules_py, top_module)
{

  top_module.doc() = "C++ implementation of the hermesmodules modules";

  // You'd want to change renameme to the name of a function which
  // you'd like to have a python binding to

  py::module_ core_module = top_module.def_submodule("core");


  hermesmodules::python::register_hermescorecontroller(core_module);
}

}  // namespace python
}  // namespace dtpcontrols
}  // namespace dunedaq
