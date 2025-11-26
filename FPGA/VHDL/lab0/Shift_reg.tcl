transcript off
vcom Shift_reg.vhd
vcom test_Shift_reg.vhd

vsim test_Shift_reg
add wave sim:/test_Shift_reg/dev_to_test/*

run 350 ns
