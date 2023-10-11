#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>

#include "hooks.h"


std::string decoder_pipe = "/tmp/decoder_pipe";
int decoder_fd = open(decoder_pipe.c_str(), O_WRONLY | O_NONBLOCK);

std::string encoder_pipe = "/tmp/encoder_pipe";
int encoder_fd = open(encoder_pipe.c_str(), O_RDONLY | O_NONBLOCK);

//Queue for holding outbound IP packets
std::deque<std::vector<uint8_t>> staging_queue;

//Queue for holding outbound IP packet fragments
std::deque<std::vector<uint8_t>> staging_queue_fragments;


//Debug round trip time
std::chrono::duration<double, std::milli> rtt_total;
auto rtt_start = std::chrono::high_resolution_clock::now();
auto rtt_end = std::chrono::high_resolution_clock::now();
bool seen_packet = false;


//Debug flag
int debug = 1;

//Limit of queue size following the Size = RTT * BW rule of thumb
unsigned long queue_limit = 50000;

//Length of Protozoa message header
int HEADER_LEN = 8;

//// Set minimum amount of bytes that a frame must have for us to embed data
const int MIN_PAYLOAD_SIZE = 20;

//Packets counter
int global_packets_retrieved_frame = 0;
int global_packets_sent_frame = 0;




void checkPipeError(int err) {
    if (err == EAGAIN || err == EWOULDBLOCK){
        std::cout << "EAGAIN" << std::endl;
    }
    else if (err == EBADF){
        std::cout << "EBADF" << std::endl;
    }
    else if (err == EFAULT){
        std::cout << "EFAULT" << std::endl;
    }
    else if (err == EFBIG){
        std::cout << "EFBIG" << std::endl;
    }
    else if (err == EINTR){
        std::cout << "EINTR" << std::endl;
    }
    else if (err == EINVAL){
        std::cout << "EINVAL" << std::endl;
    }
    else if (err == EIO){
        std::cout << "EIO" << std::endl;
    }
    else if (err == EPIPE){
        std::cout << "EPIPE" << std::endl;
    }else{
        std::cout << err << std::endl;
    }
}


void probePrint(std::string msg) {
    if(debug)
        std::cout << "probe -> " << msg << "!" << std::endl;
}


void printFrameData(uint8_t* frame_buffer, int frame_length){
    //// [DEBUG] Print 1st and last 30 bytes of data written to pipe
    for (int i = 0; i < 30; i++)
        printf("0x%02x ", frame_buffer[i]);

    printf(" ... ");

    for (int i = frame_length - 30; i < frame_length; i++)
        printf("0x%02x ", frame_buffer[i]);
    printf("\n");
}


void parseFrameHeader(uint8_t* frame_buffer){
    //// [DEBUG] Print 1st 30 bytes of data in a frame (hex and binary notation)
    for (int i = 0; i < 30; i++)
        printf("0x%02x ", frame_buffer[i]);
    printf("\n");

    for (int i = 0; i < 30; i++)
        std::cout << std::bitset<8>(frame_buffer[i]) << " ";
    std::cout << std::endl << std::endl;
}

void logChannelUtilization(int frame_length, int dct_partition_size, int payload_used, std::chrono::duration<double, std::milli> duration, std::chrono::duration<double, std::milli> assemble_time){
    std::cout << "[chromium_log_channel_performance]|" << frame_length << "|" << dct_partition_size << "|" << payload_used << "|" << duration.count() << "|" << assemble_time.count() << std::endl;
}

void logPacketsInQueue(int packets_in_queue){
    std::cout << "-------------------------" << std::endl;
    std::cout << "[packet_queue_size]|" << packets_in_queue << std::endl;
}

void logPacketsSentInFrame(int packets_sent_in_frame, std::vector<uint32_t> packet_ids, int frame_type){
    global_packets_sent_frame += packets_sent_in_frame;
    std::cout << "[packets_sent_in_frame]|" << global_packets_sent_frame <<  "|Frame Type: " << frame_type << std::endl;
    std::cout << "[packets_sent_in_frame_ids]|";
    for(uint32_t i : packet_ids) 
        std::cout << i << "|";

    std::cout << std::endl;
}

