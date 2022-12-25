#include <DNNRT.h>
#include <SDHCI.h>
#include <stdio.h>    // for sprintf
#include <Camera.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_ILI9341.h>
// #include <BmpImage.h>

#define BAUDRATE (115200)

// 画像パラメータ
#define OFFSET_WIDTH (0)
#define OFFSET_HEIGHT (0)
#define CLIP_WIDTH (320)
#define CLIP_HEIGHT (240)
#define DNN_WIDTH (160)
#define DNN_HEIGHT (120)

// ディスプレイ
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

SDClass SD;
DNNRT dnnrt;
// BmpImage BMP;
DNNVariable input(DNN_WIDTH* DNN_HEIGHT);

// 0:none  1:human  2:vehicle
const char label[3][10] = { "none", "human", "vehicle"};

// LCDに文字と背景を表示
void putStringOnLcd(String str, int color) {
  int len = str.length();
  display.fillRect(0,0, 320, 240, ILI9341_BLACK);　// 背景
  display.setTextSize(7);       // 文字サイズ
  int sx = 140 - len/2*42;      // 表示X座標
  display.setCursor(sx, 90);　  // 表示位置
  display.setTextColor(color);  // 表示色
  display.println(str);　       // 表示文字
}

/* エラーメッセージ表示 */

void printError(enum CamErr err) {
  Serial.print("Error: ");
  switch (err) {
    case CAM_ERR_NO_DEVICE:
      Serial.println("No Device");
      break;
    case CAM_ERR_ILLEGAL_DEVERR:
      Serial.println("Illegal device error");
      break;
    case CAM_ERR_ALREADY_INITIALIZED:
      Serial.println("Already initialized");
      break;
    case CAM_ERR_NOT_INITIALIZED:
      Serial.println("Not initialized");
      break;
    case CAM_ERR_NOT_STILL_INITIALIZED:
      Serial.println("Still picture not initialized");
      break;
    case CAM_ERR_CANT_CREATE_THREAD:
      Serial.println("Failed to create thread");
      break;
    case CAM_ERR_INVALID_PARAM:
      Serial.println("Invalid parameter");
      break;
    case CAM_ERR_NO_MEMORY:
      Serial.println("No memory");
      break;
    case CAM_ERR_USR_INUSED:
      Serial.println("Buffer already in use");
      break;
    case CAM_ERR_NOT_PERMITTED:
      Serial.println("Operation not permitted");
      break;
    default:
      break;
  }
}

void setup() {
  CamErr err;
  Serial.begin(115200);
  
  // ピンセットアップ
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(0, OUTPUT);
  
  // SDセットアップ
  while (!SD.begin()) 
  {
    Serial.println("Insert SD card");
    putStringOnLcd("Insert SD card", ILI9341_RED); 
  }
  
  // SDカードにある学習済モデルの読み込み
  File nnbfile = SD.open("model.nnb");
  // 学習済モデルでDNNRTを開始
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    putStringOnLcd("dnnrt.begin failed" + String(ret), ILI9341_RED);
    return;
  }
  
  display.begin();        // 液晶ディスプレイの開始
  display.setRotation(3); // ディスプレイの向きを設定

  // カメラ開始
  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /*  
  // カメラストリーミング開始
  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }
  */
  
  // オートホワイトバランス
  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  // 画像フォーマット指定
  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QQVGA_H,         // 160ピクセル
     CAM_IMGSIZE_QQVGA_V,         // 120ピクセル
     CAM_IMAGE_PIX_FMT_YUV422);   // YUV422フォーマット
  if (err != CAM_ERR_SUCCESS) {
      printError(err);
    }
}

void loop() {
  
  // 画像取得
  Serial.println("call takePicture()");
  CamImage img = theCamera.takePicture();

  // 画像取得成功
  if (img.isAvailable())  {
    Serial.print("Image data size = ");
    Serial.println(img.getHeight());
    Serial.print("Image data size = ");
    Serial.println(img.getWidth());
    
    // 推論のための画像加工
    uint16_t* imgbuf = (uint16_t*)img.getImgBuff();
    float *dnn_input = input.data();
    for (int n = 0; n < DNN_WIDTH * DNN_HEIGHT; ++n) { 
      dnn_input[n] = (float)(((imgbuf[n] & 0xf000) >> 8) 
                      | ((imgbuf[n] & 0x00f0) >> 4))/256.; // 画像データを0.0~1.0に正規化
    }
    dnnrt.inputVariable(input, 0);                 // 入力データを設定
    dnnrt.forward();                               // 推論を実行
    DNNVariable output = dnnrt.outputVariable(0);  // 出力を取得

    // 確からしさがもっとも高いインデックス値を取得
    int index = output.maxIndex();
    Serial.print("Result: ");
    Serial.println(index);
    Serial.println("Probability: " + String(output[index]));
    
    if (index == 0) {
      // SPRESENSE LED
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED0, HIGH);
      // 通知LED 消灯
      digitalWrite(0, LOW);
      // LCD表示
      putStringOnLcd("Safe", ILI9341_GREEN);
    }    
    if (index == 1) {
      // SPRESENSE LED
      digitalWrite(LED0, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED1, HIGH);      
      // 通知LED 3秒点灯
      digitalWrite(0, HIGH);
      // LCD表示
      putStringOnLcd("Human" ,ILI9341_BLUE); 
      delay(3000);
    }
    if (index == 2) {
      // SPRESENSE LED
      digitalWrite(LED0, LOW);
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      // 通知LED 3秒点灯
      digitalWrite(0, HIGH);
      // LCD表示
      putStringOnLcd("Vehicle", ILI9341_RED);
      delay(3000);      
    }
  }
  // 画像取得失敗     
  else {
    Serial.println("Failed to get video stream image");
  }
  sleep(1);
  }