module alu (
    input [2:0] mod,
    input [3:0] a,
    input [3:0] b,
    output reg [3:0] out,
    output cout,
    output CF,
    output ZF
);
    reg [3:0] cpb;
    always @(mod, a, b) begin
        case (mod)
            0: begin
                {cout, out} = a + b;
                CF = (a[3] == b[3]) && (out[3] != a[3]);
                ZF = ~(|out);
            end
            1: begin
                cpb = ~b + 1;
                {cout, out} = a + cpb;
                CF = (a[3] == b[3]) && (out[3] != a[3]) || (b == 4'b1001);
                ZF = ~(|out);
            end
            2:  out = ~a;
            3:  out = a & b;
            4:  out = a | b;
            5:  out = a ^ b;
            6:  if (a > b)
                    out = 1;
                else
                    out = 0;
            7:  if (a == b)
                    out = 1;
                else
                    out = 0;
            default: out = 0;
        endcase
    end
endmodule
