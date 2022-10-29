module main(
	input wire clock,
	input wire n_reset
);

reg [31:0] state;
reg [31:0] main_val_a;

always @ (posedge clock, negedge n_reset) begin
	if (n_reset == 1'b0) begin
		state <= 32'd0
	end else begin
		if (state == 32'd0) begin
			main_val_a <= 32'd100
			state <= 32'd1
		end else if (state == 32'd1) begin
			// ?
		end
	end 
end

endmodule
