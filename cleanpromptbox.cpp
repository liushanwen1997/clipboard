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
#include "cleanpromptbox.h"
#include "customstyle.h"
#include "customstyle_pushbutton_2.h"
#include <QDebug>
CleanPromptBox::CleanPromptBox()
{
    this->setFixedSize(400, 218);
    this->setObjectName("CleanPromptBoxWidget");
    m_pHintInformationWidget = new QWidget();
    m_pCheckBoxWidget        = new QWidget();
    m_pButtonWidget          = new QWidget();
    m_pButtonWidget->setObjectName("ButtonWidget");

    m_pHintInformationWidget->setContentsMargins(0, 0, 0, 0);
    m_pCheckBoxWidget->setContentsMargins(0, 0, 0, 0);
    m_pButtonWidget->setContentsMargins(0, 0, 0, 0);

    creatorHintInfomationWidget(); //创建提示信息
    creatorCheckBoxWidget();       //创建checkBox
    creatorButtonWidget();         //创建QPushbutton
    creatorCleanPromptBoxWidget();

    this->setLayout(m_pHintWidgetVLaout);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    return;
}

void CleanPromptBox::creatorHintInfomationWidget()
{
    m_pIconLableHLaout = new QHBoxLayout;
    m_pIconLableHLaout->setContentsMargins(0, 0, 0, 0);

    QIcon tipIcon = QIcon::fromTheme("user-trash", QIcon(CLEAN_PNG_PATH));
    m_pIconButton = new QPushButton();
    m_pIconButton->setObjectName("IconButton");
    /* 设置按钮背景颜色 */
    QPalette palette = m_pIconButton->palette();
    QColor ColorPlaceholderText(255,255,255,0);
    QBrush brush;
    brush.setColor(ColorPlaceholderText);
    palette.setBrush(QPalette::Button, brush);
    palette.setBrush(QPalette::ButtonText, brush);
    palette.setColor(QPalette::Highlight, Qt::transparent); /* 取消按钮高亮 */
    m_pIconButton->setPalette(palette);

    m_pIconButton->setFixedSize(48, 48);
    m_pIconButton->setIcon(tipIcon);
    m_pIconButton->setIconSize(QSize(48,48));

    m_pHintInformation = new QLabel(QObject::tr("Are you sure empty your clipboard history?"));
    m_pHintInformation->setFixedSize(250, 30);
    m_pIconLableHLaout->addItem(new QSpacerItem(31,20));
    m_pIconLableHLaout->addWidget(m_pIconButton);
    m_pIconLableHLaout->addItem(new QSpacerItem(16, 20));
    m_pIconLableHLaout->addWidget(m_pHintInformation);
    m_pIconLableHLaout->addItem(new QSpacerItem(55, 20));
    m_pIconLableHLaout->setSpacing(0);
    m_pHintInformationWidget->setLayout(m_pIconLableHLaout);
    return;
}

void CleanPromptBox::creatorCheckBoxWidget()
{
    m_pCheckBoxNoHint = new QCheckBox;
    m_pCheckBoxNoHint->setText(QObject::tr("Don't ask"));
    m_pCheckBoxNoHint->setFixedSize(85, 20);
    m_pChechBoxHLaout = new QHBoxLayout;
    m_pChechBoxHLaout->setContentsMargins(0,0,0,0);
    m_pChechBoxHLaout->setSpacing(0);
    m_pChechBoxHLaout->addItem(new QSpacerItem(95, 20));
    m_pChechBoxHLaout->addWidget(m_pCheckBoxNoHint);
    m_pChechBoxHLaout->addItem(new QSpacerItem(220, 20));
    m_pCheckBoxWidget->setLayout(m_pChechBoxHLaout);
    return;
}

void CleanPromptBox::creatorButtonWidget()
{
    m_pConfirmButton  = new QPushButton(QObject::tr("Confirm"));
    m_pConfirmButton->setObjectName("ConfirmButton");
    m_pCancelButton   = new QPushButton(QObject::tr("Cancel"));
    m_pCancelButton->setObjectName("CancelButton");

    connect(m_pConfirmButton, &QPushButton::clicked, this, &CleanPromptBox::ConfirmButtonSlots);
    connect(m_pConfirmButton, &QPushButton::clicked, this, &CleanPromptBox::accept);
    connect(m_pCancelButton, &QPushButton::clicked, this, &CleanPromptBox::reject);

    /* 设置按钮字体\背景颜色 */
    m_pCancelButton->setStyle(new CustomStyle_pushbutton_2("ukui-default"));

    /* 设置按钮字体\背景颜色 */
    m_pConfirmButton->setStyle(new CustomStyle("ukui-default"));

    m_pConfirmButton->setFixedSize(120, 34);
    m_pCancelButton->setFixedSize(120, 34);
    m_pButtonHLaout   = new QHBoxLayout;
    m_pButtonHLaout->setContentsMargins(0, 0, 0, 0);
    m_pButtonHLaout->setSpacing(0);
    m_pButtonHLaout->addItem(new QSpacerItem(124, 20));
    m_pButtonHLaout->addWidget(m_pCancelButton);
    m_pButtonHLaout->addItem(new QSpacerItem(16, 20));
    m_pButtonHLaout->addWidget(m_pConfirmButton);
    m_pButtonHLaout->addItem(new QSpacerItem(20, 10));
    m_pButtonWidget->setLayout(m_pButtonHLaout);
    return;
}


void CleanPromptBox::creatorCleanPromptBoxWidget()
{
    m_pHintWidgetVLaout = new QVBoxLayout;
    m_pHintWidgetVLaout->setContentsMargins(0, 0, 0, 0);
    m_pHintWidgetVLaout->setSpacing(0);
    m_pHintWidgetVLaout->addItem(new QSpacerItem(20, 48));
    m_pHintWidgetVLaout->addWidget(m_pHintInformationWidget);
    m_pHintWidgetVLaout->addItem(new QSpacerItem(10, 15));
    m_pHintWidgetVLaout->addWidget(m_pCheckBoxWidget);
    m_pHintWidgetVLaout->addItem(new QSpacerItem(10, 41));
    m_pHintWidgetVLaout->addWidget(m_pButtonWidget);
    m_pHintWidgetVLaout->addItem(new QSpacerItem(10, 18));
    return;
}

void CleanPromptBox::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);

//    p.setBrush(QBrush(QColor("#131314")));
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setOpacity(0.42);
    p.setPen(Qt::NoPen);

    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,6,6);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return;
}

void CleanPromptBox::ConfirmButtonSlots()
{
    if (m_pCheckBoxNoHint->isChecked()) {
        emit ClipBoardInternalSignal::getGlobalInternalSignal()->CheckBoxSelectedSignal();
        qDebug() << "是选中状态";
    }
    return;
}
