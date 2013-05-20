#include "bik.h"

bik::bik(QObject *parent) :
    QObject(parent)
{
    netManager = new QNetworkAccessManager();
    QUrl url("http://gdata.youtube.com/feeds/api/standardfeeds/most_popular?v=2&alt=json");
    QNetworkRequest req(url);

    reply = netManager->get(req);

    QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(reply_net(QNetworkReply*)));
}


void bik::reply_net(QNetworkReply * replylocal){
    qDebug() << "Reply found";

    int network_error = replylocal->error();
    if(network_error == 0){
        String data = (QString) replylocal->readAll();
    }
}

