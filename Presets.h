#include "Patch.h"

static const Patch PRESET_PATCH_0 = {
    {{{11, 60}, {8, 25}, {3, 200}}, true, 1, 1, 2, 2},
    {},
    {false, 0, 0, 0, 0, triangle},
    {true, 1600, 100, 16, 170, triangle},
    0.4f,
};

static const Patch PRESET_PATCH_1 = {
    {
        {{14, 40}, {13, 40}, {12, 40}, {9, 40}, {6, 100}, {3, 100}, {0, 40}},
        true,
        3,
        3,
        0,
        6,
    },
    {},
    {false, 0, 0, 0, 0, triangle},
    {true, 600, 100, 16, 170, triangle},
    0.5f,
};

static const Patch PRESET_PATCH_2 = {
    {{{12, 80}, {14, 70}, {6, 800}, {3, 1000}}, true, 1, 1, 0, 3},
    {},
    {true, 600, 100, 16, 170, triangle},
    {true, 600, 100, 16, 170, triangle},
    0.5f,
};

static const Patch PRESET_PATCH_3 = {
    {
        {{14, 40}, {13, 40}, {12, 40}, {9, 40}, {6, 100}, {3, 100}, {0, 40}},
        true,
        3,
        3,
        0,
        6,
    },
    {},
    {false, 0, 0, 0, 0, triangle},
    {true, 600, 100, 16, 170, triangle},
    0.5f,
};

extern const Patch *PRESETS[] = {
    &PRESET_PATCH_0,
    &PRESET_PATCH_1,
    &PRESET_PATCH_2,
    &PRESET_PATCH_3,
};
