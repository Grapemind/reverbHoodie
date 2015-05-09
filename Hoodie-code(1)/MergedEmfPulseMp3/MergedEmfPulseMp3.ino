
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h> 
#include <SFEMP3Shield.h>
//MP3
SdFat sd;
SFEMP3Shield MP3player;



//  VARIABLES
int pulsePin = A0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkpin = 9;                // pin to blink led at each beat
int speaker = 2;                  // speaker on pin2 makes a beep with heartbeat

int hodie_btn = 3;

//int piezo = 


//EMF variables
const int numReadings = 5;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int ledpin = 11;

int inputPin = A5;
int val = 0;
bool interupt=false; 

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.


void setup(){
  Serial.begin(9600);             // we agree to talk fast!
  
  pinMode(hodie_btn, INPUT);
  pinMode(blinkpin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(ledpin, OUTPUT);
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);
   
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
   
  //sd.begin(SD_SEL, SPI_HALF_SPEED);
  //MP3player.begin();
}



void loop(){
  
 /* while(hodie_btn = HIGH){
    // Do nothing 
  }
  
  while(hodie_btn = LOW){
    
    */
  
  sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
        sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
        QS = false;                      // reset the Quantified Self flag for next time    
        tone(speaker,1047);              // tone pin, frequency, duration in mS
     }  

     //EMF
    // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(inputPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings)
    // ...wrap around to the beginning:
    readIndex = 0;

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits

  //val = constrain(val, 1, 100);
  //val = map(average, 1, 100, 1, 255);        // to change the response distance of the device
    val = map(average, 1, 500, 1, 255);        // to change the response distance of the device
  //Serial.println(val);
  digitalWrite(ledpin, val);
  //analogWrite(led, val);
  delay(30);        // delay in between reads for stability
  
  /*
  
  if (BPM >= 60 && BPM <=70) {
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(1000);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  if (BPM >= 70 && BPM <= 80){
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(900);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  if (BPM >= 80 && BPM <= 90){
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(800);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  if (BPM >= 90 && BPM <= 100){
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(700);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  if (BPM >= 100 && BPM <= 110){
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(600);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  if (BPM >= 110 && BPM <= 120){
    for(int i = 0; i <= 5; i++){
    digitalWrite(blinkpin, HIGH);
    digitalWrite(ledpin, HIGH);
    delay(500);
    digitalWrite(blinkpin, LOW);
    digitalWrite(ledpin, LOW);
    }
  }
  */

}


void sendDataToProcessing(char symbol, int data ){
    Serial.println(BPM);
    
  }
