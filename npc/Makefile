CS=csrc
VS=vsrc
OUT_DIR=./build
OUT=$(OUT_DIR)/out
INC_PATH=$(NVBOARD_HOME)/usr/include

VSF=$(VS)/ps2_keyboard.v	\
	$(VS)/keyboard.v		\
	$(VS)/bcd2ssg.v 		\
	$(VS)/hex2ssg.v			\
	$(VS)/bcd2ascii.v
CSF=$(CS)/keyboard.cpp
TOPNAME=keyboard

VERILATOR_FLAGS = --cc --exe -j 6 --build --trace -Wall

include $(NVBOARD_HOME)/scripts/nvboard.mk
INCLUDE = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCLUDE) -ggdb
LDFLAGS += -lc++

all: $(CSF) $(VSF) $(NVBOARD_ARCHIVE)
	verilator $^ --Mdir $(OUT_DIR)  \
		-o $(abspath $(OUT))		\
		--top-module $(TOPNAME)		\
		$(VERILATOR_FLAGS)			\
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS))

sim: run
	gtkwave ./waveform.vcd

run: all
	$(OUT)

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean sim run

include ../Makefile
