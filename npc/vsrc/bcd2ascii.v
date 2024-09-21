module bcd2ascii (
    input [7:0] bcd,
    input enable,
    output reg [7:0] ascii
);

    always @(bcd) begin
        if (enable)
            case (bcd)
                0: ascii <= 30;
                1: ascii <= 31;
                2: ascii <= 32;
                3: ascii <= 33;
                4: ascii <= 34;
                5: ascii <= 35;
                6: ascii <= 36;
                7: ascii <= 37;
                8: ascii <= 38;
                9: ascii <= 39;
                10: ascii <= 61;
                11: ascii <= 62;
                12: ascii <= 63;
                13: ascii <= 64;
                14: ascii <= 65;
                15: ascii <= 66;
                default: ascii <= 0;
            endcase
        else
            ascii <= 0;
    end


endmodule
