import json
import os.path

rx_endpoints = {}
with open(os.path.expandvars('$HERMESMODULES_SHARE/config/rx_endpoints.json'), 'r') as f:
    d = json.load(f)
    for k,v in d.items():

        assert(v['mac'].count(':') == 5)
        rx_endpoints[k] = {
            'mac': int(v['mac'].replace(':', ''), 16),
            'ip': v['ip'],
            'port': int(v['port'], 16)
        }
