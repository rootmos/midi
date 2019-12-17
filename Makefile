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

.PHONY: all clean
