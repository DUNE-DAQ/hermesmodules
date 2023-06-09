
det=3

# crate=3
# target_nic=np02-srv-001-100G

crate=3
target_nic=np02-srv-001-100G

for slot in $(seq 1 5); do
    for link in 0 1; do
        wib=${crate}0${slot}
        hermesbutler.py -d np04-wib-$wib enable -l ${link} --dis
    done
done

wiblis=$(seq 1 5)
wiblis="1"
for slot in ${wiblis}; do
    for link in 0 1; do
        wib=${crate}0${slot}
        hermesbutler.py -d np04-wib-${wib} enable -l $link --dis 
        hermesbutler.py -d np04-wib-${wib} udp-config -l $link np04-wib-${wib}-d${link} ${target_nic}
        hermesbutler.py -d np04-wib-${wib} mux-config -l $link ${det} ${crate} $slot
        hermesbutler.py -d np04-wib-${wib} fakesrc-config -l $link -n 4
        hermesbutler.py -d np04-wib-${wib} enable -l $link --en
        hermesbutler.py -d np04-wib-${wib} stats -l $link
    done
done