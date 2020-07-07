#include "sidebarclipboardplugin.h"
#include <QApplication>
#include <KWindowEffects>
#include <QX11Info>
int main(int argc, char *argv[])
{
     QApplication a(argc, argv);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    a.setAttribute(Qt::AA_UseHighDpiPixmaps,true);
#endif
    SidebarClipboardPlugin clipboard;
//    clipboard.setWindowFlags(Qt::FramelessWindowHint);
    clipboard.setAttribute(Qt::WA_TranslucentBackground);
    clipboard.setFixedSize(360, 460);
    clipboard.show();
    clipboard.move(850, 300);
    KWindowEffects::enableBlurBehind(clipboard.winId(), true);
    return a.exec();
}
