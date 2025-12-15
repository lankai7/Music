/**
 * @brief   : 主窗口（网易云风格音频播放器主逻辑，严格使用现有 UI 名称）
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QListWidgetItem>
#include "networkmanager.h"
#include "mpvplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI 交互
    void on_btnSearch_clicked();
    void on_listResults_itemClicked(QListWidgetItem *item);
    void on_btnPlayPause_clicked();
    void on_btnPrev_clicked();
    void on_btnNext_clicked();
    void on_sliderPosition_sliderMoved(int position); // 用户拖动进度条（秒）
    void on_sliderVolume_valueChanged(int value);

    // NetworkManager 信号
    void onSearchFinished(const QList<NetworkManager::SearchItem> &list);
    void onGetUrlFinished(const NetworkManager::UrlResult &res);
    void onImageFetched(const QPixmap &pix);

    // MPVPlayer 信号
    void onPositionChanged(qint64 ms);
    void onDurationChanged(qint64 ms);
    void onPlayerStateChanged(bool playing);
    void on_host_btn_clicked();

    void on_new_btn_clicked();

    void on_love_btn_clicked();

    void on_collect_btn_clicked();

    void on_mode_btn_clicked();

    void onPlaybackFinished();
private:
    Ui::MainWindow *ui;
    NetworkManager *m_net;
    MPVPlayer *m_player;
    PlayMode m_playMode;   // 当前播放模式

    QList<NetworkManager::SearchItem> m_searchList;
    int m_currentIndex; // 当前播放索引（在 m_searchList 中），-1 表示无

    void setMetadataFromSearchItem(const NetworkManager::SearchItem &it);
    void updatePlayPauseUI(bool playing);
    void resetMetadataDisplay();
    QString secondsToString(qint64 ms);
    void addToFavorites(const NetworkManager::SearchItem &item);
    bool isFavorited(int songId) const;
    void removeFromFavorites(int songId);
    void updateFavoriteButton();


    void updatePlayModeButton();
    void playNextByMode();
};

#endif // MAINWINDOW_H
