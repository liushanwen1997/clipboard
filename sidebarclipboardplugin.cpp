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

#include "sidebarclipboardplugin.h"
#include <QGuiApplication>
#include <QApplication>
#include <QMimeData>
#include <QFile>
#include <QTranslator>
#include <QStyle>
#include <QTimer>
#include <QStyleFactory>
#include <QTextFrame>
ClipboardSignal *globalClipboardSignal;
SidebarClipboardPlugin::SidebarClipboardPlugin(QWidget *parent)
{
    Q_UNUSED(parent);
    translator = new QTranslator;
    QLocale locale;
    //获取系统语言环境 选择翻译文件
    if ( locale.language() == QLocale::Chinese ) {
        translator->load(QString(":/translations/test_zh_CN.qm"));
        QApplication::installTranslator(translator);
    }
    installEventFilter(this);
    m_bPromptBoxBool = true;

    m_pClipboardDb = new clipboardDb();

    /* 创建剪贴板主Widget和搜索栏与条目的ListWidget界面 */
    createWidget();

    /* 创建无剪贴板板字样 */
    createTipLable();

    /* 创建查找条目 */
    createFindClipboardWidgetItem();

    /* 往文件类型链表中加入文件后缀类型 */
    AddfileSuffix();

    /* 插件内部通信的信号类 */
    ClipBoardInternalSignal::initInternalSignal();
    ClipBoardInternalSignal *InternalSignal = ClipBoardInternalSignal::getGlobalInternalSignal();

    /* 初始化右键菜单动作 */
    initMenuAction();

    /* 初始化mune*/
    initTray();

    /* 初始化图标 */
    initTrayIcon(QIcon::fromTheme("kylin-tool-box"));

    trayIcon->setVisible(true);

    /* 在点击确认键后判断是否有勾选不再提示这一功能 */
    connect(InternalSignal, &ClipBoardInternalSignal::CheckBoxSelectedSignal, this, [=]() {
        m_bPromptBoxBool = false;
    });

    /* 当剪贴板条目发生变化的时候执行该槽函数 */
    connect(this, &SidebarClipboardPlugin::Itemchange, this, &SidebarClipboardPlugin::ItemNumchagedSlots);

    /* 将控件加入到剪贴板界面中 */
    m_pClipboardLaout = new QVBoxLayout;
    m_pClipboardLaout->setContentsMargins(16,0,0,0);
    m_pClipboardLaout->addItem(new QSpacerItem(20, 16));
    m_pClipboardLaout->addWidget(m_pSearchWidgetListWidget);
    m_pClipboardLaout->addItem(new QSpacerItem(20, 7));
    m_pClipboardLaout->addWidget(m_pShortcutOperationListWidget);
    m_pClipboardLaout->addWidget(m_pSideBarClipboardLable);
    this->setLayout(m_pClipboardLaout);
    m_pShortcutOperationListWidget->setVisible(false);
    m_pClipboardLaout->setSpacing(0);
    m_pShortcutOperationListWidget->setObjectName("ShortcutOperationList");
    m_pSearchWidgetListWidget->setObjectName("SearchWidgetListWidget");
    m_pSideBarClipboardLable->setObjectName("SideBarClipboardLable");

    /* 监听系统剪贴板 */
    m_pSidebarClipboard = QApplication::clipboard();
    connect(m_pSidebarClipboard, &QClipboard::dataChanged, this, &SidebarClipboardPlugin::createWidgetEntry);

    /* 加载数据中中保存的数据，使用线程去加载 */
    m_pThread = new QThread;
    connect(m_pThread, &QThread::started, this, &SidebarClipboardPlugin::loadClipboardDb);
    m_pThread->start();

    /* 加载样式表 */
    QFile file(SIDEBAR_CLIPBOARD_QSS_PATH);
    if (file.open(QFile::ReadOnly)) {
        QString strQss = QLatin1String(file.readAll());
        this->setStyleSheet(strQss);
        file.close();
    }
}

/* 初始化右键菜单动作 */
void SidebarClipboardPlugin::initMenuAction()
{
    Open = new QAction(QObject::tr("Open"), this);
    connect(Open, &QAction::triggered, this, &SidebarClipboardPlugin::OpenClipboardWidget);

    Exit = new QAction(QObject::tr("Exit"), this);
    connect(Exit, &QAction::triggered, this, &SidebarClipboardPlugin::ExitClipboardWidget);

    Hide = new QAction(QObject::tr("Hide"), this);
    connect(Hide, &QAction::triggered, this, &SidebarClipboardPlugin::HideClipboardWidget);

    return;
}

/* 初始化任务栏图标变量 */
void SidebarClipboardPlugin::initTray()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->setProperty("iconHighlightEffectMode", 1);
    if (trayIconMenu == nullptr) {
        qWarning() << "分配空间trayIconMenu失败";
        return ;
    }

    trayIconMenu->addAction(Exit);
    trayIconMenu->addAction(Hide);
    trayIconMenu->addAction(Open);

    trayIcon = new QSystemTrayIcon(this);

    if (nullptr == trayIcon) {
        qWarning()<< "分配空间trayIcon失败";
        return ;
    }
    trayIcon->setContextMenu(trayIconMenu);
}

void SidebarClipboardPlugin::showEvent(QShowEvent *event)
{
    qDebug() << "123123";
    this->setAttribute(Qt::WA_Mapped);

    QWidget::showEvent(event);
}

/* 初始化图标 */
void SidebarClipboardPlugin::initTrayIcon(QIcon icon)
{
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->setToolTip(tr("Sidebar"));
}

/* 创建剪贴板主Widget和搜索栏与条目的ListWidget界面 */
void SidebarClipboardPlugin::createWidget()
{
    m_pShortcutOperationListWidget = new ClipBoardLisetWidget;
    m_pShortcutOperationListWidget->setContentsMargins(16, 0, 16, 0);
    m_pShortcutOperationListWidget->setFixedWidth(344);

    m_pSearchWidgetListWidget      = new QListWidget;
    m_pSearchWidgetListWidget->setFixedSize(400, 42);
    m_pSearchWidgetListWidget->setContentsMargins(0,0,0,0);
}

/* 创建无剪贴板板字样 */
void SidebarClipboardPlugin::createTipLable()
{
    m_pSideBarClipboardLable = new QLabel(tr("No clip content"));
    m_pSideBarClipboardLable->setContentsMargins(165, 0, 0, 50);
    QTimer::singleShot(1, m_pSideBarClipboardLable, [=](){
        QFont font = m_pSideBarClipboardLable->font();
        font.setPixelSize(14);
        font.setFamily("Noto Sans CJK SC");
        m_pSideBarClipboardLable->setFont(font);
    });
}

/* 创建查找条目 */
void SidebarClipboardPlugin::createFindClipboardWidgetItem()
{
    QListWidgetItem *pListWidgetItem = new QListWidgetItem;
    pListWidgetItem->setFlags(Qt::NoItemFlags);
    m_pSearchArea = new SearchWidgetItemContent;
    connect(m_pSearchArea->m_pClearListWidgetButton, &QPushButton::clicked, this, &SidebarClipboardPlugin::removeAllWidgetItem);
    pListWidgetItem->setSizeHint(QSize(400,38));
    m_pSearchWidgetListWidget->insertItem(0,pListWidgetItem);
    m_pSearchWidgetListWidget->setItemWidget(pListWidgetItem, m_pSearchArea);
}

