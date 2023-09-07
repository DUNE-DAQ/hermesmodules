
det=3

# crate=3
# target_nic=np02-srv-001-100G

crate=1
target_nic=iceberg03-100G
#target_nic=iceberg03-10G-  # if ends with "-", link will be appended


#wiblis=$(seq 1 5)
wiblis="1"
links='0 1'      # these should be 0-indexed integers

log() { echo `date`: "$@";}

for slot in $wiblis; do
    for link in $links; do
        wib=${crate}0${slot}
	log Disable iceberg-wib-$wib link $link
        hermesbutler.py -d iceberg-wib-$wib enable -l ${link} --dis
    done
done

for slot in ${wiblis}; do
    for link in $links; do
        wib=${crate}0${slot}
	log Diable/config/enable iceberg-wib-${wib} link $link
        hermesbutler.py -d iceberg-wib-${wib} enable -l $link --dis
	if [ ${target_nic: -1:1} = '-' ];then # need " " inbetween ":" and "-1"
	    log target nic = ${target_nic}$link
            hermesbutler.py -d iceberg-wib-${wib} udp-config -l $link iceberg-wib-${wib}-d${link} ${target_nic}$link
	else
	    log target nic = ${target_nic}
            hermesbutler.py -d iceberg-wib-${wib} udp-config -l $link iceberg-wib-${wib}-d${link} ${target_nic}
	fi
        hermesbutler.py -d iceberg-wib-${wib} mux-config -l $link ${det} ${crate} $slot
        hermesbutler.py -d iceberg-wib-${wib} fakesrc-config -l $link -n 4
        hermesbutler.py -d iceberg-wib-${wib} enable -l $link --en
        hermesbutler.py -d iceberg-wib-${wib} stats -l $link
    done
done
