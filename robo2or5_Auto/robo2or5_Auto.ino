//ロボ➁or➄
#include <Wire.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <LSM303.h>
#include <ZumoBuzzer.h>

ZumoBuzzer buzzer;
LSM303 compass;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
//超音波センサ
const int trig = 2;  //TrigピンをArduinoの2番ピンに
const int echo = 4;  //EchoピンをArduinoの4番ピンに

//カラーセンサ
#define colorNum 4
unsigned int colorPos[colorNum][3] = {
  { 255, 255, 255 },  //白:0
  { 12, 25, 28 },     //黒:1
  { 121, 2, 4 },      //赤:2
  { 1, 32, 96 },      //青:3
};
#define WHITE 0  //白
#define BLACK 1  //黒
#define RED 2    //赤
#define BLUE 3   //青
#define OTHER 4  //その他
int color;

//グローバル変数
float red_G, green_G, blue_G;                     // カラーセンサで読み取ったRGB値（0-255）
static int mode_G;                                // タスクのモード
unsigned long timeInit_G, timeNow_G, timePrev_G;  //  スタート時間，経過時間
int motorR_G, motorL_G;                           // 左右のZumoのモータに与える回転力
float mx_G = 0, my_G = 0, mz_G = 0;               //地磁気センサで読み取った値
float ax_G = 0, ay_G = 0, az_G = 0;
float heading_G = 0;
static int dist_G = 0;
static int before_dist = 0;  //超音波センサ

float sum_e = 0;     //turnTo関数で使用
float rotSpeed = 0;  //worldTurn関数で使用
int count_not0 = 0;  //judge_RorC関数で使用，前の距離の差が０じゃない回数
int g_count = 0;     //task_Red,Blue で使用goal_count 探索位置（自陣か真ん中）のきりかえ

//ループ内で使用
int first = 0;  //最初のみ，初期位置の角度を取得する用
int Color = 0;  //自陣の色　赤：１，青：２
float kakudo = 0;
static unsigned long dist60_time;  //超音波センサdelay(60)計測用
// float kakudo180 = 0;  //初期位置と，反対方向の角度

//キャリブレーション内臓時
int recet = 0;


void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(trig, OUTPUT);  //trigを出力ポートに設定
  pinMode(echo, INPUT);   //echoを入力ポートに設定
  // setupCompass();         //地磁気センサ
  setupCompass_2();    //キャリブレーション内臓化した時
  setupColorSensor();  //カラーセンサ

  for (int i = 0; i < 3; i++) {  //キャリブレーション内臓化したとき
    button.waitForButton();
    calibrationCompass(i);
  }
  Compass_sendData();  //↑で計測した最小，最大値を代入

  //ボタン→キャリブレーション（3回）→ボタン→ループに入る
  button.waitForButton();  // Zumo buttonが押されるまで待機
  //CalibrationColorSensor();  // カラーセンサーのキャリブレーション
  //button.waitForButton();

  timeInit_G = millis();
  dist60_time = millis();
  timePrev_G = 0;
  mode_G = 0;
  motorR_G = 0;
  motorL_G = 0;
}

void loop() {
  //キャリブレーション内臓時↓↓
  if (button.isPressed() == true) {  //ボタンが押されている時は↓
    Color = 0;                       //色初期化でタスクに入らないようにする
    motorR_G = 0;                    //停止
    motorL_G = 0;
    recet = 1;  //リセットに値きりかえ
  } else {      //ボタンが押されてない時は↓
    if (recet == 1) {
      first = 0;               //一番最初の位置取得
      button.waitForButton();  //ユーザボタンが押されて離されるまで待機
      mode_G = 0;
      timeInit_G = millis();
      dist60_time = millis();
      timePrev_G = 0;
      // motorR_G = 0;
      // motorL_G = 0;
      recet = 0;  //初期化
    }
  }

  getColorSensor(&red_G, &green_G, &blue_G);  // カラーセンサでRGB値を取得(0-255)
  color = identifyColor(red_G, green_G, blue_G);
  getCompass();  //地磁気センサでmx_G, my_Gを取得

  timeNow_G = millis() - timeInit_G;  // 経過時間

  if (timeNow_G - dist60_time > 60) {
    dist_G = distance();  //超音波センサ
    if (dist_G == 0) {    //エラーでたら前の距離を代入
      dist_G = before_dist;
    }
    dist60_time = timeNow_G;  //時間更新
  }

  motors.setSpeeds(motorL_G, motorR_G);  // 左右モーターへの回転力入力
  sendData();                            // データ送信

  if (timeNow_G - timePrev_G < 100) {
    return;
  }

  if (first == 0) {
    kakudo = heading(mx_G, my_G);
    // Serial.println(kakudo);
    if (300 <= kakudo || kakudo <= 60) {          //ロボが北(0)→（27ぐらい）を向いていたら（printfで角度確認する？？）
      Color = 1;                                  //自陣は赤色
    } else if (130 <= kakudo && kakudo <= 230) {  //ロボが南(180)を向いていたら
      Color = 2;                                  //自陣は青色
    }

    first = 1;  //適当な数字を代入
  }

  if (Color == 1) {  //自陣：赤
    task_Red();
  } else if (Color == 2) {  //自陣：青
    task_Blue();
  }


  // Serial.println(mode_G);
  //  Serial.println(dist_G);
  // task_Red();
  // search();
  // task_Blue();
  // linetrace_1or6();
  // Serial.println(identify_color(red_G, green_G, blue_G) );

  // if (worldTurn(270)) {
  //   rotSpeed = 0;
  // }
  // motorL_G =  rotSpeed;
  // motorR_G =  - rotSpeed;

  timePrev_G = timeNow_G;
  before_dist = dist_G;  //1つ前の距離を保存しておく
}

// 通信方式2
void sendData() {
  static unsigned long timePrev = 0;
  static boolean flag_start = true;  // 最初の通信かどうか
  int inByte;

  // if文の条件： 最初の通信である || 最後のデータ送信から500ms経過 || (データ送信要求が来ている && 最後のデータ送信から50ms経過)
  if (flag_start == true || timeNow_G - timePrev > 500 || (Serial.available() > 0 && timeNow_G - timePrev > 50)) {
    flag_start = false;
    while (Serial.available() > 0) {  // 送信要求が複数来ていた場合は全て読み込む
      inByte = Serial.read();
    }

    //---------------何を送信するか後で！！！！---------------------------------------------------------

    Serial.write('H');
    Serial.write(mode_G);
    Serial.write((int)g_count);
    Serial.write((int)dist_G);
    Serial.write((int)red_G);
    Serial.write((int)green_G);
    Serial.write((int)blue_G);
    Serial.write((int)heading(mx_G, my_G));
    Serial.write((int)motorR_G);

    timePrev = timeNow_G;
  }
}
