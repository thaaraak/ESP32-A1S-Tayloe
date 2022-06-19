#include <Bounce2.h>

#define BOUNCE_PIN 12

Bounce bounce = Bounce();
int directionState = 1;

void setup() 
{
  Serial.begin(115200);

  bounce.attach( BOUNCE_PIN,INPUT_PULLUP ); // USE INTERNAL PULL-UP
  bounce.interval(5); // interval in ms
}

void loop() {

  bounce.update();

  if ( bounce.changed() ) 
  {
    int deboucedInput = bounce.read();
    if ( deboucedInput == LOW ) {

      Serial.print( "Direction: " );
      directionState = directionState * -1; 
      Serial.println( directionState );
    }
  }

}
