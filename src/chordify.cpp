// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <set>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <midi.hpp>

class Chordifyer
{
    std::vector<int> semitones = {2, 2, 1, 2, 2, 2, 1};
    std::vector<midi::Key> notes;

public:
    Chordifyer(midi::Key base, size_t mode)
    {
        std::list<midi::Key> notesList;

        int key = static_cast<int>(base);
        size_t i = 0;
        while(key <= 0b01111111)
        {
            notesList.push_back(static_cast<midi::Key>(key));
            key += semitones[i];
            i = (i + 1) % semitones.size();
        }

        i = semitones.size() - 1;
        key = base;
        while(key >= 0)
        {
            key -= semitones[i];
            notesList.push_front(static_cast<midi::Key>(key));
            i = (i - 1) % semitones.size();
        }

        notes = std::vector<midi::Key>{notesList.begin(), notesList.end()};
    }

    std::set<midi::Key> chord(midi::Key key) {
        std::set<midi::Key> chordNotes;

        auto itr = std::find(notes.begin(), notes.end(), key);
        if (itr != notes.end())
        {
            chordNotes.insert(*itr);
            chordNotes.insert(*(itr + 3 - 1));
            chordNotes.insert(*(itr + 5 - 1));
            chordNotes.insert(*(itr + 7 - 1));
            chordNotes.insert(*(itr + 9 - 1));
            chordNotes.insert(*(itr + 13 - 1));
        }

        return chordNotes;
    }

};


int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " CHANNEL" << std::endl;
        return 2;
    }

    midi::Channel chordifyChannel = std::stoi(argv[1]);
    std::cout << "Chordify-ing MIDI-channel " << chordifyChannel << std::endl;

    midi::Interface interface("chordify");

    Chordifyer c_major(60, 0);

    while (1)
    {
        midi::Command::ptr command = interface.nextCommand();
        if(command->getType() == midi::Command::Type::NoteOn)
        {
            auto noteOnCommand = std::dynamic_pointer_cast<midi::NoteOn>(command);
            if(noteOnCommand->getChannel() == chordifyChannel)
            {
                for (auto key : c_major.chord(noteOnCommand->getKey()))
                {
                    interface.sendCommand(midi::Command::ptr(new midi::NoteOn(
                                noteOnCommand->getChannel(),
                                key,
                                noteOnCommand->getVelocity())));
                }
            }
        }
        else if(command->getType() == midi::Command::Type::NoteOff)
        {
            auto noteOffCommand = std::dynamic_pointer_cast<midi::NoteOff>(command);
            if(noteOffCommand->getChannel() == chordifyChannel)
            {
                for (auto key : c_major.chord(noteOffCommand->getKey()))
                {
                    interface.sendCommand(midi::Command::ptr(new midi::NoteOff(
                                noteOffCommand->getChannel(),
                                key,
                                noteOffCommand->getVelocity())));
                }
            }
        }

    }

    return 0;
}
