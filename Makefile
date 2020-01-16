ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
export BUILD ?= $(ROOT)/build

all:
	@mkdir -p "$(BUILD)"
	$(MAKE) -C src

clean:
	rm -rf $(BUILD)

.PHONY: all clean
