/**
 * @brief   : libmpv 音频播放器封装（无视频渲染，仅音频播放）
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/
#ifndef MPVPLAYER_H
#define MPVPLAYER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <mpv/client.h>

enum PlayMode {
    PlaySequence,   // 顺序播放
    PlayRandom,     // 随机播放
    PlaySingleLoop  // 单曲循环
};


class MPVPlayer : public QObject
{
    Q_OBJECT
public:
    explicit MPVPlayer(QObject *parent = nullptr);
    ~MPVPlayer();

    void playUrl(const QString &url); // 加载并播放（替换当前）
    void play();
    void pause();
    void stop();
    void setVolume(int vol); // 0-100
    void setPosition(qint64 ms); // 毫秒

    bool isPlaying() const;

signals:
    void positionChanged(qint64 ms);
    void durationChanged(qint64 ms);
    void stateChanged(bool playing);
    void playbackFinished();

private slots:
    void onPoll(); // 定时器轮询属性

private:
    mpv_handle *m_mpv;
    QTimer m_pollTimer;
    bool m_playing;
};

#endif // MPVPLAYER_H
