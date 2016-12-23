//Name: Ethan McAuliffe
//Date: 2016-12-17
//Project: Nixie Tube Clock
//Status: In Development

//Libraries
#include <Wire.h>

//Constants
#define serial 2
#define latch 3
#define clk 4
#define DSTPin 9
#define potPin A0
#define boostPin 5
#define rtc 0x68

//Variables
String currentTime; // current time formatted for the shift registers
int8_t daylight; //daylight savings offset
uint8_t cHour; //current hour
uint8_t pHour; //past hour data -- used to cycle the nixie tube clocks each hour
uint8_t cMinute; //current minute
uint8_t cSecond;  //current second
uint16_t potIn; //position of potentiometer
uint8_t brightness; //brightness of the nixie tubes

void setup() {
  Wire.begin();

  pinMode(serial, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(boostPin, OUTPUT);

  pinMode(DSTPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);
}

void loop() {
  //Read the analog value of the potentiometer and write as a pwm signal
  //This changes the brigtness of the nixie tubes and gives the option of turning the tubes off completely
  potIn = analogRead(potPin);
  delayMicroseconds(100);
  potIn <= 2 ? brightness = 0 : brightness = map(potIn, 3, 1023, 3, 255);
  analogWrite(boostPin, brightness);

  //set the past hour to the current hour data. This is used to determine when to cycle the nixie tubes
  pHour = cHour;

  //Interface with the RTC
  Wire.beginTransmission(rtc);
  Wire.write(0); //prepare rtc to be read from
  Wire.endTransmission();
  //read current second, minute and hour
  Wire.requestFrom(rtc, 7);
  cSecond = readTime((Wire.read())); //second
  cMinute = readTime(Wire.read() & 0x7f); //minute
  cHour = readTime(Wire.read() & 0x3f); //hour

  //if the daylight savings time switch is on, add add 1 hour to the current time
  digitalRead(DSTPin) == LOW ? (cHour == 23 ? daylight = -23 : daylight = 1) : daylight = 0;

  //takes current time and sends it to the shift registers
  currentTime = timeData(cHour + daylight, cMinute, cSecond); // current time binary data. Each nibble represents a digit.
  shiftTime(serial, latch, clk, currentTime); //send current time to the shift registers

  //cycles nixie tubes every hour that they are on
  if (brightness > 0 && pHour != cHour) {
    cycleTubes();
  }
}

//Functions
String timeData(uint8_t h, uint8_t m, uint8_t s) { //takes the current time and outputs the required data for shift register
  String secondsOnes = toNibble(String((s % 10), BIN));//converts ones digit to binary nibble
  String secondsTens = toNibble(String((s / 10 % 10), BIN));//converts tens digit to binary nibble
  String minutesOnes = toNibble(String((m % 10), BIN));
  String minutesTens = toNibble(String((m / 10 % 10), BIN));
  String hoursOnes = toNibble(String((h % 10), BIN));
  String hoursTens = toNibble(String((h / 10 % 10), BIN));

  //output a string of the all binary nibbles for each digit in the time
  String output = hoursTens + hoursOnes + minutesTens + minutesOnes + secondsTens + secondsOnes;
  return output;
}

String toNibble (String input) { //converts binary character string to a binary nibble
  String output = input;
  uint8_t lngth = output.length();
  for (uint8_t i = lngth; i < 4; i++) { //adds zeros before binary value until there is a whole nibble
    output = "0" + output;
  }
  return output;
}

void shiftTime (uint8_t serialPin, uint8_t latchPin, uint8_t clockPin, String inputTime) { //sends a binary string to shift registers
  digitalWrite(latchPin, LOW);//set latch pin low to prepare for data to be sent
  for (int8_t i = inputTime.length() - 1; i > -1; i--) { //iterates over entire length of input string
    byte currentChar = inputTime.charAt(i);//gets character "i" in the binary string
    if (currentChar == '1') { //if character is a one send a high signal to the shift registers
      digitalWrite(serialPin, HIGH);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(serialPin, LOW);
    } else { //if character is a zero send a low signal
      digitalWrite(serialPin, LOW);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
    }
  }
  digitalWrite(latchPin, HIGH);//lock in data on shift register
}

void cycleTubes () {
  for (uint8_t i = 0; i < 1000; i++) {
    String cycleData = timeData(i % 10, i % 10, i % 10);
    shiftTime(serial, latch, clk, cycleData);
    delay(100);
  }
}

uint8_t readTime (uint8_t input) { //makes time human readable (0-60 and 0-24)
  return ((input / 16 * 10) + (input % 16));
}
