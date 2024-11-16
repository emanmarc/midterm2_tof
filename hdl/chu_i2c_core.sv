module chu_i2c_core
   (
    input  logic clk,
    input  logic reset,
    // slot interface
    input  logic cs,
    input  logic read,
    input  logic write,
    input  logic [4:0] addr,
    input  logic [31:0] wr_data,
    output logic [31:0] rd_data,
    // external signal    
    output tri scl,
    inout  tri sda,
    output logic tof_ss,     // Sample Start (SS) signal
    input  logic tof_irq     // Interrupt Request (IRQ) signal 
   );

   // signal declaration
   logic [15:0] dvsr_reg;
   logic wr_i2c, wr_dvsr;
   logic wr_ss; // when to write to tof_ss
   logic [7:0] dout;
   logic ready, ack;
   
   // instantiate spi controller
   i2c_master i2c_unit
   (
    .din(wr_data[7:0]), .cmd(wr_data[10:8]),
    .dvsr(dvsr_reg), .done_tick(), .*
   );
       
   // registers
   always_ff @(posedge clk, posedge reset)
      if (reset)
      begin
        dvsr_reg <= 0;
        tof_ss <= 0;  
      end
      else
      begin   
         if (wr_dvsr)
             dvsr_reg <= wr_data[15:0];
          if (wr_ss)
             tof_ss <= wr_data[31];  // tof_ss control reg
      end      
   // decoding
   assign wr_dvsr = cs & write & addr[1:0]== 2'b01;
   assign wr_i2c  = cs & write & addr[1:0]== 2'b10;
   assign wr_ss   = cs & write & addr[1:0] == 2'b11;
   // read data  
   // if addr = 0, return  {22'b0, ack, ready, dout} (i2c status)
   //if addr = 3, return tof_irq status reg
    assign rd_data = (addr[1:0] == 2'b00) ? {22'b0, ack, ready, dout} :
                    (addr[1:0] == 2'b11) ? {31'b0, tof_irq} : 
                    32'b0;
endmodule  
