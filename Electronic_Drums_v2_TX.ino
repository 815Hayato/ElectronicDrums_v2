#include <Mozzi.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define CONTROL_RATE 64
#define NUM_PIEZO 4

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

int count_all;
bool count_all_flag;


//トリガリングに関わる定数・変数
const int THRESHOLD_CENTER = 100;
const int THRESHOLD_EDGE = 30;
const int PASS_CENTER = 100;
const int PASS_EDGE = 50;
const int COUNT = 8;
const int LIMITTIME = 30;
const int COOLTIME = 8;


//MIDI信号を送信
void MIDIOut(int gain_out, float radius){
  byte velocity = (byte)gain_out / 4;
  byte note = (byte)radius * 256;
  //Serial.print(velocity);
  //Serial.print(" ");
  //Serial.println(note);
  MIDI.sendNoteOn(note, velocity, 1);
}


//センサのピークから打点(半径方向)を計算
float cal_radius(int gain_center, int gain_edge){
  float ratio = gain_edge / gain_center;
  if(ratio < 0.27){
    ratio = 0.27;
  }else if(ratio > 1.11){
    ratio = 1.11;
  }
  float radius = (int)(ratio-0.27) / 1.05;
  return radius;
}

//センサのピーク(と打点)からゲインを計算
int cal_gain(float radius, int gain_center, int gain_edge){ //要改善
  int gain_out = (int) gain_center + (gain_edge - gain_center)/0.8*radius;
  return gain_out;
}


void SerialGain(){
  for(int i=0;i<NUM_PIEZO;i++){
    Serial.print(piezo[i].gain);
    Serial.print(",");
  }
}


void setup() {
  Serial.begin(9600);
  MIDI.begin(1);
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].past = 0;
    piezo[i].delta_past = 0;
    piezo[i].count_flag = false;
    piezo[i].cooltime = 0;
    piezo[i].gain_ok = false;
  }
}

void loop() {

  ////////////////////////////////////////////////////////
  //センサの読み取り・ピークの検出
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].present = analogRead(i);
    piezo[i].delta_present = piezo[i].present - piezo[i].past;

    if(piezo[i].count_flag==false){
      if(piezo[i].cooltime<COOLTIME){
        piezo[i].cooltime += 1;
      }

      int threshold = 0;
      if(i==0){
        threshold = THRESHOLD_CENTER;
      }else{
        threshold = THRESHOLD_EDGE;
      }

      if(piezo[i].past>threshold && piezo[i].cooltime>=COOLTIME){
        if(piezo[i].delta_present<0 && piezo[i].delta_past>=0){
          piezo[i].count = 1;
          piezo[i].count_flag = true;
          if(count_all_flag==false){
            count_all_flag = true;
            count_all = 1;
          }
          piezo[i].gain_predict = piezo[i].past;
        }
      }

    }else if(piezo[i].count_flag==true && count_all_flag==true){
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
  }

  ////////////////////////////////////////////////////////
  //打点・ゲインの計算・MIDI出力
  if(count_all_flag==true){
    count_all += 1;

    int gain_ok_all = 0;
    int gain_edge = 0;
    int gain_center = 0;
    float radius = 0;
    int gain_out = 0;
    for(int i=0;i<NUM_PIEZO;i++){
      gain_ok_all += piezo[i].gain_ok;
    }

    if(count_all<LIMITTIME && gain_ok_all==4){ //<1>LIMITTIME以内に全てのセンサがピークを検出した場合
      gain_center = piezo[0].gain;
      for(int i=1;i<NUM_PIEZO;i++){
        gain_edge = max(gain_edge,piezo[i].gain);
      }

      //打点位置(半径方向)・強さを推定
      radius = cal_radius(gain_center, gain_edge);
      gain_out = cal_gain(radius, gain_center, gain_edge);

      //SerialGain();

      //MIDI信号として送信
      MIDIOut(gain_out,radius);

      for(int i=0;i<NUM_PIEZO;i++){
        piezo[i].gain_ok = false;
        piezo[i].gain_predict = 0;
        piezo[i].gain = 0;
        piezo[i].count = 0;
        piezo[i].count_flag = 0;
      }
      count_all = 0;
      count_all_flag = false;

    }else{ //LIMITIMEを超過
      if(gain_ok_all >= 2){ //<2>二つ以上のセンサがピークを検出した場合
        gain_center = piezo[0].gain;
        for(int i=1;i<NUM_PIEZO;i++){
          gain_edge = max(gain_edge,piezo[i].gain);
        }

        //打点位置(半径方向)・強さを推定
        radius = cal_radius(gain_center, gain_edge);
        gain_out = cal_gain(radius, gain_center, gain_edge);

        //SerialGain();

        //MIDI信号として送信
        MIDIOut(gain_out,radius);

      }else if(gain_ok_all==1){ //<3>一つのセンサのみがピークを検出した場合
        for(int i=0;i<NUM_PIEZO;i++){ //ピークがある閾値(PASS_CENTER/PASS_EDGE)を超えていれば一打にカウントする
          if(i==0 && piezo[i].gain>=PASS_CENTER){
            //SerialGain();
            MIDIOut(piezo[0].gain,0);
            break;
          }else if(i!=0 && piezo[i].gain>=PASS_EDGE){
            //SerialGain();
            MIDIOut(piezo[i].gain,0.8);
            break;
          }
        }
      }

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
  }

  ///////////////////////////////////////////////////////////////
  //変数を更新
  for(int i=0;i<NUM_PIEZO;i++){
    piezo[i].past = piezo[i].present;
    piezo[i].delta_past = piezo[i].delta_present;
  }

}
