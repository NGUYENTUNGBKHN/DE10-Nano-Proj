-- File : test_Shift_reg.vhd

-- Library
Library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;

-- Entity
entity test_Shift_reg is

end;

architecture test of test_Shift_reg is

component Shift_reg
port (
	A		: 	out std_logic;
	B		: 	out std_logic;
	C		: 	out std_logic;
	D		: 	out std_logic;
	data_in :	in std_logic;
	reset 	:	in std_logic;
	clk		 :	in std_logic
);
end component;

signal data_in 	: std_logic := '0';
signal reset 	: std_logic := '0';
signal clk 		: std_logic := '1';
signal A, B, C, D : std_logic := '0';

begin 
	
	dev_to_test: Shift_reg
		port map(A, B, C, D, data_in, reset, clk);
	
	clk_stimulus: process
	begin
		wait for 10 ns;
		clk <= not clk;
	end process clk_stimulus;
	
	data_stimulus: process
	begin
		wait for 40 ns;
		data_in <= not data_in;
		wait for 150 ns;
	end process data_stimulus;
end test;
