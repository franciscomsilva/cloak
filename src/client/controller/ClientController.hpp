#ifndef CLIENTCONTROLLER_H
#define CLIENTCONTROLLER_H

#include <iostream>
#include "../common/FdPair.hpp"


class SocksProxyClient;


class ClientController {


    public:
        ClientController (SocksProxyClient *sp);


        int receiveNewClientConnection();

        void handleSocksNewConnection(FdPair *fdp);

        void handleSocksRestoreConnection(FdPair *fdp);

        int notifyNewClientConnection(int client_socket);

        void handleSocksBridgeDataReady(FdPair *fdp);

        void handleSocksClientDataReady(FdPair *fdp);

        void handleSocksNewSessionError();

        void handleSocksConnectionTerminated(FdPair *fdp);


    void config(int socks_port, int local_port);

        int get_socks_port() {
            return _socks_port;
        };

        int get_local_port(){
            return _local_port;
        }

protected:

        SocksProxyClient* _sp;

        int _socks_port = -1;

        int _local_port = -1;

};

#endif