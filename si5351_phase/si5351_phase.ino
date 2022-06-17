/*
 * si5351_phase.ino - Example for setting phase with Si5351Arduino library
 *
 * Copyright (C) 2015 - 2016 Jason Milldrum <milldrum@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Setting the phase of a clock requires that you manually set the PLL and
 * take the PLL frequency into account when calculation the value to place
 * in the phase register. As shown on page 10 of Silicon Labs Application
 * Note 619 (AN619), the phase register is a 7-bit register, where a bit
 * represents a phase difference of 1/4 the PLL period. Therefore, the best
 * way to get an accurate phase setting is to make the PLL an even multiple
 * of the clock frequency, depending on what phase you need.
 *
 * If you need a 90 degree phase shift (as in many RF applications), then
 * it is quite easy to determine your parameters. Pick a PLL frequency that
 * is an even multiple of your clock frequency (remember that the PLL needs
 * to be in the range of 600 to 900 MHz). Then to set a 90 degree phase shift,
 * you simply enter that multiple into the phase register. Remember when
 * setting multiple outputs to be phase-related to each other, they each need
 * to be referenced to the same PLL.
 */

#include "si5351.h"
#include "Wire.h"

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

    Serial.print( "Frequency: " );
    Serial.println( f );
    Serial.print( "PLL Frequency: " );
    Serial.println( pllFreq );

    si5351->set_freq_manual(f, pllFreq, SI5351_CLK0);
    Serial.println( "************************" );
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

  // We will output 14.1 MHz on CLK0 and CLK1.
  // A PLLA frequency of 705 MHz was chosen to give an even
  // divisor by 14.1 MHz.
  unsigned long long freq = 1410000000ULL;
  unsigned long long pll_freq = 70500000000ULL;

  // Set CLK0 and CLK1 to output 14.1 MHz with a fixed PLL frequency
  si5351->set_freq_manual(freq, pll_freq, SI5351_CLK0);
  si5351->set_freq_manual(freq, pll_freq, SI5351_CLK2);

  // Now we can set CLK1 to have a 90 deg phase shift by entering
  // 50 in the CLK1 phase register, since the ratio of the PLL to
  // the clock frequency is 50.
  si5351->set_phase(SI5351_CLK0, 0);
  si5351->set_phase(SI5351_CLK2, 50);

  // We need to reset the PLL before they will be in phase alignment
  si5351->pll_reset(SI5351_PLLA);

  // Query a status update and wait a bit to let the Si5351 populate the
  // status flags correctly.
  si5351->update_status();

}

void loop()
{

  changeFrequency( currentFrequency );
  currentFrequency += 500000;

  if (currentFrequency > 40000000 )
    currentFrequency = 5000000;
    
  delay( 100 );

}
