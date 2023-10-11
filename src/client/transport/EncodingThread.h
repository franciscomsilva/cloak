#include <string>
#include "../common/Common.hpp"
#include "PacketData.h"
#include "../common/thread.h"
#include "../common/wqueue.h"

#ifndef ENCODINGTHREAD_H
#define ENCODINGTHREAD_H


class EncodingThread : public Thread {

private:
    int _handler_fd;
    int _client_socket;

public:
    std::string _mode;
    int get_handler_fd() const;
    int packetHandler(uint8_t *buf, int len);




public:
    EncodingThread(std::string mode, int handler_fd, int client_socket);
    void* run();
};

#endif //ENCODINGTHREAD_H