void logPacketsRetrievedInFrame(int packets_retrived_in_frame, std::vector<uint32_t> packet_ids,int frame_type){
    global_packets_retrieved_frame += packets_retrived_in_frame;
    std::cout << "[packets_retrieved_in_frame]|" << global_packets_retrieved_frame <<  "|Frame Type: " << frame_type <<  std::endl;
    std::cout << "[packets_retrieved_in_frame_ids]|";
    for(uint32_t i : packet_ids) 
        std::cout << i << "|" ;

    std::cout << std::endl;
}

void fillQueue(std::vector<uint8_t> staging_buffer){
    int slice_pos = 0;
    int ptr_pos = 0;
    uint16_t packet_size = ((uint16_t) staging_buffer[ptr_pos + 1] << 8) | staging_buffer[ptr_pos];

    while (packet_size > 0) {
        slice_pos = ptr_pos;
        ptr_pos += packet_size + HEADER_LEN;


        std::vector<uint8_t> pkt{staging_buffer.begin() + slice_pos, staging_buffer.begin() + ptr_pos};
        staging_queue.push_back(pkt);
       

        if(ptr_pos + 1 >= (int) staging_buffer.size()){
            break;
        }
        packet_size = ((uint16_t) staging_buffer[ptr_pos + 1] << 8) | staging_buffer[ptr_pos];
        

    }

}


void readFromPipe(){
    int read_n_bytes = 4096;
    uint8_t read_buffer[read_n_bytes];
    std::vector<uint8_t> staging_buffer;
    int readd = 0;
    int fill = 0;

    std::chrono::duration<double, std::milli> read_time;
    auto start_read = std::chrono::high_resolution_clock::now();
    while(true){
        if(encoder_fd <= 0)
                encoder_fd = open(encoder_pipe.c_str(), O_RDONLY | O_NONBLOCK);

        int nread = read(encoder_fd, &read_buffer, read_n_bytes);
        if(nread > 1) {
            readd = 1;
            staging_buffer.insert(staging_buffer.end(), &read_buffer[0], &read_buffer[0] + nread);
        }
        else if (nread == -1) {
            break; //Pipe is empty
        }
        else if (nread == 0) {
            break; //Error in pipe
        }
    }
    auto end_read = std::chrono::high_resolution_clock::now();
    read_time = end_read - start_read;

    if(staging_buffer.size() > 1 && staging_queue.size() < queue_limit){
        fill = 1;
        fillQueue(staging_buffer);
    }

    if(readd == 1 && fill == 0){
        std::cout << "[hooks::readFromPipe] staging_buffer.size(): " << staging_buffer.size()  << std::endl;
    }
}


