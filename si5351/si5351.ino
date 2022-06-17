#include <Adafruit_SI5351.h>

Adafruit_SI5351 clockgen = Adafruit_SI5351();

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  Serial.println("Si5351 Clockgen Test"); Serial.println("");


  TwoWire wire(0);
  wire.setPins( 23, 19 );
  
  /* Initialise the sensor */
  if (clockgen.begin( &wire ) != ERROR_NONE)
  {
    /* There was a problem detecting the IC ... check your connections */
    Serial.print("Ooops, no Si5351 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  Serial.println("OK!");

  int mult = 50;
  Serial.println("Set PLLA to 700MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 30);
  Serial.println("Set Output #0 to 7MHz");
  clockgen.setupMultisynth(0, SI5351_PLL_A, mult, 0, 1);
  clockgen.setupMultisynth(2, SI5351_PLL_A, mult, 0, 1);

  clockgen.setPhase( 0, 0 );
  clockgen.setPhase( 2, mult );

  clockgen.enableOutputs(true);
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{
}