/*创建Widgetitem条目*/
void SidebarClipboardPlugin::createWidgetEntry()
{
    const QMimeData *mimeData = m_pSidebarClipboard->mimeData();
    if (nullptr == mimeData) {
        qWarning() << "createWidgetEntry形参mimeData为空, 不创建";
        return;
    }
    QString text;
    QString format;
    QList<QUrl> fileUrls;
    QListWidgetItem *pListWidgetItem = new QListWidgetItem;
    OriginalDataHashValue *s_pDataHashValue = new OriginalDataHashValue;
    bool DeleteFlag = false;

    if (mimeData->hasImage()) {
        s_pDataHashValue->p_pixmap = new QPixmap((qvariant_cast<QPixmap>(mimeData->imageData())));
        format = IMAGE;
        if (nullptr == s_pDataHashValue->p_pixmap) {
           qWarning() << "构造数据类型有错误-->p_pixmap == nullptr";
           return;
        }
    } else if (mimeData->hasUrls()) {
        s_pDataHashValue->p_pixmap = nullptr;
        s_pDataHashValue->urls = mimeData->urls();
        text = parsingQStringList(mimeData->urls());
        format = URL;
    } else if (mimeData->hasText()) {
        s_pDataHashValue->p_pixmap = nullptr;
        text = mimeData->text();
        format = TEXT;
    } else {
        qWarning() << "剪贴板数据为空";
        return;
    }

    if (text == "" && s_pDataHashValue->p_pixmap == nullptr) {
        qWarning() << "text文本为空 或者 s_pDataHashValue->p_pixmap == nullptr";
        return ;
    }
    if (format == TEXT || format == URL) {
        if(booleanExistWidgetItem(text)) /* 过滤重复文本 */
            DeleteFlag = true;
    } else if (format == IMAGE) {
        if (booleanExistWidgetImagin(*s_pDataHashValue->p_pixmap))  /* 过滤重复图片 */
            DeleteFlag = true;
    }

    ClipboardWidgetEntry *w = new ClipboardWidgetEntry(format);

    if (DeleteFlag) {
        qDebug() << "此数据已存在，就是当前置顶的条数";
        delete pListWidgetItem;
        delete w;
        delete s_pDataHashValue;
        return;
    }

    /* hash插入QMimeData，保留原数据 */
    s_pDataHashValue->WidgetEntry  = w;
    s_pDataHashValue->Clipbaordformat = format;
    s_pDataHashValue->associatedDb = "";
    s_pDataHashValue->copyDate = QDateTime::currentDateTime().toMSecsSinceEpoch();

    if (s_pDataHashValue->Clipbaordformat == TEXT) {
        s_pDataHashValue->text     = text;
    } else if (s_pDataHashValue->Clipbaordformat == URL) {
        s_pDataHashValue->urls     = fileUrls;
        s_pDataHashValue->text     = text;
    }

    /* 设置Sequence */
    setOriginalDataSequence(s_pDataHashValue);
    qDebug() << "hash表中的Sequence" << s_pDataHashValue->Sequence;

    registerWidgetOriginalDataHash(pListWidgetItem, s_pDataHashValue);

    /* 当超过一定数目的WidgetItem数目时，删除最后一条消息 */
    if (m_pShortcutOperationListWidget->count() >= WIDGET_ENTRY_COUNT) {
        removeLastWidgetItem();
    }
    setEntryItemSize(s_pDataHashValue, w, pListWidgetItem);
    pListWidgetItem->setFlags(Qt::NoItemFlags);

    /* 设置左下角提示信息 */
    setCopyDataSize(s_pDataHashValue, w);

    /* 将text和图片写入到Widget */
    AddWidgetEntry(s_pDataHashValue, w, text);

    /* 将按钮与槽对应上 */
    connectWidgetEntryButton(w);

    /* 插入剪贴板条目 */
    m_pShortcutOperationListWidget->insertItem(0, pListWidgetItem);
    m_pShortcutOperationListWidget->setItemWidget(pListWidgetItem, w);
    emit Itemchange();
}

/* 根据剪贴板内容格式，写入到Label中 */
void SidebarClipboardPlugin::AddWidgetEntry(OriginalDataHashValue *s_pDataHashValue, ClipboardWidgetEntry *w, QString text)
{
    if (s_pDataHashValue->associatedDb == DBDATA) {
        w->m_pTimeLabel->setText(parseDateTime(QDateTime::fromMSecsSinceEpoch(s_pDataHashValue->copyDate)));
    } else {
        w->m_pTimeLabel->setText(getCurrutTime(QDateTime::fromMSecsSinceEpoch(s_pDataHashValue->copyDate)));
    }

    if (s_pDataHashValue->Clipbaordformat == TEXT) {
        // 设置...字样
        w->m_pCopyDataLabal->setTextFormat(Qt::PlainText);
        w->m_pCopyDataLabal->setContenText(text);
    } else if (s_pDataHashValue->Clipbaordformat == IMAGE) {
        int width = w->m_pCopyDataLabal->width();
        int height = w->m_pCopyDataLabal->height();
        QPixmap fitpixmap = (*s_pDataHashValue->p_pixmap).scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
        w->m_pCopyDataLabal->setPixmap(fitpixmap);
    } else if (s_pDataHashValue->Clipbaordformat == URL) {
        w->m_pCopyDataLabal->setTextFormat(Qt::PlainText);
        qDebug() << "当前的个数" << s_pDataHashValue->urls.size();
        // 当有多个文件时，显示特定图标，和特定的字符串显示
        QString specificText = setSpecificString(text);
        qDebug() << "specificText--->" << specificText;
        specificText = setMiddleFormatBody(specificText, w);
        w->m_pCopyDataLabal->setText(specificText);
        // 获取QPixmap链表
        getPixmapListFileIcon(text, w->m_pCopyFileIcon);
    }
    if (s_pDataHashValue->associatedDb == DBDATA) {
        w->m_bWhetherFix = true; // 数据库加载的都需要置为已经固定
        w->m_pLockButton->setVisible(false);
        w->m_pCancelLockButton->setVisible(false);
    }
    return;
}

/* 设置复制文字字符数 复制文件个数 图片名字 */
void SidebarClipboardPlugin::setCopyDataSize(OriginalDataHashValue *s_pDataHashValue, ClipboardWidgetEntry *w)
{
    QString formatString;
    if (s_pDataHashValue->Clipbaordformat == TEXT) {
        int size = s_pDataHashValue->text.size();
        formatString = QStringLiteral("%1%2").arg(size).arg(QObject::tr("字符"));
    } else if (s_pDataHashValue->Clipbaordformat == IMAGE) {
        formatString = QObject::tr("无信息");
    } else if (s_pDataHashValue->Clipbaordformat == URL) {
        int count = s_pDataHashValue->text.split("\n").size();
        formatString = QStringLiteral("%1%2").arg(count).arg(QObject::tr("个文件"));
    }
    w->m_pInforLabel->setText(formatString);
    return;
}

