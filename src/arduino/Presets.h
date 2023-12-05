#include "Patch.h"

/*
struct PsgPatch {
  PatchDelayConfig delay_config;
  AdsrEnvelopeShape amplitude_envelope;
  {scaling, AdsrEnvelopeShape} pitch_envelope;

  Lfo amplitude_lfo;
  Lfo pitch_lfo;
  PsgPatchVelocityConfig velocity_config;
  signed detune_cents;
};
*/

// fade-in for Ys - Feena (1987)
static const PsgPatch PRESET_PATCH_0 = {
    {true, 25, 16, 3},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_OFF},
    {120, 120, 127, 127},
    {-1, {0, 70, 0, 0}},
    {0, 0, triangle, 0},
    {0, 30, triangle, 0},
    {105, 20},
    0,
};

static const PsgPatch PRESET_PATCH_1 = {
    {false, 0, 0, 0},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_SOFT},
    {0, 70, 100, 60},
    {0},
    {0, 0, triangle, 0},
    {10, 54, triangle, 31},
    {105, 20},
    0,
};

static const PsgPatch PRESET_PATCH_2 = {
    {false},
    {2, NOTE_PRIORITY_MODE_HIGHEST, RETRIGGER_MODE_SOFT},
    {5, 0, 120, 40},
    {0, {0, 0, 0, 0}},
    {0, 0, triangle, 0},
    {0, 30, triangle, 31},
    {105, 20},
    0,
};

// percussive lead for Ys - Feena (1987)
static const PsgPatch PRESET_PATCH_3 = {
    {true, 35, 10, 3},   {2, NOTE_PRIORITY_MODE_HIGHEST, RETRIGGER_MODE_HARD},
    {0, 75, 95, 70},     {0, {0, 38, 0, 00}},
    {0, 0, triangle, 0}, {16, 30, triangle, 38},
    {105, 20},           0,
};

const PsgPatch *PSG_PRESETS[] = {
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
};

/*
Parameter Order:
  core parameters
    operators
      attack_rate
      decay_rate
      sustain_rate
      release_rate
      sustain_level
      total_level
      key_scale
      multiple
      detune
  velocity
  pitch LFO
  delay
*/
static const FmPatch FM_PRESET_PATCH_0 = {
    {FM_PANNING_CENTER,
     2,
     5,
     0,
     0,
     {{28, 15, 8, 20, 2, 40, 0, 5, 0, false},
      {28, 15, 6, 20, 2, 127, 0, 3, 0, false},
      {28, 17, 6, 20, 2, 0, 0, 2, 0, false},
      {28, 15, 5, 20, 2, 0, 0, 1, 0, false}}},
    {64, {0, 15, 0, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {16, 30, triangle, 38},
    {true, 50, 6, 7},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_SOFT},
};

// bass from Desire (1994)
static const FmPatch FM_PRESET_PATCH_1 = {
    {FM_PANNING_CENTER,
     7,
     3,
     0,
     0,
     {{31, 14, 0, 7, 3, 32, 0, 10, 3, false},
      {31, 10, 0, 8, 5, 42, 0, 2, 0, false},
      {31, 19, 0, 5, 5, 12, 0, 0, 7, false},
      {31, 31, 0, 8, 0, 0, 0, 0, 0, false}}},
    {64, {0, 3, 2, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_OFF},
};

// burst sound from Desire (1994)
static const FmPatch FM_PRESET_PATCH_2 = {
    {FM_PANNING_CENTER,
     6,
     1,
     0,
     0,
     {{31, 13, 0, 0, 2, 10, 0, 1, 7, false},
      {30, 10, 2, 0, 4, 27, 0, 7, 3, false},
      {31, 5, 2, 0, 8, 15, 0, 2, 1, false},
      {30, 0, 0, 8, 4, 0, 0, 2, 3, false}}},
    {64, {0, 0, 0, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// celeste piano from Desire (1994)
static const FmPatch FM_PRESET_PATCH_3 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {31, 25, 10, 0, 3, 23, 0, 14, 0, false},
         {31, 15, 6, 7, 2, 0, 0, 2, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
     }},
    {64, {3, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {18, 54, triangle, 63},
    {true, 75, 0, 2},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// organ from Ys I - Feena (1987)
static const FmPatch FM_PRESET_PATCH_4 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {18, 0, 0, 6, 0, 35, 0, 2, 6, false},
         {18, 0, 0, 6, 0, 0, 0, 2, 5, false},
         {18, 0, 0, 6, 0, 35, 0, 2, 1, false},
         {18, 0, 0, 6, 0, 0, 0, 2, 0, false},
     }},
    {64, {5, 15, 5, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 25},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 25},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// bass from Ys I - Feena (1987)
static const FmPatch FM_PRESET_PATCH_5 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 2, 2, 0, 0, 33, 2, 2, 6, false},
         {22, 2, 3, 4, 2, 0, 1, 2, 5, false},
         {31, 2, 2, 0, 0, 13, 1, 1, 2, false},
         {22, 2, 3, 4, 2, 0, 1, 2, 1, false},
     }},
    {64, {0, 15, 0, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_OFF},
};

// piano from Ys I - Feena (1987)
static const FmPatch FM_PRESET_PATCH_6 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 0, 8, 0, 4, 32, 3, 1, 2, false},
         {31, 7, 7, 7, 3, 0, 2, 1, 4, false},
         {22, 0, 8, 0, 4, 28, 3, 3, 5, false},
         {22, 8, 6, 7, 3, 0, 1, 1, 1, false},
     }},
    {64, {1, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// violin from Ys I - Feena (1987)
static const FmPatch FM_PRESET_PATCH_7 = {
    {FM_PANNING_CENTER,
     7,
     2,
     0,
     0,
     {
         {25, 10, 0, 5, 1, 29, 1, 1, 4, false},
         {25, 11, 0, 8, 5, 15, 1, 5, 4, false},
         {28, 13, 0, 6, 2, 48, 1, 1, 3, false},
         {14, 4, 0, 6, 0, 0, 1, 1, 4, false},
     }},
    {64, {0, 3, 4, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0, {0, 45, 0, 45}},
    {16, 30, triangle, 38},
    {true, 25, 16, 1},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_OFF},
};

// fade-in bass from YU-NO (1996)
static const FmPatch FM_PRESET_PATCH_8 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {31, 31, 0, 0, 1, 10, 1, 0, 0, false},
         {6, 5, 4, 10, 1, 0, 0, 1, 0, false},
         {31, 31, 0, 0, 1, 10, 1, 0, 6, false},
         {6, 5, 4, 10, 1, 0, 0, 1, 6, false},
     }},
    {64, {3, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// celeste piano from Desire (1994)
static const FmPatch FM_PRESET_PATCH_9 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {31, 25, 10, 0, 3, 23, 0, 14, 0, false},
         {31, 15, 6, 7, 2, 0, 0, 2, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
     }},
    {64, {3, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 60},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64}},
    {0},
    {18, 54, triangle, 63},
    {false},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LOWEST, RETRIGGER_MODE_HARD},
};

