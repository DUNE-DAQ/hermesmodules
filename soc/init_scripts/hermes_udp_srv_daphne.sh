echo "Starting Hermes IPBus UDP server (daphne mode)"
/bin/hermes_udp_srv -d daphne -c true 2>/var/log/hermes_udp_srv.err >/var/log/hermes_udp_srv.log &