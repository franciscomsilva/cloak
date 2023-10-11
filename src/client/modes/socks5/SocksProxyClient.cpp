#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <assert.h>

#include <unistd.h>

#include "SocksProxyClient.hpp"


#define IPSIZE  (4)


/**
 * Socks versions identifiers
 */
enum socks {
    RESERVED = 0x00,
    VERSION4 = 0x04,
    VERSION5 = 0x05
};

enum socks_auth_methods {
    NOAUTH = 0x00,
    USERPASS = 0x02,
    NOMETHOD = 0xff
};

enum socks_auth_userpass {
    AUTH_OK = 0x00,
    AUTH_VERSION = 0x01,
    AUTH_FAIL = 0xff
};

enum socks_command {
    CONNECT = 0x01
};

enum socks_command_type {
    IP = 0x01,
    DOMAIN = 0x03,
    LOCAL = 0x04
};

enum socks_status {
    OK = 0x00,
    FAILED = 0x05
};

/* Global saved variables*/
int global_fd_bridge = -1;


/**
 * Resolves the target domain name or IP address and establishes a connection
 * @param type the type of target identifier (e.g. Domain Name, IP, etc...)
 * @param buf the buffer containing the domain name or IP address accordingly
 * @param portnum the port number of the target host
 * @return the file descriptor of the new connection to the target host or -1
 *          in case of connection failure
 */
int SocksProxyClient::app_connect(int type, void *buf, unsigned short int portnum) {
    int fd;
    struct sockaddr_in remote;
    char address[16];

    memset(address, 0, ARRAY_SIZE(address));

    if (type == IP) {
        char *ip = (char *) buf;
        snprintf(address, ARRAY_SIZE(address), "%hhu.%hhu.%hhu.%hhu",
                 ip[0], ip[1], ip[2], ip[3]);
        memset(&remote, 0, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_addr.s_addr = inet_addr(address);
        remote.sin_port = htons(portnum);

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(fd, (struct sockaddr *) &remote, sizeof(remote)) < 0) {
            LOG(LogLevel::DEBUG, "connect() in app_connect");
            close(fd);
            return -1;
        }

        return fd;
    } else if (type == DOMAIN) {
        char portaddr[6];
        struct addrinfo *res;
        snprintf(portaddr, ARRAY_SIZE(portaddr), "%d", portnum);

        LOG(LogLevel::DEBUG, "getaddrinfo: %s %s", (char *) buf, portaddr);

        int ret = getaddrinfo((char *) buf, portaddr, NULL, &res);
        if (ret == EAI_NODATA) {
            return -1;
        } else if (ret == 0) {
            struct addrinfo *r;
            for (r = res; r != NULL; r = r->ai_next) {
                fd = socket(r->ai_family, r->ai_socktype,
                            r->ai_protocol);
                if (fd == -1) {
                    continue;
                }
                ret = connect(fd, r->ai_addr, r->ai_addrlen);
                if (ret == 0) {
                    freeaddrinfo(res);
                    return fd;
                } else {
                    close(fd);
                }
            }
        }
        freeaddrinfo(res);
        return -1;
    }
    // Local Method to deliver data to the encoding thread
    else if (type == LOCAL){

        /* If there is no saved fd_bridge*/
        if(global_fd_bridge <= 0 ){
            char *ip = (char *) buf;
            snprintf(address, ARRAY_SIZE(address), "%hhu.%hhu.%hhu.%hhu",
                     ip[0], ip[1], ip[2], ip[3]);
            memset(&remote, 0, sizeof(remote));
            remote.sin_family = AF_INET;
            remote.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            remote.sin_port = htons(_controller->get_local_port());

            fd = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(fd, (struct sockaddr *) &remote, sizeof(remote)) < 0) {
                LOG(LogLevel::DEBUG, "connect() in app_connect");
                close(fd);
                return -1;
            }
            global_fd_bridge = fd;
            return fd;
        }
        else{
            return global_fd_bridge;
        }

    }

    return -1;
}


