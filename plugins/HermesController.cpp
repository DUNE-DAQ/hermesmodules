/**
 * @file HermesController.cpp
 *
 * Implementations of HermesController's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "HermesController.hpp"

#include "hermesmodules/hermescontroller/Nljs.hpp"
#include "hermesmodules/hermescontrollerinfo/InfoNljs.hpp"

#include <string>

namespace dunedaq::hermesmodules {

HermesController::HermesController(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &HermesController::do_conf);
}

void
HermesController::init(const data_t& /* structured args */)
{}

void
HermesController::get_info(opmonlib::InfoCollector& ci, int /* level */)
{
  hermescontrollerinfo::Info info;
  info.total_amount = m_total_amount;
  info.amount_since_last_get_info_call = m_amount_since_last_get_info_call.exchange(0);

  ci.add(info);
}

void
HermesController::do_conf(const data_t& conf_as_json)
{
  auto conf_as_cpp = conf_as_json.get<hermescontroller::Conf>();
  m_some_configured_value = conf_as_cpp.some_configured_value;
}

} // namespace dunedaq::hermesmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::hermesmodules::HermesController)
