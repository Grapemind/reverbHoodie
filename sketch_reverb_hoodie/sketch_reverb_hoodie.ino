// Aaron ALAI EMF Detector April 22nd 2009 VERSION 1.0
// aaronalai1@gmail.com
// *future note, put in averaging function to average val which should result in a more
// smooth response from the led.  I will give you a hint on how to make an averaging function;
// it involves the use of an array

#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h> 
#include <SFEMP3Shield.h>

SdFat sd;
SFEMP3Shield MP3player;


int inPin = 5;             // analog 5
int val = 0;                 // where to store info from analog 5
int pin11 = 11;         // output of red led
int x;
int y;

void setup() {
 
  Serial.begin(9600);
  sd.begin(SD_SEL, SPI_HALF_SPEED);
  MP3player.begin();
 
}

void loop() {
 
  val = analogRead(inPin);                    // reads in the values from analog 5 and
  Serial.print(val);     
                                                                   //assigns them to val
  if(val >= 1){
   
    x = constrain(val, 1, 100);               // mess with these values                                      
    x = map(x, 1, 100, 1, 255);        // to change the response distance of the device
    analogWrite(pin11, val);     // *note also messing with the resistor should change 
    y = constrain(val, 1, 100);               // mess with these values                                      
    y = map(y, 1, 100, 1, 5);
    
    
    switch(y) {
	case 1: MP3player.playTrack(1); 
	case 2: MP3player.playTrack(2); 
	case 3: MP3player.playTrack(3); 
        case 4: MP3player.playTrack(4); 
        case 5: MP3player.playTrack(5); 
        break;
      }
    
                                                                   // the sensitivity
   }
                   //Serial.println(x);                                // use output to aid in calibrating
 
}
