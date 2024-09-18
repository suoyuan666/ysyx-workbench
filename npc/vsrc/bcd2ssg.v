module bcd2ssg (
    input [3:0] in,
    output reg [6:0] out
);
    always @(in) begin
        case (in)
            0: out =  7'b1000000;
            1: out =  7'b1111100;
            2: out =  7'b1100110;
            3: out =  7'b0110000;
            4: out =  7'b0011001;
            5: out =  7'b0010010;
            6: out =  7'b0000010;
            7: out =  7'b1111000;
            8: out =  7'b0000000;
            default: out =  7'b1000000;
        endcase
    end
endmodule
