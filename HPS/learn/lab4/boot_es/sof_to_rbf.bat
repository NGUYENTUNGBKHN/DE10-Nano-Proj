quartus_cpf -c -o bitstream_compression=on BVSYS.sof BVSYS.rbf
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "RBF" -d BVSYS.rbf mk-BVSYS.rbf