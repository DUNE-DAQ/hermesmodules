
#ifndef HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_
#define HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_

#include "ers/Issue.hpp"
#include "uhal/uhal.hpp"

namespace dunedaq {

ERS_DECLARE_ISSUE(hermesmodules,
                  LinkDoesNotExist,
                  "Hermes Link " << lid << " does not exist",
                  ((int)lid));

ERS_DECLARE_ISSUE(hermesmodules,
                  InputBufferDoesNotExist,
                  "Hermes Input Buffer " << bid << " does not exist",
                  ((int)bid));

namespace hermesmodules {

class HermesCoreController {

public:
  explicit HermesCoreController(uhal::HwInterface, std::string tx_mux_id="");
  virtual ~HermesCoreController();

  void load_hw_info();

  void sel_tx_mux(uint32_t i) ;

  void sel_tx_mux_buf(uint32_t i);

  void reset(bool nuke=false);

  void enable(uint32_t link, bool enable);

  void config_mux() {}

  void config_udp() {}

  void config_fake_src() {}

  void read_stats() {}


private:

  struct CoreInfo {
    uint32_t n_mgt;
    uint32_t n_src;
    uint32_t ref_freq;
  };


  uhal::HwInterface m_hw;
  const uhal::Node& m_tx_mux;
  CoreInfo m_core_info;

};

}
}

#endif /* HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_ */
