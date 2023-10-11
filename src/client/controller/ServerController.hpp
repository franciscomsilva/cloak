#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <iostream>
#include "../common/FdPair.hpp"


class SocksProxyServer;

class ServerController {

    public:

        ServerController (SocksProxyServer *sp);


        int receiveNewClientConnection();

        void handleSocksNewConnection(FdPair *fdp);

        void handleSocksRestoreConnection(FdPair *fdp);

        int notifyNewClientConnection(int client_socket);

        void handleSocksTorDataReady(FdPair *fdp);

        void handleSocksClientDataReady(FdPair *fdp);

        void handleSocksNewSessionError();

        void config(int client_port, int tor_port);

        int get_client_port() {
            return _client_port;
        };

        int get_tor_port(){
            return _tor_port;
        }


    protected:
        SocksProxyServer* _sp;

        int _client_port = -1;

        int _tor_port = -1;


};

#endif