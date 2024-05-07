//カラーセンサ
#include <Adafruit_TCS34725.h>  // カラーセンサライブラリのヘッダーファイル
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_60X);

unsigned int r_min, g_min, b_min;  // このグローバル変数はこのファイル内のみで使用
unsigned int r_max, g_max, b_max;

void CalibrationColorSensor() {
  unsigned long timeInit;
  unsigned int r, g, b, clr;

  tcs.begin();  // カラーセンサーのsetup

  motors.setSpeeds(60, 60);

  r_min = 30000;
  g_min = 30000;
  b_min = 30000;
  r_max = 0;
  g_max = 0;
  b_max = 0;

  timeInit = millis();

  while (1) {
    tcs.getRawData(&r, &g, &b, &clr);  // rowdataの取得

    if (r < r_min) r_min = r;
    if (g < g_min) g_min = g;
    if (b < b_min) b_min = b;
    if (r > r_max) r_max = r;
    if (g > g_max) g_max = g;
    if (b > b_max) b_max = b;

    if (millis() - timeInit > 2000)
      break;
  }
  motors.setSpeeds(0, 0);
}

void getRGB(float& r0, float& g0, float& b0) {
  unsigned int r, g, b, clr;

  tcs.getRawData(&r, &g, &b, &clr);  // rowdataの取得

  r0 = map(r, r_min, r_max, 0, 255);
  g0 = map(g, g_min, g_max, 0, 255);
  b0 = map(b, b_min, b_max, 0, 255);

  if (r0 < 0.0) r0 = 0.0;
  if (r0 > 255.0) r0 = 255.0;
  if (g0 < 0.0) g0 = 0.0;
  if (g0 > 255.0) g0 = 255.0;
  if (b0 < 0.0) b0 = 0.0;
  if (b0 > 255.0) b0 = 255.0;
}





//↓小林君の関数

//センサ監視
void colorSensorMonitor(int time) {
  static unsigned long _timePrev = millis();

  if (millis() - _timePrev >= time) {
    _timePrev = millis();

    //色の観測
    /*getColorSensor(&red, &green, &blue);
    Serial.print("color : ");
    Serial.print(red);
    Serial.print(' ');
    Serial.print(green);
    Serial.print(' ');
    Serial.println(blue);*/

    //色の判定
    // Serial.println(identifyColor());
  }
}

//センサのセットアップ
void setupColorSensor() {
  tcs.begin();  //カラーセンサのセットアップ
  //狩野
  // r_min = 62;
  // g_min = 54;
  // b_min = 51;
  // r_max = 255;
  // g_max = 255;
  // b_max = 255;

  //小林君
  // r_min = 47;
  // g_min = 66;
  // b_min = 67;
  // r_max = 255;
  // g_max = 255;
  // b_max = 255;

  //宇和佐君
  r_min = 44;
  g_min = 65;
  b_min = 69;
  r_max = 255;
  g_max = 255;
  b_max = 255;
}

//センサ値取得
void getColorSensor(float* _red, float* _green, float* _blue) {
  //センサ値取得
  float r, g, b = 0;
  tcs.getRGB(&r, &g, &b);

  //キャリブレーション
  r_min = min(r_min, r);
  g_min = min(g_min, g);
  b_min = min(b_min, b);
  r_max = max(r_max, r);
  g_max = max(g_max, g);
  b_max = max(b_max, b);

  //線形変換
  *_red = map(r, r_min, r_max, 0, 255);
  *_green = map(g, g_min, g_max, 0, 255);
  *_blue = map(b, b_min, b_max, 0, 255);
}

int identify_color(float r, float g, float b) {
  if (200 < r && 200 < g && 200 < b) {
    return WHITE;  //白
  } else if (r < 50 && g < 50 && b < 50) {
    return BLACK;  //黒
  } else if (100 < r && g < 50 && b < 50) {
    return RED;  //赤
                 // } else if (r < 50 && g < 50 && 70 < b) {
  } else if (r < 70 && g < 70 && 50 < b) {
    return BLUE;  //青
  } else {
    return OTHER;  //その他
  }
}


//色の判別(N近傍法)(小林君のやつ)
int identifyColor(float r, float g, float b)
{
  //定数
  const int color_limit = 100; //カラー距離閾値

  int color = -1;
  float minDistance = 99999;

  //全色との距離を計算
  for (int i = 0; i < colorNum; i++)
  {
    float distance = sqrt(pow(r - colorPos[i][0], 2)
      + pow(g - colorPos[i][1], 2)
      + pow(b - colorPos[i][2], 2));
    
    if (distance < minDistance)
    {
      minDistance = distance;
      color = i;
    }
  }

  //色検知
  if (abs(minDistance) <= color_limit)
    return color;
  else
    return -1; //閾値に近づいていない
}
