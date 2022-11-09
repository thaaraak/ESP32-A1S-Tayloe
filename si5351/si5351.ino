#include <si5351.h>

Si5351 *si5351;
TwoWire wire(0);

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/


int currentFrequency = -1;
int lastMult = -1;

void changeFrequency( int freq )
{
    int mult = 0;
    currentFrequency = freq;

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

void setupSynth()
{
  si5351 = new Si5351( &wire );
  si5351->init( SI5351_CRYSTAL_LOAD_8PF, 0, 0);
}


void setup(void)
{
  Serial.begin(115200);
  Serial.println("Si5351 Clockgen Test");

  wire.setPins( 23, 19 );

  setupSynth();
  changeFrequency(7000000);  

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
