ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD ?= $(ROOT)/build

EXEs = $(foreach exe, chordify, $(BUILD)/src/$(exe))

all: $(EXEs)

$(EXEs): $(BUILD)/Makefile $(shell git ls-files src) 
	$(MAKE) -C $(BUILD)

$(BUILD)/Makefile: $(shell git ls-files src | grep CMakeLists.txt)
	mkdir -p "$(dir $@)"
	cd $(BUILD) && cmake -DCMAKE_BUILD_TYPE=Release $(ROOT)

clean:
	rm -rf $(BUILD)

dependencies:
	sudo apt install cmake libasound2-dev crudini

$(BUILD):
	mkdir -p $@

aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) -static aclient.c -lusb-1.0 -lasound

.PHONY: all clean
