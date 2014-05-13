#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QContextMenuEvent>
#include <QStatusBar>
#include "qsoundprocessor.h"
#include "qplot.h"
#include "qharmonicprocessor.h"
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    bool open_device();
    void pause_device();
    void resume_device();
    bool close_device();
    void open_volume_dialog();
    int start_new_record();
    void about_dialog();

private:  
    void contextMenuEvent(QContextMenuEvent *event);
    QWidget *pt_placeholder;
    QVBoxLayout *pt_main_layout;
    QPlot *pt_buffer_plot;
    QPlot *pt_record_plot;
    QPlot *pt_spectrum_plot;
    QMenu *pt_menu_main;
    QAction *pt_action_exit;
    QAction *pt_action_volume;
    QAction *pt_action_open;
    QAction *pt_action_pause;
    QAction *pt_action_resume;
    QAction *pt_action_close;
    QAction *pt_action_newRecord;
    QAction *pt_action_about;
    QSoundProcessor *pt_sound_device;
    QHarmonicProcessor *pt_harmonicprocessor;
    QTimer m_timer;
};

#endif // MAINWINDOW_H
