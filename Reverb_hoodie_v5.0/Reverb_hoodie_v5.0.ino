

#include <SPI.h>
#include <SoftwareSerial.h>

#include <OSCBundle.h>
#include <OSCBoards.h>


#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);


// uses an nRF24L01p chip
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13
// and CE       ->  9

/* El-Wire */
//int El_wire = 3; // Channel A

/* LED-bar */
// Pin 3 --> Pin 12
// Not yet in loop
int led1 = 3;
int led2 = 4;
int led3 = 5;
int led4 = 6;
int led5 = 7;
int led6 = 8;
int led7 = 9;
int led8 = 10;
int led9 = 11;
int led10 = 12;

/* Hoodie-Button*/
const int buttonPin = 7;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status


/* nRF24L01P registers */
#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

// Array to hold wifi channel data
#define CE  9
#define CHANNELS  64
int channel[CHANNELS];

/* EMF Stuff */
#define sample 300
int inPin = A3;
int val;
int array1[sample];
unsigned long averaging;

/*  Pulse VARIABLES */
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 8;                // pin to blink led at each beat
int fadePin = 6;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin
// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded!
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
/* End of pulse */

// Har med logan att gra
int ledPin = 6;
int ledVal;
int newVal;


/* Setup */

void setup()
{
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(9600);   // set this as high as you can reliably run on your platform
#if ARDUINO >= 100
  while (!Serial)
    ;   // Leonardo bug
#endif


  Serial.begin(57600);
  interruptSetup();     // sets up to read Pulse Sensor signal every 2mS
  pinMode(buttonPin, INPUT);

  //pinMode(El_wire, OUTPUT);         // El-wire pin -- channel A
  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat!
  pinMode(fadePin, OUTPUT);         // pin that will fade to your heartbeat!
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);        // Hoodie Button


  // Setup SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);

  // Activate Chip Enable
  pinMode(CE, OUTPUT);
  disable();
  powerUp();
  setRegister(_NRF24_EN_AA, 0x0);
  setRegister(_NRF24_RF_SETUP, 0x0F);
}


/* The loop */

/*
  Prints emf, wifi channels and blinks logo lights. When activated by the hoodie.

  Todo

  -Serial print pulse value. # Done
  -Shift register, methods for turning desired pin on.
  -Pd
  -light animation.

*/

void loop()
//{
/*
buttonState = digitalRead(buttonPin);

while (buttonState == HIGH) {
  // turn LED on:
  digitalWrite(ledPin, HIGH);
  Serial.println("ON, Hoodie activated :)");*/
  
  
{

  if (QS == true) {                      // Quantified Self flag is true when arduino finds a heartbeat
    fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
    QS = false;                      // reset the Quantified Self flag for next time
  }

  ledFadeToBeat();
  delay(20);
  emf();            // initialise EMF print values
  scanChannels();   // Scan wifi channels
  outputChannels(); // output the result
//  Serial.print("Bpm: "); // Bpm value
//  Serial.print(BPM); // Print pulse value
//  Serial.print("\n");
//  analogWrite(ledPin, newVal);
  
}
//}
/*
while (buttonState == LOW) {
  // turn LED off:
  digitalWrite(ledPin, LOW);
  Serial.println("Hoddie is off, no current trough the circuit");
  Serial.println("Do nothing for 1 sec");
  delay(1000);
}
}
*/

// End of loop



/* Methods and functions */

void emf() {
  for (int i = 0; i < sample; i++) {
    array1[i] = analogRead(inPin);
    averaging += array1[i];
  }

  val = averaging / sample;

  val = constrain(val, 0, 100);
  val = map(val, 0, 60, 0, 255);
  averaging = 0;
//  Serial.print("EMF val: ");
//  Serial.print(val);
//  Serial.print("\t");
}


void ledFadeToBeat() {
  fadeRate -= 15;                         //  set LED fade value
  fadeRate = constrain(fadeRate, 0, 255); //  keep LED fade value from going into negative numbers!
  analogWrite(fadePin, fadeRate);         //  fade LED
}

// get the value of a nRF24L01p register
byte getRegister(byte r)
{
  byte c;

  PORTB &= ~_BV(2);
  c = SPI.transfer(r & 0x1F);
  c = SPI.transfer(0);
  PORTB |= _BV(2);

  return (c);
}

// set the value of a nRF24L01p register
void setRegister(byte r, byte v)
{
  PORTB &= ~_BV(2);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  PORTB |= _BV(2);
}



// power up the nRF24L01p chip
void powerUp(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

// switch nRF24L01p off
void powerDown(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) & ~0x02);
}

// enable RX
void enable(void)
{
  PORTB |= _BV(1);
}

// disable RX
void disable(void)
{
  PORTB &= ~_BV(1);
}

// setup RX-Mode of nRF24L01p
void setRX(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x01);
  enable();
  // this is slightly shorter than
  // the recommended delay of 130 usec
  // - but it works for me and speeds things up a little...
  delayMicroseconds(100);
}

// scanning all channels in the 2.4GHz band
void scanChannels(void)
{
  disable();
  for ( int j = 0 ; j < 200  ; j++)
  {
    for ( int i = 0 ; i < CHANNELS ; i++)
    {
      // select a new channel
      setRegister(_NRF24_RF_CH, (128 * i) / CHANNELS);

      // switch on RX
      setRX();

      // wait enough for RX-things to settle
      delayMicroseconds(40);

      // this is actually the point where the RPD-flag
      // is set, when CE goes low
      disable();

      // read out RPD flag; set to 1 if
      // received power > -64dBm
      if ( getRegister(_NRF24_RPD) > 0 )   channel[i]++;
    }
  }
}

// outputs channel data as a simple grey map
void outputChannels(void)
{
  int norm = 0;

  // find the maximal count in channel array
  for ( int i = 0 ; i < CHANNELS ; i++)
    if ( channel[i] > norm ) norm = channel[i];

  // now output the data
  for ( int i = 0 ; i < CHANNELS ; i++)
  {
    int pos;

    // calculate grey value position
    if ( norm != 0 ) pos = (channel[i] * 10) / norm;
    else          pos = 0;

    // boost low values
    if ( pos == 0 && channel[i] > 0 ) pos++;

    // clamp large values
    if ( pos > 9 ) pos = 9;

    // print it out
    //Serial.print(channel[i]);
    channel[i] = 0;
  }

  // indicate overall power
//  Serial.print("Norm :");
//  Serial.print(norm);
//  Serial.print("\t");
//  ledVal = norm;
//  newVal = map(ledVal, 5, 25, 0, 255);
//  Serial.print("LED val :");
//  Serial.print(newVal);
//  Serial.print("\t");
  
  sendData(norm, "/reverb/wifi");
  sendData(BPM, "/reverb/pulse");
  sendData(val, "/reverb/emf");
}

// give a visual reference between WLAN-channels and displayed data
void printChannels(void)
{
  // output approximate positions of WLAN-channels
//  Serial.println(">      1 2  3 4  5  6 7 8  9 10 11 12 13  14                     <");

}

void sendData(int Data,  char* Name )
{
  //declare the bundle
  OSCBundle bndl;
  //BOSCBundle's add' returns the OSCMessage so the message's 'add' can be composed together
  bndl.add(Name).add((int32_t)Data);

  SLIPSerial.beginPacket();
  bndl.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  bndl.empty(); // empty the bundle to free room for a new one

  delay(100);
}



