
#pragma once

#include "Externals/braids/dsp.h"
// add here so as not to include whole of "Externals/braids/resources.h"
const int16_t wav_sine[] = {
    -32512, -32502, -32473, -32423, -32356, -32265, -32160, -32031, -31885,
    -31719, -31533, -31331, -31106, -30864, -30605, -30324, -30028, -29712,
    -29379, -29026, -28658, -28272, -27868, -27449, -27011, -26558, -26089,
    -25604, -25103, -24588, -24056, -23512, -22953, -22378, -21793, -21191,
    -20579, -19954, -19316, -18667, -18006, -17334, -16654, -15960, -15259,
    -14548, -13828, -13100, -12363, -11620, -10868, -10112, -9347,  -8578,
    -7805,  -7023,  -6241,  -5453,  -4662,  -3868,  -3073,  -2274,  -1474,
    -674,   126,    929,    1729,   2527,   3326,   4123,   4916,   5707,
    6495,   7278,   8057,   8833,   9601,   10366,  11122,  11874,  12618,
    13353,  14082,  14802,  15512,  16216,  16906,  17589,  18260,  18922,
    19569,  20207,  20834,  21446,  22045,  22634,  23206,  23765,  24311,
    24842,  25357,  25858,  26343,  26812,  27266,  27701,  28123,  28526,
    28912,  29281,  29632,  29966,  30281,  30579,  30859,  31118,  31361,
    31583,  31788,  31973,  32139,  32286,  32412,  32521,  32608,  32679,
    32725,  32757,  32766,  32757,  32725,  32679,  32608,  32521,  32412,
    32286,  32139,  31973,  31788,  31583,  31361,  31118,  30859,  30579,
    30281,  29966,  29632,  29281,  28912,  28526,  28123,  27701,  27266,
    26812,  26343,  25858,  25357,  24842,  24311,  23765,  23206,  22634,
    22045,  21446,  20834,  20207,  19569,  18922,  18260,  17589,  16906,
    16216,  15512,  14802,  14082,  13353,  12618,  11874,  11122,  10366,
    9601,   8833,   8057,   7278,   6495,   5707,   4916,   4123,   3326,
    2527,   1729,   929,    126,    -674,   -1474,  -2274,  -3073,  -3868,
    -4662,  -5453,  -6241,  -7023,  -7805,  -8578,  -9347,  -10112, -10868,
    -11620, -12363, -13100, -13828, -14548, -15259, -15960, -16654, -17334,
    -18006, -18667, -19316, -19954, -20579, -21191, -21793, -22378, -22953,
    -23512, -24056, -24588, -25103, -25604, -26089, -26558, -27011, -27449,
    -27868, -28272, -28658, -29026, -29379, -29712, -30028, -30324, -30605,
    -30864, -31106, -31331, -31533, -31719, -31885, -32031, -32160, -32265,
    -32356, -32423, -32473, -32502, -32512};

#include "q15.h"
#include <cstring>

using namespace braids;
#define VERB_LENGTH 8203
#define VERB_AP1 113
#define VERB_AP2 162
#define VERB_AP3 241
#define VERB_AP4 373

#define VERB_AP5 615
#define VERB_AP6 773
#define VERB_D1 915

#define VERB_AP7 513
#define VERB_AP8 849
#define VERB_D2 1515

typedef struct AllPassFilter {
  int16_t *buf;
  int count = 0;
  int length = 0;
  q15_t c;
  uint32_t phase;
  uint16_t phaseOffset;
} AllPassFilter;

