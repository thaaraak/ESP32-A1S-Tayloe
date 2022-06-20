#pragma once
#include "AudioTools/AudioTypes.h"
#include "AudioBasic/Vector.h"
#include "AudioFilter/Filter.h"

namespace audio_tools {

/**
 * @brief Converter for n Channels which applies the indicated Filter
 * @author pschatzmann
 * @tparam T
 */
template <typename T>
class ChannelAddConverter : public BaseConverter<T> {
 public:

  ChannelAddConverter() {
    this->channels = 2;
    this->direction = 1;
    this->gain = 1;
  }

  void setDirection( int direction ) {
	  this->direction = direction;
  }

  void setGain( int gain ) {
	  this->gain = gain;
  }

  ~ChannelAddConverter() {
  }

  size_t convert(uint8_t *src, size_t size) {
    int count = size / channels / sizeof(T);
    T *sample = (T *)src;

    T out;

    for (size_t j = 0; j < count; j++) {

    	out = gain * ( sample[j*2] + direction * sample[j*2+1] );

        sample[j*2] = out;
        sample[j*2+1] = out;

    }
    return size;
  }

 protected:
  int direction;
  int gain;
  int channels;
};

}
