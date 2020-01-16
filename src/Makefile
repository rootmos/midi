all: $(foreach exe, unlegato chordify microGranny aclient, $(BUILD)/$(exe))

$(BUILD)/%: %.cpp midi.cpp midi.hpp
	$(CXX) -o $@ $(CXXFLAGS) -static $< midi.cpp -lasound

$(BUILD)/aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) -static $< -lusb-1.0 -lasound
