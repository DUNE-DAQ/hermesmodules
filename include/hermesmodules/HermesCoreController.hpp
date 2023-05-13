
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

ERS_DECLARE_ISSUE(hermesmodules,
                  MagicNumberError,
                  "Hermes Magic number failed " << found << " (" << expected << ")",
                  ((int)found)((int)expected));


namespace hermesmodules {

class HermesCoreController {

public:

  struct CoreInfo {
    uint32_t n_mgt;
    uint32_t n_src;
    uint32_t ref_freq;
    uint32_t srcs_per_mux;
  };

  explicit HermesCoreController(uhal::HwInterface, std::string tx_mux_id="");
  virtual ~HermesCoreController();

  const CoreInfo& get_info() const { return m_core_info; }

  void sel_tx_mux(uint16_t i) ;

  void sel_tx_mux_buf(uint16_t i);

  void reset(bool nuke=false);

  void enable(uint16_t link, bool enable);

  void config_mux(uint16_t link, uint16_t det, uint16_t crate, uint16_t slot);

  void config_udp(uint16_t link, uint64_t src_mac, uint32_t src_ip, uint16_t src_port, uint64_t dst_mac, uint32_t dst_ip, uint16_t dst_port, uint32_t filters);

  void config_fake_src(uint16_t link, uint16_t n_src, uint16_t data_len, uint16_t rate);

  void read_stats();


private:

  void load_hw_info();

  uhal::HwInterface m_hw;

  const uhal::Node& m_tx_mux;

  CoreInfo m_core_info;

};

}
}

#endif /* HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_ */
