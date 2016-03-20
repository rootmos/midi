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

#ifndef midi_hpp
#define midi_hpp

#include <memory>
#include <cstring>

namespace midi {

typedef unsigned char Status;
typedef unsigned char Data;

typedef unsigned int Channel;
typedef unsigned char Key;
typedef unsigned char Velocity;

class Command
{
public:
    enum Type { NoteOn, NoteOff, TimingClock };

    Command(Type type): type(type) { }
    virtual ~Command() {};
    virtual std::string to_string() = 0;

    Type getType() { return type; }

    typedef std::shared_ptr<Command> ptr;

private:
    Type type;
};

class NoteOn: public Command
{
    Channel channel;
    Key key;
    Velocity velocity;
public:
    NoteOn(Channel channel, Key key, Velocity velocity = 64):
        Command(Type::NoteOn), channel(channel), key(key), velocity(velocity) { }

    Channel getChannel() { return channel; }
    Key getKey() { return key; }
    Velocity getVelocity() { return velocity; }

    std::string to_string();
};

class NoteOff: public Command
{
    Channel channel;
    Key key;
    Velocity velocity;
public:

    NoteOff(Channel channel, Key key, Velocity velocity = 64):
        Command(Type::NoteOff), channel(channel), key(key), velocity(velocity) { }

    Channel getChannel() { return channel; }
    Key getKey() { return key; }
    Velocity getVelocity() { return velocity; }

    std::string to_string();
};

class TimingClock: public Command
{
public:
    TimingClock(): Command(Type::TimingClock) { }

    std::string to_string();
};


class Interface
{
    class Internals;
    std::shared_ptr<Internals> internals;

public:
    Interface(const std::string& name);

    Command::ptr nextCommand();
    bool sendCommand(const Command::ptr& command);
};

}

#endif
