/*------------------------------------------------------------------------------
Taranov Alex, 2014
An attempt to make widget with basic plot capabilities. Fell free to use it in
terms of free BSD license, but do not forget to attach this information.
The widget was created in Qt-creator 5.2.1.
------------------------------------------------------------------------------*/
#include "qplot.h"
#include <QDebug>


QPlot::QPlot(QWidget *parent, QString caption, bool BA_flag, bool D_flag) :
    QWidget(parent),
    m_caption(caption),
    m_string(""),
    draw_ByteArray_flag(BA_flag),
    draw_Data_flag(D_flag),
    pt_Data(NULL),
    Data_length(0),
    unsigned_flag(false)
{
}

void QPlot::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    draw_coordinate_system(painter);
    draw_caption(painter);
    draw_string(painter);
    if(draw_ByteArray_flag)
    {
        draw_ByteArray(painter);
    }
    if(draw_Data_flag)
    {
        draw_Data(painter);
    }
}

void QPlot::draw_coordinate_system(QPainter &painter)
{
    //Pen settings
    QPen pen(QBrush(Qt::NoBrush),1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setColor(QColor(225,225,225));
    painter.setPen(pen);

    //Drawing operations
    painter.fillRect(this->rect(),QColor(75,75,75));
    painter.drawLine(0, this->rect().height()/2, this->rect().width(), this->rect().height()/2);
}

void QPlot::draw_ByteArray(QPainter& painter)
{
    //Pen settings
    QPen pen(QBrush(Qt::NoBrush),2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setColor(QColor(0,255,0));
    painter.setPen(pen);

    //Drawing operations
    if( m_data.length() > 0 )
    {
        qreal step_X = (qreal)this->width()/m_data.length();
        qreal step_Y = (qreal)this->height()/(65535); // 2 in 16-th degree minus 1

        quint16 start_Y = this->height()/2;

        for(qint64 i = 2; i < m_data.length() - 1; i += 2)
        {
           painter.drawLine((i-2) * step_X, start_Y - step_Y * ((quint8)m_data.at(i-2) | ((qint16)m_data.at(i-1) << 8)), i * step_X, start_Y - step_Y * ((quint8)m_data.at(i) | ((qint16)m_data.at(i+1) << 8)));
        }
    }
}


void QPlot::read_ByteArray(const QByteArray &data)
{  
    m_data = data;
    this->update();
    qWarning() << "readData signal was occured, m_data.length:" << m_data.length();
}

void QPlot::draw_caption(QPainter &painter)
{
    //Font settings
    QFont font("Tahoma", 10.0, QFont::Bold);
    painter.setFont(font);

    //Drawing operations
    painter.drawStaticText(5, 5, m_caption);
}

void QPlot::draw_string(QPainter &painter)
{
    //Font settings
    painter.setPen(QColor(200,0,0));
    QFont font("Tahoma", 10.0, QFont::DemiBold);
    painter.setFont(font);

    //Drawing operations
    painter.drawText(5, this->rect().height()-5, m_string);
}

void QPlot::set_caption(const QString &new_name)
{
    m_caption.setText(new_name);
}

void QPlot::set_string(const QString &string)
{
    m_string = string;
}

void QPlot::set_draw_ByteArray_flag(bool value)
{
    draw_ByteArray_flag = value;
}

void QPlot::set_draw_Data_flag(bool value)
{
    draw_Data_flag = value;
}

void QPlot::read_Data(const qreal *pt_input, quint32 length)
{
    pt_Data = pt_input;
    Data_length = length;
    this->update();
}

void QPlot::draw_Data(QPainter &painter)
{
    if(pt_Data != NULL)
    {
        //Pen settings
        QPen pen(QBrush(Qt::NoBrush), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setColor(QColor(255,255,0));
        painter.setPen(pen);

        //Drawing operations
        qreal st_X = (qreal)this->rect().width() / Data_length;
        qreal st_Y = (qreal)this->rect().height() / 100;

        qint32 Y0 = this->rect().height()/2;
        if(unsigned_flag)
        {
            Y0 = this->rect().height();
        }

        QPainterPath path;
        path.moveTo(0, Y0);
        for(quint32 i = 0; i < Data_length; i++)
        {
            path.lineTo(st_X*i, Y0 - st_Y*pt_Data[i]);
        }
        painter.drawPath(path);
    }
}

void QPlot::set_unsigned(bool value)
{
    unsigned_flag = value;
}

