#include "qsoundprocessor.h"

#define DEFAULT_SAMPLE_RATE 8000    // in Hz
#define DEFAULT_CHANNEL_COUNT 1
#define DEFAULT_SAMPLE_SIZE 16      //bits in a sample

QSoundProcessor::QSoundProcessor(QObject *parent) :
    QObject(parent)
{
    processed_milliseconds = 0.0;
    pt_audioinput = NULL;
    pt_buffer = NULL;
    m_device_info = QAudioDeviceInfo::defaultInputDevice();
    this->set_format(DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT, DEFAULT_SAMPLE_SIZE, QString("audio/pcm"), QAudioFormat::LittleEndian, QAudioFormat::SignedInt);
}

void QSoundProcessor::set_format(quint16 sample_rate, quint16 channel_count, quint16 sample_size, const QString& codec_name, QAudioFormat::Endian byte_order, QAudioFormat::SampleType sample_type)
{
    m_format.setSampleRate(sample_rate);
    m_format.setChannelCount(channel_count);
    m_format.setSampleSize(sample_size);
    m_format.setCodec(codec_name);
    m_format.setByteOrder(byte_order);
    m_format.setSampleType(sample_type);
    if (!m_device_info.isFormatSupported(m_format))
    {
        qWarning() << "Default format not supported, trying to use the nearest.";
        m_format = m_device_info.nearestFormat(m_format);
        qWarning() << "nearest samplerate = " << m_format.sampleRate();
        qWarning() << "nearest samplesize = " << m_format.sampleSize();
        qWarning() << "nearest channelcount = " << m_format.channelCount();
        qWarning() << "nearest byteorder = " << m_format.byteOrder();
        qWarning() << "nearest sampletype = " << m_format.sampleType();
    }
}

bool QSoundProcessor::open_device_select_dialog()
{
    QDialog dialog;
    dialog.setFixedSize(320,120);
    dialog.setWindowTitle(tr("Device select dialog"));

    QVBoxLayout Lcenter;
        QHBoxLayout Lbuttons;
        QPushButton Baccept(tr("Accept"));
        Baccept.setToolTip(tr("Selected device will be used"));
        connect(&Baccept, &QPushButton::clicked, &dialog, &QDialog::accept);
        QPushButton Bcancel(tr("Cancel"));
        Bcancel.setToolTip(tr("Default device will be used"));
        connect(&Bcancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        Lbuttons.addWidget(&Baccept);
        Lbuttons.addWidget(&Bcancel);

        QGroupBox GBselect(tr("Select audio capture device"));
        QVBoxLayout Llocal;
            QComboBox CBdevice;
            QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
            for(int i = 0; i < devices.size(); ++i)
            {
                CBdevice.addItem(devices.at(i).deviceName(), qVariantFromValue(devices.at(i)));
            }
        Llocal.addWidget(&CBdevice);
        GBselect.setLayout(&Llocal);

    Lcenter.addWidget(&GBselect);
    Lcenter.addLayout(&Lbuttons);
    dialog.setLayout(&Lcenter);

    if(dialog.exec() == QDialog::Accepted)
    {
        m_device_info = CBdevice.itemData(CBdevice.currentIndex()).value<QAudioDeviceInfo>();
        return true;
    }
    else
    {
        m_device_info = QAudioDeviceInfo::defaultInputDevice();
    }
    return false;
}

bool QSoundProcessor::open_format_dialog()
{

    return false;
}

bool QSoundProcessor::open()
{
    this->close();
    pt_audioinput = new QAudioInput(m_device_info, m_format, this);
    connect(pt_audioinput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    pt_buffer = new QBuffer(this);
    if(pt_buffer->open(QIODevice::ReadWrite))
    {
        connect(pt_buffer, SIGNAL(readyRead()), this, SLOT(dataReady()));
        pt_audioinput->start(pt_buffer);
        qWarning() << "current buffersize = " << pt_audioinput->bufferSize();
        qWarning() << "current notifyinterval = " << pt_audioinput->notifyInterval() << " ms";
        return true;
    }
    return false;
}

bool QSoundProcessor::close()
{
    if(pt_audioinput)
    {
        pt_audioinput->stop();
        pt_buffer->close();
        delete pt_audioinput;
        delete pt_buffer;
        pt_audioinput = NULL;
        pt_buffer = NULL;
        return true;
    }
    return false;
}

void QSoundProcessor::handleStateChanged(QAudio::State newState)
{
    qWarning() << "Audio device state: " << newState;
    switch (newState)
    {
        case QAudio::StoppedState:
            if (pt_audioinput->error() != QAudio::NoError)
            {
                // Error handling
            }
            break;
         default:
            // Default handling
         break;
    }
}

bool QSoundProcessor::pause()
{
    if(pt_audioinput)
    {
        pt_audioinput->suspend();
        return true;
    }
    return false;
}

bool QSoundProcessor::resume()
{
    if(pt_audioinput)
    {
        pt_audioinput->resume();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
void QSoundProcessor::set_format_samplerate(quint16 value)
{
    m_format.setSampleRate(value);
}

void QSoundProcessor::set_format_channelcount(quint16 value)
{
    m_format.setChannelCount(value);
}

void QSoundProcessor::set_format_samplesize(quint16 value)
{
    m_format.setSampleSize(value);
}

void QSoundProcessor::set_format_codecname(const QString& value)
{
    m_format.setCodec(value);
}

void QSoundProcessor::set_format_byteorder(QAudioFormat::Endian value)
{
    m_format.setByteOrder(value);
}

void QSoundProcessor::set_format_sampletype(QAudioFormat::SampleType value)
{
    m_format.setSampleType(value);
}
//-----------------------------------------------------------------------------

void QSoundProcessor::set_volume(int value)
{
    if(pt_audioinput)
    {
        pt_audioinput->setVolume( (qreal)value/VOLUME_STEPS );
    }
}

void QSoundProcessor::open_volume_dialog()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("Volume select dialog"));
    dialog.setFixedSize(160,80);

    QVBoxLayout Lcenter;
    QGroupBox GBbox(tr("Set volume level:"));
    QHBoxLayout layout;
    QSlider slider;
    slider.setOrientation(Qt::Horizontal);
    slider.setRange(1, VOLUME_STEPS);
    slider.setTickInterval(10);
    slider.setTickPosition(QSlider::TicksBothSides);
    QLabel lable("error");
    connect(&slider, SIGNAL(valueChanged(int)), &lable, SLOT(setNum(int)));
    if(pt_audioinput)
    {
        slider.setValue( VOLUME_STEPS * pt_audioinput->volume() );
    }
    connect(&slider, SIGNAL(sliderMoved(int)), this, SLOT(set_volume(int)));
    layout.addWidget(&slider);
    layout.addWidget(&lable, 2, Qt::AlignRight);
    GBbox.setLayout(&layout);
    Lcenter.addWidget(&GBbox);
    dialog.setLayout(&Lcenter);
    dialog.exec();
}

void QSoundProcessor::dataReady()
{
    qWarning() << "bytesReady = " << pt_audioinput->bytesReady()
               << ", " << "elapsedUSecs = " << pt_audioinput->elapsedUSecs()
               << ", " << "processedUSecs = " << pt_audioinput->processedUSecs();

    emit readyToRead(pt_buffer->data(),(pt_audioinput->processedUSecs()- processed_milliseconds)/1000); //time in ms format
    processed_milliseconds = pt_audioinput->processedUSecs();

    pt_buffer->reset();
    pt_buffer->buffer().clear();
}


