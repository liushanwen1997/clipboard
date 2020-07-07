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


#include "clipboardwidgetentry.h"
#include "customstyle_clean_pushbutton.h"
#include <QTimer>
#include <QStyleOption>
#include <QPainter>
#include <QPalette>
#include <QEvent>

ClipboardWidgetEntry::ClipboardWidgetEntry(QString dataFormat, QWidget *parent)
{
    Q_UNUSED(parent);
    m_dataFormat = dataFormat;
    status=NORMAL;
    this->setObjectName("WidgetEntry");
    this->setContentsMargins(0,0,0,0);

    m_pMianVLayout      = new QVBoxLayout;
    m_pUpDataWidget     = new QWidget;
    m_pdownOperaWidget  = new QWidget;

    m_plineLabel        = new QLabel();
    m_plineLabel->setFixedHeight(1);
    m_plineLabel->setStyleSheet("QLabel{border: 1px solid rgba(255,255, 255, 0.08);}");

    m_pMianVLayout->setContentsMargins(0, 0, 0, 0);
    m_pUpDataWidget->setContentsMargins(0, 0, 0, 0);
    m_pdownOperaWidget->setContentsMargins(0, 0, 0, 0);

    /* 初始化三个按钮 */
    initPushbutton();

    /* 初始化Lable */
    initLable();

    /* 初始化下半部分成员变量 */
    initOperaArea();

    /* 初始化上半部分布局 */
    initdataDisplayArea();

    /* 初始化下半部分布局 */
    initButtomLayout();

    if (dataFormat != ENTRYIMAGE) {
        /* 初始化整体布局 */
        initAllLayout();
    } else {
        /* 为图像时的布局 */
        initImageLayout();
    }
    this->setLayout(m_pMianVLayout);
}

/* 初始化三个按钮 */
void ClipboardWidgetEntry::initPushbutton()
{
    QIcon EditIcon;
    EditIcon.addFile(EDIT_SVG_PATH);
    QIcon PopIcon;
    PopIcon.addFile(LOCK_SVG_PATH);
    QIcon RemoveIcon;
    RemoveIcon.addFile(REMOVE_SVG_PATH);
    QIcon CancelIcon;
    CancelIcon.addFile(CANCEL_LOCK_PNG_PATH);
    m_pLockButton     = new QPushButton();
    m_pLockButton->setStyle(new customstyle_clean_pushbutton("ukui-default"));
    m_pLockButton->setToolTip(QObject::tr("Pop"));
    m_pLockButton->setFixedSize(34, 34);
    m_pLockButton->setIcon(PopIcon);
    m_pLockButton->setObjectName("PopButton");

    m_pEditButon     = new QPushButton();
    m_pEditButon->setStyle(new customstyle_clean_pushbutton("ukui-default"));
    m_pEditButon->setToolTip(QObject::tr("EditButton"));
    m_pEditButon->setFixedSize(34, 34);
    m_pEditButon->setIcon(EditIcon);
    m_pEditButon->setObjectName("EditButon");

    m_pRemoveButton  = new QPushButton();
    m_pRemoveButton->setStyle(new customstyle_clean_pushbutton("ukui-default"));
    m_pRemoveButton->setToolTip(QObject::tr("Remove"));
    m_pRemoveButton->setFixedSize(34, 34);
    m_pRemoveButton->setIcon(RemoveIcon);
    m_pRemoveButton->setObjectName("RemoveButton");

    m_pCancelLockButton = new QPushButton();
    m_pCancelLockButton->setStyle(new customstyle_clean_pushbutton("ukui-default"));
    m_pCancelLockButton->setToolTip(QObject::tr("Cancel the fixed"));
    m_pCancelLockButton->setFixedSize(34,34);
    m_pCancelLockButton->setIcon(CancelIcon);
    m_pCancelLockButton->setObjectName("cancel fixed the button");
    return;
}

