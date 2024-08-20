#include "NetworkLayerPacket.h"
#include <iostream>

NetworkLayerPacket::NetworkLayerPacket(int _layer_ID, const string &_sender_IP, const string &_receiver_IP)
        : Packet(_layer_ID) {
    sender_IP_address = _sender_IP;
    receiver_IP_address = _receiver_IP;
}

void NetworkLayerPacket::print() {
    // TODO: Override the virtual print function from Packet class to additionally print layer-specific properties.
    std::cout << "Network Layer Packet Information:" << std::endl;
    std::cout << "Layer ID: " << layer_ID << std::endl;
    std::cout << "Sender IP Address: " << sender_IP_address << std::endl;
    std::cout << "Receiver IP Address: " << receiver_IP_address << std::endl;
}

NetworkLayerPacket::~NetworkLayerPacket() {
    // TODO: Free any dynamically allocated memory if necessary.
}
