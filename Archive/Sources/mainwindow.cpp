#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle(tr("QHeartSounds"));
    this->setMinimumSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    //----------Central_widget----------
    pt_placeholder = new QWidget();
    this->setCentralWidget(pt_placeholder);
    pt_main_layout = new QVBoxLayout();
    pt_main_layout->setMargin(5);
    pt_placeholder->setLayout(pt_main_layout);
    //----------Plotters----------------
    pt_buffer_plot = new QPlot(this, "AUDIO INPUT",true,false);
    pt_record_plot = new QPlot(this, "NORMALISED SIGNAL",false,true);
    pt_spectrum_plot = new QPlot(this, "AMPLITUDE SPECTRUM",false,true);
    pt_main_layout->addWidget(pt_buffer_plot);
    pt_main_layout->addWidget(pt_record_plot);
    pt_main_layout->addWidget(pt_spectrum_plot);
    pt_spectrum_plot->set_unsigned(true);

    //----------Actions-----------------
    pt_action_exit = new QAction(tr("&Exit"),this);
    pt_action_exit->setStatusTip(tr("Exit from the application"));
    connect(pt_action_exit, &QAction::triggered, this, &QMainWindow::close);

    pt_action_open = new QAction(tr("&OpenDevice"),this);
    pt_action_open->setStatusTip(tr("Open audio device and starts audio processing"));
    connect(pt_action_open, &QAction::triggered, this, &MainWindow::open_device);

    pt_action_pause = new QAction(tr("&PauseDvice"),this);
    pt_action_pause->setStatusTip(tr("Stop audio processing"));
    connect(pt_action_pause, &QAction::triggered, this, &MainWindow::pause_device);

    pt_action_resume = new QAction(tr("&ResumeDevice"),this);
    pt_action_resume->setStatusTip(tr("Resume audio processing"));
    connect(pt_action_resume, &QAction::triggered, this, &MainWindow::resume_device);

    pt_action_close = new QAction(tr("&CloseDevice"),this);
    pt_action_close->setStatusTip(tr("Stop current audio processing and closes current audio device"));
    connect(pt_action_close, &QAction::triggered, this, &MainWindow::close_device);

    pt_action_volume = new QAction(tr("&SetVolume"),this);
    pt_action_volume->setStatusTip(tr("Open volume level control dialog"));
    connect(pt_action_volume, &QAction::triggered, this, &MainWindow::open_volume_dialog);

    pt_action_newRecord = new QAction(tr("&NewRecord"),this);
    pt_action_newRecord->setStatusTip(tr("Will start new record session"));
    connect(pt_action_newRecord, &QAction::triggered, this, &MainWindow::start_new_record);

    pt_action_about = new QAction(tr("&About"),this);
    pt_action_about->setStatusTip(tr("Open about dialog"));
    connect(pt_action_about, &QAction::triggered, this, &MainWindow::about_dialog);

    //-----------Menus------------------
    pt_menu_main = this->menuBar()->addMenu(tr("&Menu"));
    //pt_menu_main->addAction(pt_action_newRecord);
    pt_menu_main->addAction(pt_action_about);
    pt_menu_main->addSeparator();
    pt_menu_main->addAction(pt_action_exit);

    //----------Timers------------------
    m_timer.setParent(this);
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(1500);

    //----------Audio device interface---------
    pt_sound_device = new QSoundProcessor(this);
    connect(pt_sound_device, SIGNAL(readyToBeRead(QByteArray,qreal)), pt_buffer_plot, SLOT(read_ByteArray(QByteArray,qreal)));

    //-----------------------------------------
    pt_harmonicprocessor = NULL;
    this->statusBar()->showMessage(tr("A context menu is available by right clicking"));
    this->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //statusBar()->showMessage(QString::number((qint16)0xFFFF*16,10));
}

MainWindow::~MainWindow()
{
    this->close_device();
    pt_sound_device->deleteLater();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_action_newRecord);
    menu.addAction(pt_action_open);
    menu.addAction(pt_action_close);
    menu.addSeparator();
    menu.addAction(pt_action_resume);
    menu.addAction(pt_action_pause);
    menu.addAction(pt_action_volume);
    menu.exec(event->globalPos());
}

