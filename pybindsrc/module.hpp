#ifndef __HERMESMODULES_PYBINDSRC_HERMESMODULES_WRAPPER__
#define __HERMESMODULES_PYBINDSRC_HERMESMODULES_WRAPPER__

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace dunedaq {
namespace hermesmodules {
namespace python {

void register_hermescorecontroller(py::module&);
void load_hw_info(py::module&);


}  // namespace python
}  // namespace hermes
}  // namespace dunedaq

#endif /* __DTPCONTROLS_PYBINDSRC_DTP_CONTROLS_WRAPPER__ */