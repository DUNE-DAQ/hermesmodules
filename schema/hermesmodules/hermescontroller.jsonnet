local moo = import "moo.jsonnet";
local ns = "dunedaq.hermesmodules.hermescontroller";
local s = moo.oschema.schema(ns);

local types = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",   		       doc="A string"),   
    host:     s.string(  "Host", pattern=moo.re.dnshost, doc="A hostname"),
    ipv4:     s.string(  "ipv4", pattern=moo.re.ipv4, doc="ipv4 string"),
    mac:      s.string(  "mac", pattern="^[a-fA-F0-9]{2}(:[a-fA-F0-9]{2}){5}$", doc="mac string"),

    // TO hermesmodules DEVELOPERS: PLEASE DELETE THIS FOLLOWING COMMENT AFTER READING IT
    // The following code is an example of a configuration record
    // written in jsonnet. In the real world it would be written so as
    // to allow the relevant members of HermesController to be configured by
    // Run Control
    geo_info : s.record("GeoInfo", [
        s.field("det_id", self.uint4, 0, doc="Detector ID"),
        s.field("crate_id", self.uint4, 0, doc="Crate ID"),
        s.field("slot_id", self.uint4, 0, doc="Slot ID")
    ], doc="Geo Information"),

    ipbus_device : s.record("Device", [
        s.field("name", self.string),
        s.field("uri", self.string),
        s.field("addrtab", self.string),
    ], doc="IPBus Device details"),

    link_conf: s.record("LinkConf", [
        s.field("id", self.uint4),
        s.field("src_mac", self.mac, "00:00:00:00:00:00", doc="Reaout Source MAC"),
        s.field("src_ip", self.ipv4, "0.0.0.0", doc="Reaout Source IP"),
        s.field("dst_mac", self.mac,  "00:00:00:00:00:00", doc="Reaout Destination MAC"),
        s.field("dst_ip", self.ipv4, "0.0.0.0", doc="Reaout Destination IP"),
        ], doc=""),

    link_conf_map : s.sequence("LinkConfMap", self.link_conf, doc="Link configuration map" ),

    conf: s.record("Conf", [
        s.field("device", self.ipbus_device, doc="IPBus device connection details"),
        s.field("geo_info", self.geo_info, doc="Host board geographical information"),
        s.field("port", self.uint4),
        // s.field("filter", self.uint4),
        s.field("links", self.link_conf_map),
        // s.field("some_configured_value", self.int4, 999999,doc="This line is where you'd document the value"),
        ],
        doc="This configuration is for developer education only"
    ),

};

moo.oschema.sort_select(types, ns)
