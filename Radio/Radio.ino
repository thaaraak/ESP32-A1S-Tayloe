/**
 * @file streams-i2s-filter-i2s.ino
 * @brief Copy audio from I2S to I2S using an FIR filter
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "Bounce2.h"

#include "AudioTools.h"
#include "es8388.h"
#include "Wire.h"
#include "WiFi.h"
#include "si5351.h"


//#include "fir_coeffs_251Taps_16000_350_7000.h"       // Not enough horsepower for this one
//#include "fir_coeffs_161Ta+++++++++++++++++++++++++++++++++++++ps_44100_200_19000.h"
//#include "fir_coeffs_161Taps_44100_350_19000.h"
//#include "fir_coeffs_161Taps_44100_350_6000.h"
//#include "fir_coeffs_161Taps_44100_350_6000.h"
//#include "fir_coeffs_161Taps_16000_350_6000.h"
//#include "fir_coeffs_301Taps_16000_350_7000.h"
//#include "fir_coeffs_161Taps_22000_350_6000.h"
//#include "fir_coeffs_251Taps_22000_350_6000.h"
//#include "fir_coeffs_251Taps_22000_350_10000.h"
//#include "fir_coeffs_351Taps_44100_350_10000.h"
//#include "fir_coeffs_501Taps_22000_350_10000.h"
#include "fir_coeffs_501Taps_44100_150_4000.h"
//#include "fir_coeffs_251Taps_44100_350_6000.h"      // Not enough horsepower for this one

#include "FIRConverter.h"
#include "ChannelAddConverter.h"
#include "Encoder.h"
#include "LiquidCrystal_I2C.h"

#define BOUNCE_PIN 4

uint16_t sample_rate = 44100;
//uint16_t sample_rate = 16000;
//uint16_t sample_rate = 22000;
uint16_t channels = 2;
uint16_t bits_per_sample = 16;
I2SStream in;

//FilteredStream<int16_t, float> filtered(in, channels); 
//StreamCopy copier(in, filtered, 512);              
//ChannelAddConverter<int16_t> channelAdd;

FIRAddConverter<int16_t> *fir;
StreamCopy copier(in, in); 

Encoder *myEnc;
Encoder *dirEnc;
LiquidCrystal_I2C *lcd;

int lastMult = -1;
int currentFrequency = 5000000;
float currentDir = 1.0;

Bounce bounce = Bounce();
int directionState = 1;


Si5351 *si5351;
TwoWire wire(0);
TwoWire externalWire(1);

void printFrequency()
{
  char buf[20];

  int freq = currentFrequency;
  int millions = freq / 1000000;
  int thousands = ( freq - millions * 1000000 ) / 1000;
  int remain = freq % 1000;

  sprintf( buf, "%3d.%03d.%03d %3s", millions, thousands, remain, directionState == 1 ? "USB" : "LSB" );
  lcd->setCursor(0,0);
  lcd->print( buf );
}

void changeFrequency( int freq )
{
    int mult = 0;
    currentFrequency = freq;

    printFrequency();

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


void setupI2S()
{
  // Input/Output Modes
  es_dac_output_t output = (es_dac_output_t) ( DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2 );
  es_adc_input_t input = ADC_INPUT_LINPUT2_RINPUT2;
  //  es_adc_input_t input = ADC_INPUT_LINPUT1_RINPUT1;

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
  config.pin_mck = 0;
  in.begin(config);

  fir->setGain(4);
}


void setupFIR()
{
  fir = new FIRAddConverter<int16_t>( (float*)&coeffs_hilbert_501Taps_44100_150_4000, (float*)&coeffs_delay_501, 501 );
  //fir = new FIRAddConverter<int16_t>( (float*)&coeffs_hilbert_501Taps_22000_350_10000, (float*)&coeffs_delay_501, 501 );
  //fir = new FIRAddConverter<int16_t>( (float*)&coeffs_hilbert_501Taps_44100_350_10000, (float*)&coeffs_delay_501, 501 );
  fir->setCorrection(currentDir);
  
  //filtered.setFilter(0, new FIR<float>(coeffs_hilbert_251Taps_22000_350_10000));
  //filtered.setFilter(1, new FIR<float>(coeffs_delay_251));
}


void setupSynth()
{
  externalWire.setPins(23, 19 );
  si5351 = new Si5351( &externalWire );
  si5351->init( SI5351_CRYSTAL_LOAD_8PF, 0, 0);
}

void setupLCD()
{
  
  lcd = new LiquidCrystal_I2C(0x27,20,2,&externalWire);

  lcd->init();
  lcd->init();
  lcd->backlight();
  lcd->setCursor(0,0);
}


void setupButton() 
{
  bounce.attach( BOUNCE_PIN,INPUT_PULLUP ); // USE INTERNAL PULL-UP
  bounce.interval(5); // interval in ms
}

void setup(void) 
{  

  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Error); 

  wire.setPins( 33, 32 );
  
  setupFIR();
  setupI2S();
  setupSynth();
  setupLCD();
  setupButton();

  //dirEnc = new Encoder(22, 23);
  myEnc = new Encoder(15, 12);

  WiFi.mode(WIFI_OFF);
  btStop();
  
  changeFrequency( 14200000 );
}

long oldDir = -999;
long oldPosition  = -999;

void readEncoder() 
{
  long newPosition = myEnc->read() / 4;
  if (newPosition != oldPosition) {

    if ( oldPosition != -999 )
    {
      if ( oldPosition > newPosition )
        currentFrequency -= 500;
      else
        currentFrequency += 500;

      changeFrequency( currentFrequency );

    }
    
    oldPosition = newPosition;
  }

/*
  long newDir = dirEnc->read() / 4;
  if (newDir != oldDir) {

    if ( oldDir != -999 )
    {
      if ( oldDir > newDir )
        currentDir -= 0.01;
      else
        currentDir += 0.01;

      fir->setCorrection(currentDir);
      Serial.println(currentDir);
    }
    
    oldDir = newDir;
  }
  */
  
  bounce.update();

  if ( bounce.changed() ) 
  {
    int deboucedInput = bounce.read();
    if ( deboucedInput == LOW ) {

      directionState = directionState * -1; 
      fir->setDirection(directionState );
      printFrequency();
    }
  }
  
}

void loop() 
{
  readEncoder();
  copier.copy( *fir );

}