/* 设置...字样 */
QString SidebarClipboardPlugin::SetFormatBody(QString text, ClipboardWidgetEntry *w)
{
    QFontMetrics fontMetrics(w->m_pCopyDataLabal->font());
    int LableWidth = w->m_pCopyDataLabal->width();          // label一行的像素点宽度
    int LableHeight = w->m_pCopyDataLabal->height();        // label高度
    int fontSize = fontMetrics.width(text);                 // 整个字符串的像素点长度
    int pixelsHigh = fontMetrics.height();                  // 字体高度

    qDebug() << LableWidth << LableHeight;

    QString formatBody = "";
    QStringList formatBodyList = text.split('\n');
    int count = formatBodyList.count();
    int tmp[6] = {0, 0, 0, 0, 0, 0};
    int lineCount = 0;
    int lastIndex = 0;
    bool status = false;

    /* 将当前的字符串解析出来 */
    qDebug() << fontSize << LableWidth*(LableHeight/pixelsHigh) - 4*10 << LableWidth << LableHeight/pixelsHigh;
    if (fontSize > LableWidth*(LableHeight/pixelsHigh) - 150) {
        for (int i = 0; i < count; i++) {
            int LineTextSize = fontMetrics.width(formatBodyList[i]);
//            qDebug() << "当前的字符宽度" << LineTextSize << "当前的行宽" << LableWidth;
            if (LineTextSize > LableWidth) {
                tmp[i] = (LineTextSize/LableWidth) + 1;
                qDebug() << "tmp[i] " << tmp[i];
                lineCount += tmp[i];
                qDebug() << "lineCount" << lineCount;
            } else {
                lineCount++;
                lastIndex = i;
            }
            if (lineCount >= 5) {
                lastIndex = i;
                tmp[5] = lineCount - tmp[i];
//                qDebug() << "tmp[i] " << tmp[i] << "lastIndex" << lastIndex;
//                qDebug() << "lineCount" << lineCount;
                break;
            }
        }
        status = false;
    } else {
        if (count >= 5) {
            lastIndex = 4;
        } else {
            lastIndex = count - 1;
        }
        status = true;
    }

    for (int i = 0; i < lastIndex; i++) {
        formatBody += formatBodyList[i] + '\n';
    }

    int lastTextSize;
    int remainingRows;

    /* 总行数小于4且总字数小于label的宽度 */
    if (lastIndex < 4 && status) {
        formatBody += formatBodyList[lastIndex];
        return formatBody;
    }

    /* 当前的行数大于等于四行或者字数超过了label的宽度 */
    if (status) {
        lastTextSize = fontMetrics.width(formatBodyList[lastIndex]);
        remainingRows = 1;
    } else {
        lastTextSize = fontMetrics.width(formatBodyList[lastIndex]);
        remainingRows = 5 - tmp[5];
    }

    if (lastTextSize > LableWidth * remainingRows - 10) {
        lastTextSize = LableWidth * remainingRows;
    }

    QString elidedText = formatBodyList[lastIndex];
//    qDebug() << "最后一段文本" << formatBodyList[lastIndex];
    elidedText += "aa";
    formatBody += fontMetrics.elidedText(elidedText, Qt::ElideRight, lastTextSize);
    return formatBody;
}

void SidebarClipboardPlugin::getPixmapListFileIcon(QString UrlText, pixmapLabel *pixmapListclass)
{
    QStringList FileNameList = UrlText.split("\n");
    QList<QPixmap> pixmapList;
    int tmp = FileNameList.count();
    int cnt = 1;
    for (int i = 0; i < tmp; i++) {
        QIcon icon = fileSuffixGetsIcon(FileNameList[i]);
        QPixmap pixmap = icon.pixmap(QSize(16, 16));
        pixmapList.append(pixmap);
        if (cnt >= 3) {
            break;
        }
        cnt++;
    }
    pixmapListclass->setPixmapList(pixmapList);
    return;
}

/* 复制多文件文件的时候， 从文本中间插入...字样 */
QString SidebarClipboardPlugin::setMiddleFormatBody(QString text, ClipboardWidgetEntry *w)
{
    QFontMetrics fontMetrics(w->m_pCopyDataLabal->font());
    int LableWidth = w->m_pCopyDataLabal->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if (fontSize > (LableWidth - 20)) {
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideMiddle, LableWidth - 20);
        return formatBody;
    }
    return formatBody;
}

/* 复制多个文件时设置特定的字符串 */
QString SidebarClipboardPlugin::setSpecificString(QString text)
{
    //对字符串进行截取
    QStringList UrlList = text.split("\n");
    int tmp = UrlList.count(); //记录文件个数
    QString UrlOne = catUrlFileName(UrlList[0]);
    QString specificText = UrlOne;
    return specificText;
}

QString SidebarClipboardPlugin::catUrlFileName(QString Url)
{
    QStringList UrlList = Url.split("/");
    int tmp = UrlList.count();
    return UrlList[tmp - 1];
}

/* 设置每一个条目中的Sequence */
void SidebarClipboardPlugin::setOriginalDataSequence(OriginalDataHashValue *value)
{
    if (m_pClipboardDataHash.count() == 0) {
        value->Sequence = 0;
    } else {
        value->Sequence = iterationDataHashSearchSequence(m_pClipboardDataHash.count());
    }
    return;
}

/* 去除掉空行，显示有字体的行 */
QString SidebarClipboardPlugin::judgeBlankLine(QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        dest = dest.trimmed();
        if (dest.size() != 0) {
           return list.at(i);
        }
    }
    return list.at(0);
}

/* 解析QStringList,转换成qstring */
QString SidebarClipboardPlugin::parsingQStringList(QList<QUrl> fileUrls)
{
    QString text = "";
    for (int i = 0; i < fileUrls.size(); ++i) {
        if (i == 0) {
            text += fileUrls.value(i).toString();
        } else {
            text += "\n" + fileUrls.value(i).toString();
        }
    }
    return text;
}

/* 判断当前子串位置，后面是否还有子串 */
bool SidebarClipboardPlugin::substringSposition(QString formatBody, QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        if (dest == formatBody && i == tmp - 1) {
            qDebug() << "后面没有字串，返回true";
            return true;
        }
    }
    return false;
}

/* 判断Url中当前后缀名，根据后缀名读取图标 */
QIcon SidebarClipboardPlugin::fileSuffixGetsIcon(QString Url)
{
    if (Url == nullptr) {
        qWarning() << "传入后缀名有错误， 为空";
    }
    int tmp = m_fileSuffix.size();
    QStringList UrlList = Url.split(".");
    if (UrlList.size() < 2) {
        QString  filePath = Url.mid(7);
        QFileInfo fileinfo(filePath);
        if (fileinfo.isFile()) {
            return QIcon::fromTheme("unknown");//返回其余文本图标
        } else if (fileinfo.isDir()) {
            return QIcon::fromTheme("folder");//返回文件夹的图标
        }
        return QIcon::fromTheme("unknown");;
    }
    int cnt;
    for(int i = 0; i < tmp; i++) {
        if (m_fileSuffix[i] == UrlList[1]) {
            cnt = i;
            break;
        }
    }
    return fileSuffixeMatchIcon(cnt);
}

