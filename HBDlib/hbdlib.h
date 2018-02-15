/*******************************************************************
 * Heart Beating Detection Library - aka hbdlib
 *
 * Library was designed for the detection of the heart beating
 * sounds in the audio records.
 *
 * Library is dependency free. No additional libraries are needed
 * (exept c++ standard library).
 *
 * How to use:
 *
 * 1) call hbdlib::getAudioRecordMinLength(...) to get the right audio record length in bytes
 *
 * 2) call hbdlib::searchHBinPCMAudio(...) to check does audio record contain heart beating sounds or not
 *
 * 3) read possible error code by hbdlib::errorCodeDescription(...)
 *
 * Last but not least. Use library only at your own risk,
 * no warranties are granted!!!
 *
 * Alex.A.Taranov, 2017
 * *****************************************************************/

#ifndef HBDLIB_H
#define HBDLIB_H

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
enum HBDError {NoError, UnsupportedAudioFormat, TooShortRecord};

/**
 * @brief The ByteOrder enum is used for ByteOrder args
 */
enum ByteOrder {BigEndian, LittleEndian};

/**
 * @brief The SampleType enum is used for sampletype args
 */
enum SampleType {Unknown, SignedInt, UnSignedInt, Float};

/** Call to get the answer is audio record contains heart beats or not
  * note that function waits not less than 4 seconds long record
    * @param _data - pointer to raw audio data in PCM encoding
    * @param _size - _data size in bytes
    * @param _channels - number of channels in audio
    * @param _samplesize - size in bits of one audio sample (8 bits is 1 byte, 16 bits is 2 bytes etc.)
    * @param _samplebyteorder - how sample value stored in _data
    * @param _samplerate - how many samples represent one second of audio record
    * @param *_error - optional parameter that could be helpful for debugging
    * @return true if something heartbeating-like was detected or false otherwise
 */
extern "C" DLLSPEC bool searchHBinPCMAudio(const char *_data,
                                            unsigned int _size,
                                            SampleType _sampletype,
                                            unsigned int _channels,
                                            unsigned int _samplesize,
                                            ByteOrder _samplebyteorder,
                                            unsigned int _samplerate,
                                            HBDError *_error=0);

/**
  * Use to get human readable description of the error reason
  */
extern "C" DLLSPEC const char* errorCodeDescription(HBDError _error);

/** Use to get encoded audio record length (in bytes) necessary for searchHBinPCMAudio to work
    * @param _channels - number of channels in audio
    * @param _samplesize - size in bits of one audio sample (8 bits is 1 byte, 16 bits is 2 bytes etc.)
    * @param _samplebyteorder - how sample value stored in _data
    * @param _samplerate - how many samples represent one second of audio record
    * @param *_error - optional parameter that could be helpful for debugging
    * @return number bytes necessary for searchHBinPCMAudio to work
    * @note Supported audio formats:
    *       - _sampletype = SignedInt, _samplesize = 16, _saplebyteorder = LittleEndian
    *       - _sampletype = SignedInt, _samplesize = 32, _saplebyteorder = LittleEndian
    *       - _sampletype = Float,     _samplesize = 32, _saplebyteorder = LittleEndian
    *  (for all cases _samplerate > 200, _channels > 0)
 */
extern "C" DLLSPEC unsigned int getAudioRecordMinLength(SampleType _sampletype,
                                                        unsigned int _channels,
                                                        unsigned int _samplesize,
                                                        ByteOrder _samplebyteorder,
                                                        unsigned int _samplerate,
                                                        HBDError *_error=0);
}

#endif // HBDLIB_H
