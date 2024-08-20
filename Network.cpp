 #include "Network.h"

Network::Network() {

}

void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                      const string &sender_port, const string &receiver_port) {
    // TODO: Execute the commands given as a vector of strings while utilizing the remaining arguments.
    /* Don't use any static variables, assume this method will be called over and over during testing.
     Don't forget to update the necessary member variables after processing each command. For example,
     after the MESSAGE command, the outgoing queue of the sender must have the expected frames ready to send. */

    for (const string &command : commands) {
        istringstream iss(command);
        string commandType;
        iss >> commandType;
        string commandString(command.size()+9,'-');
        cout << commandString << endl;
        cout << "Command :" << command << endl;
        cout << commandString << endl;

        string chunk;

        int hops = 0;

        if (commandType == "MESSAGE") {

            string senderID, receiverID, message;
            string temp;

            iss >> senderID >> receiverID;

            // Read the message content enclosed within '#' symbols
            while (iss >> temp) {
                if (temp.front() == '#') {
                    // Remove the '#' symbol
                    temp.erase(temp.begin());
                    message = temp;
                    while (iss >> temp) {
                        message += " " + temp;
                    }
                    // Remove the '#' symbol at the end
                    message.pop_back();
                    break;
                }
            }


            cout << "Message to be sent: " << message << endl <<endl;

                string receiverMac;

                //int hops = 0;

                for(Client& client : clients){
                    if(client.client_id == senderID){
                        for(auto& client_receiver : clients){
                            if(client_receiver.client_id == receiverID){
                                for(Client &mac_client : clients){
                                    if(client.routing_table[client_receiver.client_id] == mac_client.client_id){
                                        receiverMac= mac_client.client_mac;
                                        break;
                                    }
                                }
                                // Process the message by breaking it into chunks
                                int startPos;
                                int i = 1;
                                int frameNum = (message.length()+message_limit-1)/message_limit;
                                for(int i = 0; i< frameNum; ++i) {
                                    startPos = i* message_limit;
                                    int frameSize = min(message_limit, static_cast<int>(message.length()-startPos) );
                                    chunk = message.substr(startPos,frameSize);
                                    stack<Packet*> packet_stack;
                                    Packet* appPacket = new ApplicationLayerPacket(0,senderID,receiverID,chunk);
                                    Packet* transportPacket = new TransportLayerPacket(1,sender_port,receiver_port);
                                    Packet* networkPacket = new NetworkLayerPacket(2,client.client_ip,client_receiver.client_ip);
                                    Packet* physicalPacket = new PhysicalLayerPacket(3,client.client_mac,receiverMac);

                                    packet_stack.push(appPacket);
                                    packet_stack.push(transportPacket);
                                    packet_stack.push(networkPacket);
                                    packet_stack.push(physicalPacket);

                                    cout << "Frame #" << i+1 <<endl;
                                    cout << "Sender MAC address: " << client.client_mac << ", Receiver MAC address: " << receiverMac << endl;
                                    cout << "Sender IP address: " << client.client_ip << ", Receiver IP address: " << client_receiver.client_ip << endl;
                                    cout << "Sender port number: " << sender_port << ", Receiver port number: " << receiver_port << endl;
                                    cout << "Sender ID: " << senderID << ", Receiver ID: " << receiverID << endl;
                                    cout << "Message chunk carried: \"" << chunk <<"\"" << endl;
                                    cout << "Number of hops so far: " << hops << endl;
                                    cout << "--------" << endl;

                                    client.outgoing_queue.push(packet_stack);

                                    auto now = std::chrono::system_clock::now();
                                    std::time_t now_c = std::chrono::system_clock::to_time_t((now));
                                    std::tm tm_now = *std::localtime(&now_c);
                                    std::stringstream ss;
                                    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
                                    std::string str_time = ss.str();

                                    Log new_log(str_time,message, frameNum,0,senderID,
                                                receiverID,true,ActivityType::MESSAGE_SENT);
                                    client.log_entries.push_back(new_log);


                            }

                        }
                    }

                }




            }
        }
        else if (commandType == "SHOW_FRAME_INFO") {

            // Implement SHOW_FRAME_INFO command logic
            // Access respective client's outgoing queue and display frame information
            // Example: Show frame information for the specified client's outgoing queue


            string clientID, queueSelection;
            int frameNumber;

            iss >> clientID >> queueSelection >> frameNumber;

            // Find the client based on clientID
            Client *selectedClient = nullptr;
            for (Client &client : clients) {
                if (client.client_id == clientID) {
                    selectedClient = &client;
                    break;
                }
            }

            if (selectedClient == nullptr) {
                cout << "Client not found!" << endl;
                continue; // Move to the next command
            }

            // Determine which queue to inspect: incoming or outgoing
            queue<stack<Packet*>> *selectedQueue = nullptr;
            if (queueSelection == "in") {
                selectedQueue = &selectedClient->incoming_queue;
            } else if (queueSelection == "out") {
                selectedQueue = &selectedClient->outgoing_queue;
            } else {
                cout << "Invalid queue selection!" << endl;
                continue; // Move to the next command
            }

            // Check if the frame number is within the queue size
            if (frameNumber < 1 || frameNumber > selectedQueue->size()) {
                cout << "No such frame." << endl;
                continue; // Move to the next command
            }

            // Display frame information

            queue<stack<Packet*>> tempQueue = *selectedQueue; // Create a temporary queue to iterate
            cout << "Current Frame #" << frameNumber << " on the outgoing queue of client " << clientID << endl;

            int currentFrame = 1;
            bool foundFrame = false;

            while (!tempQueue.empty()) {
                stack<Packet*> packetStack = tempQueue.front();
                tempQueue.pop();

                if (currentFrame == frameNumber) {
                    foundFrame = true;

                    if (!packetStack.empty()) {
                        stack<Packet*> tempStack = packetStack;
                        vector<Packet*> packets;

                        while (!tempStack.empty()) {
                            packets.push_back(tempStack.top());
                            tempStack.pop();
                        }

                        for (int i = packets.size() - 1; i >= 0; --i) {

                                // Display only the specific layer's information in a formatted way
                                if (ApplicationLayerPacket* appPacket = dynamic_cast<ApplicationLayerPacket*>(packets[i])) {
                                    cout << "Carried Message: \"" << appPacket->message_data << "\"" << endl;
                                    cout << "Layer 0 info: Sender ID: " << appPacket->sender_ID << ", Receiver ID: " << appPacket->receiver_ID << endl;
                                } else if (TransportLayerPacket* transportPacket = dynamic_cast<TransportLayerPacket*>(packets[i])) {
                                    cout << "Layer 1 info: Sender port number: " << transportPacket->sender_port_number << ", Receiver port number: " << transportPacket->receiver_port_number << endl;
                                } else if (NetworkLayerPacket* networkPacket = dynamic_cast<NetworkLayerPacket*>(packets[i])) {
                                    cout << "Layer 2 info: Sender IP address: " << networkPacket->sender_IP_address << ", Receiver IP address: " << networkPacket->receiver_IP_address << endl;
                                } else if (PhysicalLayerPacket* physicalPacket = dynamic_cast<PhysicalLayerPacket*>(packets[i])) {
                                    cout << "Layer 3 info: Sender MAC address: " << physicalPacket->sender_MAC_address << ", Receiver MAC address: " << physicalPacket->receiver_MAC_address << endl;
                                }
                            }

                        cout << "Number of hops so far: " << hops << endl;
                        break; // Found and displayed the specified frame, so exit the loop
                    }
                }
                currentFrame++;
            }

            if (!foundFrame) {
                cout << "Frame not found!" << endl;
            }

            }
            else if (commandType == "SHOW_Q_INFO") {
            // Implement SHOW_Q_INFO command logic
            // Access respective client's incoming or outgoing queue and display queue information
            // Example: Show queue information for the specified client
            string clientID, queueSelection;
            iss >> clientID >> queueSelection;

            // Find the client based on clientID
            Client *selectedClient = nullptr;
            for (Client &client : clients) {
                if (client.client_id == clientID) {
                    selectedClient = &client;
                    break;
                }
            }

            if (selectedClient == nullptr) {
                cout << "Client not found!" << endl;
                continue; // Move to the next command
            }


            if (queueSelection == "out") {
                cout << "Client " << clientID << " Outgoing Queue Status" << endl;
                cout << "Current total number of frames: " << selectedClient->outgoing_queue.size() << endl;
            } else if (queueSelection == "in") {
                cout << "Client " << clientID << " Incoming Queue Status" << endl;
                cout << "Current total number of frames: " << selectedClient->incoming_queue.size() << endl;
            } else {
                cout << "Invalid queue selection!" << endl;
                continue; // Move to the next command
            }

        } else if (commandType == "SEND") {
            // Implement SEND command logic if needed
            // Example: Trigger a send operation based on the requirements
            for (Client& senderClient : clients) {
                int frameNum = 1;
                // Retrieve the outgoing queue of the sender client
                queue<stack<Packet*>> tempQueue = senderClient.outgoing_queue;

                while (!tempQueue.empty()) {
                    stack<Packet*> packetStack = tempQueue.front();

                    if (!packetStack.empty()) {
                        Packet* physicalPacket = packetStack.top(); // Physical layer packet

                        // Check if the physical packet is not null
                        if (physicalPacket != nullptr) {
                            string receiverMac = dynamic_cast<PhysicalLayerPacket*>(physicalPacket)->receiver_MAC_address;

                            // Create a temporary copy of the packetStack for processing
                            stack<Packet*> tempPacketStack = packetStack;

                            // Find the application layer packet without popping elements
                            Packet* currentPacket = nullptr;
                            stack<Packet*> tempStackCopy = tempPacketStack;
                            while (!tempStackCopy.empty()) {
                                Packet* tempPacket = tempStackCopy.top();
                                if (tempPacket->layer_ID == 0) {
                                    currentPacket = tempPacket;
                                    break;
                                }
                                tempStackCopy.pop();
                            }
                            Packet* currentPacket1 = nullptr;
                            stack<Packet*> tempStackCopy1 = tempPacketStack;
                            while (!tempStackCopy.empty()) {
                                Packet* tempPacket = tempStackCopy1.top();
                                if (tempPacket->layer_ID == 2) {
                                    currentPacket1 = tempPacket;
                                    break;
                                }
                                tempStackCopy1.pop();
                            }
                            // If an application layer packet is found, print the message chunk carried

                            for (Client& receiverClient : clients) {
                                if (receiverClient.client_mac == receiverMac) {
                                    // Forward frame to the next hop (receiver's incoming queue)
                                    receiverClient.incoming_queue.push(tempPacketStack);

                                    // Output the network trace
                                    cout << "Client " << senderClient.client_id << " sending frame #" << frameNum << " to client " << receiverClient.client_id << endl;
                                    cout << "Sender MAC address: " << senderClient.client_mac << ", Receiver MAC address: " << receiverClient.client_mac << endl;

                                    if (currentPacket1 != nullptr) {
                                        NetworkLayerPacket* networkPacket = dynamic_cast<NetworkLayerPacket*>(currentPacket1);

                                        if (networkPacket != nullptr) {
                                            // Successfully casted to NetworkLayerPacket, print IP addresses
                                            cout << "Sender IP address: " << networkPacket->sender_IP_address << ", Receiver IP address: " << networkPacket->receiver_IP_address << endl;

                                        }
                                    }
                                    cout << "Sender port number: " << sender_port << ", Receiver port number: " << receiver_port << endl;
                                    cout << "Sender ID: " << senderClient.client_id << ", Receiver ID: " << receiverClient.client_id << endl;
                                    frameNum ++;
                                    if (currentPacket != nullptr) {
                                        ApplicationLayerPacket* appPacket = dynamic_cast<ApplicationLayerPacket*>(currentPacket);
                                        if (appPacket != nullptr) {
                                            cout << "Message chunk carried: \"" << appPacket->message_data << "\"" << endl;
                                        }
                                    }

                                    cout << "Number of hops so far: " << hops << endl; // Assuming each transmission adds 1 hop
                                    cout << "--------" << endl;


                                    break; // Stop searching for the receiver client
                                }
                            }
                        }
                    }
                    tempQueue.pop();
                }
            }
        }else if (commandType == "RECEIVE") {
            // Implement RECEIVE command logic if needed
            // Example: Trigger a receive operation based on the requirements


          for(Client &receiverClient : clients){

              string new_receiver_mac_address;
                int frameNum = 1;

                bool printedForwardingMessage = false;

                while(!receiverClient.incoming_queue.empty()) {
                    string message;
                    bool clientPresent;


                    stack<Packet *> frame = receiverClient.incoming_queue.front();
                    receiverClient.incoming_queue.pop();

                    stack<Packet *> tempFrame = frame;
                    stack<Packet *> tempFrame1 = frame;

                    auto *physicallyLayerPacket = dynamic_cast<PhysicalLayerPacket *>(frame.top());

                    string receiver_mac = physicallyLayerPacket->receiver_MAC_address;
                    string sender_mac = physicallyLayerPacket->sender_MAC_address;

                    while(tempFrame.size() > 1){

                        tempFrame.pop();


                    }
                    auto *appPacket = dynamic_cast<ApplicationLayerPacket*>(tempFrame.top());
                    string receiver_ID = appPacket -> receiver_ID;

                    string sender_ID = appPacket -> sender_ID;
                    string  chunk = appPacket->message_data;


                    while(tempFrame1.size() > 3){
                        tempFrame1.pop();


                    }

                    auto *networkPacket = dynamic_cast<NetworkLayerPacket*>(tempFrame1.top());

                    string receiver_IP = networkPacket -> receiver_IP_address;

                    string sender_IP = networkPacket -> sender_IP_address;


                    if(receiver_ID == receiverClient.client_id){
                        cout << "Client: " << receiver_ID
                             << " receiving frame # " << frameNum <<" from client " << receiverClient.routing_table[sender_ID]
                             << ", orginating from client " << appPacket->sender_ID << endl;

                        cout << "Sender MAC address: " << physicallyLayerPacket->sender_MAC_address
                        << ", Receiver MAC address: " << physicallyLayerPacket->receiver_MAC_address
                        << endl;

                        cout << "Sender IP address: " << networkPacket->sender_IP_address
                        <<", Receiver IP address: " << networkPacket->receiver_IP_address << endl;

                        cout << "Sender port number: " << sender_port
                             <<", Receiver port number: " << receiver_port << endl;

                        cout << "Sender ID: " << sender_ID
                             <<", Receiver ID: " << receiver_ID << endl;

                        cout << "Message chunk carried: \"" << chunk<< "\"" <<endl;

                        message += " " + chunk;
                        cout << "Number of hops so far: " << hops <<endl;

                        cout << "--------" <<endl;

                        cout << "Client :" << receiver_ID
                             << " received the message \"" << message
                             <<"\" from client " << sender_ID
                             << endl;

                        cout << "--------" <<endl;

                        auto now = std::chrono::system_clock::now();
                        std::time_t now_c = std::chrono::system_clock::to_time_t((now));
                        std::tm tm_now = *std::localtime(&now_c);
                        std::stringstream ss;
                        ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
                        std::string str_time = ss.str();

                        Log new_log(str_time,message, frameNum,0,sender_ID,
                                    receiver_ID,true,ActivityType::MESSAGE_RECEIVED);
                        receiverClient.log_entries.push_back(new_log);

                    }

                    else{
                        if(!printedForwardingMessage) {
                            cout << "Client " << receiverClient.client_id << " receiving a message from client "
                                 << sender_ID << ", but intended for client " << receiver_ID
                                 << ". Forwarding... " << endl;
                            printedForwardingMessage = true;
                        }

                        for(Client &client: clients){
                            if(receiverClient.routing_table[receiver_ID] == client.client_id){
                                new_receiver_mac_address = client.client_mac;
                                clientPresent = true;
                                physicallyLayerPacket->sender_MAC_address = receiver_mac;
                                physicallyLayerPacket->receiver_MAC_address = new_receiver_mac_address;
                                break;
                            }
                        }

                    }

                    if(clientPresent){
                        //forward;

                        cout << "Frame #" << frameNum << " MAC address change: New sender MAC "
                        << receiver_mac << ", new receiver MAC " << new_receiver_mac_address << endl;



                        auto now = std::chrono::system_clock::now();
                        std::time_t now_c = std::chrono::system_clock::to_time_t((now));
                        std::tm tm_now = *std::localtime(&now_c);
                        std::stringstream ss;
                        ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
                        std::string str_time = ss.str();

                        Log new_log(str_time,chunk, frameNum,0,sender_ID,
                                    receiver_ID,true,ActivityType::MESSAGE_FORWARDED);
                        receiverClient.log_entries.push_back(new_log);

                        ++frameNum;

                    }

                    if(!clientPresent){
                        //dropped

                        cout << "Error: Unreachable destination. Packets are dropped after "
                        << hops << " hops!" << endl;


                        auto now = std::chrono::system_clock::now();
                        std::time_t now_c = std::chrono::system_clock::to_time_t((now));
                        std::tm tm_now = *std::localtime(&now_c);
                        std::stringstream ss;
                        ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
                        std::string str_time = ss.str();

                        Log new_log(str_time,chunk, frameNum,0,sender_ID,
                                    receiver_ID,true,ActivityType::MESSAGE_DROPPED);
                        receiverClient.log_entries.push_back(new_log);


                    }

                }
                if (printedForwardingMessage) {
                  // Separate the sequences of frames with --------
                  cout << "--------" << endl;
              }
            }



        } else if (commandType == "PRINT_LOG") {
            // Implement PRINT_LOG command logic
            // Access respective client's log_entries and print log information
            // Example: Print log information for the specified client
            string clientID;
            iss >> clientID;

            // Find the client based on clientID
            Client* selectedClient = nullptr;
            for (Client& client : clients) {
                if (client.client_id == clientID) {
                    selectedClient = &client;
                    break;
                }
            }

            if (selectedClient == nullptr) {
                cout << "Client not found!" << endl;
                continue; // Move to the next command
            }



            if (selectedClient->log_entries.empty()) {
                cout << "No log entries for client " << clientID << endl;
            } else {
                int logEntryNum = 1;
                for (const Log& log : selectedClient->log_entries) {
                    cout << "Log Entry #" << logEntryNum << ":" << endl;

                    if(log.activity_type == ActivityType::MESSAGE_RECEIVED) {
                        cout << "Activity: Message Received" << endl;
                    }
                    else if(log.activity_type == ActivityType::MESSAGE_SENT) {
                        cout << "Activity: Message Sent" << endl;
                    }
                    else if(log.activity_type == ActivityType::MESSAGE_DROPPED) {
                        cout << "Activity: Message Dropped" << endl;
                    }else if(log.activity_type == ActivityType::MESSAGE_FORWARDED) {
                        cout << "Activity: Message Forwarded" << endl;
                    }

                    cout << "Timestamp: " << log.timestamp << endl;
                    cout << "Number of frames: " << log.number_of_frames << endl;
                    cout << "Number of hops: " << log.number_of_hops << endl;
                    cout << "Sender ID: " << log.sender_id << endl;
                    cout << "Receiver ID: " << log.receiver_id << endl;
                    cout << "Success: " << (log.success_status ? "Yes" : "No") << endl;

                    if (log.activity_type == ActivityType::MESSAGE_RECEIVED ||
                        log.activity_type == ActivityType::MESSAGE_SENT) {
                        cout << "Message: \"" << log.message_content << "\"" << endl;
                    }

                    cout << "--------------" << endl;
                    logEntryNum++;
                }
            }

        } else {
            cout << "Invalid command." <<endl;
            continue;
        }
    }

    // Update any necessary member variables after processing each command
    // For example, after processing MESSAGE command, update the outgoing queue
    // Don't forget to update other necessary member variables as required

    //free memory şimdilik burada ama bi kontrol et.

}

