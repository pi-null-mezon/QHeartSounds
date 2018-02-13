#include "qeasyplot.h"

#include <QPainter>
#include <QPainterPath>
#include <QColorDialog>
#include <QFontDialog>
#include <QLinearGradient>
#include <QBrush>
#include <QSurfaceFormat>

QEasyPlot::QEasyPlot(QWidget *parent) :
    QOpenGLWidget(parent),
    pt_Array(NULL),
    m_DrawRegime(TraceRegime)
{
    // Opengl quality boost!
    setAutoFillBackground(false);
    QSurfaceFormat _format = format();
    _format.setSamples(2);
    setFormat(_format);

    set_defaultValues();
}

void QEasyPlot::updateSignal(qreal _min, qreal _max, const qreal *_pt, quint32 length)
{
    set_vertical_Borders(_min, _max);
    set_horizontal_Borders(0.0,length-1);
    pt_Array = _pt;
    m_ArrayLength = length;
    update();
}

void QEasyPlot::updateSignal(const qreal *_pt, quint32 length)
{
    auto _mm = std::minmax_element(_pt, _pt+length);
    set_vertical_Borders(0.0, *(_mm.second));
    set_horizontal_Borders(0.0,length-1);
    pt_Array = _pt;
    m_ArrayLength = length;
    update();
}


void QEasyPlot::set_defaultValues()
{
    pt_Array = NULL;
    m_ArrayLength = 0;
    //visual appearance
    m_textMargin = 3;
    m_backgroundColor = QColor(Qt::gray);

    m_tracePen = QPen( Qt::NoBrush, 1.5, Qt::SolidLine);
    m_tracePen.setColor(QColor(0,255,0));
    m_coordinatePen = QPen( Qt::NoBrush, 0.5, Qt::DashDotDotLine);
    m_coordinatePen.setColor(QColor(255,255,255,100));
    m_coordinateSystemFont = QFont("Calibri", 10, QFont::DemiBold, true );
    m_DrawRegime = QEasyPlot::TraceRegime;

    set_X_step(1.0);
    set_X_Ticks(11);
    set_Y_Ticks(11);
    set_coordinatesPrecision(2,1);
    set_horizontal_Borders(0.0,10.0);
    set_vertical_Borders(-5.0,5.0);
}

void QEasyPlot::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), m_backgroundColor);

    painter.setRenderHint(QPainter::Antialiasing);
    draw_coordinateSystem(painter);
    draw_externalArray(painter);
}

void QEasyPlot::draw_coordinateSystem(QPainter &painter)
{
    painter.setPen(m_coordinatePen);
    painter.setFont(m_coordinateSystemFont);

    qreal widgetXstep = (qreal)width() / (m_XTicks - 1);
    qreal widgetYstep = (qreal)height() / (m_YTicks - 1);

    qreal tempCoordinate = widgetYstep;
    for(quint16 i = 1; i < m_YTicks - 1; i++)  {
        painter.drawLine(0, tempCoordinate, width(), tempCoordinate);
        painter.drawText(m_textMargin, tempCoordinate - m_textMargin, QString::number((m_topBorder - i*m_yStep), 'E', m_coordinatesYPrecision));
        tempCoordinate += widgetYstep;
    }
    tempCoordinate = widgetXstep;
    for(quint16 i = 1; i < m_XTicks - 1; i++) {
        painter.drawLine(tempCoordinate, 0, tempCoordinate, height());
        //painter.drawText(tempCoordinate + m_textMargin, height() - m_textMargin, QString::number((m_leftBorder + i*m_xStep), 'f', m_coordinatesXPrecision));
        tempCoordinate += widgetXstep;
    }
   painter.drawLine(convertXY(0.0,m_topBorder),convertXY(0.0,m_bottomBorder));
   painter.drawLine(convertXY(m_leftBorder,0.0),convertXY(m_rightBorder,0.0));

   painter.drawStaticText(m_textMargin, m_textMargin, m_Y_axisName);
   painter.drawStaticText(m_textMargin, height() - 3.5*m_coordinateSystemFont.pointSize() - m_textMargin , m_X_axisName);
}

QPointF QEasyPlot::convertXY(qreal x, qreal y) const
{
    return QPointF( (x - m_leftBorder)*width()/(m_rightBorder - m_leftBorder), (m_topBorder - y)*height()/(m_topBorder - m_bottomBorder) );
}

