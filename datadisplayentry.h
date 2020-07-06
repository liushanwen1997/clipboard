#ifndef DATADISPLAYENTRY_H
#define DATADISPLAYENTRY_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QTextEdit>
class dataDisplayentry : public QWidget
{
    Q_OBJECT
public:
    explicit dataDisplayentry(QWidget *parent = nullptr);
    QWidget  *textEditWidget;
    QWidget  *OperatingWidget;

    QVBoxLayout *entryVLayout;
    QHBoxLayout *OperatingHLayout;

    QTextEdit   *TextEdit;

    QLabel      *timeLabel;
    QLabel      *inforLabel;

    QPushButton *m_pLockButton;
    QPushButton *m_pEditButon;
    QPushButton *m_pRemoveButton;
    QPushButton *m_pCancelLockButton;

signals:

public slots:
};

#endif // DATADISPLAYENTRY_H