/* 根据文件后缀找对应的图标 */
QIcon SidebarClipboardPlugin::fileSuffixeMatchIcon(int cnt)
{
    switch (cnt) {
    case Txt:
        return QIcon::fromTheme("text-x-generic");
    case Svg:
        return QIcon::fromTheme("image-svg+xml");
    case Png:
        return QIcon::fromTheme("image-x-generic");
    case Bmp:
        return QIcon::fromTheme("image-x-generic");
    case Xml:
        return QIcon::fromTheme("text-xml");
    case Docx:
        return QIcon::fromTheme("document");
    case Pptx:
        return QIcon::fromTheme("application-mspowerpoint");
    case Xlsx:
        return QIcon::fromTheme("application-msexcel");
    case Zip:
        return QIcon::fromTheme("application-zip");
    case Pdf:
        return QIcon::fromTheme("application-pdf");
    default:
        return QIcon::fromTheme("unknown");
    }
}

/* 往链表中加入文件后缀 */
void SidebarClipboardPlugin::AddfileSuffix()
{
    m_fileSuffix << "txt" << "svg" << "png" << "bmp" << "xml" << "docx" << "pptx" << "xlsx" << "zip" << "pdf" << "pro";
    return;
}

void SidebarClipboardPlugin::connectWidgetEntryButton(ClipboardWidgetEntry *w)
{
    /* 固定按钮 */
    connect(w->m_pLockButton, &QPushButton::clicked, this, [=](){
        this->fixedWidgetEntrySlots(w);
    });

    /* 取消固定按钮 */
    connect(w->m_pCancelLockButton, &QPushButton::clicked, this, [=]() {
        cancelFixedWidgetEntrySLots(w);
    });

    /* 编辑按钮 */
    connect(w->m_pEditButon, &QPushButton::clicked, this, [=](){
        this->editButtonSlots(w);
    });

    /* 删除按钮 */
    connect(w->m_pRemoveButton, &QPushButton::clicked, this, [=](){
        this->removeButtonSlots(w);
    });

    /* 单击剪贴板条目，自动置顶 */
    connect(w, &ClipboardWidgetEntry::doubleClicksignals, this, &SidebarClipboardPlugin::popButtonSlots);
}

/* 将数据保存到Hash表中 */
void SidebarClipboardPlugin::registerWidgetOriginalDataHash(QListWidgetItem *key, OriginalDataHashValue *value)
{
    if (nullptr == key || nullptr == value) {
        qDebug() << "注册：ClipboardWidgetEntry *key," << key << "OriginalDataHashValue *value值有问题"  << value;
        return;
    }
    if (m_pClipboardDataHash.value(key)) {
        qDebug() << "注册：value已存在";
        return;
    }
    m_pClipboardDataHash.insert(key, value);
}

/* 从hash表中获取OriginalDataHashValue */
OriginalDataHashValue *SidebarClipboardPlugin::GetOriginalDataValue(QListWidgetItem *key)
{
    if (key == nullptr) {
        qWarning() << "GetOriginalDataValue -->获取：ClipboardWidgetEntry *key为空";
        return nullptr;
    }

    if (m_pClipboardDataHash.contains(key)) {
        return m_pClipboardDataHash.value(key);
    } else {
        return nullptr;
    }
}

/* 将剪贴板中的数据从Hash表中移除 */
void SidebarClipboardPlugin::removeOriginalDataHash(QListWidgetItem *key)
{
    if (key == nullptr) {
        qWarning() << "removeOriginalDataHash ----> 获取：ClipboardWidgetEntry *key为空";
        return;
    }
    if (m_pClipboardDataHash.contains(key))
        m_pClipboardDataHash.remove(key);
    return;
}

/* 将新置顶widget写入到剪贴板中去 */
void SidebarClipboardPlugin::WhetherTopFirst()
{
    // 获取第一个条目 当删除为第一项时，则自动将第二项置顶
    QListWidgetItem *PopWidgetItem =  m_pShortcutOperationListWidget->item(0);
    qDebug() << "QListWidgetItem *PopWidgetItem" << PopWidgetItem;
    if (PopWidgetItem == nullptr) {
        qWarning() << "从剪贴板获取的PopWidgetItem指针为空";
        return;
    }

    OriginalDataHashValue *pOriginalData = GetOriginalDataValue(PopWidgetItem);
    auto data =  structureQmimeDate(pOriginalData);
    if (data == nullptr) {
        qWarning() << "构造Qmimedata数据有问题";
        return;
    }
    m_pSidebarClipboard->setMimeData(data);
    return;
}

/* 迭代Hash表m_pClipboardDataHash*/
QListWidgetItem* SidebarClipboardPlugin::iterationClipboardDataHash(ClipboardWidgetEntry *w)
{
    QHash<QListWidgetItem*, OriginalDataHashValue*>::const_iterator iter2 = m_pClipboardDataHash.constBegin();
    while (iter2 != m_pClipboardDataHash.constEnd()) {
        if (iter2.value()->WidgetEntry == w)
            return iter2.key();
        ++iter2;
    }
    qDebug() << "没有找到Widget所对应的Item";
    return nullptr;
}

/* 迭代Hash表查找其中的当前下标是否存在 */
int SidebarClipboardPlugin::iterationDataHashSearchSequence(int Index)
{
    QHash<QListWidgetItem*, OriginalDataHashValue*>::const_iterator iter2 = m_pClipboardDataHash.constBegin();
    int max = iter2.value()->Sequence;
    while (iter2 != m_pClipboardDataHash.constEnd()) {
        if (iter2.value()->Sequence > max)
            max = iter2.value()->Sequence;
        ++iter2;
    }
    return max + 1;
}

/* 构造QMimeData数据 */
QMimeData *SidebarClipboardPlugin::structureQmimeDate(OriginalDataHashValue *value)
{
    if (value == nullptr) {
        qWarning() << "OriginalDataHashValue 为空";
        return nullptr;
    }
    auto data = new QMimeData;
    bool isCut = false;
    QVariant isCutData = QVariant(isCut);
    if (value->Clipbaordformat == TEXT) {
        data->setData("text/plain", isCutData.toByteArray());
        data->setText(value->text);
    } else if (value->Clipbaordformat == URL) {
        data->setData("peony-qt/is-cut", isCutData.toByteArray());
        QList<QUrl> urls;
        QStringList uris = value->text.split("\n");
        qDebug() << "分解后Url的个数" << uris.count();
        for (auto uri : uris) {
            urls << uri;
        }
        value->urls = urls;
        data->setUrls(value->urls);
    } else if (value->Clipbaordformat == IMAGE) {
        QVariant ImageDate = QVariant(*(value->p_pixmap));
        data->setData("application/x-qt-image", isCutData.toByteArray());
        data->setImageData(ImageDate);
    }
    return data;
}

OriginalDataHashValue *SidebarClipboardPlugin::saveOriginalData(OriginalDataHashValue *value)
{
    if (value == nullptr) {
        qWarning() << "保存原有剪贴板数据 ---> 传入参数有误";
        return nullptr;
    }
    OriginalDataHashValue *p_saveOriginalData= new OriginalDataHashValue;
    if (value->Clipbaordformat == TEXT) {
        p_saveOriginalData->text = value->text;
        p_saveOriginalData->Clipbaordformat  = TEXT;
        p_saveOriginalData->associatedDb = DBDATA;
        p_saveOriginalData->p_pixmap = nullptr;
        setOriginalDataSequence(p_saveOriginalData);
    } else if (value->Clipbaordformat == URL) {
        p_saveOriginalData->text = value->text;
        p_saveOriginalData->Clipbaordformat = URL;
        p_saveOriginalData->p_pixmap = nullptr;
        setOriginalDataSequence(p_saveOriginalData);
        p_saveOriginalData->urls = value->urls;
        p_saveOriginalData->WidgetEntry = nullptr;
        p_saveOriginalData->associatedDb = DBDATA;
    } else if (value->Clipbaordformat == IMAGE) {
        p_saveOriginalData->text = value->text;
        p_saveOriginalData->Clipbaordformat = IMAGE;
        p_saveOriginalData->p_pixmap = new QPixmap(*(value->p_pixmap));
        setOriginalDataSequence(p_saveOriginalData);
        p_saveOriginalData->WidgetEntry = nullptr;
        p_saveOriginalData->associatedDb = DBDATA;
    }
    return p_saveOriginalData;
}

