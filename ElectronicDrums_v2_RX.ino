#include <MIDI.h>
#include <Mozzi.h>
#include <Sample.h>
#include <samples/tomc.h>
#include <samples/tome.h>

MIDI_CREATE_DEFAULT_INSTANCE();

Sample <tomc_NUM_CELLS, AUDIO_RATE> tomc(tomc_DATA); //タム(センター)の音声データ
Sample <tome_NUM_CELLS, AUDIO_RATE> tome(tome_DATA); //タム(エッジ)の音声データ

#define CONTROL_RATE 64

const float BasicFrequency_c = (float) tomc_SAMPLERATE / (float) tomc_NUM_CELLS;  // 音源(center)を再生するサンプリング周波数
const float BasicFrequency_e = (float) tome_SAMPLERATE / (float) tome_NUM_CELLS;  // 音源(edege)を再生するサンプリング周波数
bool mode; //センターかエッジか(true→センター、false→エッジ)
byte volume = 0;

void handleNoteOn(byte channel, byte note, byte velocity){
  volume = velocity;

  if(note<128){
    mode = true;
    tomc.start();
  }else{
    mode = false;
    tome.start();
  }

}

void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.begin(1);
  startMozzi(CONTROL_RATE);
}

void updateControl(){
  float pitch = mozziAnalogRead(0);
  float freq_c = BasicFrequency_c * pow(2, pitch/512.000) / 2.00;
  float freq_e = BasicFrequency_e * pow(2, pitch/512.000) / 2.00;
  tomc.setFreq(freq_c);
  tome.setFreq(freq_e);
}

AudioOutput updateAudio(){
  if(mode==true){
    return (tomc.next()*volume) >> 8;
  }else if(mode==false){
    return (tome.next()*volume) >> 8;
  }
}


void loop() {
  MIDI.read();
  audioHook();
}
