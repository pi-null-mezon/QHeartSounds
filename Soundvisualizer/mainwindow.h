#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QAudioInput>
#include <QBuffer>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void openAudioCaptureDevice();

private slots:
    void readBuffer();
    void on_openaudiodevB_clicked();
    void on_audioinputdevCB_currentIndexChanged(int index);

    void on_samplerateCB_currentIndexChanged(int index);

    void on_pauseB_clicked();

    void on_startB_clicked();

protected:
    void closeEvent(QCloseEvent *_e);

private:
    void __saveSession();
    void __loadSession();
    void __commutate();
    void __decommutate();
    void __setupView();
    void __setupAudioInputThread();
    void __queryAudioInputDevices();

    Ui::MainWindow *ui;

    QAudioInput *qaudioinput = NULL;
    QThread     *qaudioinputThread = NULL;

    QBuffer qbuffer;
    QAudioDeviceInfo audiodevinfo;
    QAudioFormat audioformat;
};


#endif // MAINWINDOW_H
