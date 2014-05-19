#include "qsoundprocessor.h"

#define DEFAULT_SAMPLE_RATE 8000    // in Hz
#define DEFAULT_CHANNEL_COUNT 1
#define DEFAULT_SAMPLE_SIZE 16      //bits in a sample
#define VOLUME_STEPS 100

QSoundProcessor::QSoundProcessor(QObject *parent) :
    QObject(parent)
{
    processed_milliseconds = 0.0;
    pt_audioinput = NULL;
    pt_buffer = NULL;
    m_device_info = QAudioDeviceInfo::defaultInputDevice();
    this->set_default_format();
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
        qWarning() << "Current audio device does not support this format, trying to use the nearest";
        m_format = m_device_info.nearestFormat(m_format);
        qWarning() << "nearest samplerate = " << m_format.sampleRate();
        qWarning() << "nearest samplesize = " << m_format.sampleSize();
        qWarning() << "nearest channelcount = " << m_format.channelCount();
        qWarning() << "nearest byteorder = " << m_format.byteOrder();
        qWarning() << "nearest sampletype = " << m_format.sampleType();
    }
}

void QSoundProcessor::set_default_format()
{
    this->set_format(DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT, DEFAULT_SAMPLE_SIZE, QString("audio/pcm"), QAudioFormat::LittleEndian, QAudioFormat::SignedInt);
}

