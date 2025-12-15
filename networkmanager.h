/**
 * @brief   : 网络模块，负责搜索、解析真实播放地址、以及加载封面图片
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    struct SearchItem {
        int id;
        QString title;
        QString singer;
        QString picurl;
        int hit;
    };

    struct UrlResult {
        QString rid;
        QString name;
        QString artist;
        QString album;
        QString quality;
        QString duration;
        QString size;
        QString url;  // 真实播放地址
        QString pic;
        QString lrc;
    };

    void search(const QString &keyword);
    void getUrlById(int id);
    void fetchImage(const QString &url); // 异步，将通过信号返回
    void getHost();
    void getNew();

signals:
    void searchFinished(const QList<SearchItem> &list);
    void getUrlFinished(const UrlResult &res);
    void imageFetched(const QPixmap &pix);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_mgr;
};

#endif // NETWORKMANAGER_H
