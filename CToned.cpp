#include "CToned.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmain"
#include "IPlug_include_in_plug_src.h"
#pragma clang diagnostic pop
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;

enum EParams
{
  mWaveform = 0,
  mAttack,
  mDecay,
  mSustain,
  mRelease,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX = 1,
  kKeybY = 230
};

CToned::CToned(IPlugInstanceInfo instanceInfo)
:   IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1) {
  TRACE;
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);
  
  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);
  
  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);
  
  pGraphics->AttachControl(mVirtualKeyboard);
  
  // Waveform switch

  GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscillatorModes);
  GetParam(mWaveform)->SetDisplayText(0, "Sine"); // Needed for VST3, thanks plunntic
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 53, mWaveform, &waveformBitmap));
  
  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64);
  
  // Attack knob:

  GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 95, 34, mAttack, &knobBitmap));
  
  // Decay knob:
  
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 177, 34, mDecay, &knobBitmap));
  
  // Sustain knob:
  
  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 259, 34, mSustain, &knobBitmap));
  
  // Release knob:
  
  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 341, 34, mRelease, &knobBitmap));
  
  AttachGraphics(pGraphics);
  
  // CreatePresets();
  
  mMIDIReceiver.noteOn.Connect(this, &CToned::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &CToned::onNoteOff);
  
  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &CToned::onBeganEnvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &CToned::onFinishedEnvelopeCycle);

}

CToned::~CToned() {}

void CToned::CreatePresets() {
}

void CToned::ProcessDoubleReplacing(
                                       double** inputs,
                                       double** outputs,
                                       int nFrames)
{
  // Mutex is already locked .
  
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  
  processVirtualKeyboard();
  for (int i = 0; i < nFrames; ++i) {
    mMIDIReceiver.advance();
    int velocity = mMIDIReceiver.getLastVelocity();
    mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
    leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0;
  }
  
  mMIDIReceiver.Flush(nFrames);
}

void CToned::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelopeGenerator.setSampleRate(GetSampleRate());
}

void CToned::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  switch(paramIdx) {
    case mWaveform:
      mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
      break;
    case mAttack:
    case mDecay:
    case mSustain:
    case mRelease:
      mEnvelopeGenerator.setStageValue(static_cast<EnvelopeGenerator::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
      break;
  }
}

void CToned::ProcessMidiMsg(IMidiMsg* pMsg) {
  mMIDIReceiver.onMessageReceived(pMsg);
  mVirtualKeyboard->SetDirty();
}

void CToned::processVirtualKeyboard() {
  IKeyboardControl* virtualKeyboard = (IKeyboardControl*) mVirtualKeyboard;
  int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;
  
  if(lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
    
	  // The note number has changed from a valid key to something else (valid key or nothing)
    
	IMidiMsg midiMessage;
    midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
    mMIDIReceiver.onMessageReceived(&midiMessage);
  }
  
  if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
    
	//Send a "note on" message to the MIDI receiver:
    
	  IMidiMsg midiMessage;
    midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
    mMIDIReceiver.onMessageReceived(&midiMessage);
  }
  
  lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
}
