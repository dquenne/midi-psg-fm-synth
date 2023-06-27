#include "Patch.h"

static const PsgPatch PRESET_PATCH_0 = {
    {true, 100, 16, 3},
    {{{5, 90}, {7, 90}, {9, 25}, {5, 200}, {3, 100}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 30, triangle, 0, 0},
    {72, 12},
    0,
};

static const PsgPatch PRESET_PATCH_1 = {
    {false, 0, 0, 0},
    {{{10, 80}, {12, 60}, {12, 60}, {3, 400}, {0, 0}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {10, 54, triangle, 500, 100},
    {72, 8},
    0,
};

static const PsgPatch PRESET_PATCH_2 = {
    {false},
    {{{13, 80}, {9, 50}, {6, 25}, {3, 400}, {1, 400}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 30, triangle, 500, 100},
    {72, 8},
    0,
};

static const PsgPatch PRESET_PATCH_3 = {
    {true, 140, 10, 3},
    {{{12, 60}, {10, 50}, {9, 25}, {3, 200}, {0, 0}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {16, 30, triangle, 600, 100},
    {72, 8},
    0,
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
  frequency LFO
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
      {28, 17, 6, 20, 2, 10, 0, 2, 0, false},
      {28, 15, 5, 20, 2, 1, 0, 1, 0, false}}},
    {127, {0, 31, 0, 31}},
    {16, 30, triangle, 600, 100},
    {true, 200, 6, 7},
};

static const FmPatch FM_PRESET_PATCH_1 = {
    {FM_PANNING_CENTER,
     7,
     3,
     0,
     0,
     {{31, 14, 0, 7, 3, 32, 0, 10, 3, false},
      {31, 19, 0, 5, 5, 12, 0, 0, 7, false},
      {31, 10, 0, 8, 5, 42, 0, 2, 0, false},
      {31, 31, 0, 8, 0, 0, 0, 0, 0, false}}},
    {127, {0, 0, 0, 31}},
    {0, 0, triangle, 0, 0},
    {false},
};

static const FmPatch FM_PRESET_PATCH_2 = {
    {FM_PANNING_CENTER,
     6,
     1,
     0,
     0,
     {{31, 13, 0, 0, 2, 10, 0, 1, 7, false},
      {31, 5, 2, 0, 8, 15, 0, 2, 1, false},
      {30, 10, 2, 0, 4, 27, 0, 7, 3, false},
      {30, 0, 0, 8, 4, 0, 0, 2, 3, false}}},
    {127, {0, 0, 0, 31}},
    {0, 0, triangle, 0, 0},
    {false},
};

// celeste piano from Desire (PC-98)
static const FmPatch FM_PRESET_PATCH_3 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {31, 25, 10, 0, 3, 23, 0, 14, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
         {31, 15, 6, 7, 2, 0, 0, 2, 0, false},
         {0, 0, 0, 15, 0, 127, 0, 0, 0, false},
     }},
    {127, {0, 0, 31, 31}},
    {18, 54, triangle, 1000, 0},
    {true, 300, 0, 2},
};

// organ from Ys I - Feena (PC-88)
static const FmPatch FM_PRESET_PATCH_4 = {
    {FM_PANNING_CENTER,
     0,
     4,
     0,
     0,
     {
         {18, 0, 0, 6, 0, 35, 0, 2, 6, false},
         {18, 0, 0, 6, 0, 35, 0, 2, 1, false},
         {18, 0, 0, 6, 0, 0, 0, 2, 5, false},
         {18, 0, 0, 6, 0, 0, 0, 2, 0, false},
     }},
    {127, {0, 0, 31, 31}},
    {0, 0, triangle, 0, 0},
    {false},
};

// bass from Ys I - Feena (PC-88)
static const FmPatch FM_PRESET_PATCH_5 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 2, 2, 0, 0, 33, 2, 2, 6, false},
         {31, 2, 2, 0, 0, 13, 1, 1, 2, false},
         {22, 2, 3, 4, 2, 0, 1, 2, 5, false},
         {22, 2, 3, 4, 2, 0, 1, 2, 1, false},
     }},
    {127, {0, 0, 31, 31}},
    {0, 0, triangle, 0, 0},
    {false},
};

// piano from Ys I - Feena (PC-88)
static const FmPatch FM_PRESET_PATCH_6 = {
    {FM_PANNING_CENTER,
     7,
     4,
     0,
     0,
     {
         {31, 0, 8, 0, 4, 32, 3, 1, 2, false},
         {22, 0, 8, 0, 4, 28, 3, 3, 5, false},
         {31, 7, 7, 7, 3, 0, 2, 1, 4, false},
         {22, 8, 6, 7, 3, 0, 1, 1, 1, false},
     }},
    {127, {0, 0, 31, 31}},
    {0, 0, triangle, 0, 0},
    {false},
};

// violin from Ys I - Feena (PC-88)
static const FmPatch FM_PRESET_PATCH_7 = {
    {FM_PANNING_CENTER,
     7,
     2,
     0,
     0,
     {
         {25, 10, 0, 5, 1, 29, 1, 1, 4, false},
         {28, 13, 0, 6, 2, 48, 1, 1, 3, false},
         {25, 11, 0, 8, 5, 15, 1, 5, 4, false},
         {14, 4, 0, 6, 0, 0, 1, 1, 4, false},
     }},
    {127, {0, 0, 0, 31}},
    {16, 30, triangle, 600, 100},
    {true, 100, 16, 10},
};

const FmPatch *FM_PRESETS[] = {
    &FM_PRESET_PATCH_0, &FM_PRESET_PATCH_1, &FM_PRESET_PATCH_2,
    &FM_PRESET_PATCH_3, &FM_PRESET_PATCH_4, &FM_PRESET_PATCH_5,
    &FM_PRESET_PATCH_6, &FM_PRESET_PATCH_7, &FM_PRESET_PATCH_0,
    &FM_PRESET_PATCH_1, &FM_PRESET_PATCH_2, &FM_PRESET_PATCH_3,
    &FM_PRESET_PATCH_0, &FM_PRESET_PATCH_1, &FM_PRESET_PATCH_2,
    &FM_PRESET_PATCH_3,
};
