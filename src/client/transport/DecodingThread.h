#include <mutex>
#include <map>
#include <string>
#include "../common/Common.hpp"
#include "FragmentManager.h"
#include "../common/thread.h"
#include "../common/utils.h"

#ifndef DECODINGTHREAD_H
#define DECODINGTHREAD_H

class DecodingThread : public Thread {

private:
    std::string _mode;
    std::map<int, FragmentManager>* _dataStore;
    int _client_socket;
    //Debug flag
    int debug = 0;


    int readFromPipe(uint8_t* msg, int read_buffer_len, int fd);
    int readNextFramePayloadSize(int fd);
    void updatePacketFragmentsMap(PacketData* data_fragment);
    void gatherPayload(uint8_t* frame_content, int bytes_read);

public:
    DecodingThread(std::string mode, int client_socket);
    void* run();
};


#endif //DECODINGTHREAD_H