int assembleData(uint8_t* frame_buffer, int dct_partition_offset, int max_payload_size, int frame_type) {
    int data_to_encode = 0;
    int packets_encoded = 0; //for debugging purposes
    int fragments_encoded = 0; //for debugging purposes
    std::vector<uint32_t> packet_id; //for debugging purposes


     if((staging_queue.size() > 0 || staging_queue_fragments.size() > 0) && debug) {
         logPacketsInQueue(staging_queue.size());
     }

    while(data_to_encode < max_payload_size){
        if(staging_queue_fragments.size() > 0){ //// If there is a fragment of an IP packet to encode
            std::vector<uint8_t> frag(staging_queue_fragments.front());
            uint16_t packet_size = ((uint16_t) frag.data()[1] << 8) | frag.data()[0];
            
            //// The fragment fits whole
            if(data_to_encode + packet_size + HEADER_LEN <= max_payload_size){
                // IP_LAST_FRAG_HEADER - if fits whole, then it is last fragment
                frag.data()[7] = 1;

                memcpy(&frame_buffer[dct_partition_offset + sizeof(int) + data_to_encode], &frag.data()[0], packet_size + HEADER_LEN);
                packets_encoded += 1; //for debugging purposes
                packet_id.push_back(((uint32_t)frag.data()[5] << 24) | frag.data()[4] << 16 | frag.data()[3] << 8 | frag.data()[2]); //for debugging purposes
                data_to_encode += packet_size + HEADER_LEN;


                staging_queue_fragments.pop_front();
                fragments_encoded += 1; //for debugging purposes

            }//// The fragment must be further fragmented but we can't even fit a header
            else if(data_to_encode + HEADER_LEN > max_payload_size){
                std::cout << "CANT FIT A HEADER" << std::endl;
                break;            
            }
            else{ //// The fragment will be further fragmented
                uint16_t packet_len_header = max_payload_size - data_to_encode - HEADER_LEN;
                //// Update fragment fields
                // IP_PACKET_LEN_HEADER
                uint8_t packet_len_header_bytes[2] = {(uint8_t) (packet_len_header >> 8), (uint8_t) (packet_len_header) };
                frag.data()[0] = packet_len_header_bytes[1];
                frag.data()[1] = packet_len_header_bytes[0];
                
                
                //// IP_LAST_FRAG_HEADER
                frag.data()[7] = 0;
                packet_id.push_back(((uint32_t)frag.data()[5] << 24) | frag.data()[4] << 16 | frag.data()[3] << 8 | frag.data()[2]);//for debugging purposes

                
                //// Write fragment to frame
                memcpy(&frame_buffer[dct_partition_offset + sizeof(int) + data_to_encode], &frag.data()[0], packet_len_header + HEADER_LEN);
                data_to_encode += packet_len_header + HEADER_LEN;
                packets_encoded += 1; //for debugging purposes


                //// Prepare resulting fragment
                std::vector<uint8_t> new_frag;
                
                // IP_PACKET_LEN_HEADER - update new size
                uint16_t new_frag_size = packet_size - packet_len_header;
                uint8_t new_frag_size_bytes[2] = {(uint8_t) (new_frag_size >> 8), (uint8_t) (new_frag_size) };
                new_frag.push_back(new_frag_size_bytes[1]);
                new_frag.push_back(new_frag_size_bytes[0]);
                
                // IP_PACKET_ID_HEADER - keep id
                new_frag.push_back(frag.data()[2]);
                new_frag.push_back(frag.data()[3]);
                new_frag.push_back(frag.data()[4]);
                new_frag.push_back(frag.data()[5]);
                
                // IP_FRAG_NUM_HEADER - increase frag number
                new_frag.push_back(frag.data()[6] + 1);
                
                // IP_LAST_FRAG_HEADER - keep last frag to 0
                new_frag.push_back(0);
                
                //Insert data
                new_frag.insert(new_frag.end(), &frag.data()[HEADER_LEN + packet_len_header], &frag.data()[HEADER_LEN + packet_size]);
                staging_queue_fragments.push_back(new_frag);
                staging_queue_fragments.pop_front();
                break;
            }
        }
        else if(staging_queue.size() > 0){ //// If there is an IP packet to encode

            std::vector<uint8_t> packet(staging_queue.front());
           
            uint16_t packet_size = ((uint16_t) packet.data()[1] << 8) | packet.data()[0];
            
            if(data_to_encode + packet_size + HEADER_LEN <= max_payload_size){ // The packet fits whole
                packet_id.push_back(((uint32_t)packet.data()[5] << 24) | packet.data()[4] << 16 | packet.data()[3] << 8 | packet.data()[2]);//for debugging purposes

                memcpy(&frame_buffer[dct_partition_offset + sizeof(int) + data_to_encode], &packet.data()[0], packet_size + HEADER_LEN);
                data_to_encode += packet_size + HEADER_LEN;
                staging_queue.pop_front();
                packets_encoded += 1; //for debugging purposes
            }
            else if(data_to_encode + HEADER_LEN > max_payload_size){ //// The packet must be further fragmented but we can't even fit a header
                staging_queue_fragments.push_back(packet);
                staging_queue.pop_front();
                std::cout << "CANT FIT A HEADER" << std::endl;
                break;            
            }
            else{ //// The packet will be fragmented
                uint16_t packet_len_header = max_payload_size - data_to_encode - HEADER_LEN;
                
                //// Update fragment fields
                // IP_PACKET_LEN_HEADER
                uint8_t packet_len_header_bytes[2] = {(uint8_t) (packet_len_header >> 8), (uint8_t) (packet_len_header) };
                packet.data()[0] = packet_len_header_bytes[1];
                packet.data()[1] = packet_len_header_bytes[0];
                
                //// IP_LAST_FRAG_HEADER
                packet.data()[7] = 0;

                packet_id.push_back(((uint32_t)packet.data()[5] << 24) | packet.data()[4] << 16 | packet.data()[3] << 8 | packet.data()[2]);//for debugging purposes

                //// Write fragment to frame
                memcpy(&frame_buffer[dct_partition_offset + sizeof(int) + data_to_encode], &packet.data()[0], packet_len_header + HEADER_LEN);
                data_to_encode += packet_len_header + HEADER_LEN;
                packets_encoded += 1; //for debugging purposes


                //// Prepare resulting fragment
                std::vector<uint8_t> new_frag;
                
                // IP_PACKET_LEN_HEADER - update new size
                uint16_t new_frag_size = packet_size - packet_len_header;
                uint8_t new_frag_size_bytes[2] = {(uint8_t) (new_frag_size >> 8), (uint8_t) (new_frag_size) };
                new_frag.push_back(new_frag_size_bytes[1]);
                new_frag.push_back(new_frag_size_bytes[0]);
                
                // IP_PACKET_ID_HEADER - keep id
                new_frag.push_back(packet.data()[2]);
                new_frag.push_back(packet.data()[3]);
                new_frag.push_back(packet.data()[4]);
                new_frag.push_back(packet.data()[5]);
                
                // IP_FRAG_NUM_HEADER - increase frag number
                new_frag.push_back(packet.data()[6] + 1);
                
                // IP_LAST_FRAG_HEADER - keep last frag to 0
                new_frag.push_back(0);
                
                //Insert data
                new_frag.insert(new_frag.end(), &packet.data()[HEADER_LEN + packet_len_header], &packet.data()[HEADER_LEN + packet_size]);
                staging_queue_fragments.push_back(new_frag);
                staging_queue.pop_front();
                break;
            }
        } else{
            break;
        } // There is still space to encode data, but no more packets
            
    }

    if((packets_encoded <= 0 || fragments_encoded <= 0) && debug){
        probePrint("[hooks::assembleData] No packets or fragments encoded");
    }

    if(packets_encoded >= 1 && debug){
        logPacketsSentInFrame(packets_encoded, packet_id, frame_type);
    }
    return data_to_encode;
}   


