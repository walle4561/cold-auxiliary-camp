#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

MAX30105 particleSensor;
SSD1306AsciiWire oled;

#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t spo2; //SPO2 值
int8_t validSPO2; //顯示 SPO2 計算是否有效的指示器
int32_t heartRate; //心率值
int8_t validHeartRate; //顯示心率計算是否有效的指標

void setup()
{
  Serial.begin(115200);

  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Arial14);

  // 初始化感測器
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  particleSensor.setup(60, 4, 2, 100, 411, 4096); //Configure sensor with these settings
}

void loop()
{

  //read the first 50 samples, and determine the signal range
  for (byte i = 0 ; i < 100 ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //在前 50 個樣本（樣本的前 4 秒）後計算心率和 SpO2
  maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //從 MAX30102 連續採樣。 每 1 秒計算一次心率和 SpO2
  while (1)
  {
    //將前 25 組樣本轉儲到內存中，並將後 25 組樣本移到頂部
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //在計算心率之前取25組樣本
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //檢查是否有新數據
        particleSensor.check(); //檢查傳感器是否有新數據

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);

    }

    //收集 25 個新樣本後重新計算 HR 和 SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    printToScreen();
  }
}

void printToScreen() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.set2X();
  if (validSPO2 && validHeartRate) {
    oled.print(F("HR: ")); oled.println(heartRate, DEC);
    oled.print(F("SPO2: ")); oled.println(spo2, DEC);
  } else {
    oled.print(F("Not valid"));
  }
}
