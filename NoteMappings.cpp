#include "NoteMappings.h"

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

unsigned getFrequencyN(unsigned frequency_cents,
                       const unsigned note_mappings[128]) {
  unsigned note = frequency_cents / 100;
  unsigned cents = frequency_cents % 100;

  return note_mappings[note] -
         getFrequencyNForCents(note, cents, note_mappings);
}
