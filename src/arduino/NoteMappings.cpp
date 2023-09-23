#include "NoteMappings.h"

// PSG

/** @returns The number to subtract from the note's N to get note + cents */
unsigned getFrequencyNForCents(unsigned note, unsigned cents,
                               const unsigned note_mappings[128]) {
  if (note == 127) {
    return note_mappings[127];
  }
  // the difference in units of N between note and its next highest note
  unsigned n_diff = note_mappings[note] - note_mappings[note + 1];

  return (n_diff * cents) / 100;
}

unsigned getFrequencyN(unsigned pitch_cents,
                       const unsigned note_mappings[128]) {
  unsigned note = pitch_cents / 100;
  unsigned cents = pitch_cents % 100;

  return note_mappings[note] -
         getFrequencyNForCents(note, cents, note_mappings);
}

// FM
unsigned getBlock(unsigned pitch_cents) {
  unsigned note = pitch_cents / 100;
  if (note < 24) {
    return note / 12; // lowest MIDI octave handled the same as second-lowest
  }
  return note / 12 - 2;
}

/** @returns The number to add to the note's F-Number to get note + cents.
 * @param block_note the note within a "block" (octave), where C=0, B=11
 */
unsigned getFNumberForCents(unsigned block_note, unsigned cents,
                            const unsigned note_mappings[13]) {

  // the difference in units of N between note and its next highest note
  unsigned n_diff = note_mappings[block_note + 1] - note_mappings[block_note];

  return (n_diff * cents) / 100;
}

unsigned getFNumber(unsigned pitch_cents, const unsigned note_mappings[13]) {
  unsigned block_note = (pitch_cents / 100) % 12;
  unsigned cents = pitch_cents % 100;

  return note_mappings[block_note] +
         getFNumberForCents(block_note, cents, note_mappings);
}
