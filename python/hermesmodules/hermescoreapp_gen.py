# This module facilitates the generation of hermesmodules DAQModules within hermesmodules apps


# Set moo schema search path                                                                              
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types                                                                                
import moo.otypes
moo.otypes.load_types("hermesmodules/hermescontroller.jsonnet")

import dunedaq.hermesmodules.hermescontroller as hermescontroller

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
#from daqconf.core.conf_utils import Endpoint, Direction

from itertools import groupby

### Move to utility module
def group_by_key(coll, key):
    """
    """
    m = {}
    s = sorted(coll, key=key)
    for k, g in groupby(s, key):
        m[k] = list(g)
    return m



def get_hermescore_app(
        name,
        host,
        dro_map,
        ):
    """
    Here the configuration for an entire daq_application instance using DAQModules from hermesmodules is generated.
    """

    # UDP port - hardocoded for now
    UDP_PORT=0x4444

    addrtab = "file://${HERMESMODULES_SHARE}/config/hermes_wib_v0.9.1/tx_mux_wib.xml"

    # Crete a list of relevant stream parameters only
    tx_infos = list(set([
        (
          s.geo_id.det_id,
          s.geo_id.crate_id,
          s.geo_id.slot_id,
          s.parameters.tx_host,
          int(s.geo_id.stream_id > 63), # hack
          s.parameters.tx_ip,
          s.parameters.tx_mac,
          s.parameters.rx_ip,
          s.parameters.rx_mac
        ) for s in dro_map.streams if s.kind == 'eth'
        ]))
    
    # group the by actual cores using geoid and host
    hermes_cores = group_by_key(tx_infos, lambda x: (x[0], x[1], x[2], x[3]))

    modules = []
    for (det, crate, slot, ctrl_ept), links in hermes_cores.items():
      d = hermescontroller.Device(name=f"hermes_{det}_{crate}_{slot}", uri=f"ipbusudp-2.0://{ctrl_ept}:50001", addrtab=addrtab)
      g = hermescontroller.GeoInfo(det_id=det, crate_id=crate, slot_id=slot)
      lm = hermescontroller.LinkConfMap(
        [hermescontroller.LinkConf(
          id=link,
          src_mac = tx_mac,
          src_ip = tx_ip,
          dst_mac = rx_mac,
          dst_ip= rx_ip
        ) for _,_,_,_,link,tx_ip,tx_mac,rx_ip,rx_mac in links]
        )
      
      modules += [DAQModule(name = f"hermes_{det}_{crate}_{slot}", 
                            plugin = "HermesController", 
                            conf = hermescontroller.Conf(
                              device = d,
                              geo_info = g,
                              port = UDP_PORT,
                              links = lm
                              )
                  )]

    mgraph = ModuleGraph(modules)
    hermesmodules_app = App(modulegraph = mgraph, host = host, name = name)

    return hermesmodules_app
