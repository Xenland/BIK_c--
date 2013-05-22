#ifndef BIK_H
#define BIK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>
#include <QStringList>

class bik : public QObject
{
    Q_OBJECT
public:
    explicit bik(QObject *parent = 0);

    /* Bitcoin Call List */
    QMap<QString, QVariant> getreceivedbyaddress(QString coin_server_id, QString coin_address, int mininimum_confirmations=1);


private:
    //Global private vars
    int id_tracker;

    int proccess_queued_request_running;

    QNetworkAccessManager * netManager;
    QNetworkReply * reply;
    QList<QVariant> * torpc_tracker_queue; //Stores a memory copy of the list of commands to submit from the client to the server

signals:
    void addRequestQueryToQueue(int, QString, QStringList);


public slots:


private slots:
    void reply_net(QNetworkReply*);
    void proccess_queued_request(int, QString, QStringList);
    
};

#endif // BIK_H
