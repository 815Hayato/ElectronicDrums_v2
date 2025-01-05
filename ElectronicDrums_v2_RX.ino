#include <MIDI.h>
#include <Mozzi.h>
#include <Sample.h>
#include <samples/tomc.h>
#include <samples/tome.h>

#define CONTROL_RATE 64

MIDI_CREATE_DEFAULT_INSTANCE();

Sample <tomc_NUM_CELLS, AUDIO_RATE> tomc(tomc_DATA); //音声(中心を叩いたときの音)を扱う
Sample <tome_NUM_CELLS, AUDIO_RATE> tome(tome_DATA); //音声(端を叩いたときの音)を扱う

const float BasicFrequency_c = (float) tomc_SAMPLERATE / (float) tomc_NUM_CELLS;
const float BasicFrequency_e = (float) tome_SAMPLERATE / (float) tome_NUM_CELLS;
bool mode;
byte volume = 0;

//MIDI信号を受信した際の処理
void handleNoteOn(byte channel, byte note, byte velocity){
  volume = velocity; //ゲイン

  if(note>128){ //打点位置(中心からの距離)
    mode = true;
    tomc.start(); //音声(中心を叩いたときの音)を再生させる
  }else{
    mode = false;
    tome.start(); //音声(端を叩いたときの音)を再生させる
  }
}

//初期設定
void setup(){
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.begin(1);
  startMozzi(CONTROL_RATE);
}

//繰り返し処理(入力)
void updateControl(){
  float pitch = mozziAnalogRead(0); //フェーダからの入力
  float freq_c = BasicFrequency_c * pow(2, pitch/512.000) / 2.00;
  float freq_e = BasicFrequency_e * pow(2, pitch/512.000) / 2.00;
  tomc.setFreq(freq_c); //音声の再生周波数を設定
  tome.setFreq(freq_e);
}

//繰り返し処理(出力)
AudioOutput updateAudio(){
  if(mode==true){
    return (tomc.next()*volume) >> 8; //音声(中心を叩いたときの音)を出力
  }else if(mode==false){
    return (tome.next()*volume) >> 8; //音声(端を叩いたときの音)を出力
  }
}

void loop(){
  MIDI.read();
  audioHook();
}
