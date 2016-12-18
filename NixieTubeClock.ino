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
#define rtcAddress 0x68

//Variables
String currentTime = "";
uint8_t daylight = 0; //daylight savings offset
uint8_t cHour = 0; //current hour
uint8_t cMinute = 0; //current minute
uint8_t cSecond = 0;  //current second
uint8_t brightness = 0;


void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(serial, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(boostPin, OUTPUT);

  pinMode(DSTPin, INPUT);
  pinMode(potPin, INPUT);
}

void loop() {

  //read the analog value of the potentiometer and write as a pwm signal.
  //This changes the brigtness of the nixie tubes and gives the option of turning the tubes off completely
  brightness = analogRead(potPin);
  delayMicroseconds(100);
  analogRead(potPin) <= 2 ? analogWrite(boostPin, 0) : analogWrite(boostPin, map(analogRead(potPin), 3, 1023, 100, 150));
  
  //if the daylight savings time switch is on, add add 1 hour to the current time
  digitalRead(DSTPin) == HIGH ? daylight = 1 : daylight = 0;

  //Interface with the RTC

  Wire.beginTransmission(rtcAddress);
  Wire.write(0); //prepare rtc to be read from
  Wire.endTransmission();
  //read current second, minute and hour
  Wire.requestFrom(rtcAddress, 7);
  cSecond = (Wire.read() & 0x7f), DEC;
  cMinute = (Wire.read()), DEC;
  cHour = (Wire.read() & 0x3f), DEC;

  //takes current time and sends it to the shift registers
  currentTime = timeData(cHour + daylight, cMinute, cSecond);
  shiftTime(serial, latch, clk, currentTime);
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

void shiftTime (int serialPin, int latchPin, int clockPin, String inputTime) { //sends a binary string to shift registers
  digitalWrite(latchPin, LOW);//set latch pin low to prepare for data to be sent
  for (uint8_t i = inputTime.length(); i > 0; i--) { //iterates over entire length of input string
    byte currentChar = inputTime.charAt(i);//gets character "i" in the binary string
    if (currentChar = 1) { //if character is a one send a high signal to the shift registers
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