void MainWindow::pause_device()
{
    m_timer.stop();
    pt_sound_device->pause();
}

void MainWindow::resume_device()
{
    pt_sound_device->resume();
    m_timer.start();
}

bool MainWindow::open_device()
{
    pt_sound_device->open_device_select_dialog();
    pt_sound_device->open_format_dialog();
    return pt_sound_device->open();
}

bool MainWindow::close_device()
{
    return pt_sound_device->close();
}

void MainWindow::open_volume_dialog()
{
   pt_sound_device->open_volume_dialog();
}

int MainWindow::start_new_record()
{
    this->pause_device();

    //--------------Should prevent start new record bug--------------
    disconnect(pt_harmonicprocessor,0,0,0);
    pt_record_plot->unset_pt_Data();
    pt_spectrum_plot->unset_pt_Data();
    //---------------------------------------------------------------

    if(!this->open_device())
    {
        return -1;//error - can not open audio device for some reasons
    }

    if(pt_harmonicprocessor)
    {
        delete pt_harmonicprocessor;
        pt_harmonicprocessor = NULL;
    }

    pt_harmonicprocessor = new QHarmonicProcessor();
    connect(pt_sound_device, SIGNAL(readyToBeRead(QByteArray,qreal)), pt_harmonicprocessor, SLOT(ReadBuffer16(QByteArray,qreal)));
    connect(pt_harmonicprocessor, &QHarmonicProcessor::SignalWasUpdated, pt_record_plot, &QPlot::read_Data);
    connect(pt_harmonicprocessor, SIGNAL(SpectrumWasUpdated(const qreal*,quint32)), pt_spectrum_plot, SLOT(read_Data(const qreal*,quint32)));
    connect(pt_harmonicprocessor, SIGNAL(FrequencyWasComputed(qreal,qreal)), pt_spectrum_plot, SLOT(take_frequency(qreal,qreal)));
    connect(&m_timer, SIGNAL(timeout()), pt_harmonicprocessor, SLOT(ComputeFrequency()));

    pt_record_plot->set_string( "data: " + QString::number(pt_harmonicprocessor->get_datalength()) );
    pt_record_plot->set_extrastring( "buffer: " + QString::number(pt_harmonicprocessor->get_bufferlength()));

    this->resume_device();
    return 0;//success code
}

void MainWindow::about_dialog()
{
    QDialog *aboutdialog = new QDialog();
    aboutdialog->setWindowTitle("About");
    aboutdialog->setFixedSize(256,128);

    QVBoxLayout *templayout = new QVBoxLayout();
    templayout->setMargin(10);
    QLabel *projectname = new QLabel("QHeartSounds v.1.0.0.2");
    projectname->setAlignment(Qt::AlignCenter);
    projectname->setFrameStyle( QFrame::Box | QFrame::Raised);
    QLabel *projectauthors = new QLabel("Biomedical department of BMSTU\nTaranov Alex\n2014");
    projectauthors->setWordWrap(true);
    projectauthors->setAlignment(Qt::AlignCenter);
    //projectauthors ->setFrameStyle( QFrame::Box | QFrame::Sunken);
    QLabel *hyperlink = new QLabel("<a href='mailto:pi-null-mezon@yandex.ru?subject=QHeartSounds'>Contact us at pi-null-mezon@yandex.ru</a>");
    hyperlink->setToolTip("Will try to open your mail client");
    hyperlink->setOpenExternalLinks(true);
    hyperlink->setAlignment(Qt::AlignCenter);

    templayout->addWidget(projectname);
    templayout->addWidget(projectauthors);
    templayout->addWidget(hyperlink);

    aboutdialog->setLayout(templayout);
    aboutdialog->exec();

    delete hyperlink;
    delete projectauthors;
    delete projectname;
    delete templayout;
    delete aboutdialog;
}


