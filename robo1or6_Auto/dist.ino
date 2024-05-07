//超音波センサ
int search() {    //障害物を探索
  int diff_dist;  //前の距離との差

  diff_dist = dist_G - before_dist;  //前の距離との差

  //距離が50cm以内かつ，前の距離との差が±5以内の時 ↓
  if (0 < dist_G && dist_G < 50 && -5 <= diff_dist && diff_dist <= 5) {
    return 1;  //障害物あり：１を返す
  }
  return 0;  //障害物なし：0を返し続ける
}

int judge_RorC() {                  //，ロボかコップか判断
  if (before_dist - dist_G != 0) {  //前の距離の差が0じゃない場合↓------------------------------------------
    count_not0++;
  }
  if (count_not0 > 8) {  //回以上差があった↓(回数は調整)----------------------------------------------------------------------------------
    count_not0 = 0;      //初期化
    return 1;            //ロボなので1を返す
  }
  return 0;  //回以下の時はずっと０を返す（カップだと予測される）
}

int Cup_dist() {                   //カップをつかめているか判断する関数
  if (0 < dist_G && dist_G < 4) {  //------------------------------------------------------------
    buzzer.play(">c32");           //一回ブザー音を鳴らす
    return 1;                      //障害物あり：１を返す
  }
  return 0;  //障害物なし：0を返し続ける
}

//距離を計測(単位はcm)
int distance() {
  unsigned long interval;  //Echoのパルス幅(μs)
  int dist;                //距離(cm)

  digitalWrite(trig, HIGH);  //10μsのパルスを超音波センサのTrigピンに出力
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  interval = pulseIn(echo, HIGH, 23071);              //Echo信号がHIGHである時間(μs)を計測
  dist = (0.61 * 25 + 331.5) * interval / 10000 / 2;  //距離(cm)に変換

  //delay(60);  //trigがHIGHになる間隔を60ms以上空ける（超音波センサの仕様）

  return dist;
}

int moving_average(int x) {
  static int count = 0;
  static int sum = 0;
  static int data[] = { 0, 0, 0, 0, 0 };

  if (count == 5) {
    count = 0;
  }

  sum -= data[count];  //古い値を削除
  data[count] = x;
  sum += data[count];  //新しい値を追加
  count++;

  return sum / 5;  //過去 5 件の移動平均を返す
}