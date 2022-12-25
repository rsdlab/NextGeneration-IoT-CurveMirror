#include <SDHCI.h>
#include <stdio.h>    // for sprintf
#include <Camera.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define BAUDRATE                (115200)

// 撮影枚数
#define TOTAL_PICTURE_COUNT     (10000)

// SDカード
SDClass  theSD;
int take_picture_count = 0;

// LCD
#define TFT_CS -1 
#define TFT_RST 8 
#define TFT_DC  9
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

/* エラーメッセージ表示 */
void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
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

/* カメラコールバック */
void CamCB(CamImage img)
{
  // 画像取得可能
  if (img.isAvailable())
    {
      // RGB565フォーマット変換
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
      tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 160, 120);
      
      // 画像データ表示
      Serial.print("Image data size = ");
      Serial.print(img.getImgSize(), DEC);
      Serial.print(" , ");
      Serial.print("buff addr = ");
      Serial.print((unsigned long)img.getImgBuff(), HEX);
      Serial.println("");
    }
  // 画像取得失敗
  else
    {
      Serial.println("Failed to get video stream image");
    }
}

void setup()
{
  CamErr err;

  tft.begin(40000000); 
  tft.setRotation(3);
  
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; // 接続待機
    }

  // SD設定
  while (!theSD.begin()) 
    {
      Serial.println("Insert SD card.");
    }
  
  // カメラ開始
  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
  {
    printError(err);
  }
  
  // カメラストリーミング
  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
  
  // オートホワイトバランス
  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
  
  // 画像フォーマット設定
  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QQVGA_H,   // 160ピクセル
     CAM_IMGSIZE_QQVGA_V,   // 120ピクセル
     CAM_IMAGE_PIX_FMT_JPG  // JPGフォーマット
     );
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
}

void loop()
{
  sleep(0.5); // 待機
  
  // 画像撮影開始
  if (take_picture_count < TOTAL_PICTURE_COUNT)
    {
      // 画像撮影
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();
      
      // 画像取得成功
      if (img.isAvailable())
        {
          // ファイル名設定
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");
          
          // グレースケール
          img.convertPixFormat(CAM_IMAGE_PIX_FMT_GRAY);
          
          // 同名画像削除と画像書き込み
          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
        }
      // 画像取得失敗
      else
        {
          Serial.println("Failed to take picture");
        }
    }
  // 撮影終了
  else if (take_picture_count == TOTAL_PICTURE_COUNT)
    {
      Serial.println("End.");
      theCamera.end();
    }
  take_picture_count++;
}