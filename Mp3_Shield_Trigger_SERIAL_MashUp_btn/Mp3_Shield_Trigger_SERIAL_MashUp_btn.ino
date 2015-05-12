/*
  MP3 Shield Trigger
  by: Jim Lindblom
      SparkFun Electronics
  date: September 23, 2013

  This is an example MP3 trigger sketch for the SparkFun MP3 Shield.
  Pins 0, 1, 5, 10, A0, A1, A2, A3, and A4 are setup to trigger tracks
  "track001.mp3", "track002.mp3", etc. on an SD card loaded into
  the shield. Whenever any of those pins are shorted to ground,
  their respective track will start playing.

  When a new pin is triggered, any track currently playing will
  stop, and the new one will start.

  A5 is setup to globally STOP playing a track when triggered.

  If you need more triggers, the shield's jumpers on pins 3 and 4
  (MIDI-IN and GPIO1) can be cut open and used as additional
  trigger pins. Also, because pins 0 and 1 are used as triggers
  Serial is not available for debugging. Disable those as
  triggers if you want to use serial.

  Much of this code was grabbed from the FilePlayer example
  included with the SFEMP3Shield library. Major thanks to Bill
  Porter, again, for this amazing library!
*/

#include <SPI.h>           // SPI library
#include <SdFat.h>         // SDFat Library
#include <SdFatUtil.h>     // SDFat Util Library
#include <SFEMP3Shield.h>  // Mp3 Shield Library

SdFat sd; // Create object to handle SD functions

SFEMP3Shield MP3player; // Create Mp3 library object
// These variables are used in the MP3 initialization to set up
// some stereo options:
const uint8_t volume = 0; // MP3 Player volume 0=max, 255=lowest (off)
const uint16_t monoMode = 1;  // Mono setting 0=off, 3=max

/* Pin setup */
#define TRIGGER_COUNT 0
int triggerPins[TRIGGER_COUNT] = {}; // you disregard the possibility of using digital pins to trigger sounds
int stopPin = A5; // This pin triggers a track stop.
int lastTrigger = 0; // This variable keeps track of which tune is playing
int lastReading = 0;
int currentlyPlaying = 0;
const int endOfReading = '*';
const int antennaPin = A2;
int hashY = 0;

/*Pulse setup*/
int pulsePin = A0;
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false; 

/* Button */
int buttonpin = 12;
int buttonstate = 0;

void setup()
{
  /* Set up all trigger pins as inputs, with pull-ups activated: */
  for (int i = 0; i < TRIGGER_COUNT; i++)
  {
    pinMode(triggerPins[i], INPUT_PULLUP);
  }
  pinMode(stopPin, INPUT_PULLUP);

  initSD();  // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield

  Serial.begin(9600);
  interruptSetup(); 
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);
  pinMode(buttonpin,INPUT);
  Serial.println("Ready");
}

// All the loop does is continuously step through the trigger
//  pins to see if one is pulled low. If it is, it'll stop any
//  currently playing track, and start playing a new one.

void loop()
{
  buttonstate = digitalRead(buttonpin);
  // read the values from the antenna
  int antennaVal = analogRead(antennaPin);
  int hashX = constrain(antennaVal, 1, 200);
  hashX = int(map(hashX, 1, 200, 1, 6));
  
  // read the values from the pulse sensor
  //serialOutput();
  if (QS == true){     //  A Heartbeat Was Found
                       // BPM and IBI have been Determined
                       // Quantified Self "QS" true when arduino finds a heartbeat
      
      hashY = constrain(IBI,1,1000);
      hashY = int(map(hashY, 1, 1000, 1, 3)); 
      }

  
  // operate the name of the file
  int fileNum = hashX * hashY ;
  Serial.println(antennaVal);
  Serial.println(hashX);
  Serial.println(hashY);
  Serial.println(fileNum);
  //if(buttonstate == LOW){ volume = 0;} else {volume = 255;}
  if (buttonstate == LOW){
    MP3player.setVolume(0,0);
  }else{
    MP3player.setVolume(255,255);
  }
  playTrack(fileNum);
  delay(1000);
  
  // if the player isnt playing, make the current file zero
  if (!MP3player.isPlaying()) currentlyPlaying = 0;

  // triggering with serial port
  while (Serial.available() > 0) {
    int serVal = Serial.read();
    if (serVal == endOfReading) {
      // this is the end of a number
      // print out the value and then use it to play a sound
      Serial.println(lastReading);
      // XXX add the play sound code here
      //if (buttonstate == LOW){playTrack(lastReading);}
      playTrack(lastReading);
      // reinitialize the counter
      lastReading = 0;
    }
    if (serVal >= '0' && serVal <= '9') {
      lastReading = lastReading * 10 + serVal - 48;
    }
  }
  

  // triggering with inputs
  for (int i = 0; i < TRIGGER_COUNT; i++)
  {
    if ((digitalRead(triggerPins[i]) == LOW) && ((i + 1) != lastTrigger))
    {
      lastTrigger = i + 1; // Update lastTrigger variable to current trigger
      playTrack(lastTrigger);
    }
  }
  // After looping through and checking trigger pins, check to
  //  see if the stopPin (A5) is triggered.
  if (digitalRead(stopPin) == LOW)
  {
    lastTrigger = 0; // Reset lastTrigger
    // If another track is playing, stop it.
    if (MP3player.isPlaying())
      MP3player.stopTrack();
  }
}

// initSD() initializes the SD card and checks for an error.
void initSD()
{
  //Initialize the SdCard.
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED))
    sd.initErrorHalt();
  if (!sd.chdir("/"))
    sd.errorHalt("sd.chdir");
}

// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if (result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode);
}

void playTrack(int lastTrigger) {
  /* If another track is playing, stop it: */
  if (MP3player.isPlaying() && lastTrigger != currentlyPlaying)
    MP3player.stopTrack();

  /* Use the playTrack function to play a numbered track: */
  if (lastTrigger != currentlyPlaying) {
    uint8_t result = MP3player.playTrack(lastTrigger);
    // An alternative here would be to use the
    //  playMP3(fileName) function, as long as you mapped
    //  the file names to trigger pins.

    if (result == 0)  // playTrack() returns 0 on success
    {
      // Success
      Serial.println("Started playing");
      currentlyPlaying = lastTrigger;
    }
    else // Otherwise there's an error, check the code
    {
      // Print error code somehow, someway
      Serial.print("Error when trying to play track ");
      Serial.println(lastTrigger);
    }
  } else {
    Serial.print("Already playing track ");
    Serial.println(currentlyPlaying);
  }
}
