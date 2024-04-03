#include "SampleInstrument.h"
#include "Application/Instruments/Filters.h"
#include "Application/Model/Table.h"
#include "Application/Player/PlayerMixer.h" // For MIX_BUFFER_SIZE.. kick out pls
#include "Application/Player/SyncMaster.h"
#include "CommandList.h"
#include "SamplePool.h"
#include "SampleVariable.h"
#include "Services/Audio/Audio.h"
#include "System/Console/Trace.h"
#include "System/io/Status.h"
#include <assert.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Application/Player/SyncMaster.h"
#include "SampleInstrumentDatas.h"

bool SampleInstrument::useDirtyDownsampling_ = false;

renderParams SampleInstrument::renderParams_[SONG_CHANNEL_COUNT];

#define SHOULD_KILL_CLICKS false

signed char SampleInstrument::lastMidiNote_[SONG_CHANNEL_COUNT];

#define KRATE_SAMPLE_COUNT 100

SampleInstrument::SampleInstrument() {

  // Reserve Observer
  ReserveObserver(1);

  // Initialize MIDI notes
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    SampleInstrument::lastMidiNote_[i] = -1;
  }

  // Initialize instruments settings
  source_ = 0;
  dirty_ = false;
  running_ = false;

  // Initialize exported variables
  WatchedVariable *wv = new SampleVariable("sample", SIP_SAMPLE);
  insert(end(), wv);
  wv->AddObserver(*this);

  volume_ = new Variable("volume", SIP_VOLUME, 0x80);
  insert(end(), volume_);

  interpolation_ =
      new Variable("interpol", SIP_INTERPOLATION, interpolationTypes, 2, 0);
  insert(end(), interpolation_);

  crush_ = new Variable("crush", SIP_CRUSH, 16);
  insert(end(), crush_);

  drive_ = new Variable("crushdrive", SIP_CRUSHVOL, 0xFF);
  insert(end(), drive_);

  downsample_ = new Variable("downsample", SIP_DOWNSMPL, 0);
  insert(end(), downsample_);

  rootNote_ = new Variable("root note", SIP_ROOTNOTE, 60);
  insert(end(), rootNote_);

  fineTune_ = new Variable("fine tune", SIP_FINETUNE, 0x7F);
  insert(end(), fineTune_);

  pan_ = new Variable("pan", SIP_PAN, 0x7F);
  insert(end(), pan_);

  cutoff_ = new Variable("filter cut", SIP_FILTCUTOFF, 0xFF);
  insert(end(), cutoff_);

  reso_ = new Variable("filter res", SIP_FILTRESO, 0x00);
  insert(end(), reso_);

  filterMix_ = new Variable("filter type", SIP_FILTMIX, 0x00);
  insert(end(), filterMix_);

  filterMode_ = new Variable("filter mode", SIP_FILTMODE, filterMode, 3, 0);
  insert(end(), filterMode_);

  start_ = new WatchedVariable("start", SIP_START, 0);
  insert(end(), start_);
  start_->AddObserver(*this);

  loopMode_ = new Variable("loopmode", SIP_LOOPMODE, loopTypes, SILM_LAST, 0);
  insert(end(), loopMode_);
  loopMode_->SetInt(0);

  slices_ = new Variable("slices", SIP_SLICES, 1);
  insert(end(), slices_);

  loopStart_ = new WatchedVariable("loopstart", SIP_LOOPSTART, 0);
  insert(end(), loopStart_);
  loopStart_->AddObserver(*this);

  loopEnd_ = new WatchedVariable("end", SIP_END, 0);
  insert(end(), loopEnd_);
  loopEnd_->AddObserver(*this);

  table_ = new Variable("table", SIP_TABLE, -1);
  insert(end(), table_);

  tableAuto_ = new Variable("table automation", SIP_TABLEAUTO, false);
  insert(end(), tableAuto_);

  // Reset table state
  tableState_.Reset();
}

SampleInstrument::~SampleInstrument() {}

bool SampleInstrument::Init() {

  SamplePool *pool = SamplePool::GetInstance();
  Variable *vSample = FindVariable(SIP_SAMPLE);
  NAssert(vSample);
  int index = vSample->GetInt();
  source_ = (index >= 0) ? pool->GetSource(index) : 0;
  tableState_.Reset();
  return false;
}

void SampleInstrument::OnStart() { tableState_.Reset(); };

