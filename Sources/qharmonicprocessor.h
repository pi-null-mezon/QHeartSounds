/*------------------------------------------------------------------------------------------------
 * Taranov Alex, 2014
 *
 * For propper work you should use only magnitudes of 2 for datalength and bufferlength
 * ---------------------------------------------------------------------------------------------*/

#ifndef QHARMONICPROCESSOR_H
#define QHARMONICPROCESSOR_H

#include <QObject>
#include "fftw3.h"

#define LOWER_HR_LIMIT 4 // in s^-1, it is 240 bpm
#define SNR_TRESHOLD -1.0 // in most cases this value is suitable when (bufferlength == 256)
#define HALF_INTERVAL 5 // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define STROBE_FACTOR 80 // defines how much raw counts represents one count in output data, in another words it is frequency divider


class QHarmonicProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QHarmonicProcessor(QObject *parent = 0, quint32 length_of_data = 512, quint32 length_of_buffer = 512);
    ~QHarmonicProcessor();

signals:
    void SignalWasUpdated(const qreal *pointer_to_data, quint32 length_of_data);
    void SpectrumWasUpdated(const qreal *pointer_to_data, quint32 length_of_data);

public slots:
    void  ReadBuffer16(const QByteArray &buffer, qreal time);//should be used only for 16 bit sample rate with QAudioFormat::LittleEndian, time should be in ms
    qreal ComputeFrequency();
    qreal getSNRE();

private:
    qreal *ptData_channel1; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *ptDataForFFT; //a pointer to data prepared for FFT, explicit float because fftwf (single precision) is used
    qreal *ptTime; //a pointer to an array for frame periods storing (values in milliseconds thus unsigned int)
    qreal current_freq_estimation; //a variable for storing a last evaluated frequency of the 'strongest' harmonic
    quint32 curpos; //a current position I meant
    quint32 datalength; //a length of data array
    quint32 bufferlength; //a lenght of sub data array for FFT (bufferlength should be <= datalength)

    qreal *ptCNSignal;  //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
    qreal SNRE; // a variable for signal-to-noise ratio estimation storing
    fftw_complex *ptSpectrum;  // a pointer to an array for FFT-spectrum
    qreal *ptAmplitudeSpectrum; // a pointer to Amplitude spectrum

    quint32 loop(qint64); //a function that return a loop-index (not '' because 'inline' )
};

// inline for speed, must therefore reside in header file
inline quint32 QHarmonicProcessor::loop(qint64 difference)
{
    return ((datalength + (difference % datalength)) % datalength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
#endif // QHARMONICPROCESSOR_H
