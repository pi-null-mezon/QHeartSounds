#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <assert.h>

#include <QDir>
#include <QLabel>
#include <QCombobox>
#include <QGroupBox>
#include <QDialog>
#include <QPushButton>
#include <QSettings>
#include <QDateTime>

#include "qconvertsound.h"
#include "hbdlib.h"

void dropsamplerate(const QVector<qreal> &_vX, QVector<qreal> &_vY, int _samplerate, int _targetsamplerate)
{
    assert(_samplerate > 0);
    assert(_targetsamplerate < _samplerate);
    // Let's convert samplerate to
    int _window = _samplerate/_targetsamplerate;

    QVector<qreal> _vtemp(_vX.size(),0.0);

    qreal _tmpsum = 0.0;
    for(int i = 0; i < _window; ++i) {
        _tmpsum += _vX[i];
    }
    _vtemp[0] = _tmpsum/_window;
    for(int i = 1; i < _vX.size() - _window; ++i) {
        _tmpsum = _tmpsum - _vX[i-1] + _vX[i+_window];
        _vtemp[i] = _tmpsum/_window;
    }

    int _outputcounts = _vX.size()/_window;
    _vY.resize(_outputcounts);

    for(int i = 0; i < _outputcounts; ++i) {
        _vY[i] = _vtemp[i*_window];
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME);

    __queryAudioInputDevices();
    __setupAudioInputThread();
    __loadSession();
    __setupView();
}

MainWindow::~MainWindow()
{
    __decommutate();

    qaudioinputThread->quit();
    qaudioinputThread->wait();

    delete ui;
}

void MainWindow::openAudioCaptureDevice()
{
    __decommutate();
    if(qaudioinput) {
        qaudioinput->stop();
        qbuffer.close();
        qaudioinput->deleteLater();
        qInfo("Audio device has been closed");
    }

    audiodevinfo = ui->audioinputdevCB->currentData().value<QAudioDeviceInfo>();

    audioformat.setSampleRate(ui->samplerateCB->currentData().value<int>());
    audioformat.setSampleSize(ui->samplesizeCB->currentData().value<int>());
    audioformat.setSampleType(ui->sampletypeCB->currentData().value<QAudioFormat::SampleType>());
    audioformat.setByteOrder(ui->byteorderCB->currentData().value<QAudioFormat::Endian>());
    audioformat.setChannelCount(ui->channelsCB->currentData().value<int>());
    audioformat.setCodec(ui->audiocodecCB->currentText());

    qaudioinput = new QAudioInput(audiodevinfo,audioformat);
    qaudioinput->moveToThread(qaudioinputThread);
    connect(qaudioinputThread,SIGNAL(finished()), qaudioinput,SLOT(deleteLater()));

    qbuffer.open(QBuffer::ReadWrite);
    qaudioinput->start(&qbuffer);
    qInfo("%s has been opened!", audiodevinfo.deviceName().toUtf8().constData());
    __commutate();
}

