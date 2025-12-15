/**
 * @brief   : 网易云风格歌词组件实现，支持滚动、时间同步、自绘高亮
 * @author  : 樊晓亮
 * @date    : 2025.02.17
 **/

#include "lyricswidget.h"
#include <QPainter>
#include <QDebug>

LyricsWidget::LyricsWidget(QWidget *parent)
    : QWidget(parent)
    , m_anim(new QPropertyAnimation(this, "offset"))
{
    m_anim->setDuration(400);
    m_anim->setEasingCurve(QEasingCurve::InOutQuad);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

/*-------------------------------
 * 设置歌词并解析
 *------------------------------*/
void LyricsWidget::setLrcText(const QString &lrc)
{
    m_lines.clear();
    parseLrc(lrc);
    m_currentIndex = -1;
    m_offset = 0;
    update();
}

/*-------------------------------
 * LRC 解析
 * 格式：[mm:ss.xx] 歌词内容
 *------------------------------*/
void LyricsWidget::parseLrc(const QString &lrc)
{
    QStringList rows = lrc.split("\n");

    for (const QString &line : rows) {
        QRegExp rx("\\[(\\d+):(\\d+\\.\\d+)\\]");
        if (rx.indexIn(line) != -1) {
            int min = rx.cap(1).toInt();
            double sec = rx.cap(2).toDouble();
            qint64 ms = (min * 60 + sec) * 1000;

            QString text = line;
            text.remove(rx);

            LrcLine ll;
            ll.timeMs = ms;
            ll.text = text.trimmed();
            m_lines.push_back(ll);
        }
    }
}

/*-------------------------------
 * 根据时间查找当前行
 *------------------------------*/
int LyricsWidget::findCurrentIndex(qint64 ms)
{
    for (int i = m_lines.size() - 1; i >= 0; --i) {
        if (ms >= m_lines[i].timeMs)
            return i;
    }
    return -1;
}

/*-------------------------------
 * 更新歌词滚动
 *------------------------------*/
void LyricsWidget::updatePosition(qint64 ms)
{
    int idx = findCurrentIndex(ms);
    if (idx < 0 || idx >= m_lines.size()) return;

    if (idx != m_currentIndex) {
        m_currentIndex = idx;

        // 每行高度 40 像素，居中滚动
        m_targetOffset = m_currentIndex * 40 - height() / 2 + 20;

        m_anim->stop();
        m_anim->setStartValue(m_offset);
        m_anim->setEndValue(m_targetOffset);
        m_anim->start();

        update();
    }
}

/*-------------------------------
 * 滚动动画 setter
 *------------------------------*/
void LyricsWidget::setOffset(qreal o)
{
    m_offset = o;
    update();
}

/*-------------------------------
 * 绘制歌词
 *------------------------------*/
void LyricsWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_lines.isEmpty()) {
        p.setPen(QColor("#888"));
        p.drawText(rect(), Qt::AlignCenter, "暂无歌词");
        return;
    }

    int y0 = -m_offset; // 顶部偏移

    for (int i = 0; i < m_lines.size(); ++i) {
        QRect r(0, y0 + i * 40, width(), 40);

        if (i == m_currentIndex) {
            // 当前行红色大字体
            p.setPen(QColor("#FF3A3A"));
            QFont f = font();
            f.setPointSize(24);
            p.setFont(f);
        } else {
            // 普通行灰色
            p.setPen(QColor("#BBBBBB"));
            QFont f = font();
            f.setPointSize(16);
            p.setFont(f);
        }

        p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, m_lines[i].text);
    }
}
