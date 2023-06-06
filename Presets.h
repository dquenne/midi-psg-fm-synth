#include "Patch.h"

static const PsgPatch PRESET_PATCH_0 = {
    {true, 100, 16, 3},
    {{{7, 60}, {9, 25}, {9, 25}, {3, 200}, {0, 0}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 170, triangle, 1600, 100},
    {72, 12},
    0,
};

static const PsgPatch PRESET_PATCH_1 = {
    {false, 0, 0, 0},
    {{{10, 80}, {12, 60}, {12, 60}, {3, 400}, {0, 0}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 160, triangle, 500, 100},
    {72, 8},
    0,
};

static const PsgPatch PRESET_PATCH_2 = {
    {false},
    {{{13, 80}, {9, 50}, {6, 25}, {3, 400}, {1, 400}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 170, triangle, 500, 100},
    {72, 8},
    0,
};

static const PsgPatch PRESET_PATCH_3 = {
    {true, 140, 6, 5},
    {{{12, 60}, {8, 50}, {6, 25}, {3, 200}, {0, 0}}, true, 2, 2, 3, 4},
    {},
    {0, 0, triangle, 0, 0},
    {0, 170, triangle, 600, 100},
    {72, 8},
    0,
};

const PsgPatch *PRESETS[] = {
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
    &PRESET_PATCH_0, &PRESET_PATCH_1, &PRESET_PATCH_2, &PRESET_PATCH_3,
};

static const FmPatch FM_PRESET_PATCH_0 = {
    {false},
    {
        FM_PANNING_CENTER,
        2,
        5,
        0,
        0,
        {
            {28, 15, 8, 20, 2, 30, 0, 5, 0, false},
            {28, 15, 6, 20, 2, 40, 0, 3, 0, false},
            {28, 15, 6, 20, 2, 30, 0, 2, 0, false},
            {28, 15, 5, 20, 2, 10, 0, 1, 0, false},
        },
    },
    {64, {1, 0, 0, 0}},
};
