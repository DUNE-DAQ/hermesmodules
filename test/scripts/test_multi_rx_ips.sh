

pkt_size=32
rate_red=7

wib_id=np02-wib-1001

hermesbutler.py -d ${wib_id} reset 

sleep 3


det=3
slot=1
crate=10
target_nic=np02-srv-003-100g-b
link=1

set -x
hermesbutler.py -d ${wib_id} enable -l ${link} --dis 
hermesbutler.py -d ${wib_id} udp-config -l ${link} ${wib_id}-d${link} ${target_nic}
hermesbutler.py -d ${wib_id} mux-config -l ${link} ${det} ${crate} ${slot}
hermesbutler.py -d ${wib_id} fakesrc-config -l ${link} -s 0 -s 1 -s 2 -s 3 -k ${pkt_size} -r ${rate_red}
hermesbutler.py -d ${wib_id} enable -l ${link} --en
hermesbutler.py -d ${wib_id} stats -l ${link}
set +x



det=3
slot=1
crate=10
target_nic=np02-srv-003-100g
link=0


set -x
hermesbutler.py -d ${wib_id} enable -l ${link} --dis 
hermesbutler.py -d ${wib_id} udp-config -l ${link} ${wib_id}-d${link} ${target_nic}
hermesbutler.py -d ${wib_id} mux-config -l ${link} ${det} ${crate} ${slot}
hermesbutler.py -d ${wib_id} fakesrc-config -l ${link} -s 0 -s 1 -s 2 -s 3 -k ${pkt_size} -r ${rate_red}
hermesbutler.py -d ${wib_id} enable -l ${link} --en
hermesbutler.py -d ${wib_id} stats -l ${link}
set +x


