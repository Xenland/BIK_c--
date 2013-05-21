#include "bik.h"

/** ** ** ** ** ** ** **
 * BIK Initialization
 */
    bik::bik(QObject *parent) :
        QObject(parent)
    {
        //Init Id tracker
        id_tracker = 0;

        //Init new network manager (by default 6 concurrent connections)
        netManager = new QNetworkAccessManager();


        QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(reply_net(QNetworkReply*)));

        //TEMP CODE
        proccessRequestQueryQueue("getbalance");
    }


    /** ** ** ** ** ** ** **
     * BIK Queue Requests
     *  Purpose
     *
     */
    /*void bik::addRequestQueryToQueue(){
        //THIS IS JUST A PLACE HOLDER UNTIL WE FIGURE OUT HOW WE WANT THE QUEUE SYSTEM TO WORK.
    }*/


    /** ** ** ** ** ** ** **
     * BIK Proccessing Request Views
     * this is temp, used as a "execute" coin query this will be changed later when we get queuing going.
     *
     */
    void bik::proccessRequestQueryQueue(QString method){
        QUrl serviceURL("http://username:password@127.0.0.1:4367/");
        QNetworkRequest request(serviceURL);

        //Increment Id tracker
        id_tracker += 1;

        //Increment json string
        QString jsonString = "{\"method\":\"%1\", \"id\":%2, \"params\":[]}\n\r";
                jsonString = jsonString.arg(method).arg(id_tracker);

        QByteArray json = jsonString.toUtf8();
        qDebug() << jsonString;

        // Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
        request.setRawHeader("User-Agent", "BIKv0.0.1");
        request.setRawHeader("X-Custom-User-Agent", "BIKv0.0.1");
        request.setRawHeader("Content-Type", "application/json");
        request.setRawHeader("Content-Length", QByteArray::number(json.size()));

        //Send request
        reply = netManager->post(request, json);

        //Record request locally (so we can track the reply data according to request data)
            //TO DO ^_^
    }



    /** ** ** ** ** ** ** **
     * BIK Networking Response Handler
     *
     *  Purpose
     *      Find the request that matches the incomming response (if json/net valid)
     */
    void bik::reply_net(QNetworkReply * replylocal){
        qDebug() << "Reply found";

        int network_error = replylocal->error();

        QString data = (QString) replylocal->readAll();
        qDebug() << data;
        qDebug() << "END DATA";
        /** Haven't decided how to handle HTTP errors as it seems it returns errors
         *  even with a valid JSON response string, so that makes things very difficult
         *  I guess resorting to checking if the json is valid would be a start...
         *
         **/
        if(network_error == 0){


        }else{
            //Error Occurred (Which one? react accordingly)
            if(network_error == 1){
                //Connection refused (Server is likely not open to the network or running)
                qDebug() << "Connection Refused / Server not running";
            }

            if(network_error == 299){
                //unknown content recieved
                qDebug() << "Unknown Content Received";
            }
            qDebug() << "ERROR" << network_error;
        }
    }

