#include <SPI.h>
#include <SoftwareSerial.h>

// uses an nRF24L01p chip
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13
// and CE       ->  9

/* El-Wire */
int El_wire = 3; // Channel A

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
//int led9 = 11;
//int led10 = 12;

/* Hoodie-Button*/
// Not yet implemented in loop
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
int blinkPin = 4;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded!
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
/* End of pulse */

int ledPin = 6;
int ledVal;
int newVal;


int yeah = 0;

/* Setup */

void setup()
{
  Serial.begin(57600);
  interruptSetup();     // sets up to read Pulse Sensor signal every 2mS

  pinMode(El_wire, OUTPUT);         // El-wire pin -- channel A
  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat!
  pinMode(fadePin, OUTPUT);         // pin that will fade to your heartbeat!
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);        // Hoodie Button

  Serial.println("Wireless 2.4GHz Scanner ...");

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

void loop()
{
  if (QS == true) {                      // Quantified Self flag is true when arduino finds a heartbeat
    fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
    QS = false;                      // reset the Quantified Self flag for next time
  }

  ledFadeToBeat();
  delay(20);
  emf();            // initialise EMF

  if (val >= 120) {
    digitalWrite(3, HIGH);
    delay(1000);
  } else {
    digitalWrite(3, LOW);
  }

  scanChannels();   // Scan channels
  outputChannels(); // output the result
  analogWrite(ledPin, newVal);
}

/* Methods and functions */
// Turned in so we can return value in the loop.


/* LED patterns */
void pattern(){
  digitalWrite(led1, HIGH);
  digitalWrite(led8, HIGH);
  delay(500);
  digitalWrite(led2, HIGH);
  digitalWrite(led7, HIGH);
  digitalWrite(led1, LOW);
  digitalWrite(led8, LOW);
  delay(500);
  digitalWrite(led3, HIGH);
  digitalWrite(led6, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led7, LOW);
  delay(500);
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);
  digitalWrite(led3, LOW);
  digitalWrite(led6, LOW);
  delay(500)
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);
  delay(500);
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);
  digitalWrite(led3, LOW);
  digitalWrite(led6, LOW);
  delay(500);
  digitalWrite(led3, HIGH);
  digitalWrite(led6, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led7, LOW);
  delay(500);
  digitalWrite(led2, HIGH);
  digitalWrite(led7, HIGH);
  digitalWrite(led1, LOW);
  digitalWrite(led8, LOW);
  delay(500);
  digitalWrite(led1, HIGH);
  digitalWrite(led8, HIGH);-
}




void allblink() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);
  digitalWrite(led6, HIGH);
  digitalWrite(led7, HIGH);
  digitalWrite(led8, HIGH);
  delay(500);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);
  digitalWrite(led6, LOW);
  digitalWrite(led7, LOW);
  digitalWrite(led8, LOW);
  delay(500);
}

void allon() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);
  digitalWrite(led6, HIGH);
  digitalWrite(led7, HIGH);
  digitalWrite(led8, HIGH);
}



void alloff() {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);
  digitalWrite(led6, LOW);
  digitalWrite(led7, LOW);
  digitalWrite(led8, LOW);
}

int emf() {
  for (int i = 0; i < sample; i++) {
    array1[i] = analogRead(inPin);
    averaging += array1[i];
  }

  val = averaging / sample;
  val = constrain(val, 0, 1023);
  val = map(val, 0, 1023, 0, 255);
  averaging = 0;

  Serial.print("EMF val: ");
  Serial.print(val);
  Serial.print("\t");

  return val;
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
  Serial.print("Norm :");
  Serial.print(norm);
  Serial.print("\t");
  ledVal = norm;
  newVal = map(ledVal, 5, 25, 0, 255);
  Serial.print("LED val :");
  Serial.print(newVal);
  Serial.print("\t");
}

// give a visual reference between WLAN-channels and displayed data
void printChannels(void)
{
  // output approximate positions of WLAN-channels
  Serial.println(">      1 2  3 4  5  6 7 8  9 10 11 12 13  14                     <");

}

void ledFadeToBeat() {
  fadeRate -= 15;                         //  set LED fade value
  fadeRate = constrain(fadeRate, 0, 255); //  keep LED fade value from going into negative numbers!
  analogWrite(fadePin, fadeRate);         //  fade LED
}


