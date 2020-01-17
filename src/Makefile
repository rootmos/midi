all: $(foreach exe, forward microGranny aclient, $(BUILD)/bin/$(exe))

$(BUILD)/bin/%: %.c midi.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ $(CFLAGS) -Wall -Werror -O2 -static $< -lasound -lr

$(BUILD)/bin/aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) -O2 -static $< -lusb-1.0 -lasound
