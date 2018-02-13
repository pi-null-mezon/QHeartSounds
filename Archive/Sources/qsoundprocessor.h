#ifndef QSOUNDPROCESSOR_H
#define QSOUNDPROCESSOR_H

#include <QObject>
#include <QBuffer>
#include <QAudioInput>
#include <QDialog>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDebug>

class QSoundProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QSoundProcessor(QObject *parent = 0);   

signals:
    void readyToBeRead(const QByteArray& value, qreal duration);

public slots:
    bool open_device_select_dialog();
    void handleStateChanged(QAudio::State newState);
    bool open(); //launch audio capture interface, automatically calls close() method before open operations
    bool close(); // destroy audio capture interface
    bool pause(); //suspend audio data capture
    bool resume();//resume audio data capture with current settings
    void set_format(quint16 sample_rate, quint16 channel_count, quint16 sample_size, const QString& codec_name, QAudioFormat::Endian byte_order, QAudioFormat::SampleType sample_type);
    bool open_format_dialog();//Works with current m_device_info instance, thus in most cases should be called after open_device_select_dialog()
    void set_volume(int value);
    void dataReady();
    void open_volume_dialog();
    void set_default_format();

private:
    QAudioFormat m_format;
    QAudioDeviceInfo m_device_info;
    QBuffer *pt_buffer;
    QAudioInput *pt_audioinput;
    qreal processed_milliseconds;

private slots:
    void set_format_samplerate(int value);
    void set_format_channelcount(int value);
    void set_format_samplesize(int value);
    void set_format_codecname(const QString& value);
    void set_format_byteorder(QAudioFormat::Endian value);
    void set_format_sampletype(QAudioFormat::SampleType value);
};

#endif // QSOUNDPROCESSOR_H
