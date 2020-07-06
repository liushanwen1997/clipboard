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


#include "searchwidgetitemcontent.h"
#include <QApplication>
#include <QStyle>
#include <QFile>

SearchWidgetItemContent::SearchWidgetItemContent(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("SearhWidget");
    this->setContentsMargins(0,0,0,0);

    m_pHBoxLayout = new QHBoxLayout();
    m_pHBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_pClipboardTextLabel = new QLabel(QObject::tr("clipboard"));
    QFont font = m_pClipboardTextLabel->font();
    font.setFamily("Noto Sans CJK SC");
    font.setPixelSize(22);
    m_pClipboardTextLabel->setFont(font);
    m_pClipboardTextLabel->setFixedHeight(26);

    m_pClearListWidgetButton = new QPushButton(tr("Clear"));
    m_pClearListWidgetButton->setFixedSize(80, 34);
    m_pClearListWidgetButton->setStyle(new customstyle_search_pushbutton("ukui-default"));
    m_pClearListWidgetButton->setObjectName("CleanList");

    m_pHBoxLayout->addWidget(m_pClipboardTextLabel);
    m_pHBoxLayout->addItem(new QSpacerItem(183, 0, QSizePolicy::Expanding));
    m_pHBoxLayout->addWidget(m_pClearListWidgetButton);
    m_pHBoxLayout->addItem(new QSpacerItem(0,0));
    m_pHBoxLayout->setSpacing(0);
    this->setLayout(m_pHBoxLayout);
    this->setFixedSize(328, 34);
}
