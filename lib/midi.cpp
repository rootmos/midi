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

#include "midi.hpp"

#include <alsa/asoundlib.h>
#include "debug.hpp"
#include <memory>
#include <cstring>
#include <stdexcept>
#include <bitset>


namespace midi {

std::string NoteOn::to_string()
{
    return "NoteOn(chan="+  std::to_string(channel) + ", key=" + std::to_string(key) + ", velocity=" + std::to_string(velocity) + ")";
}


std::string NoteOff::to_string()
{
    return "NoteOff(chan="+  std::to_string(channel) + ", key=" + std::to_string(key) + ", velocity=" + std::to_string(velocity) + ")";
}

std::string TimingClock::to_string()
{
    return "TimingClock";
}


class Interface::Internals
{
public:
    Internals(const std::string& name);
    ~Internals();

    Command::ptr nextCommand();
    bool sendCommand(const Command::ptr& command);

private:
    void openSequencer();
    void trySettingName();
    void closeSequencer();

    void tryOpenPorts();
    void tryOpenInputPort();
    void tryOpenOutputPort();

    void closePorts();
    void closeInputPort();
    void closeOutputPort();

    void buildEvent(snd_seq_event_t* ev, const Command::ptr& command);

    std::string name;

    snd_seq_t *handle;
    int client_id;

    int input_port;
    int output_port;
};

Interface::Interface(const std::string& name): internals(new Internals(name))
{
}

Interface::Internals::Internals(const std::string& name): name(name)
{
    openSequencer();

    try {
        trySettingName();
        tryOpenPorts();
    }
    catch (const std::exception& e)
    {
        closeSequencer();
        throw;
    }

    info(("Successfully created interface: %s", name.c_str()))
}

void Interface::Internals::openSequencer()
{
    int err;
    if((err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_INPUT | SND_SEQ_OPEN_OUTPUT, 0)) == 0)
        debug(("Successfully opened default sequencer"))
    else
        throw std::runtime_error("Unable to open default sequencer device: " + std::string(snd_strerror(err)));

    client_id = snd_seq_client_id(handle);
    debug(("Client id = %d", client_id));
}

void Interface::Internals::trySettingName()
{
    int err;
    if((err = snd_seq_set_client_name(handle, name.c_str())) != 0)
        throw std::runtime_error("Unable to set client name: " + std::string(snd_strerror(err)));
}

void Interface::Internals::tryOpenPorts()
{
    tryOpenInputPort();
    
    try
    {
        tryOpenOutputPort();
    }
    catch(const std::exception& e)
    {
        closeInputPort();
        throw;
    }
}

void Interface::Internals::tryOpenInputPort()
{
    unsigned int input_caps = SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE;
    unsigned int input_flags = SND_SEQ_PORT_TYPE_MIDI_GENERIC;
    input_port = snd_seq_create_simple_port(handle, "input port", input_caps, input_flags);
    if (input_port < 0)
        throw std::runtime_error("Unable to open input port: " + std::string(snd_strerror(input_port)));
}

void Interface::Internals::tryOpenOutputPort()
{
    unsigned int output_caps = SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;
    unsigned int output_flags = SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION;
    output_port = snd_seq_create_simple_port(handle, "output port", output_caps, output_flags);
    if (output_port < 0)
        throw std::runtime_error("Unable to open output port: " + std::string(snd_strerror(output_port)));
}

void Interface::Internals::closeSequencer()
{
    int err;
    if((err = snd_seq_close(handle)) == 0)
        debug(("Closed sequencer named: %s", name.c_str()))
    else
        error(("Unable to close sequencer device: %s", snd_strerror(err)))
}

void Interface::Internals::closePorts()
{
    closeInputPort();
    closeOutputPort();
}

void Interface::Internals::closeInputPort()
{
    int err;
    if ((err = snd_seq_delete_simple_port(handle, input_port)) != 0)
        error(("Unable to close input port: %s", snd_strerror(err)))
}

void Interface::Internals::closeOutputPort()
{
    int err;
    if ((err = snd_seq_delete_simple_port(handle, output_port)) != 0)
        error(("Unable to close output port: %s", snd_strerror(err)))
}

Interface::Internals::~Internals()
{
    closePorts();
    closeSequencer();

    debug(("Successfully destroyed interface: %s", name.c_str()))
}

Command::ptr Interface::nextCommand()
{
    return internals->nextCommand();
}

Command::ptr Interface::Internals::nextCommand()
{
    snd_seq_event_t* ev;
    debug(("Waiting for event on interface %s", name.c_str()))
    int err = snd_seq_event_input(handle, &ev);
    if (err < 0)
        throw std::runtime_error("Unable to read next command" + std::string(snd_strerror(err)));


    if(ev->type == SND_SEQ_EVENT_NOTEON)
    {
        auto note = Command::ptr(new NoteOn(ev->data.note.channel + 1, ev->data.note.note, ev->data.note.velocity));
        debug(("Received %s on interface %s", note->to_string().c_str(), name.c_str()))
        return note;
    }
    if(ev->type == SND_SEQ_EVENT_NOTEOFF)
    {
        auto note = Command::ptr(new NoteOff(ev->data.note.channel + 1, ev->data.note.note, ev->data.note.velocity));
        debug(("Received %s on interface %s", note->to_string().c_str(), name.c_str()))
        return note;
    }
    else
    {
        debug(("Received event of unimplemented type: %d", ev->type));
        return nextCommand();
    }
}

bool Interface::sendCommand(const Command::ptr& command)
{
    return internals->sendCommand(command);
}

bool Interface::Internals::sendCommand(const Command::ptr& command)
{
    snd_seq_event_t ev;
    buildEvent(&ev, command);

    int err;
    if((err = snd_seq_event_output(handle, &ev)) < 0)
    {
        error(("Unable send command %s in interface: %s. Reason: %s",
                    command->to_string().c_str(),
                    name.c_str(),
                    snd_strerror(err)));
        return false;
    }

    if((err = snd_seq_drain_output(handle)) < 0)
    {
        error(("Unable to drain sequencer while sending command %s in interface: %s. Reason: %s",
                    command->to_string().c_str(),
                    name.c_str(),
                    snd_strerror(err)));
        return false;
    }

    debug(("Successfully sent command %s in interface: %s", command->to_string().c_str(), name.c_str()));

    return true;
}

void Interface::Internals::buildEvent(snd_seq_event_t* ev, const Command::ptr& command)
{
    switch(command->getType())
    {
    case Command::Type::NoteOn:
        {
            auto note = std::dynamic_pointer_cast<midi::NoteOn>(command);
            snd_seq_ev_set_noteon(ev, note->getChannel() - 1, note->getKey(), note->getVelocity());
        }
        break;
    case Command::Type::NoteOff:
        {
            auto note = std::dynamic_pointer_cast<midi::NoteOff>(command);
            snd_seq_ev_set_noteoff(ev, note->getChannel() - 1, note->getKey(), note->getVelocity());
        }
        break;
    default:
        throw std::runtime_error("Trying to convert unknown type!");
    }

    snd_seq_ev_set_direct(ev);
    snd_seq_ev_set_source(ev, output_port);
    snd_seq_ev_set_subs(ev);
}

}
