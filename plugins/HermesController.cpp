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

//-----------------------------------------------------------------------------
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
    // Big to little endian
    return (__builtin_bswap64(result) >> 16);
}

//-----------------------------------------------------------------------------
uint32_t ip_atou32(const std::string& addr_str) {
  // Big to little endian
  return __builtin_bswap32(inet_addr(addr_str.c_str()));
}

//-----------------------------------------------------------------------------
HermesController::HermesController(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &HermesController::do_conf);
}

//-----------------------------------------------------------------------------
void
HermesController::init(const data_t& /* structured args */)
{
  uhal::setLogLevelTo(uhal::Error());
}

//-----------------------------------------------------------------------------
void
HermesController::get_info(opmonlib::InfoCollector& ci, int /* level */)
{

  
  hermescontrollerinfo::Info info;
  info.total_amount = m_total_amount;
  info.amount_since_last_get_info_call = m_amount_since_last_get_info_call.exchange(0);

  ci.add(info);

  const auto& core_info = m_core_controller->get_info();

  for ( uint16_t i(0); i<core_info.n_mgt; ++i){
    // Create a sub-collector per linkg
    opmonlib::InfoCollector link_ci;
    
    hermescontrollerinfo::LinkStats link_stats;


    // get link statis
    link_ci.add(m_core_controller->read_link_stats(i));

    // 
    ci.add(fmt::format("link_{}",i), link_ci);
  }
}

//-----------------------------------------------------------------------------
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
    throw FirmwareConfigLinkMismatch(ERS_HERE, conf.links.size(), core_info.n_mgt);
  }

  // Sequence id check
  std::set<uint32_t> ids;
  for( const auto& l : conf.links) {
    ids.insert(l.id);
  }

  // Look duplicate link ids
  if ( ids.size() != conf.links.size() ) {
    throw DuplicatedLinkIDs(ERS_HERE, conf.links.size(), ids.size());
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
  for( const auto& l : conf.links) {
    m_core_controller->config_udp(
      l.id,
      ether_atou64(l.src_mac),
      ip_atou32(l.src_ip),
      conf.port,
      ether_atou64(l.dst_mac),
      ip_atou32(l.dst_ip),
      conf.port,
      filter_control
    );

    m_core_controller->config_mux(
      l.id,
      conf.geo_info.det_id,
      conf.geo_info.crate_id,
      conf.geo_info.slot_id
    );

  }

  for ( uint16_t i(0); i<core_info.n_mgt; ++i){
    // Put the endpoint in a safe state
    m_core_controller->enable(i, true);
  }

  for ( uint16_t i(0); i<core_info.n_mgt; ++i){
    // Put the endpoint in a safe state
    m_core_controller->is_link_in_error(i);
  }

}

} // namespace dunedaq::hermesmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::hermesmodules::HermesController)
