#ifndef QCONVERTSOUND_H
#define QCONVERTSOUND_H

#include <QAudioFormat>
#include <QByteArray>
#include <QString>
#include <QVector>

#include <cstring>

//This function unpacks bytes sets into paticular data type
template <typename T>
inline void unpackBytes(const char *src, int index, T& data) {
    std::memcpy(&data, &src[index], sizeof(T));
}

bool unpackSoundRecord(const QAudioFormat &_aformat, const QByteArray &_barray, QVector<qreal> &_vt, QVector<QVector<qreal> > &_vvs)
{
    if(_aformat.byteOrder() != QAudioFormat::LittleEndian) {
        qWarning("Unsupported byte order in audio format!");
        return false;
    }

    const int _channels = _aformat.channelCount();
    _vvs.resize(_channels);


    int _samplestep = _aformat.sampleSize()/8;
    int _framestep = _aformat.bytesPerFrame(); // frame size in bytes, same as _aformat.bytesPerFrame()
    int _length = _barray.size() / _framestep;
    _vt.resize(_length);
    for(int j = 0; j < _channels; ++j) {
        _vvs[j].resize(_length);
    }


    switch (_aformat.sampleType()) {

        case QAudioFormat::SignedInt: {
            if(_aformat.sampleSize() == 8) {
                quint8 _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            } else if(_aformat.sampleSize() == 16) {
                qint16 _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            } else if(_aformat.sampleSize() == 32) {
                qint32 _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            }
            return true;
        } break;

        case QAudioFormat::UnSignedInt: {
            if(_aformat.sampleSize() == 8) {
                quint8 _tmp; // yes, it is Qt's strange stuff
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            } else if(_aformat.sampleSize() == 16) {
                qint16 _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            } else if(_aformat.sampleSize() == 32) {
                qint32 _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            }
            return true;
        } break;

        case QAudioFormat::Float: {
            if(_aformat.sampleSize() == 32) {
                float _tmp;
                for(int i = 0; i < _length; ++i) {
                    _vt[i] = (qreal)i/_aformat.sampleRate();
                    for(int j = 0; j < _channels; ++j) {
                        unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                        _vvs[j][i] = _tmp;
                    }
                }
            }
            return true;
        } break;

        default:
        break;
    }

    qWarning("Unsupported audio format!");
    return false;
}

#endif // QCONVERTSOUND_H
