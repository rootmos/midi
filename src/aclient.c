#include <alsa/asoundlib.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>

#define LIT(x) (x),sizeof(x)

#define check_alsa_result(res) do { \
    if(res < 0) { \
        exit(10); \
    } \
} while(0)

#define check_libusb_result(res) do { \
    if(res < 0) { \
        exit(11); \
    } \
} while(0)

static int usb_dev_from_card_id(const char* card_id,
                                int* usb_bus, int* usb_dev)
{
    char buf[1024];
    snprintf(LIT(buf), "/proc/asound/%s/usbbus", card_id);

    int fd = open(buf, O_RDONLY);
    if(fd == -1) {
        if(errno == ENOENT) {
            return -1;
        } else {
            exit(1);
        }
    }

    read(fd, LIT(buf));

    int res = sscanf(buf, "%d/%d", usb_bus, usb_dev);
    if(res != 2) {
        exit(1);
    }

    close(fd);
    return 0;
}

unsigned int mode = -1;

static void select_and_print_port(snd_seq_t* seq, int client_id)
{
    snd_seq_port_info_t* pi;
    snd_seq_port_info_alloca(&pi);

    snd_seq_port_info_set_client(pi, client_id);
    snd_seq_port_info_set_port(pi, -1);
    for(int port_id = 0; snd_seq_query_next_port(seq, pi) >= 0; port_id++) {
        if((snd_seq_port_info_get_capability(pi) & mode) == mode) {
            printf("%d:%d\n", client_id, port_id);
        }
    }
}

static void usb_port_from_device(int usb_bus, int usb_dev, int* usb_port)
{
    libusb_context* ctx;
    int res = libusb_init(&ctx);
    check_libusb_result(res);

    libusb_device** l;
    ssize_t n = libusb_get_device_list(ctx, &l);
    check_libusb_result(n);

    for(ssize_t i = 0; i < n; i++) {
        if(libusb_get_bus_number(l[i]) == usb_bus
           && libusb_get_device_address(l[i]) == usb_dev) {
            *usb_port = libusb_get_port_number(l[i]);

            libusb_free_device_list(l, 0);
            libusb_exit(ctx);
            return;
        }
    }

    exit(1);
}

static void client_id_from_usb_port(snd_seq_t* seq, int usb_bus, int usb_port)
{
    snd_seq_client_info_t* ci;
    snd_seq_client_info_alloca(&ci);

    snd_seq_client_info_set_client(ci, -1);
    while(snd_seq_query_next_client(seq, ci) >= 0) {
        int client_id = snd_seq_client_info_get_client(ci);
        int card_id = snd_seq_client_info_get_card(ci);
        if(card_id < 0) {
            continue;
        }

        char buf[1024];
        snprintf(buf, sizeof(buf), "hw:CARD=%d", card_id);

        snd_ctl_t* c;
        int res = snd_ctl_open(&c, buf, 0);
        check_alsa_result(res);

        snd_ctl_card_info_t* i;
        snd_ctl_card_info_alloca(&i);

        res = snd_ctl_card_info(c, i);
        check_alsa_result(res);

        int b, d;
        if(usb_dev_from_card_id(snd_ctl_card_info_get_id(i), &b, &d) == 0) {
            if(usb_bus == b) {
                int p;
                usb_port_from_device(usb_bus, d, &p);

                if(usb_port == p) {
                    select_and_print_port(seq, client_id);
                }
            }
        } else {
            // not a usb card
        }

        res = snd_ctl_close(c);
        check_alsa_result(res);
    }
}

int main(int argc, char* argv[])
{
    int usb_bus = -1, usb_port = -1;
    int res;
    while((res = getopt(argc, argv, "iou:")) != -1) {
        switch(res) {
        case 'i':
            if(mode == -1) {
                mode = SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;
            } else {
                mode |= SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;
            }
            break;
        case 'o':
            if(mode == -1) {
                mode = SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE;
            } else {
                mode |= SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE;
            }
            break;
        case 'u':
            res = sscanf(optarg, "%d.%d", &usb_bus, &usb_port);
            if(res != 2) {
                dprintf(2, "expected format: <BUS>.<PORT>");
                exit(1);
            }
            break;
        default:
            dprintf(2, "unsupported option");
            exit(1);
        }
    }

    snd_seq_t* seq;
    res = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
    check_alsa_result(res);

    if(usb_bus > 0 && usb_port > 0) {
        client_id_from_usb_port(seq, usb_bus, usb_port);
    } else {
        dprintf(2, "specify what to you want to find\n");
        exit(1);
    }

    res = snd_seq_close(seq);
    check_alsa_result(res);

    return 0;
}
