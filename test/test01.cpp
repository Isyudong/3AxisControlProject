#include <Arduino.h>

int relayPin = 9;

void setup() {
  // put your setup code here, to run once:
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}
void loop() 
{
  // put your main code here, to run repeatedly:
    digitalWrite(relayPin, HIGH);
    delay(1000); // Wait for 1000 millisecond(s)
    digitalWrite(relayPin, LOW); 
    delay(1000); // Wait for 1000 millisecond(s)
}
