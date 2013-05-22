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

        //Init proccessing queue stack
        proccess_queued_request_running = 0;

            //When a new request is added to the qlist, trigger proccess queue requests();
            QObject::connect(this, SIGNAL(addRequestQueryToQueue(int,QString,QStringList)), this, SLOT(proccess_queued_request(int, QString, QStringList)));
    }


    /** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
     * BIK Proccessing Request Views
     ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/
        //This will be activated everytime a new queue comes in, if its already running it won't over run, if its not running it will being to run.
        void bik::proccess_queued_request(int id_tracker, QString method, QStringList params){
            qDebug() << "PROCESSING JUMPSTARTED/CONTINUING...";

            /** Outer layer of the proccess_queue_requests
             *  This layer will add new requests when called,
             *  it will also check if the function is already running,
             *  if it is already running it will not over run,
             *  if it is NOT running it will begin to run in a loop until the queue is used up
             **/

                //Define local function variables
                QVariant new_request;
                QMap<QString, QVariant> new_request_map;

                //Generate request to later add to the coin request tracker
                new_request_map["id_tracker"] = id_tracker;
                new_request_map["method"] = method;
                new_request_map["params"] = params;

                //Add request to the coin request tracker
                    new_request = QVariant(new_request_map);
                    torpc_tracker_queue->append(new_request);


            //Check if its okay to begin/start the loop if its not okay don't start the loop to prevent overun
            if(proccess_queued_request_running == 0){
                //Lock loop from overruns
                proccess_queued_request_running = 1;

                /** Inner layer of the proccess_queue_requests
                 *  this will loop (the loop wont over run with requests becuase of the previous checks)
                 **/
                while(torpc_tracker_queue->size() > 0){
                    qDebug() << "BTC TRACKER QUEUED";
                    /*
                    QUrl serviceURL("http://username:password@127.0.0.1:4367/");
                    QNetworkRequest request(serviceURL);

                    //generate json string | client -> server
                                //Generate params string (if nessecary)
                                QString params_string;
                                int params_size = params.size();
                                if(params_size > 0){
                                    for(int a = 0; a < params_size; a++){
                                        //Append comma (if needed for the json array)
                                        if(a > 0){
                                            params_string.append(",");
                                        }

                                        //Append to json array object
                                        params_string.append("%1");
                                        params_string = params_string.arg(params[a]);
                                    }
                                }

                    QString jsonString = "{\"method\":\"%1\", \"id\":%2, \"params\":[%3]}\n\r";
                    jsonString = jsonString.arg(method).arg(id_tracker).arg(params_string);

                    QByteArray json = jsonString.toUtf8();
                    qDebug() << jsonString;

                    //Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
                    request.setRawHeader("User-Agent", "BIKv0.0.1");
                    request.setRawHeader("X-Custom-User-Agent", "BIKv0.0.1");
                    request.setRawHeader("Content-Type", "application/json");
                    request.setRawHeader("Content-Length", QByteArray::number(json.size()));

                    //Send request
                    reply = netManager->post(request, json);

                    //Record request locally (so we can track the reply data according to request data when the connect function slot is called)
                        //TO DO ^_^
                    */
                }
            }
        }



    /** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
     * BIK Networking Response Handler
     *
     *  Purpose
     *      Find the request that matches the incomming response (if json/net valid)
     ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/
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


    /** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
     * BIK Coin Calling List
     ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

        /* ==========================
         * BEGIN Get Received By Address Functions (getreceivedbyaddress)
         */


            /* *** *** *** *** *** *** ***
             * getreceivedbyaddress
             *  BIK INPUT PARAMS
             *      coin_server_id (string)
             *      coin_address    (string)
             *      minimum_confirmations (int)
             *
             *  Bitcoin API INPUT PARAMS
             *      <bitcoinaddress>
             *      [min confirmations=1]
             *
             *  OUTPUT PARAMS
             *      return_status (int)
             *      bik_tx_id       (int)
             *
             * *** *** *** *** *** *** ***/

             QMap<QString, QVariant> bik::getreceivedbyaddress(QString coin_server_id, QString coin_address, int mininimum_confirmations){
                /**
                 * Define function local variables
                 **/
                    QMap<QString, QVariant>  output;
                    output["return_status"]         = (int) -1;
                    output["bik_tx_id"]             = (int) -1;


                /**
                 * Add request to the coin queue (The signal will tell the calling app when requests have been received for proccessing)
                 **/
                    //Increment bik tx id
                    id_tracker = id_tracker + 1;
                        //Set output bik tx id
                        output["bik_tx_id"] = id_tracker;

                    //Generate request query to add to the queue
                    QStringList params;
                    params.append("\""+coin_address+"\"");
                    params.append(QString(mininimum_confirmations));

                    //Add request query to the queue
                    emit addRequestQueryToQueue(id_tracker, "getreceivedbyaddress", params);

                    qDebug() << "REQUEST ADDED";

                /**
                 * OUTPUT
                 * Return output data.
                 **/
                    //Nothing to check to see if the function ran correclty so just output success
                    output["return_status"] = 1;

                    //Output
                    return output;
            }
