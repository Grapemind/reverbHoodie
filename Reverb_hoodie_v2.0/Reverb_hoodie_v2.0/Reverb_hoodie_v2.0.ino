#include <OSCMessage.h>
#include <SPI.h>

// Poor Man's Wireless 2.4GHz Scanner
//
// uses an nRF24L01p connected to an Arduino
//
// Cables are:
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13
//
// and CE       ->  9
//
// created March 2011 by Rolf Henkel
//

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial);
#endif

#define CE  9

// Array to hold Channel data
#define CHANNELS  64
int channel[CHANNELS];

//  Pulse VARIABLES
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 4;                // pin to blink led at each beat
int fadePin = 3;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
// End of pulse


int ledPin = 6;
int ledVal;
int newVal;

/* EMF Stuff */
#define sample 300
int inPin = A3;
int val;
int array1[sample];
unsigned long averaging;


// nRF24L01P registers we need
#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

void interruptSetup(){     
        // Initializes Timer1 to throw an interrupt every 2mS.
        TCCR1A = 0x00; // DISABLE OUTPUTS AND PWM ON DIGITAL PINS 9 & 10
        TCCR1B = 0x11; // GO INTO 'PHASE AND FREQUENCY CORRECT' MODE, NO PRESCALER
        TCCR1C = 0x00; // DON'T FORCE COMPARE
        TIMSK1 = 0x01; // ENABLE OVERFLOW INTERRUPT (TOIE1)
        ICR1 = 16000;  // TRIGGER TIMER INTERRUPT EVERY 2mS  
        sei();         // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED     
      }

void sendLEDPacket(int theMessage){
  OSCMessage msg("/super/ledVal");
  msg.add(int32_t(theMessage));

  SLIPSerial.beginPacket();  
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
  delay(200);
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


void emf() {
  for (int i = 0; i < sample; i++) {
    array1[i] = analogRead(inPin);
    averaging += array1[i];
  }
  
  val = averaging / sample;

  //val = constrain(val, 0, 60);
  val = map(val, 0, 1023, 0, 255);
  averaging = 0;
  Serial.print("EMF val: "); 
  Serial.print(val);
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
  Serial.print('|');
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
    Serial.print(channel[i]);
    channel[i] = 0;
  }

  // indicate overall power
  Serial.print("| ");
  Serial.println(norm);
  ledVal = norm;
  newVal = map(ledVal, 5, 25, 0, 255);
  Serial.print("LED val :");
  Serial.print(newVal);
  Serial.print("\t");
}

void setup()
{
  Serial.begin(57600);
  SLIPSerial.begin(57600);
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);
   
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(ledPin, OUTPUT);

  // Setup SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);

  // Activate Chip Enable
  pinMode(CE, OUTPUT);
  disable();

  // now start receiver
  powerUp();

  // switch off Shockburst
  setRegister(_NRF24_EN_AA, 0x0);

  // make sure RF-section is set properly
  // - just write default value...
  setRegister(_NRF24_RF_SETUP, 0x0F);

}

void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,fadeRate);          //  fade LED
  }

void loop()
{
  
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
        QS = false;                      // reset the Quantified Self flag for next time    
     }
     
  ledFadeToBeat();
  delay(20);                             //  take a break
  // initialise EMF
  emf();
  
  // do the scan
  scanChannels();

  // output the result
  outputChannels();
  analogWrite(ledPin, newVal);
  sendLEDPacket(ledVal);
}

