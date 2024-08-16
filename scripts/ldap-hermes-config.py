#!/usr/bin/env python

from pylandb import LanDB
from rich import print
import json



def main(query: str):
    landb_handler = LanDB()

    dev_search = landb_handler.get_factory("types:DeviceSearch")
    dev_search.Name = query
    dev_search.Location = None

    srv = landb_handler.get_service()

    devices = srv.searchDevice(dev_search)


    dev_records = {}

    for dev in devices:
        print(dev)
        dev_info = srv.getDeviceInfo(dev)

        for i in dev_info.Interfaces:
            tokens = i.Name.split('.')
            if tokens[0] == dev:
                # Master interface, skip
                continue
            print(i.Name.lower(), i.IPAddress.lower(), i.BoundInterfaceCard.HardwareAddress.lower())
            dev_records[tokens[0].lower()] = {
                "mac": i.BoundInterfaceCard.HardwareAddress.lower().replace('-',':'),
                "ip": i.IPAddress.lower(),
                "port": "0x4444"
            }


    print(json.dumps(dev_records, indent=4))


if __name__ == '__main__':
    query = 'NP02-WIB*'
    query = 'NP02-SRV-003*'

    main(query)