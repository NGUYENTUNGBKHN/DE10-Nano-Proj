jtagconfig --setparam 1 JtagClock 6M
mkimage -A arm -O u-boot -T standalone -C none -a 0x7000040 -e 0 -n "2nd Boot Loader" -d boot_2nd.bin boot_2nd.img.bin
quartus_hps -c 1 -o P -a 0x00040100 boot_2nd.img.bin