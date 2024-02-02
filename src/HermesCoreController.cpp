#include "hermesmodules/HermesCoreController.hpp"

#include <chrono>         // std::chrono::seconds
#include <thread>         // std::this_thread::sleep_for
#include <fmt/core.h>

namespace dunedaq {
namespace hermesmodules {

//-----------------------------------------------------------------------------
HermesCoreController::HermesCoreController(uhal::HwInterface hw, std::string tx_mux_id) :
  m_hw(hw), m_tx_mux(m_hw.getNode(tx_mux_id)) {

    this->load_hw_info();

}

//-----------------------------------------------------------------------------
HermesCoreController::~HermesCoreController() {

}

//-----------------------------------------------------------------------------
void
HermesCoreController::load_hw_info() {

  // Check magic number
  auto magic = m_tx_mux.getNode("info.magic").read();
  m_tx_mux.getClient().dispatch();
  if (magic.value() != 0xdeadbeef){
      // TODO: add ERS exception
      throw MagicNumberError(ERS_HERE, magic.value(),0xdeadbeef);
  }


  auto design = m_tx_mux.getNode("info.versions.design").read();
  auto major = m_tx_mux.getNode("info.versions.major").read();
  auto minor = m_tx_mux.getNode("info.versions.minor").read();
  auto patch = m_tx_mux.getNode("info.versions.patch").read();


  auto n_mgt = m_tx_mux.getNode("info.generics.n_mgts").read();
  auto n_src = m_tx_mux.getNode("info.generics.n_srcs").read();
  auto ref_freq = m_tx_mux.getNode("info.generics.ref_freq").read();
  m_tx_mux.getClient().dispatch();

  // Version
  m_core_info.design = design.value();
  m_core_info.major = major.value();
  m_core_info.minor = minor.value();
  m_core_info.patch = patch.value();

  // Generics
  m_core_info.n_mgt = n_mgt.value();
  m_core_info.n_src = n_src.value();
  m_core_info.ref_freq = ref_freq.value();

  // Extra info
  m_core_info.srcs_per_mux = m_core_info.n_src/m_core_info.n_mgt;

  fmt::print("Number of links: {}\n", m_core_info.n_mgt);
  fmt::print("Number of sources: {}\n", m_core_info.n_src);
  fmt::print("Reference freq: {}\n", m_core_info.ref_freq);
}


//-----------------------------------------------------------------------------
void
HermesCoreController::sel_tx_mux(uint16_t i) {
  if ( i >= m_core_info.n_mgt ) {
    throw LinkDoesNotExist(ERS_HERE, i);
  }

  m_tx_mux.getNode("tx_path.csr_tx_mux.ctrl.tx_mux_sel").write(i);
  m_tx_mux.getClient().dispatch();
}


//-----------------------------------------------------------------------------
void
HermesCoreController::sel_tx_mux_buf(uint16_t i) {
  if ( i >= m_core_info.n_src ) {
    throw InputBufferDoesNotExist(ERS_HERE, i);
  }

  m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.sel_buf").write(i);
  m_tx_mux.getClient().dispatch();
}


//-----------------------------------------------------------------------------
void
HermesCoreController::sel_udp_core(uint16_t i) {
  if ( i >= m_core_info.n_src ) {
    throw InputBufferDoesNotExist(ERS_HERE, i);
  }

  m_tx_mux.getNode("tx_path.csr_udp_core.ctrl.udp_core_sel").write(i);
  m_tx_mux.getClient().dispatch();
}


//-----------------------------------------------------------------------------
void
HermesCoreController::reset(bool nuke) {

    if (nuke) {
        m_tx_mux.getNode("csr.ctrl.nuke").write(0x1);
        m_tx_mux.getClient().dispatch();

        // time.sleep(0.1);
        std::this_thread::sleep_for (std::chrono::milliseconds(1));

        m_tx_mux.getNode("csr.ctrl.nuke").write(0x0);
        m_tx_mux.getClient().dispatch();
    }
    
    m_tx_mux.getNode("csr.ctrl.soft_rst").write(0x1);
    m_tx_mux.getClient().dispatch();

    // time.sleep(0.1)
    std::this_thread::sleep_for (std::chrono::milliseconds(1));


    m_tx_mux.getNode("csr.ctrl.soft_rst").write(0x0);
    m_tx_mux.getClient().dispatch();

}


//-----------------------------------------------------------------------------
bool
HermesCoreController::is_link_in_error(uint16_t link, bool do_throw) {

  this->sel_tx_mux(link);

  auto err = m_tx_mux.getNode("tx_path.tx_mux.csr.stat.err").read();
  auto eth_rdy = m_tx_mux.getNode("tx_path.tx_mux.csr.stat.eth_rdy").read();
  auto src_rdy = m_tx_mux.getNode("tx_path.tx_mux.csr.stat.src_rdy").read();
  auto udp_rdy = m_tx_mux.getNode("tx_path.tx_mux.csr.stat.udp_rdy").read();
  m_tx_mux.getClient().dispatch();

  bool is_error = (err || !eth_rdy || !src_rdy || !udp_rdy);

  if ( do_throw && is_error ) {
    throw LinkInError(ERS_HERE, link, err, eth_rdy, src_rdy, udp_rdy);
  }

  return is_error;
}

//-----------------------------------------------------------------------------
void
HermesCoreController::enable(uint16_t link, bool enable) {

  this->sel_tx_mux(link);

  // Not sure what to do with this
  auto tx_en = m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.tx_en").read();
  auto buf_en = m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").read();
  auto ctrl_en = m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en").read();

  if ( enable ) {

    // Assume that all is off

    // Enable the main logic
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en").write(0x1);
    m_tx_mux.getClient().dispatch();

    // Enable transmitter first
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.tx_en").write(0x1);
    m_tx_mux.getClient().dispatch();

    // Enable buffers last
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").write(0x1);
    m_tx_mux.getClient().dispatch();


  } else {

    // Disable buffers last
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").write(0x0);
    m_tx_mux.getClient().dispatch();

    // Disable transmitter first
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.tx_en").write(0x0);
    m_tx_mux.getClient().dispatch();

    // Disable the main logic
    m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en").write(0x0);
    m_tx_mux.getClient().dispatch();

  }

}


//-----------------------------------------------------------------------------
void
HermesCoreController::config_mux(uint16_t link, uint16_t det, uint16_t crate, uint16_t slot) {
  this->sel_tx_mux(link);


  m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.detid").write(det);
  m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.crate").write(crate);
  m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.slot").write(slot);
    
}


//-----------------------------------------------------------------------------
void
HermesCoreController::config_udp( uint16_t link, uint64_t src_mac, uint32_t src_ip, uint16_t src_port, uint64_t dst_mac, uint32_t dst_ip, uint16_t dst_port, uint32_t filters) {

  if ( link >= m_core_info.n_mgt ) {
    throw LinkDoesNotExist(ERS_HERE, link);
  }

  this->sel_udp_core(link);

  // const std::string udp_ctrl_name = fmt::format("udp.udp_core_{}.udp_core_control.nz_rst_ctrl");
  const auto& udp_ctrl = m_tx_mux.getNode(fmt::format("tx_path.udp_core.udp_core_control"));


  // Load the source mac address
  udp_ctrl.getNode("src_addr_ctrl.src_mac_addr_lower").write(src_mac & 0xffffffff);
  udp_ctrl.getNode("src_addr_ctrl.src_mac_addr_upper").write((src_mac >> 32) & 0xffff);

  // Load the source ip address
  udp_ctrl.getNode("src_addr_ctrl.src_ip_addr").write(src_ip);

  // Load the dst mac address
  udp_ctrl.getNode("ctrl.dst_mac_addr_lower").write(dst_mac & 0xffffffff);
  udp_ctrl.getNode("ctrl.dst_mac_addr_upper").write((dst_mac >> 32) & 0xffff);

  // Load the dst ip address
  udp_ctrl.getNode("ctrl.dst_ip_addr").write(dst_ip);

  // Ports
  udp_ctrl.getNode("src_addr_ctrl.src_port").write(src_port);
  udp_ctrl.getNode("ctrl.dst_port").write(dst_port);


  udp_ctrl.getNode("ctrl.filter_control").write(filters);
  udp_ctrl.getClient().dispatch();

}

//-----------------------------------------------------------------------------
void
HermesCoreController::config_fake_src(uint16_t link, uint16_t n_src, uint16_t data_len, uint16_t rate) {

  this->sel_tx_mux(link);

  auto was_en_buf = m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").read();
  m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").write(0x0);
  m_tx_mux.getClient().dispatch();


  for ( size_t src_id(0); src_id<m_core_info.srcs_per_mux; ++src_id) {
    this->sel_tx_mux_buf(src_id);


    bool src_en = (src_id<n_src);
    m_tx_mux.getNode("tx_path.tx_mux.buf.ctrl.fake_en").write(src_en);
    m_tx_mux.getClient().dispatch();
    if (!src_en) {
      continue;
    }
    m_tx_mux.getNode("tx_path.tx_mux.buf.ctrl.dlen").write(data_len);
        
    m_tx_mux.getNode("tx_path.tx_mux.buf.ctrl.rate_rdx").write(rate);
    m_tx_mux.getClient().dispatch();
  }

  m_tx_mux.getNode("tx_path.tx_mux.csr.ctrl.en_buf").write(was_en_buf.value());
  m_tx_mux.getClient().dispatch();
}


//-----------------------------------------------------------------------------
HermesCoreController::LinkGeoInfo
HermesCoreController::read_link_geo_info(uint16_t link) {

  this->sel_tx_mux(link);

  auto detid = m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.detid").read();
  auto crate = m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.crate").read();
  auto slot = m_tx_mux.getNode("tx_path.tx_mux.mux.ctrl.slot").read();

  m_tx_mux.getClient().dispatch();

  return {detid.value(), crate.value(), slot.value()};
}

//-----------------------------------------------------------------------------
hermescontrollerinfo::LinkStats
HermesCoreController::read_link_stats(uint16_t link) {
  this->sel_tx_mux(link);
  this->sel_udp_core(link);

  hermescontrollerinfo::LinkStats ls;

  const auto& mux_stats = m_tx_mux.getNode("tx_path.tx_mux.csr.stat");
  auto err = mux_stats.getNode("err").read();
  auto eth_rdy = mux_stats.getNode("eth_rdy").read();
  auto src_rdy = mux_stats.getNode("src_rdy").read();
  auto udp_rdy = mux_stats.getNode("udp_rdy").read();
  mux_stats.getClient().dispatch();

  const auto& udp_ctrl = m_tx_mux.getNode(fmt::format("tx_path.udp_core.udp_core_control"));
  const auto& rx_stats = udp_ctrl.getNode("rx_packet_counters");

  auto rx_arp_count = rx_stats.getNode("arp_count").read();
  auto rx_ping_count = rx_stats.getNode("ping_count").read();
  auto rx_udp_count = rx_stats.getNode("udp_count").read();
  rx_stats.getClient().dispatch();

  const auto& tx_stats = udp_ctrl.getNode("tx_packet_counters");

  auto tx_arp_count = tx_stats.getNode("arp_count").read();
  auto tx_ping_count = tx_stats.getNode("ping_count").read();
  auto tx_udp_count = tx_stats.getNode("udp_count").read();
  tx_stats.getClient().dispatch();


  ls.err = err.value();
  ls.eth_rdy = eth_rdy.value();
  ls.src_rdy = src_rdy.value();
  ls.udp_rdy = udp_rdy.value();

  ls.rcvd_arp_count = rx_arp_count.value();
  ls.rcvd_ping_count = rx_ping_count.value();
  ls.rcvd_udp_count = rx_udp_count.value();

  ls.sent_arp_count = tx_arp_count.value();
  ls.sent_ping_count = tx_ping_count.value();
  ls.sent_udp_count = tx_udp_count.value();

  return ls;

}



}
}