/*------------------------------------------------------------------------------
Taranov Alex, 2014
An attempt to make widget with basic plot capabilities. Fell free to use it in
terms of free BSD license, but do not forget to attach this information.
The widget was created in Qt-creator 5.2.1.
------------------------------------------------------------------------------*/

#ifndef QPLOT_H
#define QPLOT_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QString>
#include <QMargins>
#include <QByteArray>
#include <QStaticText>

class QPlot : public QWidget
{
    Q_OBJECT
public:
    explicit QPlot(QWidget *parent = 0, QString caption = "NoName", bool BA_flag = false, bool D_flag = false);

signals:

public slots:
    void read_ByteArray(const QByteArray& data);   
    void read_Data(const qreal *pt_input, quint32 length);

    void set_caption(const QString &new_name);
    void set_string(const QString &string);
    void set_draw_ByteArray_flag(bool value);
    void set_draw_Data_flag(bool value);
    void set_unsigned(bool value);

protected:
    void paintEvent(QPaintEvent* /*event*/);

private:
    void draw_caption(QPainter &painter);
    void draw_coordinate_system(QPainter &painter);
    void draw_ByteArray(QPainter &painter);
    void draw_Data(QPainter &painter);
    void draw_string(QPainter &painter);

    QByteArray m_data;
    const qreal *pt_Data;
    quint32 Data_length;
    QStaticText m_caption;
    QString m_string;
    bool draw_ByteArray_flag;
    bool draw_Data_flag;
    bool unsigned_flag;
};

#endif // QPLOT_H
