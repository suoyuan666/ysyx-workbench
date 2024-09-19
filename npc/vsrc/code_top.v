module code_top (
    input [7:0] code,
    input en,
    output reg [6:0] sout,
    output reg [3:0] out
);
    code my_code(
        .cod(code),
        .en(en),
        .out(out)
    );
    bcd2ssg my_ssg(
        .in(out),
        .out(sout)
    );
    
endmodule
