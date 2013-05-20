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
    QNetworkAccessManager * netManager;
    QNetworkReply * reply;

signals:
    
public slots:


private slots:
    void reply_net(QNetworkReply*);
    
};

#endif // BIK_H
