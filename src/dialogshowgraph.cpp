#include "dialogshowgraph.h"
#include "ui_dialogshowgraph.h"
#include <QDebug>

DialogShowGraph::DialogShowGraph(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogShowGraph),
    _customPlot(nullptr)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setFixedSize(this->width(), this->height());

    // plot
    _customPlot = ui->widgetShowPlot;
    _customPlot->setInteractions(QCP::iRangeDrag |
                                 QCP::iRangeZoom |
                                 QCP::iSelectPlottables);

    _customPlot->xAxis->setAutoTicks(false);

    _customPlot->xAxis->setRange(0, 1);
    _customPlot->yAxis->setRange(0, 1);
}

DialogShowGraph::~DialogShowGraph()
{
    delete _customPlot;
    delete ui;
}

void DialogShowGraph::setParentPlot(QCustomPlot *parentPlot)
{
    if (!parentPlot)
    {
        return;
    }

    _parentPlot = parentPlot;

    if(_parentPlot->graphCount() != 0)
    {
        QCPDataMap *dataMap = _parentPlot->graph(0)->data();
        _customPlot->addGraph();
        _customPlot->graph(0)->setData(dataMap, true);
        _customPlot->xAxis->setLabel("test num");
        _customPlot->yAxis->setLabel("time, ms");


        QMap<double, QCPData>::const_iterator i = dataMap->constBegin();

        int maxTimeEx = 0;

        QVector<double> testNum(1);
        testNum[0] = 0;

        while (i != dataMap->constEnd())
        {
            if (maxTimeEx < i.value().value)
            {
                maxTimeEx = i.value().value;
            }
            testNum.append(i.value().key + 1);
            ++i;
        }

        _customPlot->xAxis->setTickVector(testNum);

        _customPlot->xAxis->setRange(0, dataMap->size() + 1);
        _customPlot->yAxis->setRange(0, maxTimeEx + (maxTimeEx/100 * 5)); // + 5%

        _customPlot->graph(0)->rescaleAxes();
        _customPlot->replot();
    }
}
