/**
 * @brief   : 主窗口实现（网易云风格音频播放器主逻辑，严格使用现有 UI 名称）
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_net(new NetworkManager(this)),
      m_player(new MPVPlayer(this)),
      m_currentIndex(-1)
{
    ui->setupUi(this);

    // 初始化 UI 初始状态（保持和 .ui 名称一致）
    ui->sliderVolume->setRange(0, 100);
    ui->sliderVolume->setValue(80);
    ui->sliderPosition->setRange(0, 0);
    ui->labelTitle->setText("");
    ui->labelArtist->setText("");
    ui->lyricsWidget->setLrcText("无歌词");
    ui->lyricsWidget->show();

    // 连接 NetworkManager 信号
    connect(m_net, &NetworkManager::searchFinished, this, &MainWindow::onSearchFinished);
    connect(m_net, &NetworkManager::getUrlFinished, this, &MainWindow::onGetUrlFinished);
    connect(m_net, &NetworkManager::imageFetched, this, &MainWindow::onImageFetched);

    // 连接 MPVPlayer 信号
    connect(m_player, &MPVPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_player, &MPVPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_player, &MPVPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);

    // 连接 UI 信号（explicit, 不使用自动槽名）
    // 回车触发搜索
    connect(ui->editKeyword, &QLineEdit::returnPressed,this, &MainWindow::on_btnSearch_clicked);

//    connect(ui->listResults, &QListWidget::itemClicked, this, &MainWindow::on_listResults_itemClicked);
//    connect(ui->sliderPosition, &QSlider::sliderMoved, this, &MainWindow::on_sliderPosition_sliderMoved);
//    connect(ui->sliderVolume, &QSlider::valueChanged, this, &MainWindow::on_sliderVolume_valueChanged);

    // 右侧封面尺寸策略（若无封面则用资源）
    ui->labelCover->setScaledContents(false);
    ui->labelCover->setPixmap(QPixmap(":/default_cover.png").scaled(ui->labelCover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

MainWindow::~MainWindow()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// 搜索
///////////////////////////////////////////////////////////////////////////////
void MainWindow::on_btnSearch_clicked()
{
    QString kw = ui->editKeyword->text().trimmed();
    if (kw.isEmpty()) {
        ui->statusbar->showMessage("请输入查找的歌曲！", 3000);
        return;
    }
    ui->statusbar->showMessage("搜索中...");
    ui->listResults->clear();
    m_searchList.clear();
    m_currentIndex = -1;

    m_net->search(kw);
}



void MainWindow::onSearchFinished(const QList<NetworkManager::SearchItem> &list)
{
    ui->statusbar->clearMessage();
    m_searchList = list;
    ui->listResults->clear();

    for (int i = 0; i < list.size(); ++i) {
        const auto &it = list.at(i);
        QListWidgetItem *item = new QListWidgetItem(QString("%1 - %2").arg(it.title, it.singer));
        item->setData(Qt::UserRole, it.id);       // 存放 id，用于解析真实地址
        item->setData(Qt::UserRole + 1, i);       // 存放 index
        ui->listResults->addItem(item);
    }

    if (list.isEmpty()) {
        ui->statusbar->showMessage("未找到结果", 3000);
    } else {
        ui->statusbar->showMessage(QString("找到 %1 条").arg(list.size()), 3000);
    }
}

///////////////////////////////////////////////////////////////////////////////
// 列表点击 -> 解析并准备播放
///////////////////////////////////////////////////////////////////////////////
void MainWindow::on_listResults_itemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    int idx = item->data(Qt::UserRole + 1).toInt();
    if (idx < 0 || idx >= m_searchList.size()) return;

    m_currentIndex = idx;
    // 先使用列表中的元数据更新界面
    setMetadataFromSearchItem(m_searchList.at(idx));
    // 异步加载封面
    m_net->fetchImage(m_searchList.at(idx).picurl);
    // 请求真实播放地址
    ui->statusbar->showMessage("解析播放地址...");
    m_net->getUrlById(id);
}

///////////////////////////////////////////////////////////////////////////////
// Network 返回真实播放地址
///////////////////////////////////////////////////////////////////////////////
void MainWindow::onGetUrlFinished(const NetworkManager::UrlResult &res)
{
    ui->statusbar->clearMessage();

    if (res.url.isEmpty()) {
        ui->statusbar->showMessage("解析失败：未获取到播放地址", 4000);
        return;
    }

    // 更新元信息（如果接口返回更精确的名称/歌手）
    if (!res.name.isEmpty()) ui->labelTitle->setText(res.name);
    if (!res.artist.isEmpty()) ui->labelArtist->setText(res.artist);

    // 歌词显示
    QString lrc = res.lrc;
    lrc.replace("<br />", "\n").replace("<br>", "\n");
    ui->lyricsWidget->setLrcText(lrc.isEmpty() ? QString("无歌词") : lrc);

    // 交给 mpv 播放（直接播放）
    m_player->playUrl(res.url);
    updatePlayPauseUI(true);
}

///////////////////////////////////////////////////////////////////////////////
// 图片回调
///////////////////////////////////////////////////////////////////////////////
void MainWindow::onImageFetched(const QPixmap &pix)
{
    if (!pix.isNull()) {
        ui->labelCover->setPixmap(pix.scaled(ui->labelCover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelCover->setPixmap(QPixmap(":/default_cover.png").scaled(ui->labelCover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

///////////////////////////////////////////////////////////////////////////////
// 播放控制：播放/暂停/停止/上一/下一
///////////////////////////////////////////////////////////////////////////////
void MainWindow::on_btnPlayPause_clicked()
{

    if (m_player->isPlaying()) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void MainWindow::on_btnPrev_clicked()
{
    if (m_searchList.isEmpty()) return;
    int n = m_searchList.size();
    if (m_currentIndex <= 0) m_currentIndex = n - 1;
    else --m_currentIndex;

    // 模拟列表点击流程
    const auto &it = m_searchList.at(m_currentIndex);
    setMetadataFromSearchItem(it);
    m_net->fetchImage(it.picurl);
    ui->statusbar->showMessage("解析播放地址...");
    m_net->getUrlById(it.id);
}

void MainWindow::on_btnNext_clicked()
{
    if (m_searchList.isEmpty()) return;
    int n = m_searchList.size();
    m_currentIndex = (m_currentIndex + 1) % n;

    const auto &it = m_searchList.at(m_currentIndex);
    setMetadataFromSearchItem(it);
    m_net->fetchImage(it.picurl);
    ui->statusbar->showMessage("解析播放地址...");
    m_net->getUrlById(it.id);
}

///////////////////////////////////////////////////////////////////////////////
// 进度/音量交互
///////////////////////////////////////////////////////////////////////////////
void MainWindow::on_sliderPosition_sliderMoved(int position)
{
    // position 单位为秒（我们把 slider 的值当作秒）
    qint64 ms = qint64(position) * 1000;
    m_player->setPosition(ms);
}

void MainWindow::on_sliderVolume_valueChanged(int value)
{
    m_player->setVolume(value);
}

///////////////////////////////////////////////////////////////////////////////
// MPVPlayer 回调：位置 / 时长 / 状态
///////////////////////////////////////////////////////////////////////////////
void MainWindow::onPositionChanged(qint64 ms)
{
    // 更新进度显示与 slider（防止在用户拖动时干扰）
    if (!ui->sliderPosition->isSliderDown()) {
        qint64 sec = ms / 1000;
        ui->labelProgress->setText(secondsToString(ms));
        ui->lyricsWidget->updatePosition(ms);
        ui->sliderPosition->blockSignals(true);
        ui->sliderPosition->setValue((int)sec);
        ui->sliderPosition->blockSignals(false);
    }
}

void MainWindow::onDurationChanged(qint64 ms)
{
    qint64 sec = ms / 1000;
    ui->labelDuration->setText(secondsToString(ms));
    ui->sliderPosition->setRange(0, (int)sec);
}

void MainWindow::onPlayerStateChanged(bool playing)
{
    updatePlayPauseUI(playing);
}

///////////////////////////////////////////////////////////////////////////////
// 工具函数
///////////////////////////////////////////////////////////////////////////////
void MainWindow::setMetadataFromSearchItem(const NetworkManager::SearchItem &it)
{
    ui->labelTitle->setText(it.title.isEmpty() ? "未选择" : it.title);
    ui->labelArtist->setText(it.singer);
}

void MainWindow::updatePlayPauseUI(bool playing)
{
    if (playing) ui->btnPlayPause->setIcon(QIcon(":/image/res/pause.png"));
    else ui->btnPlayPause->setIcon(QIcon(":/image/res/play.png"));
}

void MainWindow::resetMetadataDisplay()
{
    ui->labelTitle->setText("未选择");
    ui->labelArtist->setText("");
    ui->labelCover->setPixmap(QPixmap(":/default_cover.png").scaled(ui->labelCover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelProgress->setText("0 s");
    ui->labelDuration->setText("0 s");
    ui->sliderPosition->setRange(0, 0);
}

QString MainWindow::secondsToString(qint64 ms)
{
    qint64 totalSec = ms / 1000;
    qint64 m = totalSec / 60;
    qint64 s = totalSec % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void MainWindow::on_host_btn_clicked()
{
    ui->statusbar->showMessage("加载中...");
    ui->listResults->clear();
    m_searchList.clear();
    m_currentIndex = -1;

    m_net->getHost();
}

void MainWindow::on_new_btn_clicked()
{
    ui->statusbar->showMessage("加载中...");
    ui->listResults->clear();
    m_searchList.clear();
    m_currentIndex = -1;

    m_net->getNew();
}

void MainWindow::on_love_btn_clicked()
{

}
