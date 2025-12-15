/**
 * @brief   : libmpv 音频播放器封装实现（无视频渲染，仅音频播放）
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/
#include "mpvplayer.h"
#include <QDebug>

static void mpv_log_callback(void *userdata, const mpv_event_log_message *msg)
{
    Q_UNUSED(userdata);
    qDebug() << "[mpv]" << msg->prefix << msg->level << msg->text;
}

MPVPlayer::MPVPlayer(QObject *parent)
    : QObject(parent),
      m_mpv(nullptr),
      m_playing(false)
{
    m_mpv = mpv_create();
    if (!m_mpv) {
        qFatal("mpv_create failed");
    }

    // 请求日志输出
    mpv_request_log_messages(m_mpv, "info");

    // 如果只做音频，可以不设置 vo；但保留默认设置
    // mpv_set_option_string(m_mpv, "vo", "null");

    if (mpv_initialize(m_mpv) < 0) {
        qFatal("mpv_initialize failed");
    }

    // 轮询定时器用于更新进度/时长/播放状态
    m_pollTimer.setInterval(500);
    connect(&m_pollTimer, &QTimer::timeout, this, &MPVPlayer::onPoll);
    m_pollTimer.start();
}

MPVPlayer::~MPVPlayer()
{
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

void MPVPlayer::playUrl(const QString &url)
{
    if (!m_mpv) return;
    QByteArray u = url.toUtf8();
    const char *args[] = {"loadfile", u.constData(), nullptr};
    int r = mpv_command(m_mpv, args);
    Q_UNUSED(r);
    m_playing = true;
    emit stateChanged(true);
}

void MPVPlayer::play()
{
    if (!m_mpv) return;
    int paused = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &paused);
    m_playing = true;
    emit stateChanged(true);
}

void MPVPlayer::pause()
{
    if (!m_mpv) return;
    int paused = 1;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &paused);
    m_playing = false;
    emit stateChanged(false);
}

void MPVPlayer::stop()
{
    if (!m_mpv) return;
    const char *cmd[] = {"stop", nullptr};
    mpv_command(m_mpv, cmd);
    m_playing = false;
    emit stateChanged(false);
}

void MPVPlayer::setVolume(int vol)
{
    if (!m_mpv) return;
    double v = vol;
    mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &v);
}

void MPVPlayer::setPosition(qint64 ms)
{
    if (!m_mpv) return;
    double sec = ms / 1000.0;
    mpv_set_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &sec);
}

bool MPVPlayer::isPlaying() const
{
    return m_playing;
}

void MPVPlayer::onPoll()
{
    if (!m_mpv) return;

    /* ===== 1. 处理 mpv 事件 ===== */
    while (true) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;

        if (event->event_id == MPV_EVENT_END_FILE) {
            mpv_event_end_file *end =
                (mpv_event_end_file *)event->data;

            // 正常播放结束（不是 error / user stop）
            if (end->reason == MPV_END_FILE_REASON_EOF) {
                emit playbackFinished();
            }
        }
    }

    /* ===== 2. 原有进度轮询 ===== */
    double timePos = 0.0;
    if (mpv_get_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &timePos) >= 0) {
        emit positionChanged(qint64(timePos * 1000));
    }

    double duration = 0.0;
    if (mpv_get_property(m_mpv, "duration", MPV_FORMAT_DOUBLE, &duration) >= 0) {
        emit durationChanged(qint64(duration * 1000));
    }

    int paused = 1;
    if (mpv_get_property(m_mpv, "pause", MPV_FORMAT_FLAG, &paused) >= 0) {
        bool playing = (paused == 0);
        if (playing != m_playing) {
            m_playing = playing;
            emit stateChanged(m_playing);
        }
    }
}

