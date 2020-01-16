ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
export BUILD ?= $(ROOT)/build

all:
	@mkdir -p "$(BUILD)"
	$(MAKE) -C src

midi.tar.gz: all
	scripts/bundle.sh -f "$@" -b "$(BUILD)" -r "$(ROOT)/root"

deploy: midi.tar.gz
	scp $< midi:
	ssh midi sudo tar xf $< -C /

clean:
	rm -rf $(BUILD)

.PHONY: all clean
