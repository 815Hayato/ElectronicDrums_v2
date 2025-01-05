#include <MIDI.h>

#define CONTROL_RATE 64
#define NUM_PIEZO 4

MIDI_CREATE_DEFAULT_INSTANCE();


//入力センサ(圧電素子)に関するデータの構造体
struct Piezo {
  int present;
  int past;
  int delta_present;
  int delta_past;
  int count;
  int cooltime;
  bool count_flag;
  int gain_predict;
  bool gain_ok;
  int gain;
};
Piezo piezo[NUM_PIEZO];

//トリガリングに関わる定数・変数
const int THRESHOLD_CENTER = 50;
const int THRESHOLD_EDGE = 30;
const int PASS_CENTER = 100;
const int PASS_EDGE = 50;
const int COUNT = 50;
const int LIMITTIME = 100;
const int COOLTIME = 50;
int count_all;
bool count_all_flag;


//MIDI信号を送信
void MIDIOut(int gain_out, int radius){
  if(gain_out < 0){
    gain_out = 0;
  }else if(gain_out > 255){
    gain_out = 255;
  }

  if(radius < 0){
    radius = 0;
  }else if(radius > 255){
    radius = 255;
  }

  byte velocity = (byte)gain_out;
  byte note = (byte)radius;

  MIDI.sendNoteOn(note, velocity, 1);
}

//各入力信号からピーク(トリガリング開始地点)を検出
void signal_check(int i){
  int threshold = 0;
  if(i==0){
    threshold = THRESHOLD_CENTER;
  }else{
    threshold = THRESHOLD_EDGE;
  }

  if(piezo[i].past>threshold && piezo[i].cooltime>=COOLTIME){
    if(piezo[i].delta_present<0 && piezo[i].delta_past>=20){
      piezo[i].count = 1;
      piezo[i].count_flag = true;
      piezo[i].gain_predict = piezo[i].past;
      if(count_all_flag==false){
        count_all_flag = true;
        count_all = 1;
      }
    }
  }
}

//各入力信号からゲインを判定
void trigger_check(int i){
  if(piezo[i].gain_predict>=piezo[i].present){
    piezo[i].count += 1;
    if(piezo[i].count==COUNT){
      piezo[i].count = 0;
      piezo[i].count_flag = false;
      piezo[i].gain = piezo[i].gain_predict;
      piezo[i].cooltime = 0;
      piezo[i].gain_ok = true;
    }
  }else{
    piezo[i].gain_predict = piezo[i].present;
    piezo[i].count = 1;
  }
}

//４つの入力信号のゲイン検出状況を比較
void gain_compare(){
  int gain_ok_all;
  for(int i=0;i<NUM_PIEZO;i++){
    gain_ok_all += piezo[i].gain_ok;
  }

  if(count_all<LIMITTIME){
    count_all += 1;
  }

  if(count_all<LIMITTIME && gain_ok_all==4){
    calc();
  }else if(count_all==LIMITTIME){
    calc();
  }
}

//トリガリング終了時に関係する値をトリガリング前に戻す
void trigger_reset(){
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].gain_ok = false;
    piezo[i].gain_predict = 0;
    piezo[i].gain = 0;
    piezo[i].count = 0;
    piezo[i].count_flag = 0;
  }
  count_all = 0;
  count_all_flag = false;
}

//各入力信号のゲインから、打点位置・ゲインを総合的に判断
void calc(){
  int gain_center = piezo[0].gain;
  int gain_edge = 0;
  float ratio;
  int radius;
  int gain_out;

  for(int i=1;i<NUM_PIEZO;i++){
    gain_edge = max(gain_edge,piezo[i].gain);
  }

  if(gain_center>0 && gain_edge>0){
    ratio = (float)gain_edge / (float)gain_center;
    if(ratio < 0.27){
      ratio = 0.27;
    }
    else if(ratio > 1.11){
      ratio = 1.11;
    }
    radius  = (ratio-0.27) / 1.05 * 256;
    gain_out = gain_center + (gain_edge - gain_center) / 3.2 * radius / 256;
    MIDIOut(gain_out, radius);
  }else if(gain_center>PASS_CENTER){
    radius = 0;
    gain_out = gain_center/4;
    MIDIOut(gain_out, radius);
  }else if(gain_edge>PASS_EDGE){
    radius = 200;
    gain_out = gain_edge/4;
    MIDIOut(gain_out, radius);
  }
  trigger_reset();
}

//初期設定
void setup(){
  MIDI.begin(1);
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].past = 0;
    piezo[i].delta_past = 0;
    piezo[i].count_flag = false;
    piezo[i].cooltime = 0;
    piezo[i].gain_ok = false;
  }
}

//繰り返し処理
void loop(){
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].present = analogRead(i);
    piezo[i].delta_present = piezo[i].present - piezo[i].past;
  }

  for(int i=0;i<NUM_PIEZO;i++){
    if(piezo[i].cooltime<COOLTIME){
      piezo[i].cooltime += 1;
    }else{
      if(piezo[i].count_flag==false){
        signal_check(i);
      }else{
        trigger_check(i);
      }
    }
  }

  if(count_all_flag==true){
    gain_compare();
  }

  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].past = piezo[i].present;
    piezo[i].delta_past = piezo[i].delta_present;
  }
}
