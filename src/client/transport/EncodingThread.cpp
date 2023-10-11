#include "EncodingThread.h"
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <chrono>
#include <cstring>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>


uint32_t pktID = 0;


int EncodingThread::packetHandler(uint8_t *buf, int len) {
    auto time_packet_received = std::chrono::high_resolution_clock::now();

    uint8_t *packet_data = buf;
    int packet_length = len;

    // Library independent code

    //// Place data into vector
    std::vector<uint8_t> packet_data_copy (&packet_data[0], &packet_data[0] + packet_length);


    //Gather extra packet payload
    PacketData* packetData = new PacketData(++pktID, packet_data_copy, static_cast<uint16_t>(packet_length), 1, 1, 0);

    //Write Application packet to pipe
    std::vector<uint8_t> data_to_encode;
    // IP_PACKET_LEN_HEADER
    uint16_t packet_len_header = packetData->get_packet_length();
    uint8_t packet_len_header_bytes[2] = {(uint8_t) (packet_len_header >> 8), (uint8_t) (packet_len_header)};
    data_to_encode.push_back(packet_len_header_bytes[1]);
    data_to_encode.push_back(packet_len_header_bytes[0]);

    // IP_PACKET_ID_HEADER
    uint32_t packet_id_header = packetData->get_packet_id();
    uint8_t packet_id_header_bytes[4] = {(uint8_t) (packet_id_header >> 24), (uint8_t) (packet_id_header >> 16),
                                         (uint8_t) (packet_id_header >> 8), (uint8_t) (packet_id_header)};
    data_to_encode.push_back(packet_id_header_bytes[3]);
    data_to_encode.push_back(packet_id_header_bytes[2]);
    data_to_encode.push_back(packet_id_header_bytes[1]);
    data_to_encode.push_back(packet_id_header_bytes[0]);

    // IP_FRAG_NUM_HEADER
    uint8_t packet_fragment = packetData->get_packet_fragment();
    data_to_encode.push_back(packet_fragment);

    // IP_LAST_FRAG_HEADER
    data_to_encode.push_back(packet_fragment);

    // IP_PACKET_DATA
    data_to_encode.insert(data_to_encode.end(), packetData->get_packet_data()->data(),
                          packetData->get_packet_data()->data() + packetData->get_packet_length());

    int nwritten = write(get_handler_fd(), data_to_encode.data(), data_to_encode.size());
    if(nwritten < 0){
        LOG(LogLevel::DEBUG,"EncodingThread::encoder_pipe Error writing data\n Errno: %d\n File descriptor: %d \n", errno, get_handler_fd());
    }

    LOG(LogLevel::DEBUG, "PacketHandlerThread::packetHandler Wrote %d to encoder pipe with pktID: %d", (nwritten-8), pktID);

    // Frees allocated structure data
    free(packetData);

    return nwritten;
}


void* EncodingThread::run() {
    int nread, nwritten;
    uint8_t buf[BUFSIZE];

    //main cycle
    while(true){
        if((nread = read(_client_socket, buf, BUFSIZE)) && nread >= 0) {
            nwritten = packetHandler(buf, nread);
            if(nwritten < 0)
                exit(1);
        }
    }


}


EncodingThread::EncodingThread(std::string mode, int handler_fd, int client_socket) : _mode(mode), _handler_fd(handler_fd), _client_socket(client_socket){}

int EncodingThread::get_handler_fd() const {
    return _handler_fd;
};