/* 初始化Lable */
void ClipboardWidgetEntry::initLable()
{
    m_pCopyDataLabal = new ElidedLabel();
    m_pCopyDataLabal->setAlignment(Qt::AlignTop);
    m_pCopyDataLabal->setWordWrap(true);

    QTimer::singleShot(1, m_pCopyDataLabal, [=](){
        QFont font = m_pCopyDataLabal->font();
        font.setPixelSize(14);
        font.setFamily("Noto Sans CJK SC");
        m_pCopyDataLabal->setFont(font);
    });
    m_pCopyDataLabal->setObjectName("EntryLable");
    if (m_dataFormat == ENTRYTEXT) {
        m_pCopyDataLabal->setFixedWidth(328);
        m_pCopyDataLabal->setFixedHeight(104);
        m_pCopyDataLabal->setContentsMargins(3, 0, 0, 0);
    } else if (m_dataFormat == ENTRYURL) {
        m_pCopyDataLabal->setFixedHeight(20);
        m_pCopyDataLabal->setContentsMargins(0, 0, 0, 0);
    } else if (m_dataFormat == ENTRYIMAGE) {
        m_pCopyDataLabal->setFixedHeight(160);
        m_pCopyDataLabal->setContentsMargins(0, 0, 0, 0);
    }
}

/* 初始化下半部分的界面 */
void ClipboardWidgetEntry::initOperaArea()
{
    m_pTimeLabel        = new QLabel;
    m_pTimeLabel->setFixedHeight(32);

    m_pInforLabel       = new QLabel;
    m_pInforLabel->setFixedHeight(32);
}

/* 初始化上半部分的界面 */
void ClipboardWidgetEntry::initdataDisplayArea()
{
    m_pdataVLayout = new QVBoxLayout;
    m_pdataVLayout->setContentsMargins(0, 0, 0, 0);
    m_pdataVLayout->setSpacing(0);
    if (m_dataFormat == ENTRYURL) {
        m_pPixlabelWidget = new QWidget;
        m_pPixlabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pPixlabelWidget->setFixedHeight(80);

        m_pPixlabelHLayout = new QHBoxLayout;
        m_pPixlabelHLayout->setContentsMargins(0, 0, 0, 0);
        m_pPixlabelHLayout->setSpacing(0);

        m_pCopyFileIcon = new pixmapLabel;
        m_pCopyFileIcon->setFixedHeight(80);

        m_pPixlabelHLayout->addItem(new QSpacerItem(120, 20));
        m_pPixlabelHLayout->addWidget(m_pCopyFileIcon);

        m_pPixlabelWidget->setLayout(m_pPixlabelHLayout);

        m_pdataVLayout->addWidget(m_pPixlabelWidget);
        m_pCopyDataLabal->setFixedHeight(20);
        m_pCopyDataLabal->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    }
    m_pdataVLayout->addWidget(m_pCopyDataLabal);
    return;
}

 /* 初始化下半部分布局 */
 void ClipboardWidgetEntry::initButtomLayout()
 {
     m_pHLayout       = new QHBoxLayout();
     m_pHLayout->setContentsMargins(0,0,0,0);
     m_pHLayout->addItem(new QSpacerItem(10,20));
     m_pHLayout->addWidget(m_pTimeLabel);
     m_pHLayout->addItem(new QSpacerItem(270, 34, QSizePolicy::Expanding));
     m_pHLayout->addWidget(m_pInforLabel);
     m_pHLayout->addWidget(m_pLockButton);
     m_pHLayout->addWidget(m_pCancelLockButton);
     m_pCancelLockButton->setVisible(false);
     if (!(m_dataFormat == ENTRYURL
           || m_dataFormat == ENTRYIMAGE))
         m_pHLayout->addWidget(m_pEditButon);

     m_pHLayout->addWidget(m_pRemoveButton);
     m_pHLayout->addItem(new QSpacerItem(10,20));
     m_pHLayout->setSpacing(5);
     m_pLockButton->setVisible(false);
     m_pEditButon->setVisible(false);
     m_pRemoveButton->setVisible(false);
 }

 /* 当为图像时初始化布局方式 */
 void ClipboardWidgetEntry::initImageLayout()
 {
     m_pUpDataWidget->setFixedHeight(160);
     m_pUpDataWidget->setWindowFlags(Qt::WindowStaysOnBottomHint);
     m_pUpDataWidget->setLayout(m_pdataVLayout);

     m_pdownOperaWidget->setFixedHeight(37);
     m_pdownOperaWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
     m_pdownOperaWidget->setLayout(m_pHLayout);
     m_pdownOperaWidget->setParent(m_pUpDataWidget);
     m_pdownOperaWidget->move(0, 120);

     m_pMianVLayout->addWidget(m_pUpDataWidget);
     m_pMianVLayout->addWidget(m_plineLabel);
     m_pMianVLayout->addWidget(m_pdownOperaWidget);
     m_pMianVLayout->setSpacing(0);
     return;
 }

 /* 初始化整体布局 */
 void ClipboardWidgetEntry::initAllLayout()
 {
     m_pUpDataWidget->setLayout(m_pdataVLayout);
     m_pdownOperaWidget->setLayout(m_pHLayout);
     m_pMianVLayout->addWidget(m_pUpDataWidget);
     m_pMianVLayout->addWidget(m_plineLabel);
     m_pMianVLayout->addWidget(m_pdownOperaWidget);
     m_pMianVLayout->setSpacing(0);
     return;
 }

