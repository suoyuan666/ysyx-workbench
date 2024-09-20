module shift (
    input [7:0] source,
    output reg [7:0] rs
);

    integer i;
    always @(source) begin
        if (source == 8'b00000000)
            rs = 8'b00000000;
        else
            begin
                rs = {source[4] ^ source[3] ^ source[2] ^ source[0], source[7:1]};
                for (i = 0; i < 255 ; i = i + 1) begin
                    rs = {rs[4] ^ rs[3] ^ rs[2] ^ rs[0], rs[7:1]};
                end
            end
    end
    
endmodule
