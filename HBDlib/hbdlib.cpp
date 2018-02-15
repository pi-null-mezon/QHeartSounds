#include "hbdlib.h"

#include <cstring> // contains memcpy(...)
#include <vector>
#include <algorithm>
#include <assert.h>

#include <iostream>

namespace hbdlib {

// Use to compute uniformity of the vector's data
template<typename T>
float computeUniformity(const std::vector<T> &_vIn)
{
    auto _val = std::minmax_element(_vIn.begin(),_vIn.end());
    return 1.0 - std::abs((*(_val.first) - *(_val.second))/(*(_val.first) + *(_val.second)));
}

// Use to unpack group of bytes from the _src at _index into paticular _data value
template <typename T>
inline void unpackBytes(const char *_src, int _index, T& _data) {
    std::memcpy(&_data, &_src[_index], sizeof(T));
}

// Use to drop sample rate of the signal
template<typename T>
void dropsamplerate(const std::vector<T> &_vX, std::vector<T> &_vY, unsigned int _samplerate, unsigned int _targetsamplerate)
{
    assert(_samplerate > 0);
    assert(_targetsamplerate < _samplerate);

    unsigned int _window = _samplerate/_targetsamplerate;

    std::vector<T> _vtemp(_vX.size(),0);

    T _tmpsum = 0;
    for(unsigned int i = 0; i < _window; ++i){
        _tmpsum += _vX[i];
    }
    _vtemp[0] = _tmpsum/_window;

    for(unsigned int i = 1; i < _vX.size() - _window; ++i) {
        _tmpsum = _tmpsum - _vX[i-1] + _vX[i+_window];
        _vtemp[i] = _tmpsum/_window;
    }

    unsigned int _outputcounts = _vX.size()/_window;
    _vY.resize(_outputcounts);

    for(unsigned int i = 0; i < _outputcounts; ++i) {
        _vY[i] = _vtemp[i*_window];
    }
}

// Use to compute derivative of the vector
template<typename T>
std::vector<T> differentiate(const std::vector<T> &_vX, unsigned int _step)
{
    assert(_vX.size() > _step);
    std::vector<T> _vY (_vX.size()-_step,0);
    for(unsigned int i = 0; i < _vY.size(); ++i) {
        _vY[i] = _vX[i+_step] - _vX[i];
    }
    return _vY;
}

// Use to compute mean value of the vector's values
template<typename T>
T computeMean(const std::vector<T> &_vX)
{
    T _mean = 0;
    for(unsigned int i = 0; i < _vX.size(); ++i) {
        _mean += _vX[i];
    }
    return _mean/_vX.size();
}

// Use to compute stdev of the vector's values
template<typename T>
T computeStdev(const std::vector<T> &_vX)
{
    assert(_vX.size() > 1);
    T _mean = computeMean(_vX);
    T _stdev = 0;
    for(unsigned int i = 0; i < _vX.size(); ++i) {
        _stdev += (_vX[i]-_mean)*(_vX[i]-_mean);
    }
    return std::sqrt(_stdev) / (_vX.size() - 1);

}

// Use to unpack audio/pcm data into real values vectors
HBDError unpackSoundRecord(const char *_data, unsigned int _size, SampleType _sampletype, unsigned int _channels, unsigned int _samplesize, Endianness _sampleendianness, unsigned int _samplerate, std::vector<float> &_vt, std::vector<std::vector<float>> &_vvs)
{
    if(_sampleendianness != LittleEndian) {
        return UnsupportedEndianness;
    }
    if(_samplerate == 0) {
        return UnsupportedSamplerate;
    }
    if(_channels == 0) {
        return UnsupportedNumberOfChannels;
    }

    _vvs.resize(_channels);
    unsigned int _samplestep = _samplesize/8;
    unsigned int _framestep = _samplestep*_channels;
    unsigned int _length = _size / _framestep;
    _vt.resize(_length);
    for(unsigned int j = 0; j < _channels; ++j) {
        _vvs[j].resize(_length);
    }

    switch(_sampletype) {

        case SignedInt: {
            int16_t _tmp;
            for(unsigned int i = 0; i < _length; ++i) {
                _vt[i] = (float)i/_samplerate;
                for(unsigned int j = 0; j < _channels; ++j) {
                    unpackBytes(_data, i*_framestep + j*_samplestep, _tmp);
                    _vvs[j][i] = _tmp;
                }
            }
        } break;

        case Float: {
            float _tmp;
            for(unsigned int i = 0; i < _length; ++i) {
                _vt[i] = (float)i/_samplerate;
                for(unsigned int j = 0; j < _channels; ++j) {
                    unpackBytes(_data, i*_framestep + j*_samplestep, _tmp);
                    _vvs[j][i] = _tmp;
                }
            }
        } break;

        default:
            return UnsupportedSmpleType;

    }
    return NoError;
}

// Use to detect heart beating in audio record
bool searchHBinPCMAudio(const char *_data, unsigned int _size, SampleType _sampletype, unsigned int _channels, unsigned int _samplesize, Endianness _sampleendianness, unsigned int _samplerate, std::vector<float> &_vdebug, HBDError *_error)
{   
    std::vector<float> _vt;
    std::vector<std::vector<float>> _vvs;

    HBDError _err;
    float _recordtime_s = (((float)_size/(float)_channels)/(float)(_samplesize/8))/(float)_samplerate;
    if(_recordtime_s < 3.9) {
        _err = TooShortRecord;
    } else {
        _err = unpackSoundRecord(_data,_size,_sampletype,_channels,_samplesize,_sampleendianness,_samplerate,_vt,_vvs);
    }

    if(_error != 0) {
        *_error = _err;
    }

    if(_err != NoError) {
        return false;
    }

    // HEART BEATS DETECTION ALGORITHM

    // Drop sample rate to 200 Hz
    const unsigned int _targetsamplerate = 200; // do not change, it could damage empirical thresholds
    std::vector<float> _vY;
    dropsamplerate(_vvs[0],_vY,_samplerate,_targetsamplerate);
    //Also we need reduce number of counts in time vector
    unsigned int _window = _samplerate/_targetsamplerate;
    unsigned int _outputcounts = _vt.size()/_window;
    std::vector<float> _vTime(_outputcounts,0);
    for(unsigned int i = 0; i < _outputcounts; ++i) {
        _vTime[i] = _vt[i*_window];
    }

    // Let's transform signal to standard for this function value range
    std::vector<float> _vS = differentiate(_vY,1);
    for(unsigned int i = 0; i < _vS.size(); ++i) {
        _vS[i] = std::abs(_vS[i]);
    }
    float _stdev = computeStdev(_vS);
    for(unsigned int i = 0; i < _vS.size(); ++i) {
        _vS[i] = _vS[i] / _stdev;
    }

    // Let's count pulses
    const float _timethresh = 0.275f; // empirical constant in seconds
    std::vector<float> _vI; _vI.reserve(64);
    float _timemark = 0.0f, _timeshift;
    for(unsigned int i = 0; i < _vS.size(); ++i) {
        _timeshift = _vTime[i] - _timemark;
        if((_vS[i] > 27.5) && (_timeshift > _timethresh)) { // empirical constant
            _vI.push_back(_timeshift);
            _timemark = _vTime[i];
        }
    }
    // We need enroll last interval separately
    _timeshift = _vTime[_vS.size()-1] - _timemark;
    if(_timeshift > _timethresh/2.0f) {
        _vI.push_back(_timeshift);
    }

    _vdebug = _vS;

    // Now we are ready to compute features and classify signal
    float _uniformity = computeUniformity(_vI);
    float _hr = 60.0f / (2.0f* computeMean(_vI));
    std::cout << "Intervals size: " << _vI.size() << std::endl
              << "Intervals meanval: " << computeMean(_vI) << std::endl
              << "_uniformity: " << _uniformity << std::endl
              << "_hr:"          << _hr << std::endl;
    for(unsigned int i = 0; i < _vI.size(); ++i) {
        std::cout << _vI[i] << "; ";
    }
    std::cout << std::endl;

    if((_uniformity > 0.333) && (_uniformity < 0.7f)) {// empirical constants
        if((_hr > 55.f) && (_hr < 135.f)) {
            return true;
        }
    }
    // END

    return false;
}

}// end of hbdlib namespace
