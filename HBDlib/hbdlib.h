#ifndef HBDLIB_H
#define HBDLIB_H

#include <vector>

#ifdef HBDLIB_LIBRARY
    #ifdef TARGET_OS_LINUX
        #define DLLSPEC __attribute__((visibility("default")))
    #else
        #define DLLSPEC __declspec(dllexport)
    #endif
#else
    #ifdef TARGET_OS_LINUX
        #define DLLSPEC
    #else
        #define DLLSPEC __declspec(dllimport)
    #endif
#endif

namespace hbdlib {

/**
 * @brief The HBDError enum is used for errors handling
 */
enum HBDError {NoError, UnsupportedSmpleType, UnsupportedNumberOfChannels, UnsupportedEndianness, UnsupportedSamplerate, TooShortRecord};

/**
 * @brief The Endianness enum is used for endianness args
 */
enum Endianness {LittleEndian, BigEndian};

/**
 * @brief The SampleType enum is used for sampletype args
 */
enum SampleType {SignedInt, UnsignedInt, Float};

/** Call to get the answer is audio record stored in _data contains heart beats or not
    * @param _data - pointer to raw audio data in PCM encoding
    * @param _size - _data size in bytes
    * @param _channels - number of channels in audio
    * @param _samplesize - size in bits of one audio sample (8 bits is 1 byte, 16 bits is 2 bytes etc.)
    * @param _sampleendianness - how sample value stored in _data
    * @param _samplerate - how many samples represent one second of audio record
    * @param *_error - optional parameter that could be helpful for debugging
    * @return true if something heart beating was detected or false otherwise
 */
extern "C" DLLSPEC bool searchHBinPCMAudio(const char *_data,
                                            unsigned int _size,
                                            SampleType _sampletype,
                                            unsigned int _channels,
                                            unsigned int _samplesize,
                                            Endianness _sampleendianness,
                                            unsigned int _samplerate,
                                            std::vector<float> &_vdebug,
                                            HBDError *_error=0);

}

#endif // HBDLIB_H
