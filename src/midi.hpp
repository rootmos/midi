#pragma once

#include <memory>
#include <cstring>

namespace midi {

typedef unsigned char Status;
typedef unsigned char Data;

typedef unsigned int Channel;
typedef unsigned char Key;
typedef unsigned char Velocity;
typedef unsigned char ControlNumber;
typedef unsigned char ControlValue;

class Command
{
public:
    enum Type { NoteOn, NoteOff, CC, TimingClock };

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
    ptr withChannel(Channel c) { return ptr(new NoteOn(c, key, velocity)); }
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
    ptr withChannel(Channel c) { return ptr(new NoteOff(c, key, velocity)); }
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

class CC: public Command
{
    Channel channel;
    ControlNumber number;
    ControlValue value;
public:

    CC(Channel channel, ControlNumber number, ControlValue value):
        Command(Type::CC), channel(channel), number(number), value(value) { }

    Channel getChannel() { return channel; }
    ptr withChannel(Channel c) { return ptr(new CC(c, number, value)); }
    ControlValue getValue() { return value; }
    ControlNumber getNumber() { return number; }

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
