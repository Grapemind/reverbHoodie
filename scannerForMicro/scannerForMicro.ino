#include <OSCBundle.h>
#include <OSCBoards.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif

#include <OSCBundle.h>
#include <OSCBoards.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include "Wire.h"


//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8

RF24 radio(9, SS);

//
// Channel info
//

const uint8_t num_channels = 64;
uint8_t values[num_channels];

int ledPin = 6;

//  Pulse VARIABLES
int pulsePin = A3;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 11;                // pin to blink led at each beat
int fadePin = 3;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin



// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded!
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
// End of pulse

/* EMF Stuff */
#define sample 300
int inPin = A4;
int val;
int array1[sample];
unsigned long averaging;


void interruptSetup() {
  // Initializes Timer1 to throw an interrupt every 2mS.
  TCCR1A = 0x00; // DISABLE OUTPUTS AND PWM ON DIGITAL PINS 9 & 10
  TCCR1B = 0x11; // GO INTO 'PHASE AND FREQUENCY CORRECT' MODE, NO PRESCALER
  TCCR1C = 0x00; // DON'T FORCE COMPARE
  TIMSK1 = 0x01; // ENABLE OVERFLOW INTERRUPT (TOIE1)
  ICR1 = 16000;  // TRIGGER TIMER INTERRUPT EVERY 2mS
  sei();         // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
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

void ledFadeToBeat() {
  fadeRate -= 15;                         //  set LED fade value
  fadeRate = constrain(fadeRate, 0, 255); //  keep LED fade value from going into negative numbers!
  analogWrite(fadePin, fadeRate);         //  fade LED
}

void emf() {
  for (int i = 0; i < sample; i++) {
    array1[i] = analogRead(inPin);
    averaging += array1[i];
  }

  val = averaging / sample;
  val = constrain(val, 0, 100);
  val = map(val, 0, 60, 0, 255);
  averaging = 0;
}


void setup(void)
{
  

  
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(115200);
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS

  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat!
  pinMode(fadePin, OUTPUT);         // pin that will fade to your heartbeat!
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  //
  // Setup and configure rf radio
  //

  radio.begin();
  radio.setAutoAck(false);

  // Get into standby mode
  radio.startListening();
  radio.stopListening();
  radio.setDataRate( RF24_2MBPS ) ; // may fallback to 1Mbps


  // Print out header, high then low digit
  int i = 0;
  while ( i < num_channels )
  {
    ++i;
  }
  i = 0;
  while ( i < num_channels )
  {
    ++i;
  }
}

//
// Loop
//

const int num_reps = 100;

void loop(void)
{
  // Clear measurement values
  memset(values, 0, sizeof(values));

  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    int i = num_channels;
    while (i--)
    {
      // Select this channel
      radio.setChannel(i);

      // Listen for a little
      radio.startListening();
      delayMicroseconds(225);


      // Did we get a carrier?
      if ( radio.testCarrier() ) {
        ++values[i];
      }
      radio.stopListening();
    }
  }

  // Print out channel measurements, clamped to a single hex digit
  int i = 0;
  int norm;
  while ( i < num_channels )
  {
    norm = norm + values[i];
    printf("%x", min(0xf, values[i] & 0xf));
    ++i;
  }


  if (QS == true) {                      // Quantified Self flag is true when arduino finds a heartbeat
    fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
    QS = false;                      // reset the Quantified Self flag for next time
  }

  emf();

  sendData(norm, "/reverb/wifi");
  sendData(BPM, "/reverb/pulse");
  sendData(val, "/reverb/emf");
  Serial.println(norm);
  
  

}
