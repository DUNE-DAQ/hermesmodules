echo "Starting Hermes IPBus UDP server (zcu102 mode)"
/bin/hermes_udp_srv -d zcu102 -c true 2>/var/log/hermes_udp_srv.err >/var/log/hermes_udp_srv.log &