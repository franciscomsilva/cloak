#include <thread>
#include <condition_variable>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>


#include "SocksProxyServer.hpp"




int SocksProxyServer::readn_msg_client(FdPair *fd_pair, char *buff, int buffsize)
{
    assert(buff != NULL && buffsize > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return read(fd_pair->get_fd0(), buff, buffsize);
}

int SocksProxyServer::writen_msg_client(FdPair *fd_pair, char *buff, int size)
{
    assert(buff != NULL && size > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }


    return write(fd_pair->get_fd0(), buff, size);
}

int SocksProxyServer::readn_msg_local(FdPair *fd_pair, char *buff, int buffsize)
{
    assert(buff != NULL && buffsize > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return read(fd_pair->get_fd1(), buff, buffsize);
}



int SocksProxyServer::writen_msg_local(FdPair *fd_pair, char *buff, int size)
{
    assert(buff != NULL && size > 0);
    int status;

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    int fd = fd_pair->get_fd1();
    if (fd == INV_FD) { //No Tor connection currently established
        fd = restore_local_connection(fd_pair);
    }

    return write(fd, buff, size);
}

int SocksProxyServer::shutdown_connection(FdPair *fd_pair)
{
    assert(_fds.find(fd_pair) != _fds.end());

    if (_zombies.find(fd_pair) == _zombies.end()) {
       /* if (fd_pair->get_fd0() != INV_FD) {
            shutdown(fd_pair->get_fd0(), SHUT_RDWR);
            FD_CLR (fd_pair->get_fd0(), &_active_fd_set);
        }*/

        if (fd_pair->get_fd1() != INV_FD) {
            shutdown(fd_pair->get_fd1(), SHUT_RDWR);
            FD_CLR (fd_pair->get_fd1(), &_active_fd_set);
        }

        _zombies.insert(fd_pair);
    }
    return 0;
}

int SocksProxyServer::shutdown_local_connection(FdPair *fd_pair) {
    int status;
    char buffer_r[BUFSIZE];


    assert(_fds.find(fd_pair) != _fds.end());

    if (fd_pair->get_fd1() != INV_FD) {

        int res = recv(fd_pair->get_fd1(), &buffer_r, 1, MSG_PEEK | MSG_DONTWAIT);

        if(res > 0) {
            status = shutdown(fd_pair->get_fd1(), SHUT_RDWR);
            assert(status == 0);
        }

        FD_CLR (fd_pair->get_fd1(), &_active_fd_set);

        status = close(fd_pair->get_fd1());
        assert(status == 0);

        fd_pair->set_fd1(INV_FD);
    }

#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
    LOG(LogLevel::DEBUG, "Shutdown local connection: client %d, tor %d", fd_pair->get_fd0(), fd_pair->get_fd1());
#endif

    return 0;
}

int SocksProxyServer::restore_local_connection(FdPair *fd_pair) {

    assert(_fds.find(fd_pair) != _fds.end());

    if (fd_pair->get_fd1() != INV_FD) {
        return -1;
    }

    struct addrinfo hints, *res;
    int fd_local;

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const char *local_port = std::to_string(_port_tor).c_str();
    if (getaddrinfo("127.0.0.1", local_port, &hints, &res) != 0) {
        LOG(LogLevel::DEBUG, "Fatal error restore: getaddrinfo");
        exit(EXIT_FAILURE);
    }

    /* Create the socket */
    fd_local = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd_local == -1) {
        LOG(LogLevel::DEBUG, "Fatal error restore: socket");
        exit(EXIT_FAILURE);
    }

    /* Connect to the local host */
    if (connect(fd_local, res->ai_addr, res->ai_addrlen) == -1) {
        LOG(LogLevel::DEBUG, "Fatal error restore: connect");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    assert(fd_pair != nullptr && fd_pair->get_fd1() == INV_FD);
    fd_pair->set_fd1(fd_local);

    FD_SET(fd_local, &_active_fd_set);

#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
    LOG(LogLevel::DEBUG, "Restored local connection: client %d, local %d", fd_pair->get_fd0(),
            fd_pair->get_fd1());
#endif

    return fd_local;
}

void *SocksProxyServer::main_thread()
{
    int sock_fd, fd_client, fd_local;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */
    fd_set read_fd_set;

    signal(SIGPIPE, SIG_IGN);

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const char *client_port = std::to_string(_port_client).c_str();
    if (getaddrinfo("127.0.0.1", client_port, &hints, &res) != 0) {
        LOG(LogLevel::DEBUG, "Fatal error: getaddrinfo");
        exit(EXIT_FAILURE);
    }

    /* Create the socket */
    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd == -1) {
        freeaddrinfo(res);
        LOG(LogLevel::DEBUG, "Fatal error: socket");
        exit(EXIT_FAILURE);
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(int)) == -1)
    {
        LOG(LogLevel::DEBUG, "Fatal error: setsockopt");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    /* Bind to the address */
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(atoi(client_port));

    if (bind(sock_fd, (struct sockaddr *)&local, sizeof(local)) == -1) {
        LOG(LogLevel::DEBUG, "Fatal error: bind");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }
    if (listen(sock_fd, MAX_LISTEN_USERS) == -1) {
        LOG(LogLevel::DEBUG, "Fatal error: listen");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);


    FD_ZERO (&_active_fd_set);
    FD_SET (sock_fd, &_active_fd_set);

    while (true) {

        read_fd_set = _active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            LOG(LogLevel::ERROR,"Fatal error: select()");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET (sock_fd, &read_fd_set)) {

            struct sockaddr_in remote;
            socklen_t remotelen = sizeof(remote);
            fd_client = accept(sock_fd, (struct sockaddr *)&remote, &remotelen);
            if (fd_client < 0) {
                LOG(LogLevel::DEBUG, "Fatal error: accept()");
                exit(EXIT_FAILURE);
            }

            FdPair *fd_pair = new FdPair(fd_client, INV_FD);

            _fds.insert(fd_pair);
            FD_SET (fd_client, &_active_fd_set);
            _controller->handleSocksNewConnection(fd_pair);
        }

        for (FdPair *fdp : _fds) {
            fd_client = fdp->get_fd0();
            fd_local = fdp->get_fd1();
            if (FD_ISSET (fd_client, &read_fd_set)) {
                _controller->handleSocksClientDataReady(fdp);
            }
            if (fd_local != INV_FD && FD_ISSET (fd_local, &read_fd_set)) {
                _controller->handleSocksTorDataReady(fdp);
            }
        }

    }

    return NULL;
}

int SocksProxyServer::initialize(ServerController *controller, int run_mode)
{

    assert(controller != NULL);

    _controller = controller;

    _port_client = _controller->get_client_port();
    _port_tor = _controller->get_tor_port();


    if (run_mode == RUN_BACKGROUND) {
        std::thread th(&SocksProxyServer::main_thread, this);
        th.detach();
        return 0;
    }

    if (run_mode == RUN_FOREGROUND) {
        main_thread();
        return 0;
    }

    return -1;
}
