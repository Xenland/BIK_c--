#include "bik.h"

/** ** ** ** ** ** ** ** ** ** **
 * BIK Initializer (Constructor)
 **/
bik::bik(QObject *parent) : QObject(parent){
    //Init Networking
    netAccessManager = new QNetworkAccessManager();
        //Set accessible
        netAccessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);

        //Make connections
        connect(netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(coin_server_response(QNetworkReply*)));

    //Init request tracker
    last_request_id_tracker = 0;

    //Init proccessing request queue
    proccessing_request_queue = 0;

    //Make connections
        //New request added to queue to "send" from the client to the server.
        connect(this, SIGNAL(requestAddedToQueue()), this, SLOT(process_request_queue()));
}



void bik::addServerConfig(QString server_id_name, QString connection_string){
    //Add server to config
    server_list[server_id_name] = connection_string;
}





/** ** ** ** ** ** ** ** ** ** **
 * (Private) Networking Manager
 **/
        /* *** *** *** *** *** *** *** ***
         * Add "API Call" to the queue to be requested to the network
         **/
        void bik::addToQueue(int request_id_tracker, QString server_id, QString coin_api_call, QVariantList coin_api_parameters){
            //Define local function variables
            QMap<QString, QVariant> new_request_map;


            //Generate request to later add to the coin request tracker
            new_request_map["id_tracker"]   = request_id_tracker;
            new_request_map["server_id"]    = server_id;
            new_request_map["method"]       = coin_api_call;
            new_request_map["params"]       = coin_api_parameters;


            //Add request to the coin request tracker
            torpc_tracker_queue.append(QVariant(new_request_map));

            //Emit signal to indicate that there is a new call/request added to the queue.
            emit requestAddedToQueue();
        }

        /* *** *** *** *** *** *** *** ***
         * This is activated when an request has been added (Designed to not over run)
         **/
        void bik::process_request_queue(){


            //Don't overrun
            if(proccessing_request_queue == 0){
                //Lock
                proccessing_request_queue = 1;

                //Loop through the local request queue
                while(torpc_tracker_queue.size() > 0){

                    /** Extract the tracker row **/
                    QMap<QString, QVariant> to_tracker_request  = QVariant(torpc_tracker_queue[0]).toMap();
                    int id_tracker  = to_tracker_request["id_tracker"].toInt();
                    QString server_id = to_tracker_request["server_id"].toString();
                    QString method  = to_tracker_request["method"].toString();
                    QVariantList params = to_tracker_request["params"].toList();

                    /** Decide which service/server/api to connect to for now its just static **/
                    QNetworkRequest request;
                    request.setUrl(QUrl(server_list[server_id]));

                    /** Generate a JSON string that will be sent as a request from the client to the server **/
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
                                    //If string append like so
                                    if(params[a].type() == 10){
                                        params_string.append("\"%1\"");
                                        params_string = params_string.arg(params[a].toString());
                                    }

                                    //If int append like so
                                    if(params[a].type() == 2){
                                        params_string.append("%1");
                                        params_string = params_string.arg(params[a].toInt());
                                    }
                            }
                        }


                    QString jsonString = "{\"method\":\"%1\", \"id\":%2, \"params\":[%3]}\n\r";
                    jsonString = jsonString.arg(method).arg(id_tracker).arg(params_string);


                    QByteArray json = jsonString.toUtf8();


                    /** ADD HEADERS (This was done after the content was generated becuase of content-length) **/
                        //Add the headers specifying their names and their values with the following method : void QNetworkRequest::setRawHeader(const QByteArray & headerName, const QByteArray & headerValue);
                        request.setRawHeader("User-Agent", "BIKv0.0.1");
                        request.setRawHeader("X-Custom-User-Agent", "BIKv0.0.1");
                        request.setRawHeader("Content-Type", "application/json");
                        request.setRawHeader("Content-Length", QByteArray::number(json.size()));



                    //Record request locally (so we can track the reply data according to request data when the connect function slot is called)
                        //Generate a data/row to save locally
                        QMap<QString, QVariant> fromrpc_tracker_row;
                            //Define what we have right now
                            fromrpc_tracker_row["method"] = method;
                            fromrpc_tracker_row["params"] = params;

                            //Define what we don't have right now but will be filled in during networking response.
                            fromrpc_tracker_row["response_code"] = -1; // -1 = null
                            //fromrpc_tracker_row[""]

                        //Add to the queue named "from server rpc tracker"
                        fromrpc_tracker_queue.append(fromrpc_tracker_row);

                    //Send request
                    QNetworkReply * reply = netAccessManager->post(request, json);
                        //Add reply to list to make it global(sort of) -- just to keep it alive for response/connection/networking connections request
                        //netReplyList->append(reply);


                    //Delete the the request from the local queue
                    torpc_tracker_queue.removeFirst();
                }

                //Unlock
                proccessing_request_queue = 0;
            }
        }


        void bik::coin_server_response(QNetworkReply * net_reply){
            qDebug() << "NETWORKING RESPONSE";

            qDebug() << net_reply->readAll();
        }


/** ** ** ** ** ** ** ** ** ** **
 * (Public) Coin API Call List
 **/

        /* *** *** *** *** *** *** *** ***
         * Coin Call | Get Received By Address
         * Info | Generally with calling the Bitcoin API
         *          you will get a returned balance ever received
         *          for that selected address.
         **/
        QMap<QString, QVariant> bik::getreceviedbyaddress(QString coin_server_id, QString coin_address, int mininimum_confirmations){
            /**
             * Define function local variables
             **/
                QMap<QString, QVariant>  output;
                output["return_status"]         = (int) -1;
                output["bik_tx_id"]             = (int) -1;

                /* Sanatize Incomming Variables */
                if(mininimum_confirmations < 0){
                    mininimum_confirmations = 0;
                }


            /**
             * Generate Request, then add request to the queue
             **/
                //Generate request query to add to the queue
                QVariantList params;
                params.append(coin_address);
                params.append(mininimum_confirmations);

            /**
             * Add request to queue
             **/
                //Increment request tracker
                last_request_id_tracker += 1;

                //Add to queue
                addToQueue(last_request_id_tracker, coin_server_id, "getreceivedbyaddress", params);


            return output;
        }