void MainWindow::readBuffer()
{      
    if(qbuffer.data().size() > ui->minbuffersizeSP->value()) {

        ui->actualbuffersizeLE->setText(QString::number(qbuffer.data().size()));
        unpackSoundRecord(audioformat,qbuffer.data(),_vt,_vvs);

        hbdlib::HBDError _hbderror;
        bool _ishbdetected = hbdlib::searchHBinPCMAudio(   qbuffer.data().constData(),
                                                           qbuffer.data().size(),
                                                           (hbdlib::SampleType)audioformat.sampleType(),
                                                           audioformat.channelCount(),
                                                           audioformat.sampleSize(),
                                                           (hbdlib::ByteOrder)audioformat.byteOrder(),
                                                           audioformat.sampleRate(),
                                                           &_hbderror);

        qInfo("%s", _ishbdetected ? "Heart beating detected" : "...nothing interesting...");
        if(_hbderror != hbdlib::NoError) {
            qInfo("HBDError code: %s",  hbdlib::errorCodeDescription(_hbderror));
        }

        if(_vvs.size() > 0) {
            if(_vvs[0].size() > 0) {
                qreal _vlimT = 1.0, _vlimB = -1.0;
                if(audioformat.sampleType() != QAudioFormat::Float) {
                    _vlimT = (qreal)((quint64)0x01 << audioformat.sampleSize());;
                    _vlimB = -_vlimT;
                }
                //-------------------------------------------------------------------
                ui->plotW->updateSignal(_vlimB,_vlimT,_vvs[0].data(),_vvs[0].size());
                ui->samplesperscreenLE->setText(QString::number(_vvs[0].size()));
                ui->timelineLE->setText(QString::number(1000.0*_vvs[0].size()/audioformat.sampleRate(),'f',1));
                //-------------------------------------------------------------------
                static QVector<qreal> _vf;
                dropsamplerate(_vvs[0],_vf,audioformat.sampleRate(),ui->lpfsamplerateSP->value());
                ui->filterplotW->updateSignal(_vf.data(),_vf.size());
                ui->lpfbuffersizeLE->setText(QString::number(_vf.size()*sizeof(_vf[0])));
                ui->lpfsamplesLE->setText(QString::number(_vf.size()));
                ui->lpftimelineLE->setText(QString::number(1000.0*_vf.size()/ui->lpfsamplerateSP->value(),'f',1));
            }
        }


        // Clean buffer
        qbuffer.reset();
        qbuffer.buffer().clear();
    }
}

void MainWindow::__commutate()
{
    connect(&qbuffer,SIGNAL(readyRead()), this, SLOT(readBuffer()));
}

void MainWindow::__decommutate()
{
    disconnect(&qbuffer,SIGNAL(readyRead()), this, SLOT(readBuffer()));
}

void MainWindow::__setupView()
{
    ui->filterplotW->set_tracePen(QPen(Qt::NoBrush,1.5,Qt::SolidLine),QColor(255,190,0));
}

void MainWindow::__saveSignalBuffer()
{
    if(_vvs.size() > 0) {
        if(_vvs[0].size() > 0) {
            QDir _dir(qApp->applicationDirPath());
            _dir.mkdir("Records");
            _dir.cd("Records");
            QFile _file(_dir.absolutePath().append("/%1 (%2).csv").arg(APP_NAME,QDateTime::currentDateTime().toString("hhmmss-ddMMyyyy")));
            if(_file.open(QFile::WriteOnly)) {
                _file.write("Time[s],Value\n");
                for(int i = 0; i < _vvs[0].size(); ++i) {
                    _file.write(QString("%1,%2\n").arg(QString::number(_vt[i],'f',7),QString::number(_vvs[0][i],'f',0)).toUtf8());
                }
                qInfo("Record has been saved");
            }
        }
    }
}

void MainWindow::__setupAudioInputThread()
{
    qaudioinputThread = new QThread(this);
    connect(qaudioinputThread,SIGNAL(finished()), qaudioinputThread, SLOT(deleteLater()));
    qaudioinputThread->start();
}

void MainWindow::__queryAudioInputDevices()
{
    QList<QAudioDeviceInfo> _devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for(int i = 0; i < _devices.size(); ++i) {
        ui->audioinputdevCB->addItem(_devices.at(i).deviceName(), qVariantFromValue(_devices.at(i)));
    }
}

void MainWindow::on_openaudiodevB_clicked()
{
    openAudioCaptureDevice();
}

