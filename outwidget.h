#ifndef OUTWIDGET_H
#define OUTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QStyleOption>
#include <QPainter>

class OutWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OutWidget(QWidget *parent = nullptr);
    ~OutWidget();

signals:

protected:
    void paintEvent(QPaintEvent *);

};

#endif // OUTWIDGET_H
