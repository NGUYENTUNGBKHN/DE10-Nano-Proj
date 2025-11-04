// It has a single clock input and a 32bit output port

module simple_counter(
							clock_50,
							counter_out
							);
input clock_50;
output [31:0] counter_out;

reg [31:0] counter_out;


always @(posedge clock_50)
begin
counter_out <= #1 counter_out + 1;
end
endmodule
