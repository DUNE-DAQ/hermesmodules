/**
 * @file HermesModule.hpp
 *
 * Developer(s) of this DAQModule have yet to replace this line with a brief description of the DAQModule.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef HERMESMODULES_PLUGINS_HERMESCORECONTROLLER_HPP_
#define HERMESMODULES_PLUGINS_HERMESCORECONTROLLER_HPP_

#include "appfwk/DAQModule.hpp"

#include "hermesmodules/HermesCoreController.hpp"

#include <atomic>
#include <limits>
#include <string>

namespace dunedaq { 

ERS_DECLARE_ISSUE(hermesmodules,
                  FirmwareConfigLinkMismatch,
                  "Number of links in configuration ("<< cfg_n_links << ") and firmware (" << fw_n_links << ") don't match",
                  ((uint16_t)cfg_n_links)((uint16_t)fw_n_links)
                  );

ERS_DECLARE_ISSUE(hermesmodules,
                  DuplicatedLinkIDs,
                  "Duplicated link ids detected ( number of links in config: "<< cfg_n_links << " but only " << cfg_n_links_unique << ") are uniqe)",
                  ((uint16_t)cfg_n_links)((uint16_t)cfg_n_links_unique)
                  );

ERS_DECLARE_ISSUE(hermesmodules,
                  FailedToRetrieveStats,
                  "Failed to retrieve hermes code stats for link " << link,
                  ((uint16_t)link)
                  );

ERS_DECLARE_ISSUE(hermesmodules,
                  InvalidSourceStream,
                  "Configuration for " << what << " does not contain a detector stream",
                  ((std::string)what)
                  );

namespace appmodel {
  class HermesCoreController;
}
namespace confmodel {
  class System;
}
                  
namespace hermesmodules {

class HermesModule : public dunedaq::appfwk::DAQModule
{
public:
  explicit HermesModule(const std::string& name);

  void init(std::shared_ptr<appfwk::ModuleConfiguration>) override;

  HermesModule(const HermesModule&) = delete;
  HermesModule& operator=(const HermesModule&) = delete;
  HermesModule(HermesModule&&) = delete;
  HermesModule& operator=(HermesModule&&) = delete;

  ~HermesModule() = default;

protected:
  void generate_opmon_data() override;
private:

  // Commands HermesModule can receive

  void do_conf(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);

  std::unique_ptr<HermesCoreController> m_core_controller;
  const appmodel::HermesModule* m_dal;
  const confmodel::System* m_system;
  std::vector<uint32_t> m_enabled_link_ids;

  std::atomic<int64_t> m_total_amount {0};
  std::atomic<int>     m_amount_since_last_get_info_call {0};
};


} // namespace hermesmodules
} // namespace dunedaq

#endif // HERMESMODULES_PLUGINS_HERMESCORECONTROLLER_HPP_