vector<Client> Network::read_clients(const string &filename) {
    vector<Client> clients;
    // TODO: Read clients from the given input file and return a vector of Client instances.
    // Open the file
    ifstream file(filename);
    if (file.is_open()) {
        int num_clients = 0;
        file >> num_clients; // Read the number of clients from the first line

        // Read remaining lines to extract id, ip, and mac for each client
        string line;
        getline(file, line); // Read and ignore the rest of the first line

        for (int i = 0; i < num_clients; ++i) {
            getline(file, line); // Read a line from the file
            istringstream iss(line); // Create a string stream to parse the line

            string id, ip, mac;
            iss >> id >> ip >> mac;

            // Create a Client instance using the extracted data and add it to the vector
            Client client(id, ip, mac);
            clients.push_back(client);

        }

        file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }

    //cout << clients[0].client_id;

    return clients;
}

void Network::read_routing_tables(vector<Client> &clients, const string &filename) {
    // TODO: Read the routing tables from the given input file and populate the clients' routing_table member variable.
    ifstream file(filename);

    if (file.is_open()) {
        string line;
        int i = 0;
        while (getline(file, line)) {
            if (line == "-") {
                // Skip the delimiter line and move to the next client's routing table
                i += 1;
                continue;
            }

            // Split the line into receiverID and nexthopID
            istringstream iss(line);
            string receiverID, nexthopID;
            iss >> receiverID >> nexthopID;

            // Update the routing_table for the corresponding client

            clients[i].routing_table[receiverID] = nexthopID;
             // Stop searching once the client is found and updated


        }

        //test case for routing_table
      /*  for (const Client &client : clients) {
            cout << "Client ID: " << client.client_id << endl;
            cout << "Routing Table:" << endl;

            for (const auto &entry : client.routing_table) {
                cout << "Receiver ID: " << entry.first << ", Next Hop ID: " << entry.second << endl;
            }

            cout << "---------------------" << endl;
        }*/

        file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }

}

// Returns a list of token lists for each command
vector<string> Network::read_commands(const string &filename) {
    vector<string> commands;
    // TODO: Read commands from the given input file and return them as a vector of strings.
     ifstream file(filename);

     if (file.is_open()) {
         int num_commands = 0;
         file >> num_commands; // Read the number of commands from the first line

         // Read remaining lines to get the commands
         string line;
         getline(file, line); // Read and ignore the rest of the first line

         for (int i = 0; i < num_commands; ++i) {
             getline(file, line); // Read a line from the file
             commands.push_back(line); // Add the command to the vector
         }

         file.close();
     } else {
         cerr << "Unable to open file: " << filename << endl;
     }

     return commands;

}

 Network::~Network()  {
    // TODO: Free any dynamically allocated memory if necessary.


}
