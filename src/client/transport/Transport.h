#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <thread>
#include <assert.h>
#include <stdarg.h>

#include "EncodingThread.h"
#include "DecodingThread.h"
#include "../common/Common.hpp"

#ifndef __main_h__
#define __main_h__


class Transport {
  private:
    EncodingThread* _encoding_thread;
    DecodingThread* _decoding_thread;
    int _client_socket;

    int _port = -1;
    std::string _mode = "";
    int listenSocketConnection();
    int createSocketConnection();
    int start();


public:
    Transport(){};
    int initialize(std::string mode, int port);


};


#endif