bool QSoundProcessor::open_device_select_dialog()
{
    QDialog dialog;
    dialog.setFixedSize(320,120);
    dialog.setWindowTitle(tr("Audio device"));

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
        m_device_info = CBdevice.currentData().value<QAudioDeviceInfo>();
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
    QDialog dialog;
    dialog.setWindowTitle(tr("Audio format"));
    dialog.setFixedSize(320,320);

    QVBoxLayout central_layout;
    central_layout.setMargin(5);

    QHBoxLayout L_buttons;
    QPushButton B_apply(tr("Apply"));
    B_apply.setToolTip(tr("Apply format"));
    connect(&B_apply, SIGNAL(clicked()), &dialog, SLOT(accept()));
    QPushButton B_default(tr("Default"));
    B_default.setToolTip(tr("Set default values and close dialog"));
    connect(&B_default, SIGNAL(clicked()), this, SLOT(set_default_format()));
    connect(&B_default, SIGNAL(clicked()), &dialog, SLOT(reject()));
    QPushButton B_cancel(tr("Cancel"));
    B_cancel.setToolTip(tr("Close dialog without save"));
    connect(&B_cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
    L_buttons.addWidget(&B_apply);
    L_buttons.addWidget(&B_default);
    L_buttons.addWidget(&B_cancel);

    QTabWidget TW_controls;

    QWidget Sampleproperties_page;
        QVBoxLayout L_sampleproperties;
        L_sampleproperties.setMargin(5);

            QGroupBox GB_samplerate;
            GB_samplerate.setTitle(tr("Sample rate"));
            QVBoxLayout L_samplerate;
            QComboBox CB_samplerate;
                QList<int> samplerate = m_device_info.supportedSampleRates();
                for(int i = 0; i < samplerate.size(); i++)
                {
                    CB_samplerate.addItem( QString::number(samplerate.at(i)), qVariantFromValue(samplerate.at(i)) );
                }
            L_samplerate.addWidget(&CB_samplerate);
            GB_samplerate.setLayout(&L_samplerate);

            QGroupBox GB_samplesize;
            GB_samplesize.setTitle(tr("Sample size"));
            QVBoxLayout L_samplesize;
            QComboBox CB_samplesize;
                QList<int> samplesize = m_device_info.supportedSampleSizes();
                for(int i = 0; i < samplesize.size(); i++)
                {
                    CB_samplesize.addItem( QString::number(samplesize.at(i)) , qVariantFromValue(samplesize.at(i)) );
                }
            L_samplesize.addWidget(&CB_samplesize);
            GB_samplesize.setLayout(&L_samplesize);

            QGroupBox GB_byteorder;
            GB_byteorder.setTitle(tr("Byte order"));
            QVBoxLayout L_byteorder;
            QComboBox CB_byteorder;
            QList<QAudioFormat::Endian> byteorder = m_device_info.supportedByteOrders();
                for(int i = 0; i < byteorder.size(); i++)
                {
                    switch (byteorder.at(i))
                    {
                        case 0:
                            CB_byteorder.addItem( "BigEndian" , qVariantFromValue(byteorder.at(i)) );
                            break;
                        case 1:
                            CB_byteorder.addItem( "LittleEndian" , qVariantFromValue(byteorder.at(i)) );
                            break;
                        default:
                            CB_byteorder.addItem( QString::number(byteorder.at(i)) , qVariantFromValue(byteorder.at(i)) );
                            break;
                    }
                }
            L_byteorder.addWidget(&CB_byteorder);
            GB_byteorder.setLayout(&L_byteorder);

            QGroupBox GB_sampletype;
            GB_sampletype.setTitle(tr("Sample type"));
            QVBoxLayout L_sampletype;
            QComboBox CB_sampletype;
            QList<QAudioFormat::SampleType> sampletype = m_device_info.supportedSampleTypes();
                for(int i = 0; i < sampletype.size(); i++)
                {
                    switch (sampletype.at(i))
                    {
                        case 0:
                            CB_sampletype.addItem( "Unknown", qVariantFromValue(sampletype.at(i)) );
                            break;
                        case 1:
                            CB_sampletype.addItem( "SignedInt", qVariantFromValue(sampletype.at(i)) );
                            break;
                        case 2:
                            CB_sampletype.addItem( "UnSignedInt", qVariantFromValue(sampletype.at(i)) );
                            break;
                        case 3:
                            CB_sampletype.addItem( "Float", qVariantFromValue(sampletype.at(i)) );
                            break;
                        default:
                            CB_sampletype.addItem( QString::number(sampletype.at(i)), qVariantFromValue(sampletype.at(i)) );
                            break;
                    }
                }
            L_sampletype.addWidget(&CB_sampletype);
            GB_sampletype.setLayout(&L_sampletype);

        L_sampleproperties.addWidget(&GB_samplerate);
        L_sampleproperties.addWidget(&GB_samplesize);
        L_sampleproperties.addWidget(&GB_byteorder);
        L_sampleproperties.addWidget(&GB_sampletype);
    Sampleproperties_page.setLayout(&L_sampleproperties);

    QWidget Otherproperties_page;

        QVBoxLayout L_otherproperties;
        L_otherproperties.setMargin(5);

            QGroupBox GB_channelcount;
            GB_channelcount.setTitle(tr("Channels count"));
            QVBoxLayout L_channelcount;
            QComboBox CB_channelcount;
                QList<int> channels = m_device_info.supportedChannelCounts();
                for(int i = 0; i < channels.size(); i++)
                {
                    CB_channelcount.addItem( QString::number(channels.at(i)), qVariantFromValue(channels.at(i)) );
                }
            L_channelcount.addWidget(&CB_channelcount);
            GB_channelcount.setLayout(&L_channelcount);

            QGroupBox GB_codecname;
            GB_codecname.setTitle(tr("Audio codec"));
            QVBoxLayout L_codecname;
            QComboBox CB_codecname;
            CB_codecname.addItems( m_device_info.supportedCodecs());
            L_codecname.addWidget(&CB_codecname);
            GB_codecname.setLayout(&L_codecname);

        L_otherproperties.addWidget(&GB_channelcount);
        L_otherproperties.addWidget(&GB_codecname);
    Otherproperties_page.setLayout(&L_otherproperties);


    TW_controls.addTab(&Sampleproperties_page, tr("Sample"));
    TW_controls.addTab(&Otherproperties_page, tr("Channels/codec"));
    central_layout.addWidget(&TW_controls);
    central_layout.addLayout(&L_buttons);
    dialog.setLayout(&central_layout);

    if(dialog.exec() == QDialog::Accepted)
    {
        this->set_format_samplerate( CB_samplerate.currentData().value<int>() );
        this->set_format_samplesize( CB_samplesize.currentData().value<int>() );
        this->set_format_byteorder( CB_byteorder.currentData().value<QAudioFormat::Endian>() );
        this->set_format_sampletype( CB_sampletype.currentData().value<QAudioFormat::SampleType>() );
        this->set_format_channelcount( CB_channelcount.currentData().value<int>() );
        this->set_format_codecname( CB_codecname.currentText() );
        return true;
    }
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
        qWarning() << "current  samplerate = " << m_format.sampleRate();
        qWarning() << "current  samplesize = " << m_format.sampleSize();
        qWarning() << "current  channelcount = " << m_format.channelCount();
        qWarning() << "current  byteorder = " << m_format.byteOrder();
        qWarning() << "current  sampletype = " << m_format.sampleType();
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
void QSoundProcessor::set_format_samplerate(int value)
{
    m_format.setSampleRate( value );
}

void QSoundProcessor::set_format_channelcount(int value)
{
    m_format.setChannelCount(value);
}

void QSoundProcessor::set_format_samplesize(int value)
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
    dialog.setWindowTitle(tr("Volume level"));
    dialog.setFixedSize(160,80);

    QVBoxLayout Lcenter;
    QGroupBox GBbox(tr("Set volume level:"));
    QHBoxLayout layout;
    QSlider slider;
    slider.setOrientation(Qt::Horizontal);
    slider.setRange(1, VOLUME_STEPS);
    slider.setTickInterval(10);
    slider.setTickPosition(QSlider::TicksBothSides);
    QLabel lable(tr("error"));
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

    emit readyToBeRead(pt_buffer->data(),(pt_audioinput->processedUSecs() - processed_milliseconds)/1000); //time in ms format
    processed_milliseconds = pt_audioinput->processedUSecs();

    pt_buffer->reset();
    pt_buffer->buffer().clear();
}


