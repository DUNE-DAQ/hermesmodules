// This is the configuration schema for hermesmodules

local moo = import "moo.jsonnet";

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

local ns = "dunedaq.hermesmodules.confgen";
local s = moo.oschema.schema(ns);

local cs = {

    hermesmodules: s.record("hermesmodules", [
        s.field( "host_hermes", types.host, default="np04-srv-016", doc="Hermes application host"),
        s.field( "addrtab", types.string, default="file://${HERMESMODULES_SHARE}/config/hermes_wib_v0.9.1/tx_mux_wib.xml", doc="Hermes core address table"),
        s.field( "num_hermescontrollers", types.int4, default=1, doc="A value which configures the number of instances of HermesController"),
    ]),

    hermesmodules_gen: s.record("hermesmodules_gen", [
        s.field("boot",          bootgen.boot,         default=bootgen.boot,         doc="Boot parameters"),
        s.field("hermesmodules", self.hermesmodules,   default=self.hermesmodules,   doc="hermesmodules parameters"),
    ]),
};

// Output a topologically sorted array.
sboot + stypes + sboot + moo.oschema.sort_select(cs, ns)
