/**
 * @file streams-i2s-filter-i2s.ino
 * @brief Copy audio from I2S to I2S using an FIR filter
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "es8388.h"
#include "Wire.h"


#include "fir_coeffs_61Taps_44100_200_19000.h"
#include "fir_coeffs_101Taps_44100_200_19000.h"
#include "fir_coeffs_161Taps_44100_200_19000.h"

uint16_t sample_rate = 44100;
uint16_t channels = 2;
uint16_t bits_per_sample = 16;
I2SStream in;

FilteredStream<int16_t, float> filtered(in, channels);  // Defiles the filter as BaseConverter
StreamCopy copier(in, filtered, 512);               // copies sound into i2s

// Arduino Setup
void setup(void) 
{  

  Serial.begin(115200);
  // change to Warning to improve the quality
  AudioLogger::instance().begin(Serial, AudioLogger::Error); 

  filtered.setFilter(0, new FIR<float>(coeffs_hilbert_101Taps_44100_200_19000));
  filtered.setFilter(1, new FIR<float>(coeffs_delay_101));

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

// Arduino loop - copy sound to out 
void loop() 
{
  copier.copy();
}