/**
 * Read and interprets the socks invitation sent by the client
 * @param fd the file descriptor of the client connection
 * @param version of the SOCKS proxy protocol (SOCKS4 or SOCKS5)
 * @return
 */
int SocksProxyClient::socks_invitation(int fd, int *version) {
    char init[2];
    int nread = readn(fd, (void *) init, ARRAY_SIZE(init));
    if (nread == 2 && init[0] != VERSION5 && init[0] != VERSION4) {
        LOG(LogLevel::DEBUG, "Incompatible SOCKS version!");
        return -1;
    }
    LOG(LogLevel::DEBUG, "Initial SOCKS: %hhX %hhX", init[0], init[1]);

    *version = init[0];
    return init[1];
}


/**
 * Gathers the username used during SOCKS authentication process. (Only SOCKS5)
 * @param fd the file descriptor of the client connection
 * @return the username sent by the client during authentication process
 */
char *SocksProxyClient::socks5_auth_get_user(int fd) {
    unsigned char size;
    readn(fd, (void *) &size, sizeof(size));

    char *user = (char *) malloc(sizeof(char) * size + 1);
    readn(fd, (void *) user, (int) size);
    user[size] = 0;

    return user;
}


/**
 * Gathers the password used during SOCKS authentication process. (Only SOCKS5)
 * @param fd the file descriptor of the client connection
 * @return the password sent by the client during authentication process
 */
char *SocksProxyClient::socks5_auth_get_pass(int fd) {
    unsigned char size;
    readn(fd, (void *) &size, sizeof(size));

    char *pass = (char *) malloc(sizeof(char) * size + 1);
    readn(fd, (void *) pass, (int) size);
    pass[size] = 0;

    return pass;
}


/**
 * Accepts or denies an authentication based on credentials check
 * @param fd the file descriptor of the client connection
 * @return 0 if authentication is success, 1 if authentication failed
 */
int SocksProxyClient::socks5_auth_userpass(int fd, char *arg_username,
                                           char *arg_password) {
    char answer[2] = {VERSION5, USERPASS};
    writen(fd, (void *) answer, ARRAY_SIZE(answer));
    char resp;
    readn(fd, (void *) &resp, sizeof(resp));
    LOG(LogLevel::DEBUG, "auth %hhX", resp);

    char *username = socks5_auth_get_user(fd);
    char *password = socks5_auth_get_pass(fd);

    LOG(LogLevel::DEBUG, "l: %s p: %s", username, password);

    if (strcmp(arg_username, username) == 0
        && strcmp(arg_password, password) == 0) {
        unsigned char answer[2] = {AUTH_VERSION, AUTH_OK};
        writen(fd, (void *) answer, ARRAY_SIZE(answer));
        free(username);
        free(password);
        return 0;
    } else {
        unsigned char answer[2] = {AUTH_VERSION, AUTH_FAIL};
        writen(fd, (void *) answer, ARRAY_SIZE(answer));
        free(username);
        free(password);
        return 1;
    }
}


/**
 * Accepts a non authenticated connection
 * @param fd the file descriptor of the client connection
 * @return 0 in case of success
 */
int SocksProxyClient::socks5_auth_noauth(int fd) {
    char answer[2] = {VERSION5, NOAUTH};
    writen(fd, (void *) answer, ARRAY_SIZE(answer));
    return 0;
}


/**
 * Sends a error message to the client informing that the server do not know
 * or support the authentication message purposed by the client
 * @param fd the file descriptor of the client connection
 */
void SocksProxyClient::socks5_auth_notsupported(int fd) {
    unsigned char answer[2] = {VERSION5, NOMETHOD};
    writen(fd, (void *) answer, ARRAY_SIZE(answer));
}


/**
 * Performs the authentication (or none) during a connection
 * @param fd the file descriptor of the client connection
 * @param methods_count the number of supported methods informed by the client
 */
