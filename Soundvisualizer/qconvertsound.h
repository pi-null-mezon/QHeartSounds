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

    // Signed integer
    switch(_aformat.sampleType()) {

        case QAudioFormat::SignedInt: {
            //Let's allocate memory first
            int _samplestep = _aformat.sampleSize()/8;
            int _framestep = _aformat.bytesPerFrame(); // frame size in bytes, same as _aformat.bytesPerFrame()
            int _length = _barray.size() / _framestep;
            _vt.resize(_length);
            for(int j = 0; j < _channels; ++j) {
                _vvs[j].resize(_length);
            }
            qint16 _tmp;
            for(int i = 0; i < _length; ++i) {
                _vt[i] = (double)i/_aformat.sampleRate();
                for(int j = 0; j < _channels; ++j) {
                    unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                    _vvs[j][i] = _tmp;
                }
            }
        } break;

        case QAudioFormat::Float: {
            //Let's allocate memory first
            int _samplestep = _aformat.sampleSize()/8;
            int _framestep = _aformat.bytesPerFrame(); // frame size in bytes, same as _aformat.bytesPerFrame()
            int _length = _barray.size() / _framestep;
            _vt.resize(_length);
            for(int j = 0; j < _channels; ++j) {
                _vvs[j].resize(_length);
            }
            float _tmp;
            for(int i = 0; i < _length; ++i) {
                _vt[i] = (double)i/_aformat.sampleRate();
                for(int j = 0; j < _channels; ++j) {
                    unpackBytes(_barray.data(), i*_framestep + j*_samplestep, _tmp);
                    _vvs[j][i] = _tmp;
                }
            }
        } break;

        default:
            qWarning(QString("Unsupported sample type in audio format (%1)!").arg(QString::number(_aformat.sampleType())).toUtf8().constData());
            return false;

    }
    return true;
}

#endif // QCONVERTSOUND_H
