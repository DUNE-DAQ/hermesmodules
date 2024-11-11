/**
 * @file HermesModule.cpp
 *
 * Implementations of HermesModule's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appmodel/appmodelIssues.hpp"
#include "appmodel/HermesModule.hpp"
#include "appmodel/HermesDataSender.hpp"
#include "appmodel/IpbusAddressTable.hpp"
#include "confmodel/NetworkDevice.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/GeoId.hpp"
#include "confmodel/System.hpp"

#include "HermesModule.hpp"
#include "hermesmodules/opmon/hermescontroller.pb.h"

#include <string>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <fmt/core.h>
#include "logging/Logging.hpp"



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
HermesModule::HermesModule(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &HermesModule::do_conf);
  register_command("start", &HermesModule::do_start);
  register_command("stop", &HermesModule::do_stop);
}

//-----------------------------------------------------------------------------
void
HermesModule::init(std::shared_ptr<appfwk::ModuleConfiguration> mcfg)
{
  uhal::setLogLevelTo(uhal::Error());

  // Save our DAL for later use by do_conf
  m_dal = mcfg->module<appmodel::HermesModule>(get_name());
  m_system = mcfg->configuration_manager()->system();
}

//-----------------------------------------------------------------------------
void
HermesModule::generate_opmon_data() 
{
  opmon::ControllerInfo ginfo;
  ginfo.set_total_amount( m_total_amount );
  ginfo.set_amount_since_last_get_info_call( m_amount_since_last_get_info_call.exchange(0) );
  publish( std::move(ginfo) );

  if ( ! m_core_controller ) return ;
  
  const auto& core_info = m_core_controller->get_info();

  for ( uint16_t i(0); i<core_info.n_mgt; ++i){

    try {
      auto geo_info = m_core_controller->read_link_geo_info(i);
      publish( m_core_controller->read_link_stats(i),
	       { {"detector",std::to_string(geo_info.detid)},
		 {"crate",   std::to_string(geo_info.crateid)},
		 {"slot",    std::to_string(geo_info.slotid)},
		 {"link",    std::to_string(i)} } );
    } catch ( const uhal::exception::exception& e ) {
      ers::warning(FailedToRetrieveStats(ERS_HERE, i, e));  
    }
      
  } // loop over links
  
}

//-----------------------------------------------------------------------------
void
HermesModule::do_conf(const data_t& /*conf_as_json*/)
{ 
  // Create the ipbus 
  auto hw = uhal::ConnectionManager::getDevice(m_dal->UID(),
                                               m_dal->get_uri(),
                                               m_dal->get_address_table()->get_uri());    
  hw.setTimeoutPeriod(m_dal->get_timeout_ms());

  m_core_controller = std::make_unique<HermesCoreController>(hw);

  const auto& core_info = m_core_controller->get_info();
  fmt::print("Hermes\n");
  fmt::print("n_mgt {}\n", core_info.n_mgt);
  fmt::print("n_src {}\n", core_info.n_src);
  fmt::print("ref_freq {}\n", core_info.ref_freq);
  std::cout << std::flush;

  auto links = m_dal->get_links();
  // Size check on link conf
  if ( links.size() != core_info.n_mgt ) {
    throw FirmwareConfigLinkMismatch(ERS_HERE, links.size(), core_info.n_mgt);
  }

  // Sequence id check
  std::set<uint32_t> ids;
  for( const auto l : links) {
    ids.insert(l->get_link_id());
  }

  // Look duplicate link ids
  if ( ids.size() != links.size() ) {
    throw DuplicatedLinkIDs(ERS_HERE, links.size(), ids.size());
  }

  // Make sure that the last link id is n_mgt-1
  if ( *ids.rbegin() != (core_info.n_mgt-1)) {
    throw LinkIDConfigurationError(ERS_HERE, *ids.rend(), core_info.n_mgt-1);
  }
  
  // Check ip address consistency
  if (m_dal->get_destination()->get_ip_address().size() != 1) {
      throw MultipleIPAddressConfigurationError(ERS_HERE, m_dal->get_destination()->UID(), m_dal->get_destination()->get_ip_address().size());
  }

  for( const auto& l : links) {
    if (l->get_uses()->get_ip_address().size() != 1) {
      throw MultipleIPAddressConfigurationError(ERS_HERE, l->get_uses()->UID(), l->get_uses()->get_ip_address().size());
    }
  }
  // All good
  for ( uint16_t i(0); i<core_info.n_mgt; ++i){
    // Put the endpoint in a safe state
    m_core_controller->enable(i, false);
  }

  m_core_controller->reset();


  // FIXME: What the hell is this again?
  uint32_t filter_control = 0x07400307;
  for( const auto& l : links) {
    if (l->disabled(*m_system)) {
      continue;  
    }

    m_enabled_link_ids.push_back(l->get_link_id());

    m_core_controller->config_udp(
      l->get_link_id(),
      ether_atou64(l->get_uses()->get_mac_address()),
      ip_atou32(l->get_uses()->get_ip_address().at(0)),
      l->get_port(),
      ether_atou64(m_dal->get_destination()->get_mac_address()),
      ip_atou32(m_dal->get_destination()->get_ip_address().at(0)),
      l->get_port(),
      filter_control
    );

    // HermesDataSender may contains DetectorStreams or a ResourceSet
    // containing DetectorStreams. Just find the first DetectorStream
    // and use that for the geo id information.
    const confmodel::DetectorStream* source = nullptr;
    for (auto res : l->get_contains()) {
      source = res->cast<confmodel::DetectorStream>();
      if (source == nullptr) {
        auto streams = res->cast<confmodel::ResourceSet>();
        if (streams != nullptr) {
          for (auto stream : streams->get_contains()) {
            source = stream->cast<confmodel::DetectorStream>();
            if (source != nullptr) {
              break;
            }
          }
        }
      }
      if (source != nullptr) {
        break;
      }
    }
    if (source == nullptr) {
      throw InvalidSourceStream(ERS_HERE, l->UID());
    }

    m_core_controller->config_mux(
      l->get_link_id(),
      source->get_geo_id()->get_detector_id(),
      source->get_geo_id()->get_crate_id(),
      source->get_geo_id()->get_slot_id()
    );

  }
}

void
HermesModule::do_start(const data_t& /*d*/)
{

  for( auto id : m_enabled_link_ids) {
    // Put the endpoint in a safe state
    m_core_controller->enable(id, true);
  }


  for( auto id : m_enabled_link_ids) {
    // Put the endpoint in a safe state
    m_core_controller->is_link_in_error(id, true);
  }


  // for ( uint16_t i(0); i<core_info.n_mgt; ++i){
  //   // Put the endpoint in a safe state
  //   m_core_controller->enable(i, true);
  // }

  // for ( uint16_t i(0); i<core_info.n_mgt; ++i){
  //   // Put the endpoint in a safe state
  //   m_core_controller->is_link_in_error(i);
  // }

}

void
HermesModule::do_stop(const data_t& /*d*/)
{

  for( auto id : m_enabled_link_ids) {
    // Put the endpoint in a safe state
    m_core_controller->enable(id, false);
  }
}

} // namespace dunedaq::hermesmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::hermesmodules::HermesModule)
