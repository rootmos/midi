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

    unsigned char active_notes[32];
    unsigned char active_notes_velocity[32];
};

static void reset_active_notes(struct state* st)
{
    for(size_t i = 0; i < LENGTH(st->active_notes); i++) {
        st->active_notes[i] = 0b10000000;
    }
}

static void handle_note_on(struct ctx* ctx, struct state* st,
                           snd_seq_event_t* ev)
{
    unsigned char base = ev->data.note.note;
    debug("note on: %u", base);

    send_event(ctx, ev);
    for(size_t i = 0; i < st->notes; i++) {
        if(st->note[i].offset == 0) continue;
        ev->data.note.note = base + st->note[i].offset;
        send_event(ctx, ev);
    }

    for(size_t i = 0; i < LENGTH(st->active_notes); i++) {
        if(st->active_notes[i] == base || st->active_notes[i] & 0b10000000) {
            st->active_notes[i] = base;
            st->active_notes_velocity[i] = ev->data.note.velocity;
            break;
        }
    }
}

static void handle_note_off(struct ctx* ctx, struct state* st,
                            snd_seq_event_t* ev)
{
    unsigned char base = ev->data.note.note;
    debug("note off: %u", base);

    send_event(ctx, ev);
    for(size_t i = 0; i < st->notes; i++) {
        if(st->note[i].offset == 0) continue;
        ev->data.note.note = base + st->note[i].offset;
        send_event(ctx, ev);
    }

    for(size_t i = 0; i < LENGTH(st->active_notes); i++) {
        if(st->active_notes[i] == base) {
            st->active_notes[i] = 0b10000000;
            break;
        }
    }
}

static void handle_keypress(struct ctx* ctx, struct state* st,
                            snd_seq_event_t* ev)
{
    unsigned char base = ev->data.note.note;
    debug("keypress: %u", base);

    send_event(ctx, ev);
    for(size_t i = 0; i < st->notes; i++) {
        if(st->note[i].offset == 0) continue;
        ev->data.note.note = base + st->note[i].offset;
        send_event(ctx, ev);
    }
}

static void change_offset(struct ctx* ctx, struct state* st,
                          unsigned char param, unsigned char value)
{
    for(size_t i = 0; i < st->notes; i++) {
        if(param != st->note[i].controller_number) continue;

        unsigned char prev = st->note[i].offset;
        st->note[i].offset = value;
        info("setting note %zu: offset=%u", i + 1, value);

        for(size_t j = 0; j < LENGTH(st->active_notes); j++) {
            if(st->active_notes[j] >> 7) continue;

            unsigned char base = st->active_notes[j];
            unsigned char v = st->active_notes_velocity[j];

            snd_seq_event_t ev;
            snd_seq_ev_clear(&ev);
            snd_seq_ev_set_noteoff(&ev, st->note_channel, base + prev, v);
            send_event(ctx, &ev);

            snd_seq_ev_set_noteon(&ev, st->note_channel, base + value, v);
            send_event(ctx, &ev);

            info("resetting offset of live note: base=%u offset=%u->%u",
                 base, prev, value);
        }
    }
}

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        if(st->note_channel == ev->data.note.channel) {
            handle_note_on(ctx, st, ev);
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        if(st->note_channel == ev->data.note.channel) {
            handle_note_off(ctx, st, ev);
        }
        break;
    case SND_SEQ_EVENT_KEYPRESS:
        if(st->note_channel == ev->data.note.channel) {
            handle_keypress(ctx, st, ev);
        }
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        debug("cc %u %u %u", ev->data.control.channel, ev->data.control.param, ev->data.control.value);
        if(st->control_channel == ev->data.control.channel) {
            change_offset(ctx, st,
                          ev->data.control.param,
                          ev->data.control.value);
        } else if(st->note_channel == ev->data.control.channel) {
            send_event(ctx, ev);
            if(ev->data.control.param == 123 &&
               ev->data.control.value == 0) {
                reset_active_notes(st);
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

    reset_active_notes(&st);

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
