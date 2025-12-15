/**
 * @brief   : 网易云风格歌词组件，支持时间同步滚动、自绘高亮行
 * @author  : 樊晓亮
 * @date    : 2025.02.17
 **/

#ifndef LYRICSWIDGET_H
#define LYRICSWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPropertyAnimation>

class LyricsWidget : public QWidget
{
    Q_OBJECT

    // yOffset 用来做平滑滚动动画
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

public:
    explicit LyricsWidget(QWidget *parent = nullptr);

    // 设置歌词文本（LRC格式）
    void setLrcText(const QString &lrc);

    // 根据当前播放时间更新歌词位置
    void updatePosition(qint64 ms);

    qreal offset() const { return m_offset; }
    void setOffset(qreal o);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct LrcLine {
        qint64 timeMs;
        QString text;
    };

    QVector<LrcLine> m_lines;
    int m_currentIndex = -1;

    qreal m_offset = 0;              // 当前滚动偏移
    qreal m_targetOffset = 0;        // 目标滚动位置
    QPropertyAnimation *m_anim;

    void parseLrc(const QString &lrc);
    int findCurrentIndex(qint64 ms);
};

#endif // LYRICSWIDGET_H