void MainWindow::on_audioinputdevCB_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    QAudioDeviceInfo _audiodevinfo = ui->audioinputdevCB->currentData().value<QAudioDeviceInfo>();

    QList<int> samplerate = _audiodevinfo.supportedSampleRates();
    ui->samplerateCB->clear();
    for(int i = 0; i < samplerate.size(); i++) {
        ui->samplerateCB->addItem( QString::number(samplerate.at(i)), qVariantFromValue(samplerate.at(i)) );
    }

    QList<int> samplesize = _audiodevinfo.supportedSampleSizes();
    ui->samplesizeCB->clear();
    for(int i = 0; i < samplesize.size(); i++) {
        ui->samplesizeCB->addItem( QString::number(samplesize.at(i)), qVariantFromValue(samplesize.at(i)) );
    }


    QList<QAudioFormat::Endian> byteorder = _audiodevinfo.supportedByteOrders();
    ui->byteorderCB->clear();
    for(int i = 0; i < byteorder.size(); i++) {
        switch (byteorder.at(i))  {
            case 0:
                ui->byteorderCB->addItem( "BigEndian" , qVariantFromValue(byteorder.at(i)));
                break;
            case 1:
                ui->byteorderCB->addItem( "LittleEndian" , qVariantFromValue(byteorder.at(i)));
                break;
            default:
                ui->byteorderCB->addItem( QString::number(byteorder.at(i)), qVariantFromValue(byteorder.at(i)));
                break;
        }
    }

    QList<QAudioFormat::SampleType> sampletype = _audiodevinfo.supportedSampleTypes();
    ui->sampletypeCB->clear();
    for(int i = 0; i < sampletype.size(); i++)  {
        switch (sampletype.at(i)) {
            case 0:
                ui->sampletypeCB->addItem( "Unknown", qVariantFromValue(sampletype.at(i)) );
                break;
            case 1:
                ui->sampletypeCB->addItem( "SignedInt", qVariantFromValue(sampletype.at(i)) );
                break;
            case 2:
                ui->sampletypeCB->addItem( "UnsignedInt", qVariantFromValue(sampletype.at(i)) );
                break;
            case 3:
                ui->sampletypeCB->addItem( "Float", qVariantFromValue(sampletype.at(i)) );
                break;
            default:
                ui->sampletypeCB->addItem( QString::number(sampletype.at(i)), qVariantFromValue(sampletype.at(i)) );
                break;
        }
    }

    QList<int> channels = _audiodevinfo.supportedChannelCounts();
    ui->channelsCB->clear();
    for(int i = 0; i < channels.size(); i++) {
        ui->channelsCB->addItem( QString::number(channels.at(i)), qVariantFromValue(channels.at(i)) );
    }

    ui->audiocodecCB->clear();
    ui->audiocodecCB->addItems(_audiodevinfo.supportedCodecs());
}

void MainWindow::closeEvent(QCloseEvent *_e)
{
    __saveSession();
    QMainWindow::closeEvent(_e);
}

void MainWindow::__saveSession()
{
    QSettings _settings(qApp->applicationDirPath().append("/%1").arg(APP_NAME), QSettings::IniFormat);
    _settings.setValue("Minbuffersize_bytes", ui->minbuffersizeSP->value());
}

void MainWindow::__loadSession()
{
    QSettings _settings(qApp->applicationDirPath().append("/%1").arg(APP_NAME), QSettings::IniFormat);
    ui->minbuffersizeSP->setValue(_settings.value("Minbuffersize_bytes",1280).toInt());
}

void MainWindow::on_samplerateCB_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->lpfsamplerateSP->setMaximum(ui->samplerateCB->currentData().toInt());
    ui->lpfsamplerateSP->setValue(ui->lpfsamplerateSP->maximum()/10);
}


void MainWindow::on_pauseB_clicked()
{
    if(qaudioinput)
        qaudioinput->suspend();
}

void MainWindow::on_startB_clicked()
{
    if(qaudioinput)
        qaudioinput->resume();
}


void MainWindow::on_savebufferB_clicked()
{
    if(qaudioinput) {
        if(qaudioinput->state() == QAudio::ActiveState) {
            qaudioinput->suspend();
            __saveSignalBuffer();
            qaudioinput->resume();
        }
    }
}
