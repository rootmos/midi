EXTRA_CFLAGS ?= -Wall -Werror -O2
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -I$(BUILD)/usr/include

LDFLAGS ?= -static
LDFLAGS += -L$(BUILD)/usr/lib

all: $(foreach exe, forward microGranny aclient menu spazer, $(BUILD)/bin/$(exe))

$(BUILD)/bin/%: %.c midi.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) -lasound -lr

$(BUILD)/bin/aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) -lusb-1.0 -lasound
