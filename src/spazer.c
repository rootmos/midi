#include "midi.c"

struct note {
    unsigned int offset;
    unsigned char controller_number;
};

struct state {
    unsigned char note_channel;
    unsigned char control_channel;
    struct note note[256];
    size_t notes;
};

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        if(st->note_channel == ev->data.note.channel) {
            debug("note on: %u", ev->data.note.note);
            send_event(ctx, ev);
            unsigned char base = ev->data.note.note;
            for(size_t i = 0; i < st->notes; i++) {
                if(st->note[i].offset > 0) {
                    ev->data.note.note = base + st->note[i].offset;
                    send_event(ctx, ev);
                }
            }
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        if(st->note_channel == ev->data.note.channel) {
            debug("note off: %u", ev->data.note.note);
            send_event(ctx, ev);
            unsigned char base = ev->data.note.note;
            for(size_t i = 0; i < st->notes; i++) {
                if(st->note[i].offset > 0) {
                    ev->data.note.note = base + st->note[i].offset;
                    send_event(ctx, ev);
                }
            }
        }
        break;
    case SND_SEQ_EVENT_KEYPRESS:
        if(st->note_channel == ev->data.note.channel) {
            debug("keypress: %u", ev->data.note.note);
            send_event(ctx, ev);
            unsigned char base = ev->data.note.note;
            for(size_t i = 0; i < st->notes; i++) {
                if(st->note[i].offset > 0) {
                    ev->data.note.note = base + st->note[i].offset;
                    send_event(ctx, ev);
                }
            }
        }
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        debug("cc %u %u %u", ev->data.control.channel, ev->data.control.param, ev->data.control.value);
        if(st->control_channel == ev->data.control.channel) {
            for(size_t i = 0; i < st->notes; i++) {
                if(ev->data.control.param == st->note[i].controller_number) {
                    st->note[i].offset = ev->data.control.value;
                    info("setting note %zu: offset=%u", i + 1,
                         st->note[i].offset);
                }
            }
        }
        break;
    case SND_SEQ_EVENT_START:
    case SND_SEQ_EVENT_STOP:
    case SND_SEQ_EVENT_RESET:
    case SND_SEQ_EVENT_CONTINUE:
    case SND_SEQ_EVENT_CLOCK:
        send_event(ctx, ev);
        break;
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        break;
    default:
        warning("ignored event of type=%d", ev->type);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 4) {
        dprintf(2, "usage: %s NAME NOTE CONTROL\n", argv[0]);
        return 2;
    }

    struct state st = {
        .note_channel = -1,
        .control_channel = -1,
        .notes = 0,
    };

    st.note[st.notes++] = (struct note) { .controller_number = 110 };
    st.note[st.notes++] = (struct note) { .controller_number = 111 };
    st.note[st.notes++] = (struct note) { .controller_number = 112 };
    st.note[st.notes++] = (struct note) { .controller_number = 113 };
    st.note[st.notes++] = (struct note) { .controller_number = 17 };
    st.note[st.notes++] = (struct note) { .controller_number = 91 };
    st.note[st.notes++] = (struct note) { .controller_number = 79 };
    st.note[st.notes++] = (struct note) { .controller_number = 72 };

    for(size_t i = 0; i < st.notes; i++) {
        st.note[i].offset = 0;
        info("note (%zu): controller number=%u", i+1,
             st.note[i].controller_number);
    }

    int res = sscanf(argv[2], "%hhu", &st.note_channel);
    if(res != 1) {
        failwith("NOTE should be a MIDI channel");
    }
    st.note_channel -= 1;

    res = sscanf(argv[3], "%hhu", &st.control_channel);
    if(res != 1) {
        failwith("CONTROL should be a MIDI channel");
    }
    st.control_channel -= 1;

    info("spazer: notes=%u control=%u",
         st.note_channel + 1,
         st.control_channel + 1);

    struct ctx ctx;
    init(&ctx, "default", argv[1]);

    loop(&ctx, go, &st);

    deinit(&ctx);
    return 0;
}
