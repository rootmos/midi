#include <alsa/asoundlib.h>
#include <r.h>
#include <poll.h>
#include <math.h>
#include <unistd.h>

struct ctx {
    snd_seq_t* seq;

    int client;
    int input;
    int output;

    size_t in_count;

    struct timespec t;
    size_t clocks;
    uint64_t pulse_ns[256];
    size_t pulses;

    snd_seq_event_t queue[8];
    size_t queue_n, queue_m;
};

static void init(struct ctx* ctx,
                 const char* seq_name, const char* client_name)
{
    memset(ctx, 0, sizeof(*ctx));

    int res = snd_seq_open(&ctx->seq, seq_name,
                           SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    CHECK_ALSA(res, "snd_seq_open(%s)", seq_name);

    ctx->client = snd_seq_client_id(ctx->seq);
    CHECK_ALSA(ctx->client, "snd_seq_client_id");

    res = snd_seq_set_client_name(ctx->seq, client_name);
    CHECK_ALSA(res, "snd_seq_set_client_name(%s)", client_name);

    ctx->input = snd_seq_create_simple_port(
        ctx->seq, "input",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    CHECK_ALSA(ctx->input, "snd_seq_create_simple_port(input)");

    ctx->output = snd_seq_create_simple_port(
        ctx->seq, "output",
        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    CHECK_ALSA(ctx->output, "snd_seq_create_simple_port(output)");

    info("%d:%d -> %s -> %d:%d",
         ctx->client, ctx->input,
         client_name,
         ctx->client, ctx->output);
}

static void deinit(struct ctx* ctx)
{
    int res = snd_seq_close(ctx->seq);
    CHECK_ALSA(res, "snd_seq_close");
    ctx->seq = NULL;

    info("processed %zu incoming events", ctx->in_count);
    info("processed %zu outgoing events", ctx->queue_n);
}

static void observe_event(struct ctx* ctx, snd_seq_event_t* ev)
{
    if(ev->type == SND_SEQ_EVENT_CLOCK) {
        struct timespec t;
        int r = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
        CHECK(r, "clock_gettime(CLOCK_MONOTONIC_RAW)");
        if(ctx->clocks > 0) {
            ctx->pulse_ns[ctx->pulses++] = (t.tv_sec - ctx->t.tv_sec)*1000000000
                + (t.tv_nsec - ctx->t.tv_nsec);

            if(ctx->pulses == LENGTH(ctx->pulse_ns)) {
                if(fork() == 0) {
                    int res = nice(15);
                    CHECK(res, "nice(15)");

                    uint64_t sum = 0;
                    for(size_t i = 0; i < ctx->pulses; i++) {
                        sum += ctx->pulse_ns[i];
                    }

                    double avg_pulse = (double)sum/ctx->pulses;

                    double stddev = 0;
                    for(size_t i = 0; i < ctx->pulses; i++) {
                        double x = avg_pulse - ctx->pulse_ns[i];
                        stddev += x*x;
                    }
                    stddev = sqrt(stddev/(ctx->pulses - 1));

                    double tempo = (double)2500000000/avg_pulse;
                    info("%.2fBPM (stddev=%.2fms)", tempo, stddev/1000000);

                    exit(0);
                }

                ctx->pulses = 0;
            }
        }
        ctx->clocks += 1;
        memcpy(&ctx->t, &t, sizeof(t));
    }
}

typedef int (*event_callback_t)(struct ctx*, snd_seq_event_t*, void*);

static int loop(struct ctx* ctx, event_callback_t cb, void* opaque)
{
    while(1) {
        short events = POLLIN;

        if(ctx->queue_n < ctx->queue_m) {
            events |= POLLOUT;
        }

        int n = snd_seq_poll_descriptors_count(ctx->seq, events);
        CHECK_ALSA(n, "snd_seq_poll_descriptors_count");
        struct pollfd fds[n];

        n = snd_seq_poll_descriptors(ctx->seq, fds, LENGTH(fds), events);
        CHECK_ALSA(n, "snd_seq_poll_descriptors");

        int res = poll(fds, n, 10000); CHECK(res, "poll");
        if(res == 0) {
            info("waiting for events...");
        } else {
            unsigned short revents;
            res = snd_seq_poll_descriptors_revents(ctx->seq, fds, n, &revents);
            CHECK_ALSA(res, "snd_seq_poll_descriptors_revents");

            if(revents & POLLIN) {
                while(1) {
                    snd_seq_event_t* ev;
                    res = snd_seq_event_input(ctx->seq, &ev);
                    if(res == -EAGAIN) break;
                    CHECK_ALSA(res, "snd_seq_event_input");

                    observe_event(ctx, ev);
                    res = cb(ctx, ev, opaque);
                    if(res != 0) {
                        debug("event callback requested graceful shutdown: %d", res);
                        return res;
                    }

                    ctx->in_count += 1;
                }

                revents &= ~POLLIN;
            }

            if(revents & POLLOUT) {
                while(ctx->queue_n < ctx->queue_m) {
                    int res = snd_seq_event_output_buffer(
                        ctx->seq,
                        &ctx->queue[(ctx->queue_n++) % LENGTH(ctx->queue)]);
                    CHECK_ALSA(res, "snd_seq_event_output");
                }

                res = snd_seq_drain_output(ctx->seq);
                CHECK_ALSA(res, "snd_seq_drain_output");

                revents &= ~POLLOUT;
            }

            if(revents != 0) {
                failwith("unhandled event: %u", revents);
            }
        }
    }
}

void send_event(struct ctx* ctx, snd_seq_event_t* ev)
{
    snd_seq_ev_set_subs(ev);
    snd_seq_ev_set_direct(ev);
    snd_seq_ev_set_source(ev, ctx->output);

    memcpy(&ctx->queue[(ctx->queue_m++) % LENGTH(ctx->queue)], ev, sizeof(*ev));

    size_t l = ctx->queue_m - ctx->queue_n;
    if(l >= LENGTH(ctx->queue)) {
        warning("buffer overflow: %zu queued events in queue with size %zu",
                l, LENGTH(ctx->queue));
    } else if(l >= LENGTH(ctx->queue)/2) {
        info("many (%zu/%zu) queued events", l, LENGTH(ctx->queue));
    }
}

#define CHANNEL(ev) ((ev).channel + 1)
#define SET_CHANNEL(ev, c) do { (ev).channel = (c) - 1; } while(0)
