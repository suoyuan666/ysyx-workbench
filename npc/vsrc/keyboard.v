module keyboard (
    input clk,
    input clrn,
    input ps2_clk,
    input ps2_data,

    output overflow,
    output [13:0] total,
    output [27:0] ascii_code,
    output [13:0] code
);

    reg nextdata_n;
    reg [7:0] ps_out;
    reg ready;
    reg [7:0] code_src;
    reg ascii_convert_enable;
    reg [7:0] ascii_prev;
    reg [7:0] ascii_next;
    reg break_en;
    reg [3:0] total_prev;
    reg [3:0] total_next;

    ps2_keyboard myps2(
        .clk(clk),
        .clrn(clrn),
        .ps2_clk(ps2_clk),
        .ps2_data(ps2_data),
        .nextdata_n(nextdata_n),
        .data(ps_out),
        .ready(ready),
        .overflow(overflow)
    );

    always @(posedge clk) begin
        if (nextdata_n == 1)
            nextdata_n <= 0;
    end

    always @(posedge clk) begin
        if (ready == 1)
            begin
                if (ps_out == 0)
                    begin
                        code_src <= 0;
                    end
                else
                    begin
                        if (ps_out != 8'h000000F0 && break_en == 0)
                            begin
                                ascii_convert_enable <= 1;
                                if (code_src != ps_out )
                                    begin
                                        if (total_next > 4'b1001)
                                            begin
                                                if (total_prev > 4'b1001)
                                                    total_prev <= 0;
                                                else
                                                    total_prev <= total_prev + 1;
                                                total_next <= 0;
                                            end
                                        else
                                            total_next <= total_next + 1;
                                        code_src <= ps_out;
                                        break_en <= 0;
                                    end
                            end
                        else
                            begin
                                break_en <= break_en ? 0 : 1;
                                code_src <= 0;
                                ascii_convert_enable <= 0;
                            end
                    end
                nextdata_n <= 0;
            end
    end

    bcd2ascii my_bcdascii_1(
        .bcd(code_src / 8'b00010000),
        .enable(ascii_convert_enable),
        .ascii(ascii_prev)
    );
    bcd2ascii my_bcdascii_2(
        .bcd(code_src % 8'b00010000),
        .enable(ascii_convert_enable),
        .ascii(ascii_next)
    );

    hex2ssg my_code_1(
        .in(code_src / 8'b00010000),
        .out(code[13:7])
    );
    hex2ssg my_code_2(
        .in(code_src % 8'b00010000),
        .out(code[6:0])
    );

    hex2ssg my_ascii_1(
        .in(ascii_prev / 8'b00001010),
        .out(ascii_code[27:21])
    );
    hex2ssg my_ascii_2(
        .in(ascii_prev % 8'b00001010),
        .out(ascii_code[20:14])
    );
    hex2ssg my_ascii_3(
        .in(ascii_next / 8'b00001010),
        .out(ascii_code[13:7])
    );
    hex2ssg my_ascii_4(
        .in(ascii_next % 8'b00001010),
        .out(ascii_code[6:0])
    );

    bcd2ssg my_total_1(
        .in(total_prev),
        .out(total[13:7])
    );
    bcd2ssg my_total_2(
        .in(total_next),
        .out(total[6:0])
    );

    
endmodule
