#include <set>
#include "midi.hpp"
#include <iostream>

inline bool allowedMicroGrannyNote(const std::shared_ptr<midi::NoteOn>& note) {
    return (note->getKey() >= 1 && note->getKey() <= 6);
}

inline bool allowedMicroGrannyNote(const std::shared_ptr<midi::NoteOff>& note) {
    return (note->getKey() >= 1 && note->getKey() <= 6);
}

int main(int argc, const char* argv[])
{
    if(argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " SOUNDS NOTES OUT" << std::endl;
        return 2;
    }

    midi::Channel soundsChannel = std::stoi(argv[1]);
    midi::Channel notesChannel = std::stoi(argv[2]);
    midi::Channel outChannel = std::stoi(argv[3]);
    std::cout << "microGranny-ing MIDI-channels:"
        << " sounds=" << soundsChannel
        << " notes=" << notesChannel
        << " out=" << outChannel
        << std::endl;

    midi::Interface interface("microGranny");

    std::set<midi::Key> activeNotes;

    while (1)
    {
        midi::Command::ptr command = interface.nextCommand();
        if(command->getType() == midi::Command::Type::NoteOn)
        {
            auto noteOnCommand = std::dynamic_pointer_cast<midi::NoteOn>(command);
            if(noteOnCommand->getChannel() == soundsChannel && allowedMicroGrannyNote(noteOnCommand))
            {
                for (auto key : activeNotes)
                    interface.sendCommand(midi::Command::ptr(new midi::NoteOff(noteOnCommand->getChannel(), key)));
                activeNotes.clear();
                activeNotes.insert(noteOnCommand->getKey());
                interface.sendCommand(noteOnCommand->withChannel(outChannel));
            } else if(noteOnCommand->getChannel() == notesChannel)
            {
                interface.sendCommand(noteOnCommand->withChannel(outChannel));
            }
        }
        else if(command->getType() == midi::Command::Type::NoteOff)
        {
            auto noteOffCommand = std::dynamic_pointer_cast<midi::NoteOff>(command);
            if(noteOffCommand->getChannel() == notesChannel)
            {
                interface.sendCommand(noteOffCommand->withChannel(outChannel));
            }
        }
        else if(command->getType() == midi::Command::Type::RealTime)
        {
            interface.sendCommand(command);
        }
        else if(command->getType() == midi::Command::Type::CC)
        {
            auto ccCommand = std::dynamic_pointer_cast<midi::CC>(command);
            if(ccCommand->getChannel() == notesChannel) {
                interface.sendCommand(ccCommand->withChannel(outChannel));
            }
        }
    }

    return 0;
}
