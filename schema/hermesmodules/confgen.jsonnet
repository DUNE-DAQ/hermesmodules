// This is the configuration schema for hermesmodules

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.hermesmodules.confgen";
local s = moo.oschema.schema(ns);

local cs = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",   		       doc="A string"),   
    host:     s.string(  "Host",    moo.re.dnshost, doc="A hostname"),
    monitoring_dest: s.enum( "MonitoringDest", ["local", "cern", "pocket"]),

    hermesmodules: s.record("hermesmodules", [
        s.field( "addrtab", self.string, default="", doc="Hermes core address table"),
        s.field( "host_hermes", self.host, default="np04-srv-016", doc="Hermes application host"),
        s.field( "num_hermescontrollers", self.int4, default=1, doc="A value which configures the number of instances of HermesController"),
    ]),

    hermesmodules_gen: s.record("hermesmodules_gen", [
        s.field("boot", daqconf.boot, default=daqconf.boot, doc="Boot parameters"),
        s.field("hermesmodules", self.hermesmodules, default=self.hermesmodules, doc="hermesmodules parameters"),
    ]),
};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)