void ClipboardWidgetEntry::enterEvent(QEvent *e)
{
    if(e == nullptr) {
        return;
    }

    status=HOVER;
    repaint();

    m_pInforLabel->setVisible(false);
    if (m_dataFormat == ENTRYURL) {
    } else if (m_dataFormat == ENTRYIMAGE) {
    } else {
        m_pEditButon->setVisible(true);
    }

    if (m_bWhetherFix) {
        m_pCancelLockButton->setVisible(true);
    } else {
        m_pLockButton->setVisible(true);
    }

    m_pRemoveButton->setVisible(true);
    this->update();
    QWidget::enterEvent(e);
}

void ClipboardWidgetEntry::leaveEvent(QEvent *e)
{
    if (e == nullptr)
        return;
    status=NORMAL;
    m_pInforLabel->setVisible(true);
    if (m_bWhetherFix) {
        m_pCancelLockButton->setVisible(false);
    } else {
        m_pLockButton->setVisible(false);
    }
    m_pEditButon->setVisible(false);
    m_pRemoveButton->setVisible(false);

    this->update();
    QWidget::leaveEvent(e);
}

void ClipboardWidgetEntry::mousePressEvent(QMouseEvent *event)
{
    m_ptext = this->m_pCopyDataLabal->text();
    if (event->button() == Qt::LeftButton) {
        emit doubleClicksignals(this);
        QWidget::mousePressEvent(event);
        return;
    }
    QWidget::mousePressEvent(event);
    return;
}

void ClipboardWidgetEntry::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

QString ClipboardWidgetEntry::setMiddleFormatBody(QString text)
{
    QFontMetrics fontMetrics(m_pCopyDataLabal->font());
    int LableWidth = m_pCopyDataLabal->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if (fontSize > (LableWidth - 20)) {
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideMiddle, LableWidth - 20);
        return formatBody;
    }
    return formatBody;
}

/* 去除掉空行，显示有字体的行 */
QString ClipboardWidgetEntry::judgeBlankLine(QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        dest = dest.trimmed();
        if (dest.size() != 0)
           return list.at(i);
    }
    return list.at(0);
}

/* 判断当前子串位置，后面是否还有子串 */
bool ClipboardWidgetEntry::substringSposition(QString formatBody, QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        if (dest == formatBody && i == tmp - 1)
            return true;
    }
    return false;
}

void ClipboardWidgetEntry::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    switch (status) {
      case NORMAL: {
              p.setBrush(QBrush(QColor(255, 255, 255)));
              p.setOpacity(0.15);
              p.setPen(Qt::NoPen);
              break;
          }
      case HOVER: {
//              QColor color(opt.palette.color(QPalette::WindowText));
              QColor color(255, 255, 255);
              p.setBrush(QBrush(color));
              p.setOpacity(0.25);
              p.setPen(Qt::NoPen);
              break;
          }
      case PRESS: {
              break;
          }
    }
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect, 6, 6);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
