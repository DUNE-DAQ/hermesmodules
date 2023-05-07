# This module facilitates the generation of hermesmodules DAQModules within hermesmodules apps


# Set moo schema search path                                                                              
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types                                                                                
import moo.otypes
moo.otypes.load_types("hermesmodules/hermescorecontroller.jsonnet")

import dunedaq.hermesmodules.hermescorecontroller as hermescorecontroller

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
#from daqconf.core.conf_utils import Endpoint, Direction

def get_hermesmodules_app(nickname, num_hermescorecontrollers, some_configured_value, host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from hermesmodules is generated.
    """

    modules = []

    for i in range(num_hermescorecontrollers):
        modules += [DAQModule(name = f"nickname{i}", 
                              plugin = "HermesCoreController", 
                              conf = hermescorecontroller.Conf(some_configured_value = some_configured_value
                                )
                    )]

    mgraph = ModuleGraph(modules)
    hermesmodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return hermesmodules_app