void encodeDataIntoFrame(uint8_t* frame_buffer, int frame_length, int dct_partition_offset, int frame_type) {

    ////Open FIFO for advertising current payload length (P2_len - 4 bytes due to payload header -2 bytes due to terminator)
    int dct_partition_size = frame_length - dct_partition_offset;
    int useful_dct_partition_size = dct_partition_size - sizeof(int); //// 4 bytes are used for header aka the memcpy function below where we encode the number of bytes
    int max_payload_size = useful_dct_partition_size - 2; //// 2 bytes for packet data terminator. Client must respect it
    int data_to_encode = 0;



    if(dct_partition_size >= MIN_PAYLOAD_SIZE) {
    
        ////Encode number of bytes in DCT partition (that the reader must read)
        memcpy(&frame_buffer[dct_partition_offset], &useful_dct_partition_size, sizeof(int));
        
        data_to_encode = assembleData(frame_buffer, dct_partition_offset, max_payload_size, frame_type);

        //Encode message terminator if anything was written, otherwise leave frame intact
        memset(&frame_buffer[dct_partition_offset + sizeof(int) + data_to_encode], 0, 2);

    }
}


void retrieveDataFromFrame(uint8_t* frame_buffer, int frame_length, int dct_partition_offset, int frame_type) {

    int dct_partition_size = frame_length - dct_partition_offset;
    int total_written_bytes = dct_partition_offset;




    if(dct_partition_size >= MIN_PAYLOAD_SIZE) {

        if(debug) {

            int ptr_pos = 0;
            int packets_encoded = 0; //for debugging purposes
            std::vector <uint32_t> packet_ids; //for debugging purposes



            // Packet length header represented in 2 bytes
            // Build the first packet len by concatenation to unsigned short
            uint16_t packet_size = ((uint16_t) frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 1] << 8) |
                                   frame_buffer[dct_partition_offset + ptr_pos + sizeof(int)];

            if (packet_size > 0) {
                packets_encoded = 1;
            }


            //Packet splitting loop
            while (packet_size > 0) {
                packet_ids.push_back(
                        ((uint32_t) frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 5] << 24) |
                        frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 4] << 16 |
                        frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 3] << 8 |
                        frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 2]);

                ptr_pos += packet_size + HEADER_LEN;

                packet_size = ((uint16_t) frame_buffer[dct_partition_offset + ptr_pos + sizeof(int) + 1] << 8) |
                              frame_buffer[dct_partition_offset + ptr_pos + sizeof(int)];


                if (packet_size > 0)
                    packets_encoded++;
            }

            if (packets_encoded >= 1 && debug) {
                logPacketsRetrievedInFrame(packets_encoded, packet_ids, frame_type);
            }
        }

        if(decoder_fd <= 0) {
            decoder_fd = open(decoder_pipe.c_str(), O_WRONLY | O_NONBLOCK);
            if(decoder_fd < 0 ){
                printf("Pipe Error Open - fd: %d Errno: ", decoder_fd);
                checkPipeError(errno);
                printf("\n");
            }

        }

        ////Write decoded data
        while(total_written_bytes < frame_length){
            int nwritten = write(decoder_fd, &frame_buffer[total_written_bytes], frame_length - total_written_bytes);

            if(nwritten == -1) {
                printf("Pipe Error Write - fd: %d Errno: ", decoder_fd);
                checkPipeError(errno);
                break;
            }
            if(nwritten > 0)
                total_written_bytes += nwritten;

        }

    }
}

