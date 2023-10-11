#include "Transport.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


int Transport::initialize(std::string mode, int port){

    assert(mode != "");

    _mode = mode;
    _port = port;


    std::thread th(&Transport::start, this);
    th.detach();

    return 0;
}

int Transport::start() {


    int result=0;

    std::string encoder_pipe = "/tmp/encoder_pipe";

    result = mkfifo(encoder_pipe.c_str(), 0600);

    if (result == -1 && errno == EEXIST) {
        LOG(LogLevel::INFO, "Encoder pipe already exists");
    }
    else if (result == -1){
        LOG(LogLevel::ERROR, "Error creating encoder pipe!");
        exit(1);
    }

    //TODO: Remove O_NONBLOCK IN final release
    int encoder_fd = open(encoder_pipe.c_str(), O_WRONLY  );

    if(encoder_fd == -1){
        LOG(LogLevel::ERROR, "Error opening encoder pipe");
        exit(1);
    }

    int MAX_PIPE_SIZE = 1048576;

    result = fcntl(encoder_fd, F_SETPIPE_SZ, MAX_PIPE_SIZE);

    if(result == -1){
        LOG(LogLevel::ERROR, "Error setting encoder pipe size!");
        exit(1);
    }

    LOG(LogLevel::INFO, "Started Encoder Pipe");

    std::string decoder_pipe = "/tmp/decoder_pipe";

    result = mkfifo(decoder_pipe.c_str(), 0600);

    if (result == -1 && errno == EEXIST) {
        LOG(LogLevel::INFO, "Decoder pipe already exists");
    }
    else if (result == -1){
        LOG(LogLevel::ERROR, "Error creating decoder pipe!");
        exit(1);
    }

    LOG(LogLevel::INFO,"Started Decoder Pipe");

    if(_mode == "client"){
        /* In client mode we setup a socket and wait for the SocksProxyClient to connect to us, so we are SocketProxyClient's fd_bridge*/
        _client_socket = listenSocketConnection();
    }else{
        /* In server mode we connect to the SocksProxyServer socket as the client, so we are SocketProxyServer's fd_client*/
        _client_socket = createSocketConnection();
    }


    _encoding_thread = new EncodingThread(_mode, encoder_fd, _client_socket);
    _encoding_thread->start();

    _decoding_thread = new DecodingThread(_mode,_client_socket);
    _decoding_thread->start();


    _encoding_thread->join();
    _decoding_thread->join();


    return 0;
}

int Transport::createSocketConnection(){
    struct addrinfo hints, *res;
    int fd_local;

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const char *local_port = std::to_string(_port).c_str();

    if (getaddrinfo("127.0.0.1", local_port, &hints, &res) != 0) {
        LOG(LogLevel::DEBUG, "Fatal error create socket: getaddrinfo");
        exit(EXIT_FAILURE);
    }

    /* Create the socket */
    fd_local = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd_local == -1) {
        LOG(LogLevel::DEBUG, "Fatal error create socket: socket");
        exit(EXIT_FAILURE);
    }

    /* Connect to the local host */
    if (connect(fd_local, res->ai_addr, res->ai_addrlen) == -1) {
        LOG(LogLevel::DEBUG, "Fatal error create socket: connect");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);


#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
    LOG(LogLevel::DEBUG, "[Server] SocksProxyServer Connection: %d", fd_local);
#endif

    return fd_local;

}

int Transport::listenSocketConnection() {
    int sock_fd, net_fd;
    int optval = 1;
    struct sockaddr_in local, remote;
    socklen_t remotelen;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(1);
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,sizeof(optval)) < 0) {
        exit(1);
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    local.sin_port = htons(_port);

    if (bind(sock_fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        exit(1);
    }

    if (listen(sock_fd, 25) < 0) {
        exit(1);
    }

    remotelen = sizeof(remote);
    memset(&remote, 0, sizeof(remote));

    if ((net_fd = accept(sock_fd, (struct sockaddr *)&remote, &remotelen)) < 0) {
        exit(1);
    }
    LOG(LogLevel::INFO, "[Client] SocksProxyClient Connection: %d", net_fd );

    int one = 1;
    setsockopt(sock_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

    return net_fd;
}
