#ifndef DIALOGSHOWGRAPH_H
#define DIALOGSHOWGRAPH_H

#include <QDialog>
#include <qcustomplot/qcustomplot.h>

namespace Ui {
class DialogShowGraph;
}

class DialogShowGraph : public QDialog
{
    Q_OBJECT

public:
    explicit DialogShowGraph(QWidget *parent = 0);
    ~DialogShowGraph();

    void setParentPlot(QCustomPlot *parentPlot);

private:
    Ui::DialogShowGraph *ui;

    // plot
    QCustomPlot *_customPlot;
    QCustomPlot *_parentPlot;
};

#endif // DIALOGSHOWGRAPH_H
