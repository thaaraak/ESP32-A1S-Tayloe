
#define ENCODER_USE_INTERRUPTS 1

#include "si5351.h"
#include "Wire.h"
#include "Encoder.h"

Encoder* myEnc;

int lastMult = -1;
int currentFrequency = 5000000;

Si5351 *si5351;
TwoWire wire(0);

void changeFrequency( int freq )
{
    int mult = 0;

    if ( freq < 5000000 )
      mult = 150;
    else if ( freq < 6000000 )
      mult = 120;
    else if ( freq < 8000000 )
      mult = 100;
    else if ( freq < 11000000 )
      mult = 80;
    else if ( freq < 15000000 )
      mult = 50;
    else if ( freq < 22000000 )
      mult = 40;
    else if ( freq < 30000000 )
      mult = 30;
    else if ( freq < 40000000 )
      mult = 20;
    else if ( freq < 50000000 )
      mult = 15;

    uint64_t f = freq * 100ULL;
    uint64_t pllFreq = freq * mult * 100ULL;

    si5351->set_freq_manual(f, pllFreq, SI5351_CLK0);
    si5351->set_freq_manual(f, pllFreq, SI5351_CLK2);

    if ( mult != lastMult )
    {
      si5351->set_phase(SI5351_CLK0, 0);
      si5351->set_phase(SI5351_CLK2, mult);
      si5351->pll_reset(SI5351_PLLA);
      si5351->update_status();

      lastMult = mult;
    }
}

void setup()
{
  // Start serial and initialize the Si5351
  Serial.begin(115200);

  wire.setPins( 23, 19 );

  si5351 = new Si5351( &wire );
  si5351->init( SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  myEnc = new Encoder(15, 4);

}

long oldPosition  = -999;

void loop() 
{
  long newPosition = myEnc->read() / 4;
  if (newPosition != oldPosition) {

    if ( oldPosition != -999 )
    {
      if ( oldPosition > newPosition )
        currentFrequency -= 500;
      else
        currentFrequency += 500;

      Serial.print( "Frequency: " );
      Serial.println( currentFrequency );
      changeFrequency( currentFrequency );

    }
    
    oldPosition = newPosition;
  }
}
