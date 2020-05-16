CFLAGS ?= -Wall -Werror -O2 -static
CFLAGS += -I$(BUILD)/usr/include
LDFLAGS += -L$(BUILD)/usr/lib

all: $(foreach exe, forward microGranny aclient menu, $(BUILD)/bin/$(exe))

$(BUILD)/bin/%: %.c midi.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ $(CFLAGS) $< -lasound -lr $(LDFLAGS)

$(BUILD)/bin/aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) $< -lusb-1.0 -lasound $(LDFLAGS)