int SocksProxyClient::socks5_auth(int fd, int methods_count, int auth_type,
                                  char *arg_username, char *arg_password) {
    int supported = 0;
    int num = methods_count;
    for (int i = 0; i < num; i++) {
        char type;
        readn(fd, (void *) &type, 1);

        LOG(LogLevel::DEBUG, "Method AUTH %hhX", type);

        if (type == auth_type) {
            supported = 1;
        }
    }
    if (supported == 0) {
        socks5_auth_notsupported(fd);
        return -1;
    }
    int ret = 0;
    switch (auth_type) {
        case NOAUTH:
            ret = socks5_auth_noauth(fd);
            break;
        case USERPASS:
            ret = socks5_auth_userpass(fd, arg_username, arg_password);
            break;
    }
    if (ret == 0) {
        return 0;
    } else {
        return -1;
    }
}


/**
 * Given a message received retrieves the SOCKS command
 * @param fd the file descriptor of the client connection
 * @return the SOCKS command
 */
int SocksProxyClient::socks5_command(int fd) {
    char command[4];
    readn(fd, (void *) command, ARRAY_SIZE(command));

    LOG(LogLevel::DEBUG, "Command %hhX %hhX %hhX %hhX", command[0], command[1],
        command[2], command[3]);

    return command[3];
}


/**
 * Retrieves the port number of a request to a target connection
 * @param fd the file descriptor of the client connection
 * @return the port of target host
 */
unsigned short int SocksProxyClient::socks_read_port(int fd) {
    unsigned short int p;
    readn(fd, (void *) &p, sizeof(p));

    LOG(LogLevel::DEBUG, "Port %hu", ntohs(p));

    return p;
}


/**
 * Retrieves the IP address of a request to a target connection
 * @param fd the file descriptor of the client connection
 * @return the IP address of a target host
 */
char *SocksProxyClient::socks_ip_read(int fd) {
    char *ip = (char *) malloc(sizeof(char) * IPSIZE);
    readn(fd, (void *) ip, IPSIZE);

    LOG(LogLevel::DEBUG, "IP %hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);

    return ip;
}


/**
 * Sends a SOCKS5 response
 * @param fd the file descriptor of the client connection
 * @param ip
 * @param port
 */
void SocksProxyClient::socks5_ip_send_response(int fd, char *ip,
                                               unsigned short int port) {
    char response[4] = {VERSION5, OK, RESERVED, IP};
    writen(fd, (void *) response, ARRAY_SIZE(response));
    writen(fd, (void *) ip, IPSIZE);
    writen(fd, (void *) &port, sizeof(port));
}


/**
 * Displays the address of a domain
 * @param fd
 * @param size
 * @return
 */
char *SocksProxyClient::socks5_domain_read(int fd, unsigned char *size) {
    unsigned char s;
    readn(fd, (void *) &s, sizeof(s));
    char *address = (char *) malloc((sizeof(char) * s) + 1);
    readn(fd, (void *) address, (int) s);
    address[s] = 0;

    LOG(LogLevel::DEBUG, "Address %s", address);

    *size = s;
    return address;
}


/**
 * Sends a response to a domain request
 * @param fd
 * @param domain
 * @param size
 * @param port
 */
void SocksProxyClient::socks5_domain_send_response(int fd, char *domain,
                                                   unsigned char size, unsigned short int port) {
    char response[4] = {VERSION5, OK, RESERVED, DOMAIN};
    writen(fd, (void *) response, ARRAY_SIZE(response));
    writen(fd, (void *) &size, sizeof(size));
    writen(fd, (void *) domain, size * sizeof(char));
    writen(fd, (void *) &port, sizeof(port));
}


/**
 * Checks whether the client connection is made using SOCKS version 4 or 5
 * @param ip the content of ip packet
 * @return 1 if client is using SOCKS version 4, 0 if client is using SOCKS
 *        version 5
 */
int SocksProxyClient::socks4_is_4a(char *ip) {
    return (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] != 0);
}


