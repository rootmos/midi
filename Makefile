ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
export BUILD ?= $(ROOT)/build

CFLAGS += -I$(BUILD)/usr/include -L$(BUILD)/usr/lib

all: libr
	@mkdir -p "$(BUILD)"
	$(MAKE) -C src

libr:
	@mkdir -p "$(BUILD)"
	$(MAKE) -C libr install PREFIX="$(BUILD)/usr" EXTRA_CFLAGS="$(CFLAGS)"

midi.tar.gz: all
	scripts/bundle.sh -f "$@" -b "$(BUILD)" -r "$(ROOT)/root"

PREFIX ?= /usr/local/bin
install: all
	install -D -t "$(DESTDIR)$(PREFIX)" "$(BUILD)"/bin/*

SPL_ROOT=$(HOME)/git/spl
img:
	$(SPL_ROOT)/raspberry.sh -c $(SPL_ROOT)/.cache -l $(ROOT)/.log \
		-1 -s "$(ROOT)/root" -S -u 'make install DESTDIR="$$0"'

deploy: midi.tar.gz
	scp $< midi:
	ssh midi sudo tar xf $< -C /

clean:
	rm -rf $(BUILD)

.PHONY: all clean libr deploy
