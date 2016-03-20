# vim: set ft=make:

RASPBIAN_URL := "https://downloads.raspberrypi.org/raspbian_lite_latest"
RASPBIAN_ZIP := $(shell curl --silent --head $(RASPBIAN_URL) | grep "Location:" | sed "s/^Location:\s\+//" | xargs basename )

RASPBIAN_IMG := $(RASPBIAN_ZIP:.zip=.img)

install: $(RASPBIAN_IMG)
ifndef SD_CARD
	$(error SD_CARD is not set)
endif
	sudo dd bs=4M if=$< of=$(SD_CARD)

$(RASPBIAN_ZIP):
	wget --output-document=$@ $(RASPBIAN_URL)

%.img: %.zip
	unzip $<
	touch $@
