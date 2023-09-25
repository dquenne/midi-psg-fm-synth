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
    // block 0 corresponds to C1-B1 _and_ the two octaves below. The two octaves
    // below use lower F-numbers instead of the default F-numbers because block
    // 0 is the lowest block.
    return 0;
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

unsigned getFNumber(unsigned pitch_cents, const unsigned note_mappings[37]) {
  unsigned note = (pitch_cents / 100);
  unsigned block_note = note % 12;
  unsigned cents = pitch_cents % 100;

  // by default, use the top octave of F-numbers, which is the highest
  // resolution supported by 11-bit numbers.
  unsigned note_mappings_offset = 24;

  // only use F-numbers below 618 if the pitch is below C1 (F-number 618 in
  // block 0), because block 0 is the lowest block.
  if (note < 12) {
    note_mappings_offset = 0;
  } else if (note < 24) {
    note_mappings_offset = 12;
  }

  return note_mappings[block_note + note_mappings_offset] +
         getFNumberForCents(block_note + note_mappings_offset, cents,
                            note_mappings);
}