class Reverb2 {
public:
  Reverb2() {
    // --- EXPERIMENT WITH THESE VALUES ---

    // 1. All-pass coefficient. Controls diffusion. Higher values =
    // smoother/more diffuse. Range: ~0.6f to ~0.85f. Default was 0.7f.
    const float all_pass_coeff = 0.7f;

    // 2. Damping. Controls high-frequency decay. Lower values = darker reverb.
    // Range: 0.0f (no highs) to 0.99f (bright). Default was 0.9f.
    dampAmount = f32_to_q15(0.2f);

    // 3. Feedback. Controls the decay time / size of the room.
    // Range: 0.0f (no decay) to <1.0f. Default was 0.87f.
    feedbackAmount = f32_to_q15(0.85f);

    // --- END OF EXPERIMENTAL VALUES ---

    for (int i = 0; i < 10; i++) {
      allpass[i].count = 0;
      allpass[i].c = f32_to_q15(all_pass_coeff);
      allpass[i].phaseOffset = phaseOffset[i];
      allpass[i].phase = 0;
    }
    int bufCount = 0;
    allpass[0].buf = buf + bufCount;
    bufCount += VERB_AP1;
    allpass[0].length = VERB_AP1;
    allpass[1].buf = buf + bufCount;
    bufCount += VERB_AP2;
    allpass[1].length = VERB_AP2;
    allpass[2].buf = buf + bufCount;
    bufCount += VERB_AP3;
    allpass[2].length = VERB_AP3;
    allpass[3].buf = buf + bufCount;
    bufCount += VERB_AP4;
    allpass[3].length = VERB_AP4;

    allpass[4].buf = buf + bufCount;
    bufCount += VERB_AP5;
    allpass[4].length = VERB_AP5;
    allpass[5].buf = buf + bufCount;
    bufCount += VERB_AP6;
    allpass[5].length = VERB_AP6;
    allpass[6].buf = buf + bufCount;
    bufCount += VERB_D1;
    allpass[6].length = VERB_D1;

    allpass[8].buf = buf + bufCount;
    bufCount += VERB_AP7;
    allpass[8].length = VERB_AP7;
    allpass[9].buf = buf + bufCount;
    bufCount += VERB_AP8;
    allpass[9].length = VERB_AP8;
    allpass[7].buf = buf + bufCount;
    bufCount += VERB_D2;
    allpass[7].length = VERB_D2;

    position[0] = f32_to_q15(0.9f);
    position[1] = f32_to_q15(-0.9f);
    position[2] = f32_to_q15(1.f);
    position[3] = f32_to_q15(-1.f);
  }
  int16_t last_l, last_r;
  inline void process(int16_t in, int16_t &l, int16_t &r) {
    if ((++count) % 2 == 0) {
      l = last_l;
      r = last_r;
      return;
    }
    for (int i = 0; i < 4; i++) {
      in = ProcessAllPass(in, &allpass[i]);
    }
    // low pass filter the feedback
    feedback = add_q15(mult_q15(feedback, dampAmount),
                       mult_q15(damp[0], sub_q15(Q15_MAX, dampAmount)));
    damp[0] = feedback;

    in = ProcessAllPass(add_q15(in, mult_q15(feedbackAmount, feedback)),
                        &allpass[4]);
    in = ProcessAllPass(in, &allpass[5]);
    in = DelayProcess(in, &allpass[6]);
    in = ProcessAllPass(in, &allpass[8]);
    in = ProcessAllPass(in, &allpass[9]);
    feedback = DelayProcess(in, &allpass[7]);

    // 4. Delay Tap Positions. These are other good candidates for
    // experimentation. I've changed them to different prime numbers to break up
    // regularities.
    int16_t dt1 = DelayTap(310, &allpass[6]);
    int16_t dt2 = DelayTap(611, &allpass[6]);
    int16_t dt3 = DelayTap(937, &allpass[7]);
    int16_t dt4 = DelayTap(1201, &allpass[7]);

    dt3 = mult_q15(dt2, f32_to_q15(0.8));
    dt4 = mult_q15(dt2, f32_to_q15(0.8));

    l = 0;
    r = 0;

    l = add_q15(l, Mix(0, dt1, position[0]));
    r = add_q15(r, Mix(0, dt1, 0xffff - position[0]));

    l = add_q15(l, Mix(0, dt2, position[1]));
    r = add_q15(r, Mix(0, dt2, 0xffff - position[1]));

    l = add_q15(l, Mix(0, dt3, position[2]));
    r = add_q15(r, Mix(0, dt3, 0xffff - position[2]));

    last_l = l = add_q15(l, Mix(0, dt4, position[3]));
    last_r = r = add_q15(r, Mix(0, dt4, 0xffff - position[3]));
  }

private:
  inline int16_t ProcessAllPass(int16_t in, AllPassFilter *ap) {
    int16_t apdelayed = ap->buf[ap->count];
    int16_t inSum = ap->buf[ap->count] =
        add_q15(mult_q15(-ap->c, apdelayed), in);
    ap->count = (ap->count + 1) % ap->length;
    return add_q15(mult_q15(inSum, ap->c), apdelayed);
  }
  inline int16_t DelayWobbleLookup(AllPassFilter *d) {
    int16_t lfo = Interpolate824(wav_sine, d->phase);
    int32_t offset = ((int32_t)d->length * (int32_t)(lfo >> 1));
    int16_t a = DelayTap(offset >> 16, d);
    int16_t b = DelayTap((offset >> 16) + 1, d);
    return Mix(a, b, lfo);
  }
  inline int16_t DelayProcessWobble(int16_t in, AllPassFilter *d) {
    DelayProcess(in, d);
    d->phase += d->phaseOffset;
    return DelayWobbleLookup(d);
  }
  inline int16_t DelayProcess(int16_t in, AllPassFilter *d) {
    int16_t delayed = d->buf[d->count];
    d->buf[d->count] = in;
    d->count = (d->count + 1) % d->length;
    return delayed;
  }
  inline int16_t DelayTap(int16_t tap, AllPassFilter *d) {
    int offset = tap + d->count;
    if (offset > d->length - 1) {
      offset -= d->length;
    }
    return d->buf[offset % d->length];
  }

  // int16_t TapAp(int tap, AllPassFilter *ap)
  // {
  //     int offset = tap+ap->count;
  //     while (offset < 0)
  //         offset += ap->length;
  //     while(offset > ap->length-1)
  //     {
  //         offset -= ap->length;
  //     }
  //     return ap->buf[offset];
  // }
  q15_t damp[2] = {0};
  q15_t dampAmount, feedbackAmount;
  uint16_t phaseOffset[10] = {803, 447,  1243, 1089, 304,
                              183, 4128, 481,  513,  713};

  AllPassFilter allpass[10];
  int16_t buf[VERB_LENGTH] = {0};
  int16_t feedback = 0;
  uint8_t count = 0;
  int length = VERB_LENGTH;

  uint16_t position[4] = {0x7fff, 0x7fff, 0x4000, 0xffff};
};