#include "midi.c"

struct state {
    unsigned char notes_channel;
    unsigned char triggers_low_channel;
    unsigned char triggers_high_channel;
    unsigned char out_channel;
};

unsigned char map_high_notes(unsigned char note)
{
    switch(note - 12) { // TODO: who adds this octave? the beatstep pro?
    case 48: return 1; // C3
    case 50: return 2; // D3
    case 52: return 3; // E3
    case 53: return 4; // F3
    case 55: return 5; // G3
    case 57: return 6; // A3
    default: return 0;
    }
}

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        if(CHANNEL(ev->data.note) == st->notes_channel) {
            debug("note on %u (note channel: %u)",
                  NOTE(ev->data.note), CHANNEL(ev->data.note));
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        } else if(CHANNEL(ev->data.note) == st->triggers_low_channel &&
                  NOTE(ev->data.note) <= 6) {
            debug("note on %u (low triggers channel: %u)",
                  NOTE(ev->data.note), CHANNEL(ev->data.note));
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        } else if(CHANNEL(ev->data.note) == st->triggers_high_channel) {
            unsigned char n = map_high_notes(NOTE(ev->data.note));
            if(n > 0) {
                debug("note off %u -> %u (high triggers channel: %u)",
                      NOTE(ev->data.note), n, CHANNEL(ev->data.note));
                SET_NOTE(ev->data.note, n);
                SET_CHANNEL(ev->data.note, st->out_channel);
                send_event(ctx, ev);
            }
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        if(CHANNEL(ev->data.note) == st->notes_channel) {
            debug("note off %u (note channel: %u)",
                  NOTE(ev->data.note), CHANNEL(ev->data.note));
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        } else if(CHANNEL(ev->data.note) == st->triggers_low_channel &&
                  NOTE(ev->data.note) <= 6) {
            debug("note off %u (low triggers channel: %u)",
                  NOTE(ev->data.note), CHANNEL(ev->data.note));
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        } else if(CHANNEL(ev->data.note) == st->triggers_high_channel) {
            unsigned char n = map_high_notes(NOTE(ev->data.note));
            if(n > 0) {
                debug("note off %u -> %u (high triggers channel: %u)",
                      NOTE(ev->data.note), n, CHANNEL(ev->data.note));
                SET_NOTE(ev->data.note, n);
                SET_CHANNEL(ev->data.note, st->out_channel);
                send_event(ctx, ev);
            }
        }
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        if(CHANNEL(ev->data.control) == st->notes_channel) {
            SET_CHANNEL(ev->data.control, st->out_channel);
            send_event(ctx, ev);
            info("cc %u %u", ev->data.control.param, ev->data.control.value);
        }
        break;
    case SND_SEQ_EVENT_CLOCK:
        send_event(ctx, ev);
        break;
    case SND_SEQ_EVENT_START:
    case SND_SEQ_EVENT_STOP:
    case SND_SEQ_EVENT_RESET:
    case SND_SEQ_EVENT_CONTINUE:
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        break;
    default:
        debug("ignored event of type=%d", ev->type);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 4) {
        dprintf(2, "usage: %s TRIGGERS_LOW_RANGE TRIGGERS_HIGH_RANGE NOTES OUT\n", argv[0]);
        return 2;
    }

    struct ctx ctx;
    init(&ctx, "default", "microGranny");

    struct state st = { 0 };

    int res = sscanf(argv[1], "%hhu", &st.triggers_low_channel);
    if(res != 1) { failwith("TRIGGERS_LOW_RANGE should be a MIDI channel"); }

    res = sscanf(argv[2], "%hhu", &st.triggers_high_channel);
    if(res != 1) { failwith("TRIGGERS_HIGH_RANGE should be a MIDI channel"); }

    res = sscanf(argv[3], "%hhu", &st.notes_channel);
    if(res != 1) { failwith("NOTES should be a MIDI channel"); }

    res = sscanf(argv[4], "%hhu", &st.out_channel);
    if(res != 1) { failwith("OUT should be a MIDI channel"); }

    loop(&ctx, go, &st);

    deinit(&ctx);
    return 0;
}
