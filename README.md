# Sound Chip Synthesizer

Flexible Arduino-based engine for MIDI-controlled, sound chip-powered synthesizers.

## Scope

This project currently supports the following sound generator chips:

- TI SN76489
- Yamaha YM2203 (OPN)

with intention to extend to support other PSG and 4-operator FM chips in the future:

- GI AY-3-8910
- Yamaha YM2149
- Yamaha YM2151 (OPM) and YM2164 (OPP)
- Yamaha YM2608 (OPNA)
- Yamaha YM2612 (OPN2) and YM3438 (OPN2C)

This project currently targets Adafruit's [ItsyBitsy M0 Express](https://learn.adafruit.com/introducing-itsy-bitsy-m0), with plans to support [ItsyBitsy M4](https://learn.adafruit.com/introducing-adafruit-itsybitsy-m4/overview) as well.

## Synthesizer Engine

In addition to MIDI-based control of the built-in parameters of the supported PSG and FM chips, this engine also implements extended synthesizer voice functionality in software, including:

- flexible amplitude and pitch envelopes for PSG voices
- pitch envelope for FM voices
- vibrato for PSG and FM voices with highly flexible LFOs
- delay / echo / unison effect for PSG and FM voices

The engine also provides some useful MIDI control features:

- monophonic and polyphonic voice modes
- velocity sensitivity
- pitch bend
- 8-slot modulation matrix for FM voices
- (WIP) rhythm mode for FM voices (PSG rhythm sets planned)

Patch storage via the ItsyBitsy M0's built-in SPI flash is accessible by sysex dump.
