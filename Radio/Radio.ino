/**
 * @file streams-i2s-filter-i2s.ino
 * @brief Copy audio from I2S to I2S using an FIR filter
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "es8388.h"
#include "Wire.h"
#include "si5351.h"
#include "fir_coeffs_161Taps_44100_200_19000.h"

uint16_t sample_rate = 44100;
uint16_t channels = 2;
uint16_t bits_per_sample = 16;
I2SStream in;

FilteredStream<int16_t, float> filtered(in, channels);  // Defiles the filter as BaseConverter
StreamCopy copier(in, filtered, 512);               // copies sound into i2s


int lastMult = -1;
int currentFrequency = 5000000;

Si5351 *si5351;
TwoWire wire(0);
TwoWire externalWire(1);

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


void setupI2S()
{
  // Input/Output Modes
  es_dac_output_t output = (es_dac_output_t) ( DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2 );
  es_adc_input_t input = ADC_INPUT_LINPUT2_RINPUT2;
  //  es_adc_input_t input = ADC_INPUT_LINPUT1_RINPUT1;

  TwoWire wire(0);
  wire.setPins( 33, 32 );
  
  es8388 codec;
  codec.begin( &wire );
  codec.config( bits_per_sample, output, input, 90 );

  // start I2S in
  Serial.println("starting I2S...");
  auto config = in.defaultConfig(RXTX_MODE);
  config.sample_rate = sample_rate; 
  config.bits_per_sample = bits_per_sample; 
  config.channels = channels;
  config.i2s_format = I2S_STD_FORMAT;
  config.pin_ws = 25;
  config.pin_bck = 27;
  config.pin_data = 26;
  config.pin_data_rx = 35;
  //config.fixed_mclk = 0;
  config.pin_mck = 0;
  in.begin(config);
}


void setupFIR()
{
  filtered.setFilter(0, new FIR<float>(coeffs_hilbert_161Taps_44100_200_19000));
  filtered.setFilter(1, new FIR<float>(coeffs_delay_161));
}


void setupSynth()
{
  externalWire.setPins( 23, 19 );

  si5351 = new Si5351( &externalWire );
  si5351->init( SI5351_CRYSTAL_LOAD_8PF, 0, 0);
}

void setup(void) 
{  

  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Error); 

  setupI2S();
  setupFIR();
  setupSynth();

  changeFrequency( 7000000 );
}



void loop() 
{
  copier.copy();
}
