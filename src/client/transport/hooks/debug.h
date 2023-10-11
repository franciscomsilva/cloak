#ifndef DEBUG_TORCLOAK_H_
#define DEBUG_TORCLOAK_H_

#include <cstdint>
#include <string.h>
#include <string>
#include <mutex>
#include <map>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <thread>

class DebugTorcloak {
    public:
        // Structs

        struct ResolutionChange {
            std::string origin;
            std::string destination;
            int counter = 0;
        };

        struct SsrcChange {
            uint32_t origin;
            uint32_t destination;
            int counter = 0;
        };
        // Variables

         std::map<std::string, int> resolution_counter;
         std::string last_resolution;
         std::map<std::string, ResolutionChange> resolution_change_tracker;

         // Frames
         std::atomic<uint32_t> global_frame_receive_counter = 0;
         std::atomic<uint32_t> global_frame_send_counter = 0;
         std::atomic<uint32_t> global_dropped_frames_congestion_window = 0;
         std::atomic<uint32_t> global_dropped_frames_media_optimization = 0;
         std::atomic<uint32_t> global_dropped_frames_by_encoder = 0;
         std::atomic<uint32_t> global_dropped_frames_by_source = 0;
         std::atomic<uint32_t> global_dropped_frames_size = 0;
         std::atomic<uint32_t> global_dropped_frames_by_encoder_queue = 0;
         std::atomic<uint32_t> global_dropped_frames_by_decoder = 0;

         // Packets
         std::atomic<uint32_t>  global_packets_sent = 0;
         std::atomic<uint32_t>  global_packets_received = 0;



    std::map<uint32_t, int>  ssrc_counter;
        std::map<uint32_t, SsrcChange> ssrc_change_tracker;
        uint32_t last_ssrc= 0;


        // Functions

        void printResolutionDebugInfo(std::string prefix){
            // resolution
            std::cout << prefix << '\n';
            std::cout << "\t[debug_resolution_counter]" << '\n';
            for (const auto& [key, value] : resolution_counter) {
                std::cout << '[' << key << "] = " << value << ";";
            }
            std::cout << '\n';

            std::cout << "\t[debug_resolution_change]" << '\n';
            for (const auto& [key, value] : resolution_change_tracker) {
                std::cout << '[' << key << "] = " << value.counter << ";";
            }
            std::cout << "\n\n";

            //ssrc
            std::cout << "\t[debug_ssrc_counter]" << '\n';
            for (const auto& [key, value] : ssrc_counter) {
                std::cout << "\t\t[" << key << "] = " << value << ";";
            }
            std::cout << '\n';

            std::cout << "\t[debug_ssrc_change]" << '\n';
            for (const auto& [key, value] : ssrc_change_tracker) {
                std::cout << "\t\t[" << key << "] = " << value.counter << ";";
            }
            std::cout << "\n\n";

            //frame counter
            std::cout << "\t[debug_frame_counter_sender]: " << global_frame_send_counter << '\n';

            std::cout << "\t[debug_frame_counter_receiver]: " << global_frame_receive_counter << '\n';

        }

        std::string toString(int number){
            return std::to_string(number);
        }


        DebugTorcloak(){}
        ~DebugTorcloak() {}

};
#endif //DEBUG_TORCLOAK_H_