/* 置顶槽函数 */
void SidebarClipboardPlugin::popButtonSlots(QWidget *w)
{
    if (w == nullptr) {
        qWarning() << "置顶槽函数ClipboardWidgetEntry *w 为空";
        return;
    }
    if (m_pPreviewImage != nullptr) {
        delete m_pPreviewImage;
        m_pPreviewImage = nullptr;
    }
    ClipboardWidgetEntry *widget = dynamic_cast<ClipboardWidgetEntry*>(w);
    QListWidgetItem *Item = iterationClipboardDataHash(widget);
    OriginalDataHashValue *value = GetOriginalDataValue(Item);
    qDebug() << value->associatedDb << value->text;
    auto data= structureQmimeDate(value);  //构造QMimeData数据
    if (value->associatedDb == DBDATA) {
        OriginalDataHashValue *p = saveOriginalData(value);  //保存原有的数据类型
        removeOriginalDataHash(Item); //移除Hash表中的原始数据
        QListWidgetItem *deleteItem = m_pShortcutOperationListWidget->takeItem(m_pShortcutOperationListWidget->row(Item)); //删除Item;
        delete deleteItem;
        deleteItem = nullptr;
        popCreatorDbHaveDate(p);
        m_pSidebarClipboard->setMimeData(data); //将新的数据set剪贴板中
        return;
    }
    removeOriginalDataHash(Item); //移除Hash表中的原始数据
    QListWidgetItem *deleteItem = m_pShortcutOperationListWidget->takeItem(m_pShortcutOperationListWidget->row(Item)); //删除Item;
    delete deleteItem;
    deleteItem = nullptr;
    m_pSidebarClipboard->setMimeData(data); //将新的数据set剪贴板中
    return;
}

/* 删除槽函数 */
void SidebarClipboardPlugin::removeButtonSlots(ClipboardWidgetEntry *w)
{
    if (w == nullptr) {
        qWarning() << "删除槽函数ClipboardWidgetEntry *w 为空";
        return;
    }
    QListWidgetItem *Item = iterationClipboardDataHash(w);
    OriginalDataHashValue *s_deleteDataHashValue = GetOriginalDataValue(Item);
    if (s_deleteDataHashValue->Clipbaordformat ==IMAGE && s_deleteDataHashValue->associatedDb == DBDATA) {
        QString DeleteFile = QStringLiteral("rm %1").arg(s_deleteDataHashValue->text.mid(7));
        QProcess::execute(DeleteFile);//删除保存在本地的文件
    }
    int tmp = m_pShortcutOperationListWidget->row(Item); //记录删除时哪一行
    m_pClipboardDb->deleteSqlClipboardDb(s_deleteDataHashValue->text);
    removeOriginalDataHash(Item);
    QListWidgetItem *item =  m_pShortcutOperationListWidget->takeItem(tmp); //删除Item;
    delete item;
    // 判断当前删除的是不是第一个条目
    if (0 == tmp) {
        qDebug() << "删除当前的条目为第一个条目";
        WhetherTopFirst();
    }
    emit Itemchange();
    return;
}

/* 编辑槽函数 */
void SidebarClipboardPlugin::editButtonSlots(ClipboardWidgetEntry *w)
{
    /* 防止重复创建 */
    if (w == nullptr) {
        qWarning() << "传入值为空";
        return;
    }
    qDebug() << "当前label中的文本数据" << w->m_pCopyDataLabal->text();
    EditorWidget EditWidget;
    /* 获取保存在hash表中的数据，改变之前保存的数据 */
    QListWidgetItem *Item = iterationClipboardDataHash(w);
    OriginalDataHashValue* pOriginalData = GetOriginalDataValue(Item);
    QString text = pOriginalData->text;
    EditWidget.m_pEditingArea->setPlainText(text);

    /* 设置左右填充 */
    QTextDocument *document = EditWidget.m_pEditingArea->document();
    QTextFrame *rootFrame=document->rootFrame();
    QTextFrameFormat format;
    format.setPadding(10);
    format.setBorderStyle(QTextFrameFormat::BorderStyle_Dotted);
    rootFrame->setFrameFormat(format);

    int nRet = EditWidget.exec();
    if (nRet == QDialog::Accepted) {
        QString formatBody = SetFormatBody(EditWidget.m_pEditingArea->toPlainText(), w);  // 设置...字样
        qDebug () << "formatBody....." << formatBody;
        if (EditWidget.m_pEditingArea->toPlainText() != text) {
            //当编辑后数据改变时，就需要将m_pLabelText中的value改变
            w->m_pCopyDataLabal->setContenText(formatBody);
            pOriginalData->text = EditWidget.m_pEditingArea->toPlainText();
            structureQmimeDate(pOriginalData);
            if (pOriginalData->associatedDb == DBDATA) {
                m_pClipboardDb->updateSqlClipboardDb(pOriginalData->text, pOriginalData->Clipbaordformat, pOriginalData->Sequence, text); //更新数据库表中的数据
            }
        }
        //获取当前条目所在位置，是不是在第一
        int row_num = m_pShortcutOperationListWidget->row(Item);
        if (row_num == 0) {
            QMimeData *data = structureQmimeDate(pOriginalData);
            m_pSidebarClipboard->setMimeData(data); //将新的数据set剪贴板中去
        }
        qDebug() << "当前所在的条木" << row_num;
    } else if (nRet == QDialog::Rejected) {
        qDebug() << "编辑框取消操作";
    }
    return;
}

/* 固定条目槽函数 */
void SidebarClipboardPlugin::fixedWidgetEntrySlots(ClipboardWidgetEntry *w)
{
    if (w == nullptr) {
        qWarning() << "删除槽函数fixedWidgetEntrySlots *w 为空";
        return;
    }
    QListWidgetItem *Item = iterationClipboardDataHash(w);
    OriginalDataHashValue *s_pDataHashValue = GetOriginalDataValue(Item);
    s_pDataHashValue->associatedDb = DBDATA;
    qDebug() << "s_pDataHashValue->Clipbaordformat" << s_pDataHashValue->Clipbaordformat;
    if (s_pDataHashValue->Clipbaordformat == TEXT || s_pDataHashValue->Clipbaordformat == URL) {
        m_pClipboardDb->insertSqlClipbarodDb(s_pDataHashValue->text, s_pDataHashValue->Clipbaordformat, s_pDataHashValue->Sequence, s_pDataHashValue->copyDate);
    } else if (s_pDataHashValue->Clipbaordformat == IMAGE) {
        int seq = m_pClipboardDb->SelectSqlClipbaordDbId();
        QString url_filepath =  QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/%1.bmp").arg(seq + 1);
        s_pDataHashValue->text = "file://" + url_filepath;          //将文件路径已Url的方式保存
        m_pClipboardDb->insertSqlClipbarodDb(s_pDataHashValue->text, s_pDataHashValue->Clipbaordformat, s_pDataHashValue->Sequence, s_pDataHashValue->copyDate);
        s_pDataHashValue->p_pixmap->save(url_filepath, "bmp", 100);
    }
    w->m_pLockButton->setVisible(false);
    w->m_pCancelLockButton->setVisible(true);
    w->m_bWhetherFix = true;
    return;
}

