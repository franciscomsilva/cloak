#ifndef SOCKSPROXYSERVER_H
#define SOCKSPROXYSERVER_H

#include <set>
#include "../../common/Common.hpp"

#include "../../controller/ServerController.hpp"


class SocksProxyServer {

public:

    SocksProxyServer(){};

    virtual ~SocksProxyServer(){};

    int initialize(ServerController *controller, int run_mode);

    int readn_msg_client(FdPair *fd_pair, char *buff, int buffsize);

    int writen_msg_client(FdPair *fd_pair, char *buff, int size);

    int readn_msg_local(FdPair *fd_pair, char *buff, int buffsize);

    int writen_msg_local(FdPair *fd_pair, char *buff, int size);

    int shutdown_connection(FdPair *fd_pair);

    int shutdown_local_connection(FdPair *fd_pair);

    int restore_local_connection(FdPair *fd_pair);

private:

    void *main_thread();

private:

    int _port_client;

    int _port_tor;

    ServerController *_controller;

    std::set<FdPair*> _fds;

    fd_set _active_fd_set;

    std::set<FdPair*> _zombies;


};

#endif //SOCKSPROXYSERVER_H
