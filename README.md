# Music and MIDI processing tools

These are some tools I use in [my synthesizer setup](https://soundcloud.com/rootmos)
to make everything connect and correctly talk to each other.
I deploy the MIDI tools on a [Raspberry Pi](https://www.raspberrypi.org/) using
the [Single Purpose Linux distribution](https://github.com/rootmos/spl)
to get a [low-latency kernel](http://www.tedfelix.com/linux/linux-midi.html#installing-a-low-latency-kernel).

For a more feature rich (but less bare-metal) set of tools I recommend: [mididings](http://das.nasophon.de/mididings/).

## MIDI processing tools
* [midi.c](src/midi.c) - minimal [ALSA MIDI sequencer](https://www.alsa-project.org/alsa-doc/alsa-lib/seq.html) event loop
* [forward](src/forward.c) - forward only clock and filter out/merge selected channels
  - some synthesizers I use aren't too happy being bombarded with irrelevant
    MIDI-messages, so I use this to make them keep up
* [aclient](src/aclient.c) - lookup an ALSA sequencer client based on USB-port
  - ALSA sequencer client id:s are arbitrary, USB ports are not
  - I use this to automatically set up the routing [on boot](root/etc/init.d/rc.local)
* [menu](src/menu.c) - a MIDI-based menu to start/stop and control tempo
  - hold A0 + press B0: start clock
  - hold A0 + press C1: stop clock
  - hold A0 + press D1: set tempo based on value of next note
  - hold A0 + press C#1: decrease tempo
  - hold A0 + press D#1: increase tempo
  - implemented by emitting clock ticks, but a hack away from controlling
    external sequencers
* [microGranny](src/microGranny.c) - personal hack to play a
  [microGranny](https://bastl-instruments.com/instruments/microgranny/)
  using the drum sequencer of a [BeatStep Pro](https://www.arturia.com/beatstep-pro)

## Music tools
* [piano-randomizer](scripts/piano-randomizer) - piano practice tool
  - randomly select a scale, base note and starting finger
  - randomly select chord progressions
  - overlay display using [conky](https://github.com/brndnmtthws/conky)
* [microGranny](scripts/microGranny) - tiny command line interface for
  converting and uploading samples to the [microGranny](https://bastl-instruments.com/instruments/microgranny/)