bool SampleInstrument::Start(int channel, unsigned char midinote,
                             bool cleanstart) {
  // Look if we're dirty & need to update this instrument's data

  if (dirty_) {
    updateInstrumentData(false);
  }

  running_ = true;

  if (source_ == 0)
    return false;

  // Get Rendering params for current voice & fill init data

  renderParams *rp = renderParams_ + channel;

  rp->midiNote_ = midinote;

  if (lastMidiNote_[channel] == -1) // To prevent First LEGA to go bonkers
  {
    lastMidiNote_[channel] = midinote;
  }

  // Duplicate variable value to local rendering
  // to alter with commands

  rp->sampleBuffer_ = source_->GetSampleBuffer(rp->midiNote_);
  if (rp->sampleBuffer_ == 0) {
    return false;
  };
  rp->channelCount_ = source_->GetChannelCount(rp->midiNote_);

  int rootNote =
      (rootNote_->GetInt() - 60) + source_->GetRootNote(rp->midiNote_);

  rp->volume_ = rp->baseVolume_ = i2fp(volume_->GetInt());

  rp->pan_ = rp->basePan_ = i2fp(pan_->GetInt());

  if (!source_->IsMulti()) {
    rp->rendLoopStart_ = loopStart_->GetInt();
    rp->rendLoopEnd_ = loopEnd_->GetInt();
  } else {
    long start = source_->GetLoopStart(rp->midiNote_);
    if (start > 0) {
      rp->rendLoopStart_ =
          source_->GetLoopStart(rp->midiNote_); // hack at the moment
      rp->rendLoopEnd_ = source_->GetLoopEnd(rp->midiNote_);
      loopMode_->SetInt(SILM_LOOP);
    } else {
      rp->rendLoopStart_ = 0;                             // hack at the moment
      rp->rendLoopEnd_ = source_->GetSize(rp->midiNote_); // hack at the moment
      loopMode_->SetInt(SILM_ONESHOT);
    };
  }
  SampleInstrumentLoopMode loopmode =
      (SampleInstrumentLoopMode)loopMode_->GetInt();

  /*	 if (loopmode==SILM_OSCFINE) {
                  if (rp->rendLoopEnd_>source_->GetSize()-1) { // check for
     older instrument that were not correctly handled
                          rp->rendLoopEnd_=source_->GetSize()-1 ;
                  }
           }*/
  rp->reverse_ = false;

  float driverRate = float(Audio::GetInstance()->GetSampleRate());

  switch (loopmode) {
  case SILM_ONESHOT:
  case SILM_LOOP:
  case SILM_LOOP_PINGPONG:

    // Compute speed factor
    // if instrument sampled below 44.1Khz, should
    // travel slower in sample

    rp->rendFirst_ = start_->GetInt();
    rp->position_ = float(rp->rendFirst_);
    rp->baseSpeed_ = fl2fp(source_->GetSampleRate(rp->midiNote_) / driverRate);
    rp->reverse_ = (rp->rendLoopEnd_ < rp->position_);

    break;

  case SILM_OSC:
    //		case SILM_OSCFINE:
    {

      float freq = 261.6255653006f; // C3
                                    /*			if (loopmode==SILM_OSCFINE) {
                                                                    freq=float(pow(2.0,-0.75))*440; // C3
                                                            }*/
      int length = rp->rendLoopEnd_ - rp->rendLoopStart_;
      if (length == 0)
        length = 1;
      if (length < 0) {
        rp->reverse_ = true;
        length = -length;
      };
      rp->baseSpeed_ = fl2fp((freq * length) / driverRate);
      rp->rendFirst_ = rp->rendLoopStart_;
      if (cleanstart) {
        rp->position_ = float(rp->rendFirst_);
      }
      break;
    }
  case SILM_LOOPSYNC: {
    int length = rp->rendLoopEnd_ - rp->rendLoopStart_;
    if (length < 0) {
      rp->reverse_ = true;
      length = -length;
    };
    SyncMaster *sm = SyncMaster::GetInstance();
    int sampleCount = int(sm->GetTickSampleCount());
    sampleCount *= (6 * 16);
    rp->baseSpeed_ = fl2fp(length / float(sampleCount));
    rp->rendFirst_ = rp->rendLoopStart_;
    if (cleanstart) {
      rp->position_ = float(rp->rendFirst_);
    }
    break;
  }
  case SILM_SLICE: {
    int note = rp->midiNote_;
    if (note > slices_->GetInt() - 1)
      break; // No sound outside of slice range
    int slice = rp->rendLoopEnd_ / slices_->GetInt();

    rp->position_ = float(note * slice);
    rp->baseSpeed_ = fl2fp(source_->GetSampleRate(note) / driverRate);
    rp->speed_ = rp->baseSpeed_;
    rp->rendLoopEnd_ = (note + 1) * slice;
    break;
  }
  case SILM_LAST:
    NAssert(0);
    break;
  }

  // Compute octave & note difference from root

  float fineTune = float(fineTune_->GetInt() - 0x7F);
  fineTune /= float(0x80);
  int offset = midinote - rootNote;
  if (loopmode == SILM_SLICE) {
    offset = rootNote - source_->GetRootNote(rp->midiNote_);
  }
  while (offset > 127) {
    offset -= 12;
  }

  fixed freqFactor = fl2fp(float(pow(2.0, (offset + fineTune) / 12.0)));
  rp->baseSpeed_ = fp_mul(rp->baseSpeed_, freqFactor);
  rp->speed_ = rp->baseSpeed_;

  // Init k rate counter

  rp->krateCount_ = 0;

  // We allow processing

  rp->finished_ = false;

  // If we do a clean start (there was a instr number on the line)

  if (cleanstart) {

    // Clear retrigger data

    rp->retrig_ = false;
    rp->retrigLoop_ = 0;
    rp->retrigCount_ = 0;
    rp->retrigOffset_ = 0;

    // Could click

    rp->couldClick_ = SHOULD_KILL_CLICKS;

    // Init filter params

    rp->cutoff_ = rp->baseFCut_ = fl2fp(cutoff_->GetInt() / 255.0f);
    rp->reso_ = rp->baseFRes_ = fl2fp(reso_->GetInt() / 255.0f);

    // Init crush params
    rp->crush_ = crush_->GetInt();
    rp->drive_ = drive_->GetInt();

    // Init downsampling
    rp->downsample_ = downsample_->GetInt();

    // Disable all active updaters for new voice
    for (auto it = rp->activeUpdaters_.begin(); it != rp->activeUpdaters_.end();
         it++) {
      I_SRPUpdater *current = *it;
      current->Disable();
    }
    rp->activeUpdaters_.clear();
  }
  return true;
}

