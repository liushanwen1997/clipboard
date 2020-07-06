#include "pixmaplabel.h"

pixmapLabel::pixmapLabel()
{

}

void pixmapLabel::setPixmapList(const QList<QPixmap> &list)
{
    m_pixmapList = list;
    update();
}

void pixmapLabel::paintEvent(QPaintEvent *event)
{
//    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);

    QStyle *style = QWidget::style();
    QStyleOption opt;
    opt.initFrom(this);
    int x = 4;
    int y = 4;
    //drawPixmaps
    if (m_pixmapList.size() == 1) {
        QPixmap pix = m_pixmapList[0];
        if (!isEnabled())
            pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
        QPixmap newPix = pix.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        style->drawItemPixmap(&painter, QRect(QPoint(4, 4), newPix.size()), Qt::AlignCenter, newPix);
    } else {
        for (int i = 0 ; i < m_pixmapList.size(); ++i) {
            QPixmap pix = m_pixmapList[m_pixmapList.size() - i - 1];
//            qDebug() << "当前图片的大小" << pix.size();
            if (pix.size() == QSize(0, 0))
                continue;
            if (!isEnabled())
                pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
            QPixmap newPix = pix.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            style->drawItemPixmap(&painter, QRect(QPoint(x, y), newPix.size()), Qt::AlignCenter, newPix);
            x += 6;
            y += 6;
        }
    }
    QLabel::paintEvent(event);
    return;
}
