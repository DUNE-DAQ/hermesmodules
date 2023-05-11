#include "hermesmodules/HermesCoreController.hpp"

#include <chrono>         // std::chrono::seconds
#include <thread>         // std::this_thread::sleep_for

namespace dunedaq {
namespace hermesmodules {

//-----------------------------------------------------------------------------
HermesCoreController::HermesCoreController(uhal::HwInterface hw, std::string tx_mux_id) :
  m_hw(hw), m_tx_mux(m_hw.getNode(tx_mux_id)) {

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
      // raise ValueError(f"Magic number check failed. Expected "0xdeadbeef", found '{hex(self.magic)}'")

  }

  auto n_mgt = m_tx_mux.getNode("info.generics.n_mgts").read();
  auto n_src = m_tx_mux.getNode("info.generics.n_srcs").read();
  auto ref_freq = m_tx_mux.getNode("info.generics.ref_freq").read();
  m_tx_mux.getClient().dispatch();

  // Generics
  m_core_info.n_mgt = n_mgt.value();
  m_core_info.n_src = n_src.value();
  m_core_info.ref_freq = ref_freq.value();
  // Extra info
  // self.n_srcs_p_mgt = self.n_src//self.n_mgt
}


//-----------------------------------------------------------------------------
void
HermesCoreController::sel_tx_mux(uint32_t i) {
  if ( i >= m_core_info.n_mgt ) {
    throw LinkDoesNotExist(i);
  }

  m_tx_mux.getNode("csr.ctrl.sel").write(i);
  m_tx_mux.getClient()dispatch();
}


//-----------------------------------------------------------------------------
void
HermesCoreController::sel_tx_mux_buf(uint32_t i) {
  if ( i >= m_core_info.n_src ) {
    throw InputBufferDoesNotExist(i);
  }

  m_tx_mux.getNode("mux.csr.ctrl.sel_buf").write(i);
  m_tx_mux.getClient()dispatch();
}


//-----------------------------------------------------------------------------
void
HermesCoreController::reset(bool nuke) {

    if (nuke) {
        m_tx_mux.getNode("csr.ctrl.nuke").write(0x1);
        m_tx_mux.getClient()dispatch();

        // time.sleep(0.1);
        std::this_thread::sleep_for (std::chrono::milliseconds(1));

        m_tx_mux.getNode("csr.ctrl.nuke").write(0x0);
        m_tx_mux.getClient()dispatch();
    }
    
    m_tx_mux.getNode("csr.ctrl.soft_rst").write(0x1);
    m_tx_mux.getClient()dispatch();

    // time.sleep(0.1)
    std::this_thread::sleep_for (std::chrono::milliseconds(1));


    m_tx_mux.getNode("csr.ctrl.soft_rst").write(0x0);
    m_tx_mux.getClient()dispatch();

}


//-----------------------------------------------------------------------------
void
HermesCoreController::enable(uint32_t link, bool enable) {

  if (link >= m_core_info.n_mgt) {
    // TODO: add ERS exception
    // raise ValueError(f"MGT {link} not instantiated")
    throw LinkDoesNotExist(link);
  }

  this->sel_tx_mux(link);

  auto tx_en = m_tx_mux.get_node("mux.csr.ctrl.tx_en").read();
  auto buf_en = m_tx_mux.get_node("mux.csr.ctrl.en_buf").read();
  auto ctrl_en = m_tx_mux.get_node("mux.csr.ctrl.en").read();



  if ( enable ) {

    // Enable transmitter first
    m_tx_mux.getNode("mux.csr.ctrl.tx_en").write(0x1);

    // Enable then input buffers

    // Then 

  } else {

  }

  // tx_en = tx_en if tx_en is not None else enable
  // buf_en = buf_en if buf_en is not None else enable

  //   if tx_en is not None:
  //       print(f"- {'Enabling' if tx_en else 'Disabling'} 'tx block'")
  //       hrms.get_node('mux.csr.ctrl.tx_en').write(tx_en)
  //   print()


  

    if buf_en :
        print(f"- {'Enabling' if buf_en else 'Disabling'} 'input buffers'")
        hrms.get_node('mux.csr.ctrl.en_buf').write(buf_en)

    time.sleep(0.1)

    if enable is not None:
        print(f"- {'Enabling' if enable else 'Disabling'} 'mux'")
        hrms.get_node('mux.csr.ctrl.en').write(enable)


    
    hrms.dispatch()

}

}
}