void SampleInstrument::Stop(int channel) { running_ = false; }

void SampleInstrument::doTickUpdate(int channel) {

  // Process updaters
  renderParams *rp = renderParams_ + channel;
  for (auto it = rp->activeUpdaters_.begin(); it != rp->activeUpdaters_.end();
       it++) {
    I_SRPUpdater *current = *it;
    current->Trigger(true);
  }
};

void SampleInstrument::doKRateUpdate(int channel) {

  renderParams *rp = renderParams_ + channel;
  for (auto it = rp->activeUpdaters_.begin(); it != rp->activeUpdaters_.end();
       it++) {
    I_SRPUpdater *current = *it;
    current->Trigger(false);
  }
};

// Size in samples

bool SampleInstrument::Render(int channel, fixed *buffer, int size,
                              bool updateTick) {

  bool somethingToMix = false;

  // Get Current render parameters

  renderParams *rp = renderParams_ + channel;
  lastMidiNote_[channel] = rp->midiNote_;
  bool *rpFinished = &(rp->finished_);

  if (source_) {

    if (*rpFinished)
      return false;

    // clear the fixed point buffer

    SYS_MEMSET(buffer, 0, size * 2 * sizeof(fixed));

    bool hasUpdaters = !(rp->activeUpdaters_.empty());

    int filterMix = filterMix_->GetInt();
    FilterMode filterMode = (FilterMode)filterMode_->GetInt();
    bool filterBoost = (filterMode == FM_SCREAM);
    bool bassyFilter = (filterMode == FM_BASSY);

    // Be sure filters are properly initialized

    set_filter(channel, FLT_LOWPASS, rp->cutoff_, rp->reso_, filterMix,
               bassyFilter);

    filter_t *flt = get_filter(channel);
    bool filtering = (rp->cutoff_ < i2fp(1)) || (rp->reso_ > i2fp(0));

    // Process tick-level updates

    if (updateTick) {

      if (hasUpdaters) {

        doTickUpdate(channel);

        struct RUParams rup;
        rup.cutOffset_ = rup.resOffset_ = rup.volumeOffset_ = rup.panOffset_ =
            0;
        rup.speedOffset_ = FP_ONE;

        for (auto it = rp->activeUpdaters_.begin();
             it != rp->activeUpdaters_.end(); it++) {
          I_SRPUpdater *current = *it;
          current->UpdateSRP(rup);
        }

        rp->volume_ = rp->baseVolume_ + rup.volumeOffset_;
        rp->speed_ = fp_mul(rp->baseSpeed_, rup.speedOffset_);
        rp->pan_ = rp->basePan_ + rup.panOffset_;
      }

      // Process retrig

      if (rp->retrig_) {
        if (rp->retrigCount_ == 0) {
          int ticks = rp->retrigOffset_ - rp->retrigLoop_;
          long offset =
              long(ticks * SyncMaster::GetInstance()->GetTickSampleCount());
          rp->position_ += offset * fp2fl(rp->speed_);
          if (rp->position_ < 0) {
            rp->position_ = 0;
          };
          rp->retrigCount_ = rp->retrigLoop_;
        }
        rp->retrigCount_--;
      };
    }

    // Get additional parameters from variables

    // Crush

    int shift = 16 - rp->crush_;
    fixed mask = 0xFFFFFFFF;
    if (shift != 0) {
      mask <<= FIXED_SHIFT + shift;
    }

    // Crush vol

    int crushvol = rp->drive_;
    fixed fpcrushvol = fl2fp(crushvol / 255.0F);

    // downsample

    int downsmpl = rp->downsample_;
    unsigned int dsMask = 0xFFFFFFFF << downsmpl;

    // Loop mode

    SampleInstrumentLoopMode loopMode =
        (SampleInstrumentLoopMode)loopMode_->GetInt();

    // Interpolation

    int interpol = interpolation_->GetInt();

    // Get sound characteristics

    char *wavbuf = (char *)rp->sampleBuffer_;

    int channelCount = rp->channelCount_;

    int count = size; // number of samples to treat

    fixed *result = buffer;

    // Get volume factor and pan

    fixed volscale = fl2fp(0.003921568627450980392156862745098f);
    fixed volfactor = fp_mul(rp->volume_, volscale);
    int pan = fp2i(rp->pan_);
    fixed fixedpanl = panlaw[pan];
    fixed fixedpanr = panlaw[254 - pan];

    // filter constants

    fixed f_k = fl2fp(1.0F / 3.0F);
    fixed f_s = FP_ONE - f_k;

    // Get pan multiplicators, and take volume into account

    int n = int(rp->position_);
    short *input = (short *)(wavbuf + 2 * channelCount *
                                          n); // input is the current
                                              // sample to the left of position

    fixed fpPos = fl2fp(rp->position_ - n); // fpPos is current pos from input
    fixed fpSpeed = rp->speed_;             // speed in fixed
    if (rp->reverse_) {
      fpSpeed = -rp->speed_;
    }

    fixed s1, s2, t2, eta, inveta;
    s2 = 0;
    t2 = 0;

    short *loopPosition =
        (short *)(wavbuf + rp->rendLoopStart_ * 2 * channelCount);
    short *lastSample =
        (short *)(wavbuf + (rp->rendLoopEnd_ - 1) * 2 * channelCount);

    if (/*(loopMode==SILM_OSCFINE)||*/ (rp->reverse_)) {
      lastSample = (short *)(wavbuf + rp->rendLoopEnd_ * 2 * channelCount);
    }

    fixed zerofive = fl2fp(0.5f);

    // try to speed up access using pointers rather than structure access

    bool rpReverse = rp->reverse_;
    int rpKrateCount = rp->krateCount_;

    fixed *fltSpeed = flt->speed;
    fixed *fltHeight = flt->height;
    fixed fltMix = flt->mix;
    fixed fltMixInv = FP_ONE - fltMix;
    fixed *fltDelay = flt->hipdelay;
    fixed fltParm1 = flt->freq;
    fixed fltParm2 = flt->reso;
    fixed fltDirt = flt->dirt;

    fixed *fltSpeedPtr = 0;
    fixed *fltDelayPtr = 0;
    fixed *fltHeightPtr = 0;

    short *dsBasePtr = ((short *)wavbuf) + rp->rendFirst_ * channelCount;

    while (count > 0) {

      // look where we are, if we need to

      if (!rpReverse) {
        if (input >= lastSample /*-((loopMode==SILM_OSCFINE)?1:0)*/) {
          switch (loopMode) {
          case SILM_ONESHOT:
          case SILM_SLICE:
            *rpFinished = true;
            break;
          case SILM_LOOP:
          case SILM_OSC:
          case SILM_LOOPSYNC:
            input = loopPosition;
            rpReverse = (loopPosition > lastSample);
            if (rpReverse) {
              fpSpeed = -rp->speed_;
            } else {
              fpSpeed = rp->speed_;
            }
            break;
          case SILM_LOOP_PINGPONG:
            if ((loopPosition > lastSample)) {
              if (input <= lastSample || input >= loopPosition) {
                rpReverse = !rpReverse;
                fpSpeed = -fpSpeed;
              }
            } else {
              if (input >= lastSample || input <= loopPosition) {
                rpReverse = !rpReverse;
                fpSpeed = -fpSpeed;
              }
            }
            break;
            /*						case SILM_OSCFINE:
                                                            {
                                                                    int
               offset=(input-lastSample)/channelCount ;
                                                                    rpReverse=(loopPosition>lastSample)
               ; if (rpReverse) { fpSpeed=-rp->speed_ ;
                                                                            input=loopPosition-offset
               ; } else { fpSpeed=rp->speed_ ; input=loopPosition+offset ;
                                                                    }
                                                                    break ;
                                                            }*/
          case SILM_LAST:
            NAssert(0);
            break;
          };
        }
      } else {
        if (input < lastSample) {
          switch (loopMode) {
          case SILM_ONESHOT:
          case SILM_SLICE:
            *rpFinished = true;
            break;
          case SILM_LOOP:
          case SILM_OSC:
          case SILM_LOOPSYNC:
            input = loopPosition;
            rpReverse = (loopPosition > lastSample);
            if (rpReverse) {
              fpSpeed = -rp->speed_;
            } else {
              fpSpeed = rp->speed_;
            }
            break;
          case SILM_LOOP_PINGPONG:
            if ((loopPosition > lastSample)) {
              if (input <= lastSample || input >= loopPosition) {
                rpReverse = !rpReverse;
                fpSpeed = -fpSpeed;
              }
            } else {
              if (input >= lastSample || input <= loopPosition) {
                rpReverse = !rpReverse;
                fpSpeed = -fpSpeed;
              }
            }
            break;
            /*						case SILM_OSCFINE:
                                                            {
                                                                    int
               offset=(lastSample-input)/channelCount ;
                                                                    rpReverse=(loopPosition>lastSample)
               ; if (rpReverse) { fpSpeed=-rp->speed_ ;
                                                                            input=loopPosition-offset
               ; } else { fpSpeed=rp->speed_ ; input=loopPosition+offset ;
                                                                    }
                                                                    break ;
                                                            }*/
          case SILM_LAST:
            NAssert(0);
            break;
          };
        }
      };

      if (*rpFinished) {
        count = -1;
      } else {

        // See if time to process k-rate change

        if (rpKrateCount-- == 0) {
          rpKrateCount = KRATE_SAMPLE_COUNT;

          if (hasUpdaters) {
            doKRateUpdate(channel);
            struct RUParams rup;
            rup.cutOffset_ = rup.resOffset_ = rup.volumeOffset_ =
                rup.panOffset_ = rup.fbMixOffset_ = rup.fbTunOffset_ = 0;
            rup.speedOffset_ = FP_ONE;

            for (auto it = rp->activeUpdaters_.begin();
                 it != rp->activeUpdaters_.end(); it++) {
              I_SRPUpdater *current = *it;
              current->UpdateSRP(rup);
            }

            rp->volume_ = rp->baseVolume_ + rup.volumeOffset_;
            rp->pan_ = rp->basePan_ + rup.panOffset_;
            rp->speed_ = fp_mul(rp->baseSpeed_, rup.speedOffset_);
            rp->cutoff_ = rp->baseFCut_ + rup.cutOffset_;
            rp->reso_ = rp->baseFRes_ + rup.resOffset_;
            rp->fbMix_ = rp->baseFbMix_ + rup.fbMixOffset_;
            rp->fbTun_ = rp->baseFbTun_ + rup.fbTunOffset_;

            set_filter(channel, FLT_LOWPASS, rp->cutoff_, rp->reso_, filterMix,
                       bassyFilter);
            filtering = (rp->cutoff_ < i2fp(1)) || (rp->reso_ > i2fp(0));

            volfactor = fp_mul(rp->volume_, volscale);
            pan = fp2i(rp->pan_);

            if (rpReverse) {
              fpSpeed = -rp->speed_;
            } else {
              fpSpeed = rp->speed_;
            }
          }
        }

        // get input sample to interpolate from
        // s= left channel
        // t= right channel

        short *i1 = input;
        if (dsMask != 0xFFFFFFFF) {
          if (useDirtyDownsampling_) {
            i1 = (short *)(((uintptr_t)input) & dsMask);
          } else {
            unsigned int distance =
                (unsigned int)(input - dsBasePtr) / channelCount;
            i1 = dsBasePtr + (distance & dsMask) * channelCount;
          }
        }

        short *i2 = i1 + channelCount;

        if (filtering) {
          fltSpeedPtr = fltSpeed;
          fltHeightPtr = fltHeight;
          fltDelayPtr = fltDelay;
        }

        for (int i = 0; i < channelCount; i++) {
          t2 = s2; // move L to R if necessary
          s1 = i2fp(*i1++);
          s2 = i2fp(*i2++);

          switch (interpol) {

          case 0: // Linear interpolation

            eta = fpPos;
            inveta = fp_sub(FP_ONE, eta);

            // interpolate

            s1 = fp_mul(s1, inveta);
            s2 = fp_mul(s2, eta);

            // Compute interpolated sample

            s1 += s2;
            break;

          case 1: // Nearest neighbor

            if (fpPos > zerofive) {
              s1 = s2;
            };
            break;
          }

          // crush predrive

          s2 = fp_mul(s1, fpcrushvol);

          // store result, applying crush

          s2 = (s2 & mask);

          // apply volume

          s2 = fp_mul(s2, volfactor);

          // apply filtering if needed

          if (filtering) {

            fixed lpin = fp_mul(s2, fltMixInv);
            fixed hpin = -fp_mul(s2, fltMix);

            fixed difr = fp_sub(lpin, *fltHeightPtr);

            // Introduce non-linearity if screamin'

            if (filterBoost) {
              if (*fltSpeedPtr < -FP_ONE) {
                *fltSpeedPtr = -f_s;
              } else if (fltSpeed[i] > FP_ONE) {
                *fltSpeedPtr = f_s;
              };
              *fltSpeedPtr = fp_mul(*fltSpeedPtr, fltDirt);
            }

            *fltSpeedPtr =
                fp_mul(*fltSpeedPtr, fltParm2); // mul by res, it's some kind of
                                                // inertia.
            /*HOG:5*/ *fltSpeedPtr = fp_add(
                *fltSpeedPtr,
                fp_mul(difr, fltParm1)); // mul by cutoff, less cutoff = no
                                         // sound, so it's better not be 0.

            *fltHeightPtr += *fltSpeedPtr;
            *fltHeightPtr += *fltDelayPtr - hpin;
            s2 = *fltHeightPtr;

            *fltDelayPtr = hpin;
            fltDelayPtr++;
            fltHeightPtr++;
            fltSpeedPtr++;
          }
        }

        if (channelCount == 1) {
          t2 = s2;
        }

        // introduce panning & vol - store result

        s2 = fp_mul(s2, fixedpanl);
        t2 = fp_mul(t2, fixedpanr);

        *result++ = s2;
        *result++ = t2;

        // Computes new pos for next input sample
        // fpPos is always relative to 'input' pointer

        fpPos = fp_add(fpPos, fpSpeed);
        int delta = fp2i(fpPos);
        input += channelCount * delta;
        fpPos = fp_sub(fpPos, i2fp(delta));
        count--;
      }
    }
    // Update 'reverse' mode if changed

    rp->reverse_ = rpReverse;

    // Update final sample position
    rp->position_ =
        (((char *)input) - wavbuf) / (2 * channelCount) + fp2fl(fpPos);

    somethingToMix = true;
  }

  return somethingToMix;
};

