transcript on
if ![file isdirectory lab0_iputf_libs] {
	file mkdir lab0_iputf_libs
}

if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

###### Libraries for IPUTF cores 
###### End libraries for IPUTF cores 
###### MIF file copy and HDL compilation commands for IPUTF cores 


vlog "E:/intelFPGA/DE10-Nano-Proj/FPGA/lab0/pll_sim/pll.vo"

vlog -vlog01compat -work work +incdir+E:/intelFPGA/DE10-Nano-Proj/FPGA/lab0 {E:/intelFPGA/DE10-Nano-Proj/FPGA/lab0/simple_counter.v}
vlog -vlog01compat -work work +incdir+E:/intelFPGA/DE10-Nano-Proj/FPGA/lab0 {E:/intelFPGA/DE10-Nano-Proj/FPGA/lab0/counter_bus_mux.v}

