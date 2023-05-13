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
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <fmt/core.h>



namespace dunedaq::hermesmodules {

uint64_t ether_atou64( const std::string& addr_str ) {
    union {
        uint64_t          result;
        struct ether_addr address;
    };
    result = 0;
    struct ether_addr* ptr = ether_aton_r( addr_str.c_str(), &address );
    if( !ptr ) {
        return (~0);
    }
    return result;
}

HermesController::HermesController(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &HermesController::do_conf);
}

void
HermesController::init(const data_t& /* structured args */)
{
  uhal::setLogLevelTo(uhal::Error());
}

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
  auto conf = conf_as_json.get<hermescontroller::Conf>();


  // Create the ipbus 
  auto hw = uhal::ConnectionManager::getDevice(conf.device.name, conf.device.uri, conf.device.addrtab);

  m_core_controller = std::make_unique<HermesCoreController>(hw);

  const auto& core_info = m_core_controller->get_info();
  fmt::print("Hermes\n");
  fmt::print("n_mgt {}\n", core_info.n_mgt);
  fmt::print("n_src {}\n", core_info.n_src);
  fmt::print("ref_freq {}\n", core_info.ref_freq);
  std::cout << std::flush;


  // Size check on link conf
  if ( conf.links.size() != core_info.n_mgt ) {
    fmt::print("ERROR: Number of links in configuration ({}) and firmware ({}) don't match",conf.links.size(), core_info.n_mgt);
    std::cout << std::flush;
    // FIXME : ERS exception here
    assert(false);
  }

  // Sequence id check
  std::set<uint32_t> ids;
  for( const auto& l : conf.links) {
    ids.insert(l.id);
  }

  // Look duplicate link ids
  if ( ids.size() != conf.links.size() ) {
    // FIXME : ERS exception here
    assert(false);
  }

  // Make sure that the last link id is n_mgt-1
  if ( *ids.rbegin() != (core_info.n_mgt-1)) {
    fmt::print("ERROR: last link id ({}) doesn't match expected ({})",*ids.rend(), core_info.n_mgt-1);
    std::cout << std::flush;

    // FIXME : ERS exception here
    assert(false);
  }

  // All good
  for ( uint16_t i(0); i<core_info.n_mgt; ++i){
    // Put the endpoint in a safe state
    m_core_controller->enable(i, false);
  }

  m_core_controller->reset();

  // FIXME: What the hell is this again?
  uint32_t filter_control = 0x07400307;
  uint32_t port = 0x4444;
  for( const auto& l : conf.links) {
    m_core_controller->config_udp(
      l.id,
      ether_atou64(l.src_mac),
      inet_addr(l.src_ip.c_str()),
      port,
      ether_atou64(l.dst_mac),
      inet_addr(l.dst_ip.c_str()),
      port,
      filter_control
    );

    m_core_controller->config_mux(
      l.id,
      conf.geo_info.det_id,
      conf.geo_info.crate_id,
      conf.geo_info.slot_id
    );

  }

  for ( uint16_t i(0); i<m_core_controller->get_info().n_mgt; ++i){
    // Put the endpoint in a safe state
    m_core_controller->enable(i, true);
  }
  

}

} // namespace dunedaq::hermesmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::hermesmodules::HermesController)
