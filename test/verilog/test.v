module test();
  reg clk = 0;
  reg n_reset = 0;

  main #(8'd8) u(
    clk,
	n_reset
  );

  always
    #1  clk = ~clk;

  initial begin
	#1 n_reset=1;
	//#2 n_reset=0;
    #1000 $finish;
  end

  initial begin
    $dumpfile("test.vcd");
    $dumpvars(0,test);
  end

endmodule
