
#ifndef HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_
#define HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_

#include "ers/Issue.hpp"
#include "uhal/uhal.hpp"

#include "hermesmodules/opmon/hermescontroller.pb.h"
namespace dunedaq {

ERS_DECLARE_ISSUE(hermesmodules,
                  LinkDoesNotExist,
                  "Hermes Link " << lid << " does not exist",
                  ((int)lid)
                  );

ERS_DECLARE_ISSUE(hermesmodules,
                  InputBufferDoesNotExist,
                  "Hermes Input Buffer " << bid << " does not exist",
                  ((int)bid)
                  );

ERS_DECLARE_ISSUE(hermesmodules,
                  MagicNumberError,
                  "Hermes Magic number failed " << found << " (" << expected << ")",
                  ((uint32_t)found)((uint32_t)expected)
                  );


ERS_DECLARE_ISSUE(hermesmodules,
                  LinkInError,
                  "Hermes Link " << link << " is in error (err:" << err << ", eth_rdy:" << eth_rdy << ", src_rdy:" << src_rdy << ", udp_rdy:" << udp_rdy <<" )",
                  ((uint16_t)link)((bool)err)((bool)eth_rdy)((bool)src_rdy)((bool)udp_rdy)
                  );

namespace hermesmodules {

class HermesCoreController {

public:

  struct CoreInfo {
    uint32_t design;
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t n_mgt;
    uint32_t n_src;
    uint32_t ref_freq;
    uint32_t srcs_per_mux;
  };

  struct LinkGeoInfo {
    uint16_t detid;
    uint16_t crateid;
    uint16_t slotid;
  };

  explicit HermesCoreController(uhal::HwInterface, std::string readout_id="");
  virtual ~HermesCoreController();

  const CoreInfo& get_info() const { return m_core_info; }

  void sel_tx_mux(uint16_t i) ;

  void sel_tx_mux_buf(uint16_t i);

  void sel_udp_core(uint16_t i);

  void reset(bool nuke=false);

  bool is_link_in_error(uint16_t link, bool do_throw=false);

  void enable(uint16_t link, bool enable);

  void config_mux(uint16_t link, uint16_t det, uint16_t crate, uint16_t slot);

  void config_udp(uint16_t link, uint64_t src_mac, uint32_t src_ip, uint16_t src_port, uint64_t dst_mac, uint32_t dst_ip, uint16_t dst_port, uint32_t filters);

  void config_fake_src(uint16_t link, uint16_t n_src, uint16_t data_len, uint16_t rate);

  LinkGeoInfo read_link_geo_info(uint16_t link);

  opmon::LinkInfo read_link_stats(uint16_t link);


private:

  void load_hw_info();

  uhal::HwInterface m_hw;

  const uhal::Node& m_readout;

  CoreInfo m_core_info;

};

}
}

#endif /* HERMESMODULES_INCLUDE_HERMESCORECONTROLLER_HPP_ */
