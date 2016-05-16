# vim: set ft=make:

SERVICES=/etc/systemd/system
SERVICES_TEMPLATE=template.service

CONFIG=midi.config
GET_CONFIG=crudini --get $(CONFIG)

BUILD=build

UNLEGATO_BIN=$(BUILD)/unlegato
UNLEGATO_CHANNEL=$(shell $(GET_CONFIG) unlegato channel)
UNLEGATO_INPUT=$(shell $(GET_CONFIG) unlegato input)
UNLEGATO_OUTPUT=$(shell $(GET_CONFIG) unlegato output)
UNLEGATO_SERVICE=unlegato.service
UNLEGATO_SERVICE_FILE=$(SERVICES)/$(UNLEGATO_SERVICE)

CHORDIFY_BIN=$(BUILD)/chordify
CHORDIFY_CHANNEL=$(shell $(GET_CONFIG) chordify channel)
CHORDIFY_INPUT=$(shell $(GET_CONFIG) chordify input)
CHORDIFY_OUTPUT=$(shell $(GET_CONFIG) chordify output)
CHORDIFY_SERVICE=chordify.service
CHORDIFY_SERVICE_FILE=$(SERVICES)/$(CHORDIFY_SERVICE)

MICROGRANNY_BIN=$(BUILD)/microGranny
MICROGRANNY_CHANNEL=$(shell $(GET_CONFIG) microGranny channel)
MICROGRANNY_INPUT=$(shell $(GET_CONFIG) microGranny input)
MICROGRANNY_OUTPUT=$(shell $(GET_CONFIG) microGranny output)
MICROGRANNY_SERVICE=microGranny.service
MICROGRANNY_SERVICE_FILE=$(SERVICES)/$(MICROGRANNY_SERVICE)

install: unlegato chordify microGranny

dependencies:
	sudo apt install cmake libasound2-dev crudini

.PHONY: unlegato
unlegato: $(UNLEGATO_BIN) $(UNLEGATO_SERVICE_FILE)
	sudo systemctl start $(UNLEGATO_SERVICE)

$(UNLEGATO_SERVICE_FILE): $(UNLEGATO_BIN) $(CONFIG)
	sudo install -D $(SERVICES_TEMPLATE) $@
	sudo sed -i "s#%%EXECUTABLE%%#$(shell readlink -f $<) $(UNLEGATO_CHANNEL)#" $@
	sudo sed -i "s#%%SERVICE%%#unlegato#" $@
	sudo sed -i "s#%%INPUT%%#$(UNLEGATO_INPUT)#" $@
	sudo sed -i "s#%%OUTPUT%%#$(UNLEGATO_OUTPUT)#" $@
	sudo systemctl enable $(UNLEGATO_SERVICE)

.PHONY: chordify
chordify: $(CHORDIFY_BIN) $(CHORDIFY_SERVICE_FILE)
	sudo systemctl start $(CHORDIFY_SERVICE)

$(CHORDIFY_SERVICE_FILE): $(CHORDIFY_BIN) $(CONFIG)
	sudo install -D $(SERVICES_TEMPLATE) $@
	sudo sed -i "s#%%EXECUTABLE%%#$(shell readlink -f $<) $(CHORDIFY_CHANNEL)#" $@
	sudo sed -i "s#%%SERVICE%%#chordify#" $@
	sudo sed -i "s#%%INPUT%%#$(CHORDIFY_INPUT)#" $@
	sudo sed -i "s#%%OUTPUT%%#$(CHORDIFY_OUTPUT)#" $@
	sudo systemctl enable $(CHORDIFY_SERVICE)

.PHONY: microGranny
microGranny: $(MICROGRANNY_BIN) $(MICROGRANNY_SERVICE_FILE)
	sudo systemctl start $(MICROGRANNY_SERVICE)

$(MICROGRANNY_SERVICE_FILE): $(MICROGRANNY_BIN) $(CONFIG)
	sudo install -D $(SERVICES_TEMPLATE) $@
	sudo sed -i "s#%%EXECUTABLE%%#$(shell readlink -f $<) $(MICROGRANNY_CHANNEL)#" $@
	sudo sed -i "s#%%SERVICE%%#microGranny#" $@
	sudo sed -i "s#%%INPUT%%#$(MICROGRANNY_INPUT)#" $@
	sudo sed -i "s#%%OUTPUT%%#$(MICROGRANNY_OUTPUT)#" $@
	sudo systemctl enable $(MICROGRANNY_SERVICE)

.PHONY: $(UNLEGATO_BIN) $(CHORDIFY_BIN) $(MICROGRANNY_BIN)
$(CHORDIFY_BIN) $(UNLEGATO_BIN): | $(BUILD) dependencies
	cd $(BUILD); cmake -DCMAKE_BUILD_TYPE=Release ..
	cd $(BUILD); make

$(BUILD):
	mkdir -p $@