void SampleInstrument::AssignSample(int i) {

  Variable *v = FindVariable(SIP_SAMPLE);
  v->SetInt(i);
};

int SampleInstrument::GetSampleIndex() {
  Variable *v = FindVariable(SIP_SAMPLE);
  return v->GetInt();
};

void SampleInstrument::SetVolume(int volume) {
  Variable *v = FindVariable(SIP_VOLUME);
  v->SetInt(volume);
};

int SampleInstrument::GetVolume() {
  Variable *v = FindVariable(SIP_VOLUME);
  return v->GetInt();
};

int SampleInstrument::GetSampleSize(int channel) {
  if (source_) {
    renderParams *rp = renderParams_ + channel;
    return source_->GetSize(rp->midiNote_);
  };
  return 0;
};

bool SampleInstrument::IsInitialized() { return (source_ != 0); };

void SampleInstrument::updateInstrumentData(bool search) {

  SamplePool *pool = SamplePool::GetInstance();

  // Get the source index

  Variable *vSample = FindVariable(SIP_SAMPLE);
  int index = vSample->GetInt();
  int instrSize = 0;

  if (index != NO_SAMPLE) {
    source_ = pool->GetSource(index);
    if (source_ && (!source_->IsMulti())) {
      instrSize = source_->GetSize(-1);
    }
  }

  Variable *v = FindVariable(SIP_END);
  v->SetInt(instrSize);
  v = FindVariable(SIP_LOOPSTART);
  v->SetInt(0);
  v = FindVariable(SIP_START);
  v->SetInt(0);
  dirty_ = false;
};