/* 取消固定槽函数 */
void SidebarClipboardPlugin::cancelFixedWidgetEntrySLots(ClipboardWidgetEntry *w)
{
    if (w == nullptr) {
        qWarning() << "删除槽函数fixedWidgetEntrySlots *w 为空";
        return;
    }
    QListWidgetItem *Item = iterationClipboardDataHash(w);
    OriginalDataHashValue *s_pDataHashValue = GetOriginalDataValue(Item);
    m_pClipboardDb->deleteSqlClipboardDb(s_pDataHashValue->text);  //删除数据库中此条数据
    s_pDataHashValue->associatedDb = "";
    w->m_pLockButton->setVisible(true);
    w->m_pCancelLockButton->setVisible(false);
    w->m_bWhetherFix = false;
    return;
}

/* 当超过限制条数时删除最后一条消息 */
void SidebarClipboardPlugin::removeLastWidgetItem()
{
    ClipboardWidgetEntry *w = nullptr;
    QListWidgetItem *Item   = nullptr;
    OriginalDataHashValue *value = nullptr;
    int tmp = m_pShortcutOperationListWidget->count()-1;
    int i;
    for (i = tmp; i >= 0; i--) {
        w = (ClipboardWidgetEntry*)m_pShortcutOperationListWidget->itemWidget(m_pShortcutOperationListWidget->item(i));
        Item = iterationClipboardDataHash(w);
        OriginalDataHashValue *value = GetOriginalDataValue(Item);
        if (value->associatedDb != DBDATA) {
            break;
        }
    }
    if (i == -1) {
        /* 说明全部都是固定的条目，删除最后一条固定条目 */
        w = (ClipboardWidgetEntry*)m_pShortcutOperationListWidget->itemWidget(m_pShortcutOperationListWidget->item(tmp));
        Item = iterationClipboardDataHash(w);
        value = GetOriginalDataValue(Item);
        m_pClipboardDb->deleteSqlClipboardDb(value->text);
        i = tmp;
    }
    removeOriginalDataHash(Item);
    QListWidgetItem *tmpItem = m_pShortcutOperationListWidget->takeItem(i);
    delete tmpItem;
    return;
}

/* 判断在ListWidget是否存在，如果不存在则返回fasle，创建，返回true，不创建 */
bool SidebarClipboardPlugin::booleanExistWidgetItem(QString Text)
{
    int tmp = m_pShortcutOperationListWidget->count();
    for (int i = 0; i < tmp; i++) {
        OriginalDataHashValue *p = GetOriginalDataValue(m_pShortcutOperationListWidget->item(i));
        if (p->Clipbaordformat == TEXT || p->Clipbaordformat == URL) {
            QString WidgetText = p->text;
            if (WidgetText == Text) {
                if(i == 0) {
                    qDebug() << "当前的数据就是置顶数据------>booleanExistWidgetItem";
                    return true;
                }
                if (p->associatedDb == DBDATA) {
                    popButtonSlots(p->WidgetEntry);
                    return true;
                }
                removeButtonSlots(GetOriginalDataValue(m_pShortcutOperationListWidget->item(i))->WidgetEntry);
                return false;
            }
        }
    }
    return false;
}

/* 判断图片是否在hash表中，如果存在，则删除，然后将图片重新写入到剪贴板中去 */
bool SidebarClipboardPlugin::booleanExistWidgetImagin(QPixmap Pixmap)
{
    //将从剪贴板拿到的数据转换成Bit位进行比较
    QImage clipboardImage = Pixmap.toImage();
    int Clipboard_hight = clipboardImage.height();
    int Clipboard_width = clipboardImage.width();
    unsigned char *clipboard_data = clipboardImage.bits();
    int tmp = m_pShortcutOperationListWidget->count();
    if (tmp == 0) {
        qDebug() << "当前hash表中不存在数据， 直接返回即可";
        return false;
    }
    unsigned char r1, g1, b1, r2, g2, b2;
    int j;
    for (int i = 0; i < tmp; i++) {
        OriginalDataHashValue *p = GetOriginalDataValue(m_pShortcutOperationListWidget->item(i));
        if (p->Clipbaordformat == IMAGE) {
            //hash表中的Pixmap和刚从剪贴板中拿到的数据进行比较
            QPixmap Hash_Pixmap = *(p->p_pixmap);
            QImage Hash_Image = Hash_Pixmap.toImage();
            if (Clipboard_hight == Hash_Image.height() && Clipboard_width == Hash_Image.width()) {
                unsigned char *Hash_data = Hash_Image.bits();
                for (j = 0; j < Clipboard_hight; j++) {
                    for (int k = 0; k < Clipboard_width; k++) {
                        r1 = *(Hash_data + 2);
                        b1 = *(Hash_data + 1);
                        g1 = *(Hash_data);
                        r2 = *(clipboard_data + 2);
                        b2 = *(clipboard_data + 1);
                        g2 = *(clipboard_data);
                        if (r1 == r2 && b1 == b2 && g1 == g2) {
                            clipboard_data += 4;
                            Hash_data += 4;
                        } else {
                            return false;   //比对图片像素点不相等直接返回false
                        }
                    }
                }
                if (j == Clipboard_hight) {
                    //说明图片已经对比完成，且图片存在
                    if (i == 0)
                        return true; //当前数据就是第一条数据， 不需要做其余处理，直接清理内存，退出；
                    /* 说明已存在此图片但是需要将该放置在第一个位置上去，同时写入剪贴板当中 */
                    removeButtonSlots(GetOriginalDataValue(m_pShortcutOperationListWidget->item(i))->WidgetEntry);
                    return false;
                }
            }
        }
    }
    return false;
}

/* 删除所有的WidgetItem */
void SidebarClipboardPlugin::removeAllWidgetItem()
{
    int tmp = m_pShortcutOperationListWidget->count();
    qDebug() << "m_pShortcutOperationListWidget->count()" << tmp;
    if (tmp <= 0) {
        qDebug() << "条目为零，不需要清空";
        return;
    }
    /* 判断用户是否勾选了不再提醒这一功能 */
    if (m_bPromptBoxBool) {
        CleanPromptBox PromptBoxWidget;
        int nRet = PromptBoxWidget.exec();
        if (nRet == QDialog::Accepted) {
            qDebug() << "nRet == QDialog::Accepted";
        } else if (nRet == QDialog::Rejected) {
            qDebug() << "nRet == QDialog::Rejected";
            return;
        }
    }
    for (int i = 0; i < tmp; i++) {
        OriginalDataHashValue *p_deleteDataHashValue = GetOriginalDataValue(m_pShortcutOperationListWidget->item(0));
        removeOriginalDataHash(m_pShortcutOperationListWidget->item(0));
        QListWidgetItem *tmp = m_pShortcutOperationListWidget->takeItem(0);
        //删除保存在数据库的中的数据
        if (p_deleteDataHashValue->associatedDb == DBDATA) {
            m_pClipboardDb->deleteSqlClipboardDb(p_deleteDataHashValue->text); //删除数据库中的数据
        }
        //图片且保存到数据库中，需要删除保存在本地的文件
        if (p_deleteDataHashValue->Clipbaordformat == IMAGE && p_deleteDataHashValue->associatedDb == DBDATA) {
            QString DeleteFile = QStringLiteral("rm %1").arg(p_deleteDataHashValue->text.mid(7));
            QProcess::execute(DeleteFile);//删除保存在本地的文件
        }
        delete tmp;
    }
    emit Itemchange();
}

