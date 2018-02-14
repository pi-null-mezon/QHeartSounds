#include "hbdlib.h"

#include <cstring> // contains memcpy(...)
#include <vector>
#include <algorithm>

// Estimates uniformity of the vector's data
template<typename T>
float estimateUniformity(const std::vector<T> &_vIn)
{
    auto _val = std::minmax_element(_vIn.begin(),_vIn.end());
    return 1.0 - std::abs((*(_val.first) - *(_val.second))/(*(_val.first) + *(_val.second)));
}

// Use to unpack group of bytes from the _src at _index into paticular _data value
template <typename T>
inline void unpackBytes(const char *_src, int _index, T& _data) {
    std::memcpy(&_data, &_src[_index], sizeof(T));
}


HDBError unpackSoundRecord(const char *_data, unsigned int _size, SampleType _sampletype, unsigned int _channels, unsigned int _samplesize, Endianness _sampleendianness, unsigned int _samplerate, std::vector<float> &_vt, std::vector<std::vector<float>> &_vvs)
{
    if(_sampleendianness != LittleEndian) {
        return UnsupportedEndianness;
    }

    _vvs.resize(_channels);
    unsigned int _samplestep = _samplesize/8;
    unsigned int _framestep = _samplestep*_channels;
    unsigned int _length = _size / _framestep;
    _vt.resize(_length);
    for(int j = 0; j < _channels; ++j) {
        _vvs[j].resize(_length);
    }

    switch(_sampletype) {

        case SignedInt: {
            int16_t _tmp;
            for(int i = 0; i < _length; ++i) {
                _vt[i] = (float)i/_samplerate;
                for(int j = 0; j < _channels; ++j) {
                    unpackBytes(_data, i*_framestep + j*_samplestep, _tmp);
                    _vvs[j][i] = _tmp;
                }
            }
        } break;

        case Float: {
            float _tmp;
            for(int i = 0; i < _length; ++i) {
                _vt[i] = (float)i/_aformat.sampleRate();
                for(int j = 0; j < _channels; ++j) {
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


bool detectHeartBeating(const char *_data, unsigned int _size, SampleType _sampletype, unsigned int _channels, unsigned int _samplesize, Endianness _sampleendianness, unsigned int _samplerate, HDBError *_error)
{




}
