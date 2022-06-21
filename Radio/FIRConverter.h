#pragma once
#include "AudioTools/AudioTypes.h"
#include "AudioBasic/Vector.h"
#include "AudioFilter/Filter.h"

namespace audio_tools {


typedef struct fir_f32_s {
    float  *coeffs;     /*!< Pointer to the coefficient buffer.*/
    float  *delay;      /*!< Pointer to the delay line buffer.*/
    int     N;          /*!< FIR filter coefficients amount.*/
    int     pos;        /*!< Position in delay line.*/
    int     decim;      /*!< Decimation factor.*/
    int     d_pos;      /*!< Actual decimation counter.*/
} fir_f32_t;


/**
 * @brief Converter for n Channels which applies the indicated Filter
 * @author pschatzmann
 * @tparam T
 */
template <typename T>
class FIRConverter : public BaseConverter<T> {
 public:

  FIRConverter( float* coeffsLeft, float* coeffsRight, int firLen ) {

    this->channels = 2;
    this->direction = 1;
    this->gain = 1;

    float* delayLeft = calloc( fir->firLen, sizeof(float) );
    float* delayRight = calloc( fir->firLen, sizeof(float) );

    fir_init( &firLeft, coeffsLeft, delayLeft, firLen );
    fir_init( &firRight, coeffsRight, delayRight, firLen );

    int n = DEFAULT_BUFFER_SIZE;

    srcLeft = calloc( n, sizeof(float) );
    srcRight = calloc( n, sizeof(float) );
	destLeft = calloc( n, sizeof(float) );
	destRight = calloc( n, sizeof(float) );
  }

  void setDirection( int direction ) {
	  this->direction = direction;
  }

  void setGain( int gain ) {
	  this->gain = gain;
  }

  ~FIRConverter() {
  }

  void fir_init( fir_f32_t *fir, float *coeffs, float *delay, int N )
  {
      fir->coeffs = coeffs;
      fir->delay = delay;
      fir->N = N;
      fir->pos = 0;

      for (int i = 0 ; i < N; i++) {
          fir->delay[i] = 0;
      }
  }

  void fir(fir_f32_t *fir, float *input, float *output, int len)
  {
      for (int i = 0 ; i < len ; i++) {
          float acc = 0;
          int coeff_pos = fir->N - 1;
          fir->delay[fir->pos] = input[i];
          fir->pos++;
          if (fir->pos >= fir->N) {
              fir->pos = 0;
          }
          for (int n = fir->pos; n < fir->N ; n++) {
              acc += fir->coeffs[coeff_pos--] * fir->delay[n];
          }
          for (int n = 0; n < fir->pos ; n++) {
              acc += fir->coeffs[coeff_pos--] * fir->delay[n];
          }
          output[i] = acc;
      }
  }

  size_t convert(uint8_t *src, size_t size) {

    int count = size / channels / sizeof(T);
    T *sample = (T *)src;

    for ( int i = 0 ; i < count ; i += 2 )
    {
    	srcLeft[i/2] = sample[i];
    	srcRight[i/2] = sample[i+1];
    }

    fir(&firLeft, srcLeft, destLeft, count/2);
    fir(&firRight, srcRight, destRight, count/2);

    for ( int i = 0 ; i < count ; i += 2 )
    {
    	sample[i] = fir->destLeft[i/2];
    	sample[i+1] = fir->destRight[i/2];
    }

    return size;
  }

 protected:
  int direction;
  int gain;
  int channels;

  float* 	coeffsLeft;
  float* 	coeffsRight;
  int		firLen;

  float*	delayLeft;
  float*	delayRight;

  float*	srcLeft;
  float*	srcRight;
  float*	destLeft;
  float*	destRight;

  fir_f32_t firLeft;
  fir_f32_t firRight;

};

}
