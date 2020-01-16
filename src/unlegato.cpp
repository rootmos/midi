#include <set>
#include "midi.hpp"
#include <iostream>

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " CHANNEL" << std::endl;
        return 2;
    }

    midi::Channel unlegatoChannel = std::stoi(argv[1]);
    std::cout << "Unlegato-ing MIDI-channel " << unlegatoChannel << std::endl;

    midi::Interface interface("unlegato");

    std::set<midi::Key> activeNotes;

    while (1)
    {
        midi::Command::ptr command = interface.nextCommand();
        if(command->getType() == midi::Command::Type::NoteOn)
        {
            auto noteOnCommand = std::dynamic_pointer_cast<midi::NoteOn>(command);
            if(noteOnCommand->getChannel() == unlegatoChannel)
            {
                for (auto key : activeNotes)
                    interface.sendCommand(midi::Command::ptr(new midi::NoteOff(noteOnCommand->getChannel(), key)));
                activeNotes.clear();
                activeNotes.insert(noteOnCommand->getKey());
                interface.sendCommand(command);
            }
        }
        else if(command->getType() == midi::Command::Type::NoteOff)
        {
            auto noteOffCommand = std::dynamic_pointer_cast<midi::NoteOff>(command);
            if(noteOffCommand->getChannel() == unlegatoChannel)
            {
                if(activeNotes.erase(noteOffCommand->getKey()))
                    interface.sendCommand(command);
            }
        }

    }

    return 0;
}