void printEncodedImageInfo(uint32_t encodedWidth,
                           uint32_t encodedHeight,
                           size_t length,
                           size_t size,
                           bool completeFrame,
                           int qp,
                           int frameType,
                           uint8_t* buffer,
                           webrtc::VideoCodecType codec,
                           bool printBuffer) {

    std::cout << "[Encoded Image Structure]" << std::endl;
    std::cout << "Width: " << std::to_string(encodedWidth) << std::endl;
    std::cout << "Height: " << std::to_string(encodedHeight) << std::endl;
    std::cout << "Length: " << length << std::endl;
    std::cout << "Size: " << size << std::endl;
    std::cout << "Quantizer Value: " << qp << std::endl;
    std::cout << "Is complete Frame: " << completeFrame << std::endl;
    std::cout << "Video Codec: " << codec << std::endl;


    switch (frameType) {
        case 0:
            std::cout << "Frame Type: EmptyFrame" << std::endl;
            break;
        case 1:
            std::cout << "Frame Type: AudioFrameSpeech" << std::endl;
            break;
        case 2:
            std::cout << "Frame Type: AudioFrameCN" << std::endl;
            break;
        case 3:
            std::cout << "Frame Type: VideoFrameKey" << std::endl;
            break;
        case 4:
            std::cout << "Frame Type: VideoFrameDelta" << std::endl;
            break;
    }

    if(printBuffer){
        for (unsigned long i = 0; i < length; i++)
            printf("0x%02x ", buffer[i]);
        printf("\n");
    }
    std::cout << std::endl;
}

// void printResolutionDebugInfo(std::string prefix, std::map<std::string, int> resolution_counter, std::map<std::string, DebugTorcloak::ResolutionChange> resolution_change_tracker) {
//     std::cout << prefix << '\n';
//     std::cout << "\t[debug_resolution_counter]" << '\n';
//     for (const auto& [key, value] : resolution_counter) {
//         std::cout << '[' << key << "] = " << value << ";";
//     }
//     std::cout << '\n';

//     std::cout << "\t[debug_resolution_change]" << '\n';
//     for (const auto& [key, value] : resolution_change_tracker) {
//         std::cout << '[' << key << "] = " << value.counter << ";";
//     }
//     std::cout << '\n';
// }


void printEncodedImageInfo(uint32_t encodedWidth,
                           uint32_t encodedHeight,
                           size_t length,
                           size_t size,
                           bool completeFrame,
                           int qp,
                           int frameType,
                           uint8_t* buffer,
                           bool printBuffer) {

    std::cout << "[Encoded Image Structure]" << std::endl;
    std::cout << "Width: " << encodedWidth << std::endl;
    std::cout << "Height: " << encodedHeight << std::endl;
    std::cout << "Length: " << length << std::endl;
    std::cout << "Size: " << size << std::endl;
    std::cout << "Quantizer Value: " << qp << std::endl;
    std::cout << "Is complete Frame: " << completeFrame << std::endl;


    switch (frameType) {
        case 0:
            std::cout << "Frame Type: EmptyFrame" << std::endl;
            break;
        case 1:
            std::cout << "Frame Type: AudioFrameSpeech" << std::endl;
            break;
        case 2:
            std::cout << "Frame Type: AudioFrameCN" << std::endl;
            break;
        case 3:
            std::cout << "Frame Type: VideoFrameKey" << std::endl;
            break;
        case 4:
            std::cout << "Frame Type: VideoFrameDelta" << std::endl;
            break;
    }

    if(printBuffer){
        for (unsigned long i = 0; i < length; i++)
            printf("0x%02x ", buffer[i]);
        printf("\n");
    }
    std::cout << std::endl;
}




