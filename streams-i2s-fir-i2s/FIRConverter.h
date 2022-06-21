#pragma once
#include "AudioTools/AudioTypes.h"
#include "AudioBasic/Vector.h"
#include "AudioFilter/Filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct fir_f32_s {
    float  *coeffs;     /*!< Pointer to the coefficient buffer.*/
    float  *delay;      /*!< Pointer to the delay line buffer.*/
    int     N;          /*!< FIR filter coefficients amount.*/
    int     pos;        /*!< Position in delay line.*/
    int     decim;      /*!< Decimation factor.*/
    int     d_pos;      /*!< Actual decimation counter.*/
} fir_f32_t;

int dsps_fir_f32_ae32(fir_f32_t* fir, const float* input, float* output, int len);

#ifdef __cplusplus
}
#endif


namespace audio_tools {


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

    float* delayLeft = (float*)calloc( firLen, sizeof(float) );
    float* delayRight = (float*)calloc( firLen, sizeof(float) );

    fir_init( &firLeft, coeffsLeft, delayLeft, firLen );
    fir_init( &firRight, coeffsRight, delayRight, firLen );

    int n = DEFAULT_BUFFER_SIZE;

    srcLeft = (float*)calloc( n, sizeof(float) );
    srcRight = (float*)calloc( n, sizeof(float) );
	destLeft = (float*)calloc( n, sizeof(float) );
	destRight = (float*)calloc( n, sizeof(float) );

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

    T out;

    if ( loop++ % 100 == 0 )
    	LOGE( "Size=%d Count=%d", size, count );

    for ( int i = 0 ; i < count ; i++ )
    {
    	srcLeft[i] = sample[i*2];
    	srcRight[i] = sample[i*2+1];

/*
    	if ( loop % 5000 == 0 )
    	{
    		LOGE( "Src: %f Sample: %d", srcLeft[i], sample[i*2])
    	}
*/

    }

    dsps_fir_f32_ae32(&firLeft, srcLeft, destLeft, count);
    dsps_fir_f32_ae32(&firRight, srcRight, destRight, count);

    for ( int i = 0 ; i < count ; i++ )
    {
    	sample[i*2] = destLeft[i];
    	sample[i*2+1] = destRight[i];
    }

    return size;
  }

  size_t convert2(uint8_t *src, size_t size) {

    int count = size / channels / sizeof(T);
    T *sample = (T *)src;

    Serial.print( size );
    Serial.print( "=" );
    Serial.println( count );

    for ( int i = 0 ; i < count ; i += 2 )
    {
    	srcLeft[i/2] = sample[i];
    	srcRight[i/2] = sample[i+1];
    }

    //fir(&firLeft, srcLeft, destLeft, count/2);
    //fir(&firRight, srcRight, destRight, count/2);

    for ( int i = 0 ; i < count ; i += 2 )
    {
    	sample[i] = destLeft[i/2];
    	sample[i+1] = destRight[i/2];
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

  int loop = 0;
};

}