/* 搜索 槽函数 */
void SidebarClipboardPlugin::searchClipboardLableTextSlots(QString Text)
{
    /* 在清除条目前，先记住每一个当前的位置 */
    if (m_bsortEntryBool)
        sortingEntrySequence();

    /* 清空之前listWidget中的條目 */
    int tmp = m_pShortcutOperationListWidget->count();
    for (int i = 0; i < tmp; i++) {
        ClipboardWidgetEntry *w = (ClipboardWidgetEntry*)m_pShortcutOperationListWidget->itemWidget(m_pShortcutOperationListWidget->item(0));
        m_pShortcutOperationListWidget->item(0);
        m_pShortcutOperationListWidget->takeItem(0);
    }

    /* 当搜索栏中内容为空，还原之前的数据 */
    if (Text == "") {
        sortingEntryShow();
        return;
    }

    /* 将包含有关键字的条目显示出来 */
    QHash<QListWidgetItem*, OriginalDataHashValue*>::const_iterator iter2 = m_pClipboardDataHash.constBegin();
    while (iter2 != m_pClipboardDataHash.constEnd()) {
        if (iter2.value()->text.contains(Text, Qt::CaseSensitive) && iter2.value()->Clipbaordformat != IMAGE) {
            m_pShortcutOperationListWidget->insertItem(0, iter2.key());
            ClipboardWidgetEntry *w = new ClipboardWidgetEntry(iter2.value()->Clipbaordformat);
            iter2.value()->WidgetEntry = w;
            setEntryItemSize(iter2.value(), w, iter2.key());
            AddWidgetEntry(iter2.value(), w, iter2.value()->text);
            connectWidgetEntryButton(w);
            m_pShortcutOperationListWidget->setItemWidget(iter2.key(), w);
        }
        ++iter2;
    }
    WhetherTopFirst();
    return;
}

/* Item数目发生变化 */
void SidebarClipboardPlugin::ItemNumchagedSlots()
{
    int num = m_pClipboardDataHash.size();
    if (num > 0) {
        m_pSideBarClipboardLable->setVisible(false);
        m_pShortcutOperationListWidget->setVisible(true);
    } else {
        m_pSideBarClipboardLable->setVisible(true);
        m_pShortcutOperationListWidget->setVisible(false);
    }
    return;
}

/* 加载数据Text线程槽函数 */
void SidebarClipboardPlugin::loadClipboardDb()
{
    QString url_filepath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +"/.config/Clipboard.db";
    QSqlQuery query(QSqlDatabase::database(url_filepath));
    QString SelectSql = QStringLiteral("SELECT * FROM Clipboard_table");
    if (!query.exec(SelectSql)) {
        qWarning() << "数据Select失败";
        return;
    }
    while (query.next()) {
        OriginalDataHashValue *p_DataHashValueDb = new OriginalDataHashValue();
        p_DataHashValueDb->text = query.value(1).toString();
        p_DataHashValueDb->Clipbaordformat = query.value(2).toString();
        p_DataHashValueDb->copyDate = query.value(4).toLongLong();
        p_DataHashValueDb->associatedDb = DBDATA;
        creatLoadClipboardDbData(p_DataHashValueDb);
    }
    return;
}

/* 加载从数据库中拿到的数据，加入到剪贴板 */
void SidebarClipboardPlugin::creatLoadClipboardDbData(OriginalDataHashValue *value)
{
    if (value == nullptr) {
        qWarning() << "参数错误 ---> 参数类型 --->OriginalDataHashValue";
        return;
    }
    //第一次加载不需要管剪贴板条目是否存在
    bool DeleteFlag = false;
    QListWidgetItem *pListWidgetItem = new QListWidgetItem;
    ClipboardWidgetEntry *w = new ClipboardWidgetEntry(value->Clipbaordformat);

    /* 判断文件是否存在 */
    if (value->Clipbaordformat == TEXT) {
        DeleteFlag = true;
    } else if (value->Clipbaordformat == URL && judgeFileExit(value->text)) {
        DeleteFlag = true;  //修改 文件是否存在标志位
        QList<QUrl> urls;
        QStringList uris = value->text.split("\n");
        for (auto uri : uris) {
            urls << uri;
        }
        value->urls = urls;
    } else if (value->Clipbaordformat == IMAGE && judgeFileExit(value->text)) {
        DeleteFlag = true;
        value->p_pixmap = new QPixmap(value->text.mid(7));
    }

    if (!DeleteFlag) {
        qDebug() << "此文件不存在";
        m_pClipboardDb->deleteSqlClipboardDb(value->text);
        delete pListWidgetItem;
        delete w;
        delete value;
        return;
    }

    if (m_pClipboardDataHash.count() == 0) {
        value->Sequence = 0;
    } else {
        value->Sequence = iterationDataHashSearchSequence(m_pClipboardDataHash.count());
    }

    setCopyDataSize(value, w);
    AddWidgetEntry(value, w, value->text);

    value->WidgetEntry = w;

    setEntryItemSize(value, w, pListWidgetItem);
    pListWidgetItem->setFlags(Qt::NoItemFlags);

    registerWidgetOriginalDataHash(pListWidgetItem, value);

    //将按钮与槽对应上
    connectWidgetEntryButton(w);
    //插入剪贴板条目
    m_pShortcutOperationListWidget->insertItem(0, pListWidgetItem);
    m_pShortcutOperationListWidget->setItemWidget(pListWidgetItem, w);
    emit Itemchange();
}

void SidebarClipboardPlugin::popCreatorDbHaveDate(OriginalDataHashValue *value)
{
    if (value == nullptr) {
        qWarning() << "popCreatorDbHaveDate ---> 传入形参错误";
        return;
    }
    QListWidgetItem *pListWidgetItem = new QListWidgetItem;
    ClipboardWidgetEntry *w = new ClipboardWidgetEntry(value->Clipbaordformat);

    value->WidgetEntry = w;
    AddWidgetEntry(value, w, value->text);

    if (value->Clipbaordformat == TEXT || value->Clipbaordformat == URL) {
        w->setFixedSize(328, 160);
        pListWidgetItem->setSizeHint(QSize(328, 168));
    } else if (value->Clipbaordformat == IMAGE) {
        w->setFixedSize(328, 160);
        pListWidgetItem->setSizeHint(QSize(328, 168));
    }
    pListWidgetItem->setFlags(Qt::NoItemFlags);

    registerWidgetOriginalDataHash(pListWidgetItem, value);

    //将按钮与槽对应上
    connectWidgetEntryButton(w);
    //插入剪贴板条目
    m_pShortcutOperationListWidget->insertItem(0, pListWidgetItem);
    m_pShortcutOperationListWidget->setItemWidget(pListWidgetItem, w);
    emit Itemchange();
    return;
}

