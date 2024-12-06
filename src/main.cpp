#include <FS.h>
#include <SD.h>
#include <Adafruit_BMP085.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
Adafruit_BMP085 bmp;
#define SD_CS 5

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
void logSDCard() {
  String dataMessage = String(millis()) + "," + String(bmp.readTemperature()) + "," + String(bmp.readPressure()) +","+String(map(analogRead(4),0,4096,0,100))+ "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());
}

void setup()
{
  pinMode(4,INPUT);
  pinMode(27,OUTPUT);
  Serial.begin(9600);
  tone(27,1000);
  unsigned long sttime = millis();
  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1)
    {
    }
  }
  while (millis()-sttime < 300);
  tone(27,1500);
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.csv");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.csv", "millis,tmp,prs \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
  delay(300);
  tone(27,2000);
  delay(600);
  noTone(27);
}

void loop()
{
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");
  
  Serial.print("analog = ");
  Serial.print(analogRead(4));
  Serial.println("");
  logSDCard();
  delay(1000);
}