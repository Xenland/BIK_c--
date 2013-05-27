#include "bik.h"

/** ** ** ** ** ** ** ** ** ** **
 * BIK Initializer (Constructor)
 **/
bik::bik(QObject *parent) : QObject(parent){
    //Init Networking
    netAccessManager = new QNetworkAccessManager();

    //Init request tracker
    last_request_id_tracker = 0;

    //Init proccessing request queue
    proccessing_request_queue = 0;


    //Config Networking
        //Set accessible
        //netAccessManager->setNetworkAccessible(QNetworkAccessManager::Accessible)

        //Make connections
        connect(netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(coin_server_response(QNetworkReply*)));
        connect(netAccessManager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(netAccessChanged(QNetworkAccessManager::NetworkAccessibility)));


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
            qDebug() << "ADD TO QUEUE";

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

                    qDebug() << jsonString;
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
                            fromrpc_tracker_row["id_tracker"] = id_tracker;
                            fromrpc_tracker_row["method"] = method;
                            fromrpc_tracker_row["params"] = params;

                            //Define what we don't have right now but will be filled in during networking response.
                            fromrpc_tracker_row["response_code"] = -1; // -1 = null

                        //Add to the queue named "from server rpc tracker"
                        fromrpc_tracker_queue.append(fromrpc_tracker_row);

                    //Send request
                    QNetworkReply * reply = netAccessManager->post(request, json);

                    //Delete the the request from the local queue
                    torpc_tracker_queue.removeFirst();
                }

                //Unlock
                proccessing_request_queue = 0;
            }
        }


        void bik::coin_server_response(QNetworkReply * net_reply){

            /* ** Define Local Variables ** */
            QByteArray server_reply = net_reply->readAll();
            qDebug() << server_reply;

            /** Was this net reply error or success response? **/
            if(net_reply->error() == 0){
                //Successfull reply
                qDebug() << "SUCCESS";
                    //Convert JSON into a readable format for QT to read from.

                    QScriptValue sc;
                    QScriptEngine engine;
                    sc = engine.evaluate("("+QString(server_reply)+")"); // In new versions it may need to look like engine.evaluate("(" + QString(result) + ")");

                    //Get id tracker (the rest of the json data will be parsed according to method which will be done below
                    int id_tracker = sc.property("id").toInteger();

                    //Loop through the fromrpc_tracker_queue list and find the associated response
                    int total_fromrpc_tracker_queue = fromrpc_tracker_queue.size();
                    for(int a = 0; a < total_fromrpc_tracker_queue; a++){
                        //Is this the droids we are looking for? (is this the associated response request)
                        QMap<QString, QVariant> fromrpc_tracker_row = fromrpc_tracker_queue[a].toMap();
                        if(fromrpc_tracker_row["id_tracker"] == id_tracker){
                            //We found the fromrpc_tracker_queue index; Append responses to the List; Stop this local for() loop;
                                //Update/Append response data
                                    //Successfull BIK response code
                                    fromrpc_tracker_row["response_code"] = 1;

                                    //Return the appropriate data
                                    if(fromrpc_tracker_row["method"] == "getreceivedbyaddress"){
                                        fromrpc_tracker_row["coin_balance_display"] = QVariant(sc.property("result").toString()).toString();

                                        double coin_api_balance = sc.property("result").toNumber();
                                        coin_api_balance = coin_api_balance * 100000000;
                                        fromrpc_tracker_row["coin_balance_int"]     = QVariant(coin_api_balance).toInt();

                                    }


                                    //Set/update data to the fromrpc_tracker_queue
                                    fromrpc_tracker_queue[a] = fromrpc_tracker_row;

                                //Stop for loop
                                a = total_fromrpc_tracker_queue;
                        }
                    }
            }else{
                //Failed to get a successfull response reply.
                qDebug() << "FAILED";
            }


            //Emit a signal that will indcate to the calling app that we have a coin api response for you to review/proccess (error or not)
            emit coin_response_received();
        }


        void bik::netAccessChanged(QNetworkAccessManager::NetworkAccessibility){
            qDebug() << "ACCESSIBILITY CHANGED";
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
        QMap<QString, QVariant> bik::getreceivedbyaddress(QString coin_server_id, QString coin_address, int mininimum_confirmations){

            /**
             * Define function local variables
             **/
                int next_tracker_id = getNextRequestTrackerId();

                QMap<QString, QVariant>  output;
                output["read"]          = 0; //If non zero this has been read by the calling app.
                output["return_status"] = -1;
                output["bik_tx_id"]     = next_tracker_id;


                /* Sanatize Incomming Variables */
                if(mininimum_confirmations < 0){
                    mininimum_confirmations = 0;
                }


            /**
             * Generate Request, then add request to the queue
             **/
                //Generate request query to add to the queue
                QList<QVariant> params;
                params.append(QVariant(coin_address));
                params.append(QVariant(mininimum_confirmations));

            /**
             * Add request to queue
             **/
                //Set return status
                output["return_status"] = 1;

                //Add to queue
                addToQueue(next_tracker_id, coin_server_id, "getreceivedbyaddress", params);

            return output;
        }

/** Low Level Functions **/
        int bik::getNextRequestTrackerId(){
            last_request_id_tracker +=1;

            return last_request_id_tracker;
        }

        QList<QVariant> bik::getCurrentResponseList(){
            //Loop through fromrpc_tracker_queue up to the current .size() and set it to read = 1
            for(int a = 0; a < fromrpc_tracker_queue.size(); a++){
                //Set read to 1
                QMap<QString, QVariant> fromrpc_tracker_queue_row = fromrpc_tracker_queue[a].toMap();
                fromrpc_tracker_queue_row["read"] = 1;

                //Update row
                fromrpc_tracker_queue[a] = fromrpc_tracker_queue_row;
            }

            return fromrpc_tracker_queue;
        }
