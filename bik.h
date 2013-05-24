#ifndef BIK_H
#define BIK_H

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QObject>
#include <QList>
#include <QVariant>
#include <QDebug>

class bik : public QObject
{
    Q_OBJECT
public:
    explicit bik(QObject *parent = 0);

    /* BIK Server Config */
    void addServerConfig(QString, QString);

    /* Coin Api Call list */
        //Variables
            //No vars

        //Functions
        QMap<QString, QVariant> getreceviedbyaddress(QString coin_server_id, QString coin_address, int mininimum_confirmations=1);

private:
    /* Server Config */
        QMap<QString, QString> server_list;

    /* Networking Manager */
        //Variables
        QNetworkAccessManager * netAccessManager;

        int last_request_id_tracker;
        QList<QVariant> torpc_tracker_queue; //Stores a memory copy of the list of commands to submit from the client to the server
        QList<QVariant> fromrpc_tracker_queue; //Stores as a 2 phase system; as network requests to the server are submitted they are added to here, the rows/datas/values are filled as the networking responses come in so that a slot can alert the calling app when new data has arraived)

        int proccessing_request_queue;

        //Functions
        void addToQueue(int request_id_tracker, QString server_id, QString coin_api_call, QVariantList coin_api_parameters);

signals:

        //New request added
        void requestAddedToQueue();
    
public slots:
    

private slots:
        void process_request_queue();
        void coin_server_response(QNetworkReply*);
};

#endif // BIK_H