void SampleInstrument::Update(Observable &o, I_ObservableData *d) {
  WatchedVariable &v = (WatchedVariable &)o;
  FourCC id = v.GetID();

  switch (id) {
  case SIP_SAMPLE: {
    if (running_) {
      dirty_ = true; // we'll update later, when instrument gets re-triggered
    } else {
      updateInstrumentData(true);
      SetChanged();
      NotifyObservers();
    };
  } break;

  default:
    //         Trace::Dump("Got notification from
    //         %c%c%c%c",fourcc[0],fourcc[1],fourcc[2],fourcc[3]) ;
    break;
  };
};

void SampleInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {

  renderParams *rp = renderParams_ + channel;
  if (!source_)
    return;

  switch (cc) {
  case I_CMD_LPOF:

    if (value > 0x8000) {
      value = 0x10000 - value;
      if (rp->rendLoopStart_ > value) {
        rp->rendLoopEnd_ -= value;
        rp->rendLoopStart_ -= value;
      }
    } else {
      if (rp->rendLoopEnd_ + value < source_->GetSize(rp->midiNote_)) {
        rp->rendLoopEnd_ += value;
        rp->rendLoopStart_ += value;
      };
    }
    break;

  case I_CMD_PLOF: {
    if (!source_)
      return;
    int wavSize = source_->GetSize(rp->midiNote_);
    float chkSize = wavSize / 256.0f;
    int absShft = value >> 8;
    if (absShft != 0) {
      rp->position_ = chkSize * absShft;
    };
    int relSfht = value & 0xFF;
    if (relSfht > 127)
      relSfht = relSfht - 256;
    rp->position_ += relSfht * chkSize;
    while (rp->position_ < 0) {
      rp->position_ = wavSize + rp->position_;
    };
    while (rp->position_ >= wavSize) {
      rp->position_ -= wavSize;
    };
    rp->couldClick_ = SHOULD_KILL_CLICKS;
  } break;

  case I_CMD_ARPG: {
    rp->arp_.SetData(value);
    if (!rp->arp_.Enabled()) {
      rp->arp_.Enable();
      rp->activeUpdaters_.push_back(&rp->arp_);
    }
  } break;

  case I_CMD_VOLM: {
    float targetVolume = float(value & 0xFF);
    float speed = float(value >> 8);
    float startVolume = fp2fl(rp->volume_);
    float baseVolume = fp2fl(rp->baseVolume_);

    int sampleCount = int(4 * SyncMaster::GetInstance()->GetTickSampleCount());
    speed = (speed == 0) ? 0
                         : fabs(targetVolume - startVolume) *
                               KRATE_SAMPLE_COUNT / float(speed) / sampleCount;
    rp->volumeRamp_.SetData(targetVolume - baseVolume, speed,
                            startVolume - baseVolume);
    if (!rp->volumeRamp_.Enabled()) {
      rp->volumeRamp_.Enable();
      rp->activeUpdaters_.push_back(&rp->volumeRamp_);
    }
  } break;

  case I_CMD_PAN_: {
    float targetPan = float(value & 0xFF);
    if (targetPan == 0xFF) {
      targetPan = 0xFE;
    }
    float basePan = fp2fl(rp->basePan_);
    float speed = float(value >> 8);
    float startPan = fp2fl(rp->pan_);
    int sampleCount = int(4 * SyncMaster::GetInstance()->GetTickSampleCount());
    speed = (speed == 0) ? 0
                         : fabs(targetPan - startPan) * KRATE_SAMPLE_COUNT /
                               float(speed) / sampleCount;
    rp->panner_.SetData(targetPan - basePan, speed, startPan - basePan);
    if (!rp->panner_.Enabled()) {
      rp->panner_.Enable();
      rp->activeUpdaters_.push_back(&rp->panner_);
    }
  } break;

  case I_CMD_FCUT: {
    float target = float(value & 0xFF) / 255.0f;
    float speed = float(value >> 8);
    float start = fp2fl(rp->cutoff_);
    float baseCut = fp2fl(rp->baseFCut_);
    int sampleCount = int(4 * SyncMaster::GetInstance()->GetTickSampleCount());
    speed = (speed == 0) ? 0
                         : fabs(target - start) * KRATE_SAMPLE_COUNT /
                               float(speed) / sampleCount;
    rp->cutRamp_.SetData(target - baseCut, speed, start - baseCut);
    if (!rp->cutRamp_.Enabled()) {
      rp->cutRamp_.Enable();
      rp->activeUpdaters_.push_back(&rp->cutRamp_);
    }
  } break;

  case I_CMD_FRES: {
    float target = float(value & 0xFF) / 255.0f;
    float speed = float(value >> 8);
    float start = fp2fl(rp->reso_);
    float baseRes = fp2fl(rp->baseFRes_);
    int sampleCount = int(4 * SyncMaster::GetInstance()->GetTickSampleCount());
    speed = (speed == 0) ? 0
                         : fabs(target - start) * KRATE_SAMPLE_COUNT /
                               float(speed) / sampleCount;
    rp->resRamp_.SetData(target - baseRes, speed, start - baseRes);
    if (!rp->resRamp_.Enabled()) {
      rp->resRamp_.Enable();
      rp->activeUpdaters_.push_back(&rp->resRamp_);
    }
  } break;
  case I_CMD_PTCH: {
    int pitch = (char)(value & 0xFF); // number of semi tones
    float speed = float(value >> 8);  // get speed parameter
    if (pitch > 127)
      pitch = pitch - 256;

    // Target speed for the ramp

    float targetSpeed = float(pow(2.0, (pitch) / 12.0));
    float srcSpeed = fp2fl(rp->speed_) / fp2fl(rp->baseSpeed_);

    // speed of ramp

    speed = (speed == 0) ? 0.0f
                         : fp2fl(rp->speed_) * 255.0f / speed /
                               KRATE_SAMPLE_COUNT / 32.0f;

    // Fill ramp data & enable

    rp->speedRamp_.SetData(targetSpeed, speed, srcSpeed);
    if (!rp->speedRamp_.Enabled()) {
      rp->speedRamp_.Enable();
      rp->activeUpdaters_.push_back(&rp->speedRamp_);
    }
  }; break;

  case I_CMD_LEGA: {
    int pitch = (char)(value & 0xFF); // number of semi tones
    float speed = float(value >> 8);  // get speed parameter

    if (pitch > 127)
      pitch = pitch - 256;

    // Target speed for the ramp, taken from channel' last note
    // if no pitch is given

    float targetSpeed, initSpeed;
    if (pitch == 0) {
      pitch = lastMidiNote_[channel] - rp->midiNote_;
      targetSpeed = 1.0;
      initSpeed = float(pow(2.0f, pitch / 12.0f));
    } else {
      initSpeed = fp2fl(rp->speed_) / fp2fl(rp->baseSpeed_);
      targetSpeed = float(pow(2.0f, pitch / 12.0f));
    }

    // speed of ramp

    speed = (speed == 0) ? 0.0f : float(1 + 50.0 / KRATE_SAMPLE_COUNT / speed);

    // Fill ramp data & enable

    rp->legato_.SetData(targetSpeed, speed, initSpeed);
    if (!rp->legato_.Enabled()) {
      rp->legato_.Enable();
      rp->activeUpdaters_.push_back(&rp->legato_);
    }
  }; break;

  case I_CMD_PFIN: {

    float semi = (value & 0xFF) / float(0x80); // number of semi tones
    if (semi > 1)
      semi = semi - 2;

    float speed = float(value >> 8); // get speed parameter

    float initSpeed = rp->pfin_.Enabled() ? rp->pfin_.GetCurrent() : 1;
    float targetSpeed = float(pow(2.0f, semi / 12.0f));

    // speed of ramp

    speed = (speed == 0) ? 0.0f : float(1 + 50.0 / KRATE_SAMPLE_COUNT / speed);

    // Fill ramp data & enable

    rp->pfin_.SetData(targetSpeed, speed, initSpeed);

    if (!rp->pfin_.Enabled()) {
      rp->pfin_.Enable();
      rp->activeUpdaters_.push_back(&rp->pfin_);
    }
  }; break;

  case I_CMD_RTRG: {
    unsigned char loop = (value & 0xFF); // number of ticks before repeat
    unsigned char offset =
        (value >> 8); // number of ticks to offset at each repeat
    if (loop != 0) {
      rp->retrig_ = true;
      rp->retrigLoop_ = loop;
      rp->retrigCount_ = loop;
      rp->retrigOffset_ = offset;
      rp->couldClick_ = SHOULD_KILL_CLICKS;
    } else {
      rp->retrig_ = false;
    }
  } break;
  case I_CMD_FLTR: {
    float cut =
        (value >> 8) / 255.0f; // cutoff frequency (FF=all pass, 0=none pass)
    float res =
        (value & 0xFF) / 255.0f; // resonance, aka Q (0=none) so default is FF00
    rp->cutoff_ = rp->baseFCut_ = fl2fp(cut);
    rp->reso_ = rp->baseFRes_ = fl2fp(res);
    if (rp->cutRamp_.Enabled()) {
      rp->cutRamp_.Disable();
      auto it = rp->activeUpdaters_.begin();
      while (it != rp->activeUpdaters_.end()) {
        if (*it == &rp->cutRamp_) {
          (*it)->Disable();
          it = rp->activeUpdaters_.erase(it);
          break;
        }
        it++;
      }
    }
    if (rp->resRamp_.Enabled()) {
      rp->resRamp_.Disable();
      auto it = rp->activeUpdaters_.begin();
      while (it != rp->activeUpdaters_.end()) {
        if (*it == &rp->resRamp_) {
          (*it)->Disable();
          it = rp->activeUpdaters_.erase(it);
          break;
        }
        it++;
      }
    }

  } break;
  case I_CMD_CRSH: {
    unsigned char drive = (value >> 8);
    unsigned char crush = (value & 0x0F);
    if (drive > 0)
      rp->drive_ = drive;
    if (crush > 0)
      rp->crush_ = crush;
  }
  default:
    break;
  };
};

