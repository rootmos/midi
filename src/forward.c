#include "midi.c"

struct state {
    uint16_t in_channels;
    unsigned char out_channel;
};

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        if((1 << ev->data.note.channel) & st->in_channels) {
            info("note on %u", ev->data.note.note);
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        if((1 << ev->data.note.channel) & st->in_channels) {
            info("note off %u", ev->data.note.note);
            SET_CHANNEL(ev->data.note, st->out_channel);
            send_event(ctx, ev);
        }
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        if((1 << ev->data.control.channel) & st->in_channels) {
            info("cc %u %u", ev->data.control.param, ev->data.control.value);
            SET_CHANNEL(ev->data.control, st->out_channel);
            send_event(ctx, ev);
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
        debug("ignored event of type=%d", ev->type);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 4) {
        dprintf(2, "usage: %s NAME OUT IN+\n", argv[0]);
        return 2;
    }

    struct ctx ctx;
    init(&ctx, "default", argv[1]);

    struct state st = { 0 };

    int res = sscanf(argv[2], "%hhu", &st.out_channel);
    if(res != 1) {
        failwith("OUT should be a MIDI channel");
    }
    for(int i = 3; i < argc; i++) {
        unsigned char c;
        res = sscanf(argv[i], "%hhu", &c);
        if(res != 1) {
            failwith("IN[%d] should be a MIDI channel", i - 3);
        }

        st.in_channels |= 1 << (c - 1);
    }

    char m[] = "................";
    for(size_t i = 0; i < 16; i++) {
        if(st.in_channels & (1 << i)) {
            m[i] = '1' + (i % 10);
        }
    }
    info("MIDI channel inputs: %s", m);
    info("MIDI channel output: %hhu", st.out_channel);

    loop(&ctx, go, &st);

    deinit(&ctx);
    return 0;
}
