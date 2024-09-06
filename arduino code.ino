#include <SoftwareSerial.h>
#include "HX711.h"
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;
Servo myservo1;

// Define pins
uint8_t dataPin = 2; // For ESP32
uint8_t clockPin = 3; // For ESP32
const int button = A1;

// Variables
int i = 0, x = 0, st = 0;
float wt = 0;
float targetWeight = 1000; // Target weight in grams
float halfTargetWeight = targetWeight / 2;
const int dhtPin = 4; // Pin for the DHT sensor
DHT dht(dhtPin, DHT22); 

void setup() {
  Serial.begin(9600);
  Serial.println(HX711_LIB_VERSION);
  pinMode(button, INPUT);

  myservo1.attach(9);
  myservo1.write(0);
  delay(2000);
  myservo1.write(90);
  delay(2000);
  myservo1.write(0);

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("WELCOME TO ");
  lcd.setCursor(2, 1);
  lcd.print("LOAD CELL");
  delay(2000);
  lcd.clear();

  scale.begin(dataPin, clockPin);
  scale.tare(); // Reset the scale to 0
  lcd.setCursor(0, 0);
  lcd.print("EMPTY THE TRAY ");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("units=");
  lcd.print(scale.get_units(10));
  delay(100);
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("PUT 1000 GRAMS");
  delay(2000);
  scale.calibrate_scale(1000, 5); // Calibrate the scale with 1000 grams
  lcd.setCursor(0, 0);
  lcd.print("units=");
  lcd.print(scale.get_units(10));
  delay(2000);
  lcd.clear();

  scale.set_unit_price(0.031415); 
  // Initialize the DHT sensor
  dht.begin();
}

void loop() {
  x = digitalRead(button);

  wt = scale.get_units(10);
  delay(250);

  lcd.setCursor(0, 0);
  lcd.print("LOAD=");
  lcd.print(wt);
  lcd.print("Grms");

  // Check compost weight and control the servo motor
  if (wt >= halfTargetWeight && wt < targetWeight) {
    lcd.setCursor(0, 0);
    lcd.print("COMPOST HALFWAY");
    myservo1.write(0); // Stop the motor or change its position
  } else if (wt >= targetWeight) {
    lcd.setCursor(0, 0);
    lcd.print("COMPOST FULL");
    myservo1.write(0); // Stop the motor
    lcd.setCursor(0, 1);
    lcd.print("COMPOST READY");
    delay(3000);
    lcd.clear();
    st = 0;
  } else {
    if (st > 2) {
      lcd.setCursor(0, 0);
      lcd.print("COMPOST PROCESS ");
      for (i = 0; i < 30; i++) {
        myservo1.write(0);
        delay(2000);
        myservo1.write(180);
        delay(2000);
      }
      lcd.setCursor(0, 0);
      lcd.print("PROCESS STOP");
      lcd.setCursor(0, 1);
      lcd.print("COMPOST READY");
      delay(3000);
      lcd.clear();
      st = 0;
    }
  }

  if (x == LOW) {
    lcd.setCursor(0, 1);
    lcd.print("WAIT ");
    st = 5;
    delay(2000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("PRESS TO START ");
  }

  // Read and display temperature and humidity
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius by default

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C H:");
  lcd.print(humidity);
  lcd.print("%");
}