int SocksProxyClient::socks4_read_nstring(int fd, char *buf, int size) {
    char sym = 0;
    int nread = 0;
    int i = 0;

    while (i < size) {
        nread = recv(fd, &sym, sizeof(char), 0);

        if (nread <= 0) {
            break;
        } else {
            buf[i] = sym;
            i++;
        }

        if (sym == 0) {
            break;
        }
    }

    return i;
}


void SocksProxyClient::socks4_send_response(int fd, int status) {
    char resp[8] = {0x00, (char) status, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    writen(fd, (void *) resp, ARRAY_SIZE(resp));
}


int SocksProxyClient::init_socks_session(int net_fd, int auth_type,
                                         char *arg_username, char *arg_password, bool connect) {
    int version = 0;
    int inet_fd = -1;
    char methods = socks_invitation(net_fd, &version);

    switch (version) {
        case VERSION5: {
            socks5_auth(net_fd, methods, auth_type, arg_username, arg_password);
            int command = socks5_command(net_fd);

            if (command == IP) {
                char *ip = socks_ip_read(net_fd);
                unsigned short int p = socks_read_port(net_fd);

                if (connect) {
                    inet_fd = app_connect(LOCAL, (void *) ip, ntohs(p));
                    if (inet_fd == -1) {
                        return -1;
                    }
                } else {
                    inet_fd = 0;
                }
                socks5_ip_send_response(net_fd, ip, p);
                free(ip);
                break;
            } else if (command == DOMAIN) {
                unsigned char size;
                char *address = socks5_domain_read(net_fd, &size);
                unsigned short int p = socks_read_port(net_fd);

                if (connect) {
                    inet_fd = app_connect(LOCAL, (void *) address, ntohs(p));
                    if (inet_fd == -1) {
                        return -1;
                    }
                } else {
                    inet_fd = 0;
                }

                socks5_domain_send_response(net_fd, address, size, p);
                free(address);
                break;
            } else {
                return -1;
            }
        }
        case VERSION4: {
            if (methods == 1) {
                char ident[255];
                unsigned short int p = socks_read_port(net_fd);
                char *ip = socks_ip_read(net_fd);
                socks4_read_nstring(net_fd, ident, sizeof(ident));

                if (socks4_is_4a(ip)) {
                    char domain[255];
                    socks4_read_nstring(net_fd, domain, sizeof(domain));

                    LOG(LogLevel::DEBUG, "Socks4A: ident:%s; domain:%s;", ident, domain);

                    if (connect) {
                        inet_fd = app_connect(LOCAL, (void *) domain, ntohs(p));
                    } else {
                        inet_fd = 0;
                    }

                } else {

                    LOG(LogLevel::DEBUG, "Socks4: connect by ip & port");

                    if (connect) {
                        inet_fd = app_connect(LOCAL, (void *) ip, ntohs(p));
                    } else {
                        inet_fd = 0;
                    }
                }

                if (inet_fd != -1) {
                    socks4_send_response(net_fd, 0x5a);
                } else {
                    socks4_send_response(net_fd, 0x5b);
                    free(ip);
                    return -1;
                }

                free(ip);
            } else {
                LOG(LogLevel::DEBUG, "Unsupported mode");
                return -1;
            }
            break;
        }
    }

    LOG(LogLevel::DEBUG, "Socks proxy session initialized");


    /* Set fd to NON-BLOCKING */
    int flags = fcntl(inet_fd, F_GETFL, 0);
    fcntl(inet_fd, F_SETFL, flags | O_NONBLOCK);

    return inet_fd;
}

int SocksProxyClient::bind_socks_port() {
    fd_set read_fd_set;
    int sock_fd = -1;
    struct sockaddr_in local;

    signal(SIGPIPE, SIG_IGN);

    int optval = 1;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(LogLevel::DEBUG, "Fatal error: socket()");
        exit(EXIT_FAILURE);
    }

    /* Set fd to NON-BLOCKING */
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
                   sizeof(optval)) < 0) {
        LOG(LogLevel::DEBUG, "Fatal error: setsockopt()");
        exit(EXIT_FAILURE);
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(_port);

    if (bind(sock_fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        LOG(LogLevel::DEBUG, "Fatal error: bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, 25) < 0) {
        LOG(LogLevel::DEBUG, "Fatal error: listen()");
        exit(EXIT_FAILURE);
    }

    return sock_fd;
}

void *SocksProxyClient::main_thread() {
    int auth_type = NOAUTH;
    char *arg_username = (char *) "user";
    char *arg_password = (char *) "pass";
    int fd_bridge = -1;
    fd_set read_fd_set;
    int sock_fd, fd_client = -1, max_sock;
    struct sockaddr_in remote;
    socklen_t remotelen;


    LOG(LogLevel::DEBUG, "Starting with authtype %X", auth_type);


    if (auth_type != NOAUTH) {
        LOG(LogLevel::DEBUG, "Username is %s, password is %s", arg_username, arg_password);
    }

    if (!_direct_connect) {
        sock_fd = bind_socks_port();
        remotelen = sizeof(remote);
        memset(&remote, 0, sizeof(remote));
        FD_SET(sock_fd, &_active_fd_set);
    }

    while (true) {

        read_fd_set = _active_fd_set;

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            LOG(LogLevel::DEBUG, "Fatal error: select() - Errno: %d", errno);
            exit(EXIT_FAILURE);
        }

        if (!_direct_connect && FD_ISSET(sock_fd, &read_fd_set)) {

            fd_client = accept(sock_fd, (struct sockaddr *) &remote, &remotelen);
            FD_SET(fd_client, &_active_fd_set);



            if (fd_client < 0) {
                LOG(LogLevel::DEBUG, "Fatal error: accept()");
                exit(EXIT_FAILURE);
            }

            if (_fds.empty()) {
                fd_bridge = init_socks_session(fd_client, auth_type, arg_username,
                                               arg_password);

                init_conn_handler(fd_bridge, fd_client);
            }
            else {
                //restore connection to the local Tor
                fd_bridge = init_socks_session(fd_client, auth_type, arg_username,
                                               arg_password);
                restore_conn_handler(fd_bridge, fd_client);
            }


        }

        for (FdPair *fdp: _fds) {
            fd_client = fdp->get_fd0();
            fd_bridge = fdp->get_fd1();


            if (fd_client != INV_FD && FD_ISSET(fd_client, &read_fd_set)) {
                _controller->handleSocksClientDataReady(fdp);
            }
            if (fd_client != INV_FD && FD_ISSET(fd_bridge, &read_fd_set)) {
                _controller->handleSocksBridgeDataReady(fdp);
            }
        }

        for (FdPair *fd_zombie : _zombies) {
            _controller->handleSocksConnectionTerminated(fd_zombie);
            _fds.erase(fd_zombie);
            close(fd_zombie->get_fd0());
            //close(fd_zombie->get_fd1());
            delete fd_zombie;
        }
        _zombies.clear();

    }

    return NULL;
}

int SocksProxyClient::init_conn_handler(int fd_bridge, int fd_client) {
    if (fd_bridge < 0) {
        _controller->handleSocksNewSessionError();
        return -1;
    } else {

        FdPair *fd_pair = new FdPair(fd_client, fd_bridge);

        _fds.insert(fd_pair);
        if (fd_client != INV_FD)
            FD_SET(fd_client, &_active_fd_set);
        FD_SET(fd_bridge, &_active_fd_set);
        _controller->handleSocksNewConnection(fd_pair);
    }
    return 0;
}

void SocksProxyClient::restore_conn_handler(int fd_bridge, int fd_client) {
    //restore connection to the local Tor
    FdPair *fd = (*_fds.begin());
    assert(fd->get_fd0() == INV_FD);
    fd->set_fd0(fd_client);
    fd->set_fd1(fd_bridge);
    FD_SET(fd_client, &_active_fd_set);
    _controller->handleSocksRestoreConnection(fd);
}


int SocksProxyClient::initialize(ClientController *controller, bool direct_connect,
                                 int fd_bridge, int run_mode) {

    assert(controller != NULL);

    _direct_connect = direct_connect;

    assert(controller != nullptr);
    _controller = controller;

    if (_direct_connect) {
        assert(fd_bridge != INV_FD && fd_bridge > 0);

        /* fd_client points to nowhere, since there are no real Tor proxy client.
            Instead of relying on Tor to trigger the connection to the bridge,
            connect directly instead. */
        init_conn_handler(fd_bridge, INV_FD);
    } else {
        int proxyport = _controller->get_socks_port();
        if (proxyport < 1024 || proxyport > 65534) {
            LOG(LogLevel::ERROR, "Socks Port must be an 1024 - 65534 integer");
            return -1;
        }
        _port = proxyport;
    }

    if (run_mode == RUN_BACKGROUND) {
        std::thread th(&SocksProxyClient::main_thread, this);
        th.detach();
        return 0;
    }

    if (run_mode == RUN_FOREGROUND) {
        main_thread();
        return 0;
    }

    return -1;
}


int SocksProxyClient::readn_msg_client(FdPair *fd_pair, char *buff, int buffsize) {
    assert(buff != NULL && buffsize > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return read(fd_pair->get_fd0(), buff, buffsize);
}


int SocksProxyClient::writen_msg_client(FdPair *fd_pair, char *buff, int size) {
    assert(buff != NULL && size > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return write(fd_pair->get_fd0(), buff, size);
}

int SocksProxyClient::readn_msg_bridge(FdPair *fd_pair, char *buff, int buffsize) {
    assert(buff != NULL && buffsize > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return read(fd_pair->get_fd1(), buff, buffsize);

}

int SocksProxyClient::writen_msg_bridge(FdPair *fd_pair, char *buff, int size) {
    assert(buff != NULL && size > 0);

    if (_fds.find(fd_pair) == _fds.end()) {
        return -1;
    }

    return write(fd_pair->get_fd1(), buff, size);

}

int SocksProxyClient::shutdown_connection(FdPair *fd_pair) {
    assert(_fds.find(fd_pair) != _fds.end());

    LOG(LogLevel::DEBUG, "SocksProxyClient::shutdown_connection");

    if (_zombies.find(fd_pair) == _zombies.end()) {
        if (fd_pair->get_fd0() != INV_FD) {
            shutdown(fd_pair->get_fd0(), SHUT_RDWR);
            FD_CLR(fd_pair->get_fd0(), &_active_fd_set);
            fd_pair->set_fd0(INV_FD);
        }
        /*if (fd_pair->get_fd1() != INV_FD) {
            shutdown(fd_pair->get_fd1(), SHUT_RDWR);
            FD_CLR(fd_pair->get_fd1(), &_active_fd_set);
        }*/

        _zombies.insert(fd_pair);
    }
    return 0;
}

int SocksProxyClient::shutdown_client_connection(FdPair *fd_pair) {
    int status;
    char buffer_r[BUFSIZE];


    assert(_fds.find(fd_pair) != _fds.end());


    if (fd_pair->get_fd0() != INV_FD) {
        /* Check if socket is still connected*/
        int res = recv(fd_pair->get_fd0(), &buffer_r, 1, MSG_PEEK | MSG_DONTWAIT);
        /* If connected, shutdown and close. Else, do nothing*/
        if(res > 0){
            status = shutdown(fd_pair->get_fd0(), SHUT_RDWR);
            LOG(LogLevel::DEBUG, "SocksProxyClient::shutdown_local_connection - Errno: %d", errno);
            assert(status == 0);
        }
        FD_CLR(fd_pair->get_fd0(), &_active_fd_set);

        status = close(fd_pair->get_fd0());
        assert(status == 0);

        fd_pair->set_fd0(INV_FD);
    }

    return 0;
}