/* 判断此路径下文件是否存在 */
bool SidebarClipboardPlugin::judgeFileExit(QString fullFilePath)
{
    if (fullFilePath == "") {
        qWarning() << "参数错误 ---> 参数类型 Qstring" << fullFilePath;
        return false;
    }
    QStringList filePath = fullFilePath.split('\n');
    if (filePath.count() == 1) {
        QFileInfo fileInfo(fullFilePath.mid(7));
        if (fileInfo.exists()) {
            return true;
        }
    } else {
        int tmp = filePath.count();
        for(int i = 0; i < tmp; i++) {
            QFileInfo fileInfo(filePath[i].mid(7));
            qDebug() << "文件路径" << filePath[i].mid(7) << fileInfo.exists();
            if (fileInfo.exists()) {
                if (i == tmp - 1) {
                    return true;
                }
            } else {
                return false;
            }
        }
    }
    return false;
}

/* 设定预览窗口位置 */
int SidebarClipboardPlugin::setClipBoardWidgetScaleFactor()
{
    /* 获取当前屏幕分辨率 */
    QScreen* pScreen = QGuiApplication::primaryScreen();
    QRect DeskSize   = pScreen->geometry();
    m_nScreenWidth   = DeskSize.width();                      //桌面分辨率的宽
    m_nScreenHeight  = DeskSize.height();                     //桌面分辨率的高
    if (m_nScreenHeight >= 600 && m_nScreenHeight <= 768) {
        return m_nScreenHeight - m_nScreenHeight/2 - 60 - m_taskHeight + m_nclipboardsite_y;
    } else if (m_nScreenHeight >= 900 && m_nScreenHeight <= 1080) {
        return m_nScreenHeight - m_nScreenHeight/3 - m_taskHeight + m_nclipboardsite_y;
    } else if (m_nScreenHeight >= 1200 && m_nScreenHeight <= 2160) {
        return m_nScreenHeight - m_nScreenHeight/4 - m_taskHeight + m_nclipboardsite_y;
    } else {
        return m_nScreenHeight/2 - m_nScreenHeight + m_nclipboardsite_y;
    }
}

void SidebarClipboardPlugin::OpenClipboardWidget()
{
    qDebug() << "打开剪贴板";
}

void SidebarClipboardPlugin::ExitClipboardWidget()
{
    qDebug() << "退出侧边栏";
}

void SidebarClipboardPlugin::HideClipboardWidget()
{
    qDebug() << "隐藏侧边栏";
}

void SidebarClipboardPlugin::sortingEntrySequence()
{
    int tmp = m_pShortcutOperationListWidget->count();
    for (int i = 0; i < tmp; i++) {
        GetOriginalDataValue(m_pShortcutOperationListWidget->item(i))->Sequence = tmp - i - 1;
    }
    m_bsortEntryBool = false;
}

void SidebarClipboardPlugin::setEntryItemSize(OriginalDataHashValue* value, ClipboardWidgetEntry *w, QListWidgetItem *item)
{
    if (value == nullptr || w == nullptr || item == nullptr) {
        qWarning() << "setEntryItemSize  ------> 设置条目大小和Item大小传入参数失败";
        return;
    }
    if (value->Clipbaordformat == TEXT || value->Clipbaordformat == URL) {
        w->setFixedSize(328, 160);
        item->setSizeHint(QSize(328,168));
    } else if (value->Clipbaordformat == IMAGE) {
        w->setFixedSize(328, 160);
        item->setSizeHint(QSize(328,168));
    }
    return;
}

void SidebarClipboardPlugin::TimedTriggerUpdateTimeSlots()
{
    QLocale usLocale = QLocale::system();
    auto dateTime = QDateTime::currentDateTime();
    QString curretTime = usLocale.toString(dateTime.time(),"hh:mm");
    if (curretTime == "00:00") {
        qDebug() << "更新剪贴板复制时间";
    }
}

void SidebarClipboardPlugin::updateClipboardDataTime()
{
    QHash<QListWidgetItem*, OriginalDataHashValue*>::const_iterator iter2 = m_pClipboardDataHash.constBegin();
    while (iter2 != m_pClipboardDataHash.constEnd()) {
        if (iter2.value())
            updateLabelTime(iter2.value());
        ++iter2;
    }
}

void SidebarClipboardPlugin::updateLabelTime(clipboardOriginalDataHash *value)
{
    value->WidgetEntry->m_pTimeLabel->setText(parseDateTime(QDateTime::fromMSecsSinceEpoch(value->copyDate)));
}

QString SidebarClipboardPlugin::parseDateTime(QDateTime dateTime)
{
    QLocale usLocale = QLocale::system();
    QString d;
    auto currDateTime = QDateTime::currentDateTime();
    if (dateTime.date() == currDateTime.date()) {
        d = tr("Today ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    } else if (dateTime.daysTo(currDateTime) == 1) {
        d = tr("Yesterday ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    }
    return dateTime.toString("yyyy/MM/dd hh:mm");
}

void SidebarClipboardPlugin::initTimer()
{
    m_pUpdateTime = new QTimer();
    connect(m_pUpdateTime, &QTimer::timeout, this, &SidebarClipboardPlugin::TimedTriggerUpdateTimeSlots);
    m_pUpdateTime->start(1*1000*30);    //30秒调用一次
}

QString SidebarClipboardPlugin::getCurrutTime(QDateTime dateTime)
{
    QLocale usLocale = QLocale::system();
    QString currentTime;
    currentTime = tr("Today ");
    currentTime.append(usLocale.toString(dateTime.time(),"hh:mm"));
    qDebug() << "当前发生的时间";
    return currentTime;
}

void SidebarClipboardPlugin::sortingEntryShow()
{
    int index = 0;
    int count = m_pClipboardDataHash.size();
    bool IndexFlag = true;
    while(index != count) {
        IndexFlag = true;
        qDebug() << "当前条目的下标" << index;
        QHash<QListWidgetItem*, OriginalDataHashValue*>::const_iterator iter1 = m_pClipboardDataHash.constBegin();
        while (iter1 != m_pClipboardDataHash.constEnd()) {
            qDebug() << "当前条目所处位置的位置-->Sequence -->" << iter1.value()->Sequence;
            if (index == iter1.value()->Sequence) {
                m_pShortcutOperationListWidget->insertItem(0, iter1.key());
                ClipboardWidgetEntry *w = new ClipboardWidgetEntry(iter1.value()->Clipbaordformat);
                iter1.value()->WidgetEntry = w;
                setEntryItemSize(iter1.value(), w, iter1.key());
                connectWidgetEntryButton(w);
                iter1.value()->WidgetEntry = w;
                AddWidgetEntry(iter1.value(), w, iter1.value()->text);
                m_pShortcutOperationListWidget->setItemWidget(iter1.key(), w);
                IndexFlag = false;
                index++;
                qDebug() << "进入循环当前条目的下标" << index;
            }
            ++iter1;
        }
        /* 如果没有进入上面的While循环，说明此Index不存在，继续往后面走 */
        if (IndexFlag) {
            index++;
            count++;
        }
        if (index > 1000 || count > 1000) {
            qDebug() << "查找机制已经进入死循环";
            return;
        }
    }
    m_bsortEntryBool = true;
}
