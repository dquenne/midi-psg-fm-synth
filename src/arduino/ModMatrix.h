#ifndef ModMatrix_h
#define ModMatrix_h

#include <Arduino.h>

enum ModSource : byte {
  MOD_SRC_NONE,

  MOD_SRC_VELOCITY,
  MOD_SRC_CHANNEL_AFTERTOUCH,
  MOD_SRC_POLY_AFTERTOUCH,
  MOD_SRC_PITCH_BEND,

  // 4-63 are reserved for future non-CC controller sources

  MOD_SRC_ENV_1 = 64,
  MOD_SRC_ENV_2,
  MOD_SRC_PITCH_LFO,
  MOD_SRC_LFO_1,
  MOD_SRC_LFO_2,

  // 68 - 127 are reserved for future internal sources

  MOD_SRC_CC1_MOD_WHEEL = 128,
  MOD_SRC_CC2_BREATH,
  MOD_SRC_CC3,
  MOD_SRC_CC4_FOOT_PEDAL,
  MOD_SRC_CC5,
  MOD_SRC_CC6,
  MOD_SRC_CC7_VOLUME,
  MOD_SRC_CC8,
  MOD_SRC_CC9,
  MOD_SRC_CC10_PAN,
  MOD_SRC_CC11_EXPRESSION,
  MOD_SRC_CC12,
  MOD_SRC_CC13,
  MOD_SRC_CC14,
  MOD_SRC_CC15,
  MOD_SRC_CC16_RIBBON,
};

#define MOD_DESTINATION_COUNT 96 // can extend up to 127 as needed

enum ModDestination : byte {
  MOD_DEST_NONE,

  MOD_DEST_PITCH,

  MOD_DEST_PAN, // For future use by non-YM2203 targets
  MOD_DEST_FEEDBACK,
  MOD_DEST_ALGORITHM,
  MOD_DEST_CARRIER_LEVEL,

  MOD_DEST_PITCH_LFO_AMPLITUDE,
  MOD_DEST_PITCH_LFO_RATE,
  MOD_DEST_PITCH_LFO_WAVE,

  MOD_DEST_LFO_1_AMPLITUDE,
  MOD_DEST_LFO_1_RATE,
  MOD_DEST_LFO_1_WAVE,

  MOD_DEST_LFO_2_AMPLITUDE,
  MOD_DEST_LFO_2_RATE,
  MOD_DEST_LFO_2_WAVE,

  // 15-63 are reserved for future non-operator destinations

  MOD_DEST_TL_OP0 = 64,
  MOD_DEST_TL_OP1,
  MOD_DEST_TL_OP2,
  MOD_DEST_TL_OP3,

  MOD_DEST_AR_OP0,
  MOD_DEST_AR_OP1,
  MOD_DEST_AR_OP2,
  MOD_DEST_AR_OP3,

  MOD_DEST_DR_OP0,
  MOD_DEST_DR_OP1,
  MOD_DEST_DR_OP2,
  MOD_DEST_DR_OP3,

  MOD_DEST_SR_OP0,
  MOD_DEST_SR_OP1,
  MOD_DEST_SR_OP2,
  MOD_DEST_SR_OP3,

  MOD_DEST_RR_OP0,
  MOD_DEST_RR_OP1,
  MOD_DEST_RR_OP2,
  MOD_DEST_RR_OP3,

  MOD_DEST_SL_OP0,
  MOD_DEST_SL_OP1,
  MOD_DEST_SL_OP2,
  MOD_DEST_SL_OP3,

  MOD_DEST_MUL_OP0,
  MOD_DEST_MUL_OP1,
  MOD_DEST_MUL_OP2,
  MOD_DEST_MUL_OP3,

  MOD_DEST_DT_OP0,
  MOD_DEST_DT_OP1,
  MOD_DEST_DT_OP2,
  MOD_DEST_DT_OP3,
};

struct ModMatrixEntry {
  ModDestination destination;
  ModSource source;
  /** 0 - 127 */
  byte amount;
  /** 0 - 127 */
  byte center;
};

#endif
