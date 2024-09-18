module select (
  input [1:0] yi,
  input [7:0] Xi,
  output reg [1:0] Fo
);

  always @ (yi, Xi)
    case (yi)
      0: Fo = Xi[1:0];
      1: Fo = Xi[3:2];
      2: Fo = Xi[5:4];
      3: Fo = Xi[7:6];
      default: Fo = 2'b00;
    endcase
endmodule