void QEasyPlot::draw_externalArray(QPainter &painter)
{
    if(pt_Array) {

        painter.setPen(m_tracePen);
        QPainterPath path1;
        switch(m_DrawRegime) {

            case QEasyPlot::TraceRegime:
                path1.moveTo(convertXY( 0.0 , pt_Array[0] ));
                for(quint32 i = 1; i < m_ArrayLength; i++)
                    path1.lineTo( convertXY( stepX*i, pt_Array[i] ) );
                painter.drawPath(path1);
            break;

            case QEasyPlot::FilledTraceRegime:
                path1.moveTo(convertXY( 0.0 , 0.0 )); // only for filling, from zero
                path1.lineTo(convertXY( 0.0 , pt_Array[0] ));
                for(quint32 i = 1; i < m_ArrayLength; i++)
                    path1.lineTo( convertXY( stepX*i, pt_Array[i] ) );
                path1.lineTo(convertXY( m_ArrayLength-1, 0.0 )); // only for filling, to zero                
                painter.fillPath(path1, Qt::Dense3Pattern);
                painter.drawPath(path1);

            break;

            case QEasyPlot::PhaseRegime: {
                int _shift = 3;
                path1.moveTo(convertXY( pt_Array[0] , pt_Array[_shift] ));
                for(quint32 i = 1; i < (m_ArrayLength - _shift); i++)
                    path1.lineTo( convertXY( pt_Array[i], pt_Array[i + _shift] ) );
                painter.drawPath(path1);
            }
            break;
        }
    }
}

bool QEasyPlot::set_horizontal_Borders(qreal left_value, qreal right_value)
{
    if(right_value > left_value)  {
        m_rightBorder = right_value;
        m_leftBorder = left_value;
        update_X_TicksStep();
        return true;
    }
    return false;
}

bool QEasyPlot::set_vertical_Borders(qreal bottom_value, qreal top_value)
{
    if(top_value > bottom_value)   {
        m_topBorder = top_value;
        m_bottomBorder = bottom_value;
        update_Y_TicksStep();
        return true;
    }
    return false;
}

void QEasyPlot::update_X_TicksStep()
{
    m_xStep = (m_rightBorder - m_leftBorder) / (m_XTicks - 1);
}

void QEasyPlot::update_Y_TicksStep()
{
    m_yStep = (m_topBorder - m_bottomBorder) / (m_YTicks - 1);
}

void QEasyPlot::set_coordinatesPrecision(quint8 value_for_x, quint8 value_for_y)
{
   m_coordinatesXPrecision = value_for_x;
   m_coordinatesYPrecision = value_for_y;
}

bool QEasyPlot::set_X_Ticks(quint16 value)
{
    if(value > 2)  {
        m_XTicks = value;
        update_X_TicksStep();
        return true;
    }
    return false;
}

bool QEasyPlot::set_Y_Ticks(quint16 value)
{
    if(value > 2)  {
        m_YTicks = value;
        update_Y_TicksStep();
        return true;
    }
    return false;
}

void QEasyPlot::set_coordinatePen(const QPen &pen,const QColor &color)
{
    m_coordinatePen = pen;
    m_coordinatePen.setColor(color);
}

void QEasyPlot::set_tracePen(const QPen &pen, const QColor &color)
{
    m_tracePen = pen;
    m_tracePen.setColor(color);
}

bool QEasyPlot::open_colorSelectDialog_for(VisualEntity value)
{
    QColor default_color;
    switch(value)  {
        case QEasyPlot::Background:
            default_color = QColor(55,55,55);
            break;
        case QEasyPlot::Coordinates:
            default_color = QColor(Qt::white);
            break;
        case QEasyPlot::Trace:
            default_color = QColor(Qt::green);
            break;
        default:
            default_color = Qt::red;
    }
    QColor result_color = QColorDialog::getColor(default_color, this);
    if(result_color.isValid())  {
        switch(value)  {
            case QEasyPlot::Background:
                m_backgroundColor = result_color;
                break;
            case QEasyPlot::Coordinates:
                m_coordinatePen.setColor( result_color );
                break;
            case QEasyPlot::Trace:
                m_tracePen.setColor( result_color );
                break;
        }
        return true;
    }
    return false;
}

bool QEasyPlot::open_fontSelectDialog()
{
    bool result_flag;
    QFont result_font = QFontDialog::getFont(&result_flag, m_coordinateSystemFont, this);
    if(result_flag)  {
        m_coordinateSystemFont = result_font;
        return true;
    }
    return false;
}

void QEasyPlot::set_axis_names(const QString &name_for_x, const QString &name_for_y)
{
    m_X_axisName = QStaticText(name_for_x);
    m_Y_axisName = QStaticText(name_for_y);
}

void QEasyPlot::set_DrawRegime(DrawRegime value)
{
    m_DrawRegime = value;
}

bool QEasyPlot::open_traceColorDialog()
{
    return open_colorSelectDialog_for(Trace);
}

bool QEasyPlot::open_backgroundColorDialog()
{
    return open_colorSelectDialog_for(Background);
}

bool QEasyPlot::open_coordinatesystemColorDialog()
{
    return open_colorSelectDialog_for(Coordinates);
}

void QEasyPlot::set_X_step(qreal value)
{
    stepX = value;
}


