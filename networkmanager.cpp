/**
 * @brief   : 网络模块实现（search/geturl/fetchImage）
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/
#include "networkmanager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent),
      m_mgr(new QNetworkAccessManager(this))
{
    connect(m_mgr, &QNetworkAccessManager::finished, this, &NetworkManager::onReplyFinished);
}

void NetworkManager::search(const QString &keyword)
{
    QUrl url;
    QUrlQuery q;
    if(keyword.isEmpty()){
        url = (QStringLiteral("https://a.buguyy.top/newapi/gethot.php?t=1"));
    }
    else{
        url = (QStringLiteral("https://a.buguyy.top/newapi/search.php"));
        q.addQueryItem("keyword", keyword);

    }
    url.setQuery(q);
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "QtMusicPlayer/1.0");
    m_mgr->get(req);
}

void NetworkManager::getUrlById(int id)
{
    QUrl url(QStringLiteral("https://a.buguyy.top/newapi/geturl2.php"));
    QUrlQuery q;
    q.addQueryItem("id", QString::number(id));
    url.setQuery(q);
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "QtMusicPlayer/1.0");
    m_mgr->get(req);
}

void NetworkManager::fetchImage(const QString &url)
{
    if (url.isEmpty()) {
        emit imageFetched(QPixmap());
        return;
    }
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "QtMusicPlayer/1.0");
    // 新建 manager 单次请求（避免与主 manager 混淆）
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    QNetworkReply *reply = mgr->get(req);
    connect(reply, &QNetworkReply::finished, this, [reply, mgr, this]() {
        QPixmap pix;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            pix.loadFromData(data);
        }
        emit imageFetched(pix);
        reply->deleteLater();
        mgr->deleteLater();
    });
}

void NetworkManager::getHost()
{
    QUrl url(QStringLiteral("https://a.buguyy.top/newapi/gethot.php?t=1"));
    QUrlQuery q;
    url.setQuery(q);
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "QtMusicPlayer/1.0");
    m_mgr->get(req);
}

void NetworkManager::getNew()
{
    QUrl url(QStringLiteral("https://a.buguyy.top/newapi/getnew.php?t=1"));
    QUrlQuery q;
    url.setQuery(q);
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "QtMusicPlayer/1.0");
    m_mgr->get(req);
}

void NetworkManager::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;
    QUrl url = reply->request().url();
    QByteArray body = reply->readAll();
    QString path = url.path();

    if (path.contains("search.php") || path.contains("gethot.php") || path.contains("getnew.php")) {
        QList<SearchItem> list;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(body, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value("code").toInt() == 200) {
                QJsonObject data = obj.value("data").toObject();
                QJsonArray arr = data.value("list").toArray();
                for (const QJsonValue &v : arr) {
                    QJsonObject it = v.toObject();
                    SearchItem si;
                    si.id = it.value("id").toInt();
                    si.title = it.value("title").toString();
                    si.singer = it.value("singer").toString();
                    si.picurl = it.value("picurl").toString();
                    si.hit = it.value("hit").toInt();
                    list.append(si);
                }
            }
        }
        emit searchFinished(list);
    }
    else if (path.contains("geturl2.php")) {
        UrlResult res;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(body, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value("code").toInt() == 200) {
                QJsonObject data = obj.value("data").toObject();
                res.rid = data.value("rid").toString();
                res.name = data.value("name").toString();
                res.artist = data.value("artist").toString();
                res.album = data.value("album").toString();
                res.quality = data.value("quality").toString();
                res.duration = data.value("duration").toString();
                res.size = data.value("size").toString();
                res.url = data.value("url").toString();
                res.pic = data.value("pic").toString();
                res.lrc = data.value("lrc").toString();
            }
        }
        emit getUrlFinished(res);
    }

    reply->deleteLater();
}
