
/* Pin setup */


/* EMF Stuff */
#define sample 300
int inPin = 3;
int val;
int pin11 = 13;
int array1[sample];
unsigned long averaging;


/* Pulse Blink */


void setup() {
  Serial.begin(9600);
}

void loop() {
  emf();
}


void emf() {
  for (int i = 0; i < sample; i++) {
    array1[i] = analogRead(inPin);
    averaging += array1[i];
  }
  
  val = averaging / sample;

  val = constrain(val, 0, 100);
  val = map(val, 0, 60, 0, 255);
  analogWrite(pin11, val); 
  averaging = 0; 
  Serial.println(val);
}
