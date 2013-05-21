#ifndef BIK_H
#define BIK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

class bik : public QObject
{
    Q_OBJECT
public:
    explicit bik(QObject *parent = 0);

private:
    //Global vars
    int id_tracker;
    QNetworkAccessManager * netManager;
    QNetworkReply * reply;

    //Global func
    void proccessRequestQueryQueue(QString method);
signals:
    
public slots:


private slots:
    void reply_net(QNetworkReply*);
    
};

#endif // BIK_H