// from YU-NO (1996), "Elly"
static const FmPatch FM_PRESET_PATCH_10 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 0, 0, 0, 0, 35, 0, 2, 7, false},
         {31, 0, 2, 12, 0, 0, 0, 2, 7, false},
         {31, 0, 0, 0, 0, 44, 0, 2, 7, false},
         {31, 15, 2, 12, 2, 0, 0, 4, 3, false},
     }},
    {64, {3, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 23},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 82},
     {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 32},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 82}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_HIGHEST, RETRIGGER_MODE_HARD},
};

// bass from YU-NO, girl in the lake (1996)
static const FmPatch FM_PRESET_PATCH_11 = {
    {FM_PANNING_CENTER,
     5,
     4,
     0,
     0,
     {
         {31, 0, 0, 0, 0, 10, 0, 2, 7, false},
         {31, 0, 2, 12, 0, 1, 0, 2, 7, false},
         {31, 6, 0, 0, 4, 25, 0, 2, 7, false},
         {31, 15, 2, 12, 2, 0, 0, 4, 3, false},
     }},
    {64, {3, 15, 3, 15}},
    {
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 22},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
        {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
    },
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// from YU-NO (1996), "Impatience"
static const FmPatch FM_PRESET_PATCH_12 = {
    {FM_PANNING_CENTER,
     4,
     0,
     0,
     0,
     {
         {31, 7, 7, 8, 2, 38, 0, 6, 3, false},
         {31, 10, 6, 0, 3, 48, 3, 5, 3, false},
         {31, 9, 6, 8, 1, 30, 2, 0, 3, false},
         {31, 15, 4, 9, 1, 0, 0, 1, 3, false},
     }},
    {64, {3, 15, 3, 15}},
    {
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 32},
        {FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 24},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
    },
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_OFF},
};

// from YU-NO (1996), "Impatience"
static const FmPatch FM_PRESET_PATCH_13 = {
    {FM_PANNING_CENTER,
     5,
     4,
     0,
     0,
     {
         {31, 0, 0, 0, 0, 38, 0, 1, 7, false},
         {31, 0, 2, 12, 0, 48, 0, 1, 7, false},
         {31, 6, 0, 0, 4, 32, 0, 1, 7, false},
         {31, 15, 2, 12, 2, 0, 0, 2, 3, false},
     }},
    {64, {3, 15, 3, 15}},
    {
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 12},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 1},
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 20},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
    },
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// from YU-NO (1996), "Quickening"
static const FmPatch FM_PRESET_PATCH_14 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 0, 0, 0, 0, 23, 0, 2, 7, false},
         {31, 0, 2, 12, 0, 0, 0, 2, 7, false},
         {31, 6, 0, 0, 4, 2, 0, 2, 7, false},
         {31, 15, 2, 12, 2, 4, 0, 4, 3, false},
     }},
    {64, {3, 15, 3, 15}},
    {{FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
     {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 18},
     {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 68}},
    {0},
    {0, 0, triangle, 0},
    {false},
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

// piano from YU-NO (1996), "Arabesque"
static const FmPatch FM_PRESET_PATCH_15 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 0, 0, 0, 0, 39, 0, 2, 3, false},
         {25, 11, 6, 8, 3, 0, 0, 2, 7, false},
         {31, 0, 0, 0, 0, 40, 0, 6, 7, false},
         {31, 11, 6, 8, 3, 0, 2, 2, 7, false},
     }},
    {64, {3, 15, 3, 15}},
    {
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 32},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
        {FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL, 28},
        {FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY, 64},
    },
    {0},
    {3, 54, triangle, 63},
    {false},
    {MAX_POLYPHONY_UNLIMITED, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
};

const FmPatch *FM_PRESETS[] = {
    &FM_PRESET_PATCH_0,  &FM_PRESET_PATCH_1,  &FM_PRESET_PATCH_2,
    &FM_PRESET_PATCH_3,  &FM_PRESET_PATCH_4,  &FM_PRESET_PATCH_5,
    &FM_PRESET_PATCH_6,  &FM_PRESET_PATCH_7,  &FM_PRESET_PATCH_8,
    &FM_PRESET_PATCH_9,  &FM_PRESET_PATCH_10, &FM_PRESET_PATCH_11,
    &FM_PRESET_PATCH_12, &FM_PRESET_PATCH_13, &FM_PRESET_PATCH_14,
    &FM_PRESET_PATCH_15,
};
