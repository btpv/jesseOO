#include <FS.h>
#include <SD.h>
#include <Adafruit_BMP085.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
TinyGPSPlus gps;
Adafruit_BMP085 bmp;
#define SD_CS 5
#define R 12
#define G 13
#define B 14
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}
void logSDCard()
{
  String dataMessage =
      String(gps.date.day()) + "-" +
      String(gps.date.month()) + "-" +
      String(gps.date.year()) + "," +
      String(gps.time.hour()) + ":" +
      String(gps.time.minute()) + ":" +
      String(gps.time.second()) + "," +
      String(gps.altitude.meters()) + "," +
      String(gps.location.lat(),6) + "," +
      String(gps.location.lng(),6) + "," +
      String(bmp.readTemperature()) + "," +
      String(bmp.readPressure()) + "," +
      String(map(analogRead(4), 0, 4096, 0, 100)) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  SerialBT.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());
}
void writergb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
  analogWrite(R, r * (255 / 100));
  analogWrite(G, g * (255 / 100));
  analogWrite(B, b * (255 / 100));
}

void sendgps(String nmeaSentence)
{
  uint8_t checksum = 0;
  String send = "$PMTK";
  send += nmeaSentence;
  for (int i = 0; i < nmeaSentence.length(); i++)
  {
    checksum ^= nmeaSentence[i];
  }

  // Convert checksum to a 2-character hexadecimal string
  char hexChecksum[3]; // 2 characters + null terminator
  sprintf(hexChecksum, "%02X", checksum);
  send += "*" + String(hexChecksum) + "\r\n";
  for (int i = 0; i < send.length(); i++)
  {
    Serial2.write((char)send[i]);
  }
}
void setup()
{
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(27, OUTPUT);
  writergb(0,0,0, 50);
  tone(27, 1000);
  unsigned long sttime = millis();
  pinMode(4, INPUT);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1); // Hardware Serial of ESP32
  SerialBT.begin("ESP32test");     // Bluetooth device name

  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1)
    {
    }
  }
  while (millis() - sttime < 300)
    ;
  tone(27, 1500);
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS))
  {
    Serial.println("ERROR - SD card initialization failed!");
    return; // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.csv");
  if (!file)
  {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.csv", "millis,tmp,prs \r\n");
  }
  else
  {
    Serial.println("File already exists");
  }
  file.close();

  while (millis() - sttime < 600)
    ;
  // Serial2.print("$PMTK103*30\r\n");
  // Serial2.print("$PMTK314,-1*04\r\n");
  tone(27, 2000);
  delay(600);
  noTone(27);
}

unsigned long last = millis();
void loop()
{
  if (Serial2.available() > 0)
  {
    delay(100);
    while (Serial2.available() > 0)
    {
      char c = Serial2.read();
      gps.encode(c);
      Serial.print(c);
      SerialBT.print(c);
    }
  }
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    SerialBT.print(c);
    Serial2.print(c);
  }
  while (SerialBT.available() > 0)
  {
    char c = SerialBT.read();
    Serial.print(c);
    Serial2.print(c);
  }
  if (millis() - last > 1000)
  {
    logSDCard();
    last = millis();
    Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
    // sendgps("353,1,1,0");
    // sendgps("314,-1");
    sendgps("869,1,1");
  }
}