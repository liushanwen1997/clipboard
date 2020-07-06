/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef CLIPBOARDWIDGETENTRY_H
#define CLIPBOARDWIDGETENTRY_H
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>
#include <QLine>
#include <QGroupBox>
#include <QEvent>
#include <QDebug>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QTextEdit>
#include <QStackedLayout>
#include "clipboardsignal.h"
#include "clipBoardInternalSignal.h"
#include "pixmaplabel.h"
#include "elidedlabel.h"

#define  EDIT_SVG_PATH  ":/image/editor.svg"
#define  COPY_SVG_PATH  ":/image/copy.svg"
#define  REMOVE_SVG_PATH  ":/image/delete.svg"
#define  SEARCH_SVG_CLEAN ":/image/button-close-hover-click-two.svg"
#define  SEARCH_SVG_CLEAN_BACK ":/image/button-close-hover-click-add-background-one.svg"
#define  LOCK_SVG_PATH    ":/image/lock.png"
#define  CANCEL_LOCK_PNG_PATH ":/image/cancel-lock.png"

#define  ENTRYURL    "Url"
#define  ENTRYIMAGE  "Image"
#define  ENTRYTEXT   "Text"

class ClipboardWidgetEntry : public QWidget
{
    Q_OBJECT
public:
    ClipboardWidgetEntry(QString dataFormat, QWidget *parent = nullptr);

    QPushButton  *m_pLockButton;
    QPushButton  *m_pEditButon;
    QPushButton  *m_pRemoveButton;
    QPushButton  *m_pCancelLockButton;

    ElidedLabel  *m_pCopyDataLabal;
    pixmapLabel  *m_pCopyFileIcon;

    QLabel       *m_pTimeLabel;
    QLabel       *m_pInforLabel;
    QLabel       *m_plineLabel;
    QLabel       *m_pUrlLabel;

    QVBoxLayout  *m_pMianVLayout;
    QVBoxLayout  *m_pdataVLayout;
    QHBoxLayout  *m_pHLayout;
    QHBoxLayout  *m_pPixlabelHLayout;

    QString      m_ptext;
    QString      m_dataFormat;

    QWidget      *m_pUpDataWidget;
    QWidget      *m_pdownOperaWidget;
    QWidget      *m_pPixlabelWidget;

    bool         m_bWhetherFix = false;

    QString SetFormatBody(QString text);
    QString setMiddleFormatBody(QString text);
    QString judgeBlankLine(QStringList list);                               /* 去除掉空行，显示有字体的行 */

    void    initPushbutton();                                               /* 初始化三个按钮 */
    void    initLable();                                                    /* 初始化Lable */

    void    initOperaArea();                                                /* 初始化下半部分的界面 */

    void    initdataDisplayArea();                                          /* 初始化放置图标的Icon */

    void    initButtomLayout();                                             /* 初始化下半部分布局 */

    void    initImageLayout();                                              /* 当为图像时，初始化布局方式 */

    void    initAllLayout();                                                /* 初始化整体布局 */

    bool    substringSposition(QString formatBody, QStringList list);       /* 判断后面是否还有子串 */


    enum TaskWidgetStatus{NORMAL, HOVER, PRESS};
    TaskWidgetStatus status;

protected:
    void enterEvent(QEvent *);                      //进入QWidget瞬间事件
    void leaveEvent(QEvent *);                      //离开QWidget瞬间事件
    void mouseDoubleClickEvent(QMouseEvent *event); //双击事件
    void mousePressEvent(QMouseEvent *event);       //单击事件
    void paintEvent(QPaintEvent *e);                //重绘事件

signals:
    void doubleClicksignals(QWidget *w);
};

#endif // CLIPBOARDWIDGETENTRY_H
