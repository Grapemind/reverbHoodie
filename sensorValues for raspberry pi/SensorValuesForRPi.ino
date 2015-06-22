int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(sensorPin);
  //Serial.println(sensorValue);
  delay(500);

  if (sensorValue >= 0 && sensorValue < 50) {
    Serial.println(1);
  }

  if (sensorValue > 50 && sensorValue < 100) {
    Serial.println(2);
  }
}
