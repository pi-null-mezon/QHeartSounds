#include "qharmonicprocessor.h"

#ifdef MINGW_COMPILER
#include <cmath>
#endif

QHarmonicProcessor::QHarmonicProcessor(QObject *parent, quint32 length_of_data, quint32 length_of_buffer) :
                    QObject(parent),
                    datalength(length_of_data),
                    bufferlength(length_of_buffer),
                    curpos(0),
                    SNRE(-5.0),
                    current_freq_estimation(0.0)
{   
    ptData_channel1 = new qreal[datalength];
    ptCNSignal = new qreal[datalength];
    ptTime = new qreal[datalength];

    ptDataForFFT = new qreal[bufferlength];
    ptSpectrum = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * (bufferlength/2+1) );
    ptAmplitudeSpectrum = new qreal[bufferlength/2+1];

    for (unsigned int i = 0; i < datalength; i++)
    {
        ptData_channel1[i] = 0.0;
        ptTime[i] = 10; // discretization period 10 ms for 100 Hz sample rate
        ptCNSignal[i] = 0.0;
    }
}

QHarmonicProcessor::~QHarmonicProcessor()
{
    delete[] ptData_channel1;
    delete[] ptCNSignal;
    delete[] ptTime;
    delete[] ptDataForFFT;
    delete[] ptAmplitudeSpectrum;
    fftw_free(ptSpectrum);
}

void QHarmonicProcessor::ReadBuffer16(const QByteArray &buffer, qreal time)// should be used only for 16 bit sample rate with QAudioFormat::LittleEndian
{  
    //Data reading with 8-bit to 16-bit conversion
    quint32 record_length = buffer.length()/2;
    /*for(quint32 i = 0; i < record_length; i++)
    {
        ptData_channel1[ loop(curpos + i) ] = (quint8)buffer.at(2*i) | ((qint16)buffer.at(2*i+1) << 8);
    }*/

    qreal temp_sum = 0.0;
    //-----------------------------------------------
    for(quint32 i = 0; i < record_length/STROBE_FACTOR; i++)
    {
        for(quint32 j = 0; j < STROBE_FACTOR; j++)
        {
             temp_sum += (quint8)buffer.at(2*j + i*STROBE_FACTOR) | ((qint16)buffer.at(2*j + 1 + i*STROBE_FACTOR) << 8);
        }
        ptData_channel1[ loop(curpos + i) ] = temp_sum / STROBE_FACTOR;
        temp_sum = 0.0;
    }
    //-----------------------------------------------

    //Mean of all data counts evaluation
    qreal temp_mean = 0.0;
    for(quint32 i = 0; i < datalength; i++)
    {
        temp_mean += ptData_channel1[i];
    }
    temp_mean /= datalength;

    //Sample standart deviation of all data evaluation
    qreal temp_sko = 0.0;
    for (quint32 i = 0; i < datalength; i++)
    {
        temp_sko += (ptData_channel1[i] - temp_mean)*(ptData_channel1[i] - temp_mean);
    }
    temp_sko = sqrt(temp_sko / (datalength - 1));

    //CNSignal and ptTime counts
    qreal time_step = time * STROBE_FACTOR / record_length;
    for(quint32 i = 0; i < record_length / STROBE_FACTOR; i++)
    {
        ptCNSignal[ loop(curpos + i) ] =  ((( ptData_channel1[ loop(curpos + i) ] - temp_mean ) / temp_sko ) + ptCNSignal[ loop(curpos + i - 1) ] ) / 2;
        ptTime[ loop(curpos + i) ] = time_step;
    }
    emit SignalWasUpdated(ptCNSignal,datalength);
    //Update curpos counter
    curpos = (curpos + record_length / STROBE_FACTOR) % datalength; // for loop-like usage of ptData and the other arrays...
}

qreal QHarmonicProcessor::ComputeFrequency()
{
    quint32 temp_curpos = curpos; // save current curpos
    qreal buffer_duration = 0.0; // refresh data duration

    fftw_plan p = fftw_plan_dft_r2c_1d(bufferlength, ptDataForFFT, ptSpectrum, FFTW_ESTIMATE);
    //Data preparation
    quint32 position = 0;
    for (quint32 i = 0; i < bufferlength; i++)
    {
        position = loop(temp_curpos - (bufferlength - 1) + i);
        ptDataForFFT[i] = ptCNSignal[ position ];
        buffer_duration += ptTime[ position ];
    }
    fftw_execute(p);

    for(quint32 i = 0; i < bufferlength/2 + 1; i++)
    {
        ptAmplitudeSpectrum[i] = (ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1])/1000;
    }
    emit SpectrumWasUpdated(ptAmplitudeSpectrum,bufferlength/2 + 1);

    //position of max harmonic searching
    quint32 lower_bound = (quint32)(LOWER_HR_LIMIT * buffer_duration / 1000); //You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )
    quint32 index_of_maxpower = 0;
    qreal maxpower = 0.0;

    for (unsigned int i = ( lower_bound + HALF_INTERVAL ); i < ( (bufferlength / 2 + 1) - HALF_INTERVAL ); i++)
    {
        qreal temp_power = ptAmplitudeSpectrum[i];
        if ( maxpower < temp_power )
        {
            maxpower = temp_power;
            index_of_maxpower = i;
        }
    }
    //SNR estimation
    qreal noise_power = 0.0;
    qreal signal_power = 0.0;
    for (quint32 i = lower_bound; i < (bufferlength / 2 + 1); i++)
    {
        if ( ( (i > (index_of_maxpower - HALF_INTERVAL )) && (i < (index_of_maxpower + HALF_INTERVAL) ) ) || ( (i > (2 * index_of_maxpower - HALF_INTERVAL )) && (i < (2 * index_of_maxpower + HALF_INTERVAL) ) ) )
        {
            signal_power += ptAmplitudeSpectrum[i];
        }
        else
        {
            noise_power += ptAmplitudeSpectrum[i];
        }
    }
    if ((noise_power > 0.0) && (signal_power > 0.0))
    {
        SNRE = 10 * log10( signal_power / noise_power );
        if ( SNRE > SNR_TRESHOLD )
        {
            qreal power_multiplyed_by_index = 0.0;
            qreal power_of_first_harmonic = 0.0;
            for (unsigned int i = (index_of_maxpower - HALF_INTERVAL + 1); i < (index_of_maxpower + HALF_INTERVAL); i++)
            {
                power_of_first_harmonic += ptAmplitudeSpectrum[i];
                power_multiplyed_by_index += i * ptAmplitudeSpectrum[i];
            }
            current_freq_estimation = (power_multiplyed_by_index / power_of_first_harmonic) * 60000 / buffer_duration;            
         }
     }
     fftw_destroy_plan(p);
     emit FrequencyWasComputed(current_freq_estimation, SNRE);
     return current_freq_estimation;
}

qreal QHarmonicProcessor::get_SNRE() const
{
    return SNRE;
}

quint32 QHarmonicProcessor::get_datalength() const
{
    return datalength;
}

quint32 QHarmonicProcessor::get_bufferlength() const
{
    return bufferlength;
}

