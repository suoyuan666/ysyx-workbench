module code (
    input [7:0] cod,
    input en,
    output reg [3:0] out,
    output reg [6:0] sout
);
    always @(cod, en) begin
       if (en) begin
        out = 0;
            for (integer i = 0; i < 8; i = i + 1)
                if (cod[i] == 1) out = i[3:0];
       end
       else
        out = 0;
    end
    bcd2ssg sg(
        .in(out),
        .out(sout)
    );
endmodule
