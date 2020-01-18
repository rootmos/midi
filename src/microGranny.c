#include "midi.c"

struct state {
    unsigned char notes_channel;
    unsigned char sounds_channel;
    unsigned char out_channel;
};

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        if(CHANNEL(ev->data.note) == st->notes_channel
           || (CHANNEL(ev->data.note) == st->sounds_channel
               && ev->data.note.note <= 6)) {
            debug("note on %u", ev->data.note.note);
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        if(CHANNEL(ev->data.note) == st->notes_channel
           || (CHANNEL(ev->data.note) == st->sounds_channel
               && ev->data.note.note <= 6)) {
            debug("note off %u", ev->data.note.note);
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
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
        dprintf(2, "usage: %s SOUNDS NOTES OUT\n", argv[0]);
        return 2;
    }

    struct ctx ctx;
    init(&ctx, "default", "microGranny");

    struct state st = { 0 };

    int res = sscanf(argv[1], "%hhu", &st.sounds_channel);
    if(res != 1) { failwith("SOUNDS should be a MIDI channel"); }

    res = sscanf(argv[2], "%hhu", &st.notes_channel);
    if(res != 1) { failwith("NOTES should be a MIDI channel"); }

    res = sscanf(argv[3], "%hhu", &st.out_channel);
    if(res != 1) { failwith("OUT should be a MIDI channel"); }

    loop(&ctx, go, &st);

    deinit(&ctx);
    return 0;
}