etl::string<24> SampleInstrument::GetName() {
  Variable *v = FindVariable(SIP_SAMPLE);
  return v->GetString();
};

void SampleInstrument::Purge() {
  auto it = begin();
  for (size_t i = 0; i < size(); i++) {
    (*it)->Reset();
    it++;
  }
  source_ = NULL;
  /*    Variable *v=FindVariable(SIP_SAMPLE) ;
          if (v->GetInt()!=-1) {
              v->SetInt(-1) ;
          }*/
};

bool SampleInstrument::IsEmpty() {
  Variable *v = FindVariable(SIP_SAMPLE);
  return (v->GetInt() == -1);
};

int SampleInstrument::GetTable() {
  int result = table_->GetInt();
  if (result > TABLE_COUNT) {
    return VAR_OFF;
  }
  return result;
};

bool SampleInstrument::GetTableAutomation() { return tableAuto_->GetBool(); };

void SampleInstrument::GetTableState(TableSaveState &state) {
  memcpy(state.hopCount_, tableState_.hopCount_,
         sizeof(uchar) * TABLE_STEPS * 3);
  memcpy(state.position_, tableState_.position_, sizeof(int) * 3);
};

void SampleInstrument::SetTableState(TableSaveState &state) {
  memcpy(tableState_.hopCount_, state.hopCount_,
         sizeof(uchar) * TABLE_STEPS * 3);
  memcpy(tableState_.position_, state.position_, sizeof(int) * 3);
};

bool SampleInstrument::IsMulti() { return source_->IsMulti(); }

void SampleInstrument::EnableDownsamplingLegacy() {
  useDirtyDownsampling_ = true;
  Trace::Log("CONFIG", "Enabling downsampling legacy");
}
