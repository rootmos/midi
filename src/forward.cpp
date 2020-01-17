#include "midi.hpp"

#include <iostream>
#include <set>

int main(int argc, const char* argv[])
{
    if(argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " NAME OUT IN*" << std::endl;
        return 2;
    }

    midi::Interface interface(argv[1]);

    midi::Channel out = std::stoi(argv[2]);
    std::set<midi::Channel> ins;
    for(int i = 3; i < argc; i++) {
        ins.insert(std::stoi(argv[i]));
    }

    while (1)
    {
        midi::Command::ptr cmd = interface.nextCommand();
        if(cmd->getType() == midi::Command::Type::NoteOn)
        {
            auto noteOn = std::dynamic_pointer_cast<midi::NoteOn>(cmd);
            if(ins.find(noteOn->getChannel()) != ins.end())
            {
                interface.sendCommand(noteOn->withChannel(out));
            }
        }
        else if(cmd->getType() == midi::Command::Type::NoteOff)
        {
            auto noteOff = std::dynamic_pointer_cast<midi::NoteOff>(cmd);
            if(ins.find(noteOff->getChannel()) != ins.end())
            {
                interface.sendCommand(noteOff->withChannel(out));
            }
        }
        else if(cmd->getType() == midi::Command::Type::RealTime)
        {
            interface.sendCommand(cmd);
        }
        else if(cmd->getType() == midi::Command::Type::CC)
        {
            auto cc = std::dynamic_pointer_cast<midi::CC>(cmd);
            if(ins.find(cc->getChannel()) != ins.end())
            {
                interface.sendCommand(cc->withChannel(out));
            }
        }
    }

    return 0;
}
