all: $(foreach exe, unlegato chordify microGranny aclient, $(BUILD)/$(exe))

$(BUILD)/%: %.cpp midi.cpp midi.hpp
	$(CXX) -o $@ $(CXXFLAGS) -O2 -DNO_DEBUG -static $< midi.cpp -lasound

$(BUILD)/aclient: aclient.c
	$(CC) -o $@ $(CFLAGS) -O2 -static $< -lusb-1.0 -lasound
