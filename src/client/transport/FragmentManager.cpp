#include "FragmentManager.h"


bool compareBySeqNum(const PacketData* a, const PacketData* b){
    return a->get_packet_fragment() < b->get_packet_fragment();
}


void FragmentManager::setTotalFragmentNumber(int total_fragments_to_completion){
    _total_fragments_to_completion = total_fragments_to_completion;
}


bool FragmentManager::isPacketComplete() {
    //Packet is complete when total fragment number has been set and all fragments have been received
    return _total_fragments_to_completion > 0 && _data_fragments.size() == _total_fragments_to_completion;
}


void FragmentManager::initDataStage(PacketData* fragment){
    _data_fragments.push_back(fragment);
    _total_fragments_to_completion = 0;
}


bool FragmentManager::isFragmentPresent(int packet_fragment_number){
    bool found = false;
    for(auto it = _data_fragments.begin(); it != _data_fragments.end(); ++it){
        PacketData* p = *it;
        if(p->get_packet_fragment() == packet_fragment_number)
            found = true;
    }
    return found;
}


void FragmentManager::addFragment(PacketData* data_fragment){
    _data_fragments.push_back(data_fragment);
}


void FragmentManager::printFragment(){
    LOG(LogLevel::DEBUG, "FragmentManager Printing pieces numbers");
    for(auto it = _data_fragments.begin(); it!=_data_fragments.end(); ++it){
        PacketData* p = *it;
        std::cout << p->get_packet_fragment() << std::endl;
    }
}


void FragmentManager::deliverPacket(std::vector<uint8_t>& packet_data){
    //Write packet_data to client socket
    if (write(_client_socket, packet_data.data(), packet_data.size())  < 0){
        LOG(LogLevel::ERROR,"FragmentManager::deliverPacket writen() failed");
    }
}

void FragmentManager::deleteFragments(){
    for(auto it = _data_fragments.begin(); it!=_data_fragments.end(); ++it){
        PacketData* p = *it;
        delete p;
    }
}

void FragmentManager::assemblePacket(){
    //Sort packet fragments
    sort(_data_fragments.begin(), _data_fragments.end(), compareBySeqNum);

    std::vector<uint8_t> packet_data;

    //Build packet_data by concatenating all individual fragment data
    for(auto it = _data_fragments.begin(); it!=_data_fragments.end(); ++it){
        PacketData* p = *it;
        packet_data.insert(packet_data.end(), p->get_packet_data()->data(), p->get_packet_data()->data() + p->get_packet_length());
    }

    //LOG(LogLevel::DEBUG, "[FragmentManager::assemblePacket] Delivering Packet ID: " + _data_fragments[0]->get_packet_id() + " and size: " + packet_data.size());

    deliverPacket(packet_data);
    deleteFragments();
}


FragmentManager::FragmentManager(std::string mode, int client_socket) : _mode(mode), _client_socket(client_socket) {};
