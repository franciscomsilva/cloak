#include <string>
#include <vector>
#include "PacketData.h"
#include "../common/Common.hpp"


#ifndef PROTOZOA_FRAGMENTMANAGER_H
#define PROTOZOA_FRAGMENTMANAGER_H

class FragmentManager {

private:
    std::vector<PacketData*> _data_fragments;
    int _total_fragments_to_completion;
    std::string _mode;
    int _client_socket;

    void printFragment();
    void deleteFragments();
    void deliverPacket(std::vector<uint8_t>& packet_data);

public:
    FragmentManager(std::string mode, int client_socket);

    bool isPacketComplete();

    void initDataStage(PacketData* fragment);

    bool isFragmentPresent(int packet_fragment_number);

    void addFragment(PacketData* data_fragment);

    void assemblePacket();

    void setTotalFragmentNumber(int total_fragments_to_completion);

};

#endif //PROTOZOA_FRAGMENTMANAGER_H

