/**
 * @file streams-i2s-filter-i2s.ino
 * @brief Copy audio from I2S to I2S using an FIR filter
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "es8388.h"
#include "Wire.h"

#include "FIRConverter.h"
#include "MixerConverter.h"
#include "fir_coeffs_61Taps_44100_200_19000.h"
#include "fir_coeffs_101Taps_44100_200_19000.h"
#include "fir_coeffs_161Taps_44100_200_19000.h"
#include "fir_coeffs_251Taps_44100_350_6000.h"
#include "fir_coeffs_501Taps_44100_350_10000.h"
#include "fir_coeffs_801Taps_44100_350_10000.h"
#include "fir_coeffs_351Taps_44100_150_21000.h"

#include "120-tap-plus-45.h"
#include "120-tap-minus-45.h"
#include "120-tap-4khz-lowpass.h"
#include "120-tap-1khz-lowpass.h"

uint16_t sample_rate = 44100;
uint16_t channels = 2;
uint16_t bits_per_sample = 16;
I2SStream in;

StreamCopy copier(in, in);               // copies sound into i2s
FIRConverter<int16_t> *fir;
FIRConverter<int16_t> *lowpass_fir;
MixerConverter<int16_t> *mix;
MultiConverter<int16_t> *multi;

// Arduino Setup
void setup(void) 
{  
  Serial.begin(115200);
  Serial.println("starting I2S...");
  Serial.println("starting I2S...");


  
  AudioLogger::instance().begin(Serial, AudioLogger::Error); 

  //fir = new FIRConverter<int16_t>( (float*)&coeffs_hilbert_351Taps_44100_350_10000, (float*)&coeffs_delay_351, 351 );
//  fir = new FIRConverter<int16_t>( (float*)&coeffs_hilbert_801Taps_44100_350_10000, (float*)&coeffs_delay_801, 801 );

  multi = new MultiConverter<int16_t>();

//  fir = new FIRConverter<int16_t>( (float*)&coeffs_hilbert_501Taps_44100_350_10000, (float*)&coeffs_delay_501, 501 );
  //fir = new FIRSplitterConverter<int16_t>( (float*)&plus_45_120, (float*)&minus_45_120, 120, true );
  //mix = new MixerConverter<int16_t>();
  
  fir = new FIRSplitterConverter<int16_t>( (float*)&coeffs_hilbert_351Taps_44100_150_21000, (float*)&coeffs_delay_351, 351, true );
  //lowpass_fir = new FIRConverter<int16_t>( (float*)&lowpass_4KHz, (float*)&lowpass_4KHz, 120 );
  lowpass_fir = new FIRConverter<int16_t>( (float*)&lowpass_1khz, (float*)&lowpass_1khz, 120 );
  
  multi->add( *fir );
  //multi->add( *lowpass_fir );
  
  // Input/Output Modes
  es_dac_output_t output = (es_dac_output_t) ( DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2 );
  es_adc_input_t input = ADC_INPUT_LINPUT2_RINPUT2;
  //  es_adc_input_t input = ADC_INPUT_LINPUT1_RINPUT1;

  TwoWire wire(0);
  wire.setPins( 23, 22 );
  
  es8388 codec;
  codec.begin( &wire );
  codec.config( bits_per_sample, output, input, 90 );


  // start I2S in
  Serial.println("starting I2S...");
  auto config = in.defaultConfig(RXTX_MODE);
  config.sample_rate = sample_rate; 
  config.bits_per_sample = bits_per_sample; 
  config.channels = 2;
  config.i2s_format = I2S_STD_FORMAT;
  config.pin_ws = 17;
  config.pin_bck = 18;
  config.pin_data = 5;
  config.pin_data_rx = 16;
  //config.fixed_mclk = 0;
  config.pin_mck = 3;
  in.begin(config);
}


void loop() 
{
  //copier.copy();
  copier.copy(*multi);
//  copier.copy(*fir);
}
