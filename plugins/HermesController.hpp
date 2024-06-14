/**
 * @file HermesController.hpp
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
#include "hermesmodules/hermescontrollerinfo/InfoNljs.hpp"

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
                  "Failed to retrieve hermes code stats" << what,
                  ((std::string)what)
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
  class Session;
}
                  
namespace hermesmodules {

class HermesController : public dunedaq::appfwk::DAQModule
{
public:
  explicit HermesController(const std::string& name);

  void init(std::shared_ptr<appfwk::ModuleConfiguration>) override;

  void get_info(opmonlib::InfoCollector&, int /*level*/) override;

  HermesController(const HermesController&) = delete;
  HermesController& operator=(const HermesController&) = delete;
  HermesController(HermesController&&) = delete;
  HermesController& operator=(HermesController&&) = delete;

  ~HermesController() = default;

private:

  // Commands HermesController can receive

  void do_conf(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);

  std::unique_ptr<HermesCoreController> m_core_controller;
  const appmodel::HermesController* m_dal;
  const confmodel::Session* m_session;
  std::vector<uint32_t> m_enabled_link_ids;

  std::atomic<int64_t> m_total_amount {0};
  std::atomic<int>     m_amount_since_last_get_info_call {0};
};


} // namespace hermesmodules
} // namespace dunedaq

#endif // HERMESMODULES_PLUGINS_HERMESCORECONTROLLER_HPP_
