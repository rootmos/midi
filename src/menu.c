#include "midi.c"

struct state {
    unsigned char menu_note;
    unsigned char start_note;
    unsigned char stop_note;
    unsigned char tempo_mode_note;
    unsigned char tempo_inc_note;
    unsigned char tempo_dec_note;

    int active;
    unsigned char channel;

    int tempo_mode;
    float tempo;
    float tempo_step;
};

// grand: A0 -> C8
#define MIN_NOTE 21
#define MAX_NOTE 108

int go(struct ctx* ctx, snd_seq_event_t* ev, void* opaque)
{
    struct state* st = opaque;

    switch(ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        debug("note on: %hhu", ev->data.note.note);
        if(st->tempo_mode && ev->data.note.channel == st->channel) {
            st->tempo = (((float)300 - 30)/(MAX_NOTE - MIN_NOTE)) * (ev->data.note.note - MIN_NOTE) + 30;
            st->tempo_mode = 0;
            info("tempo: %f BPM", st->tempo);
            if(is_clock_running(ctx)) start_clock(ctx, st->tempo);
            break;
        }
        if(ev->data.note.note == st->menu_note) {
            st->active = 1;
            st->channel = ev->data.note.channel;
            debug("menu activated from channel %hhu", st->channel);
        } else if(st->active && ev->data.note.channel == st->channel) {
            if (ev->data.note.note == st->start_note) {
                info("start clock");
                start_clock(ctx, st->tempo);
            } else if(ev->data.note.note == st->stop_note) {
                info("stop clock");
                stop_clock(ctx);
            } else if(ev->data.note.note == st->tempo_mode_note) {
                info("next note sets tempo");
                st->tempo_mode = 1;
            } else if (ev->data.note.note == st->tempo_dec_note) {
                st->tempo = MAX(30, st->tempo - st->tempo_step);
                info("tempo: %f BPM", st->tempo);
                if(is_clock_running(ctx)) start_clock(ctx, st->tempo);
            } else if (ev->data.note.note == st->tempo_inc_note) {
                st->tempo = MIN(300, st->tempo + st->tempo_step);
                info("tempo: %f BPM", st->tempo);
                if(is_clock_running(ctx)) start_clock(ctx, st->tempo);
            }
        }
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        debug("note off: %hhu", ev->data.note.note);
        if(ev->data.note.note == st->menu_note) {
            st->active = 0;
            debug("menu deactivated");
        }
        break;
    case SND_SEQ_EVENT_KEYPRESS:
    case SND_SEQ_EVENT_CONTROLLER:
    case SND_SEQ_EVENT_START:
    case SND_SEQ_EVENT_STOP:
    case SND_SEQ_EVENT_RESET:
    case SND_SEQ_EVENT_CONTINUE:
    case SND_SEQ_EVENT_CLOCK:
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        break;
    default:
        warning("ignored event of type=%d", ev->type);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 2) {
        dprintf(2, "usage: %s NAME\n", argv[0]);
        return 2;
    }

    struct state st = {
        .active = 0,
        .channel = -1,
        .tempo_mode = 0,
        .tempo = 120,
        .tempo_step = 1,
        .menu_note = MIN_NOTE,
        .start_note = MIN_NOTE + 2,
        .stop_note = MIN_NOTE + 3,
        .tempo_dec_note = MIN_NOTE + 4,
        .tempo_mode_note = MIN_NOTE + 5,
        .tempo_inc_note = MIN_NOTE + 6,
    };

    struct ctx ctx;
    init(&ctx, "default", argv[1]);

    loop(&ctx, go, &st);

    deinit(&ctx);

    return 0;
}
