#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress(){
    return my_address;
}

//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++)
    {
        if (network[i] != NULL)
        {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else               {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else               {cout << "\n"; outgoing->display(); }
    
}

void laptop::consumeDatagram(){
    incoming = NULL;
}

int laptop::canReceiveDatagram(){
    if (incoming == NULL) return 1;
    return 0;
}

/**************new*************/
laptop::~laptop()
{
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}
/**************new*************/

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}


int server::myType() {
    return SERVER;
}

int server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void server::transferDatagram(){
    
    datagram* d = dlist->returnFront();
    
    if (d == NULL) return;
    
    msg_list* m = new msg_list;
    IPAddress own_ip = myAddress();
    
    extern node* network[MAX_MACHINES];


    
    
    
    
    //While the dlist isn't empty
    while(d != NULL){
        //get the destination of the datagram
        IPAddress dst = d->destinationAddress();
        
        //If the datagram destination is in the servers LAN
        if (dst.firstOctad() == own_ip.firstOctad()){
            
            //If there are no connected laptops, we can't send the datagram anywhere
            if (number_of_laptops == 0){
                m->append(d);
                break;
            };
            
            int not_connected = 0;
            
            //Check if the destination laptop connected to the server
            for (int i = 0; i < number_of_laptops; i++){
                
                //Yes, the destination laptop is connected
                if (dst.sameAddress(laptop_list[i])){
                    
                    //Find the actual laptop in the network
                    for (int j = 0; j <  MAX_MACHINES; j++){
                        if (network[j] != NULL){
                            if (network[j]->amIThisComputer(dst)){
                                //If it can reveive the datagram
                                if (((laptop*)network[j])->canReceiveDatagram()){
                                    
                                    //do it
                                    network[j]->receiveDatagram(d);
                                }
                                //else, append the temp msg_list
                                else m->append(d);
                            }
                        }
                    }
                }else not_connected++;
            }
            if (not_connected == number_of_laptops) m->append(d);
        }
        else{
        
            //datagram not in LAN, send to a WAN
            
            //For WAN's: send to closest WANT?
            
            int dif = 999;
            int wan_index = -1;
            if (number_of_wans == 0){
                m->append(d);
                break;
            }
            
            for (int i = 0; i < number_of_wans; i++){
                
                //Find which WAN to send the datagram too.
                for (int j = 0; j < MAX_MACHINES; j++){
                    if (network[j] != NULL){
                        //found a WAN
                        if (network[j]->amIThisComputer(WAN_list[i])){
                            
                            //if dif > new dif, store dif and location in network
                            if (dif > abs(WAN_list[i].firstOctad() - dst.firstOctad())){
                                dif = abs(WAN_list[i].firstOctad() - dst.firstOctad());
                                wan_index = j;
                            }
                        }
                    }
                
                }
            }
            network[wan_index]->receiveDatagram(d);
    
        }
        //get next item in dlist
        d = dlist->returnFront();
    }
    dlist = m;
}


/**************new*************/
server::~server()
{
    dlist->deleteList();
}
/**************new*************/

//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

/**************new*************/
WAN::~WAN()
{
    dlist->deleteList();
}
/**************new*************/

void WAN::transferDatagram(){
    msg_list* m = new msg_list;
    
    datagram* d = dlist->returnFront();
    if (d == NULL) return;
    
    IPAddress own_ip = myAddress();
    
    bool connected = false;
    
    extern node* network[MAX_MACHINES];
    
    
    //while the dlist isn't empty
    while (d != NULL){
        IPAddress dst = d->destinationAddress();

        //Check if the server the datagram wants to go to is connected to the WAN
        for (int k = 0; k < number_of_servers; k++){
            //if the WAN is connected to the server the datagram wants to go to
            if (dst.firstOctad() == server_list[k].firstOctad()){
                
                for (int j = 0; j < MAX_MACHINES; j++){
                    if (network[j] != NULL){
                        if (network[j]->myAddress().sameAddress(server_list[k])){
                            network[j]->receiveDatagram(d);
                            connected = true;
                        }
                    }
                }
            }
        }
        
        //Server not connected to the WAN
        if (!connected){
            if (number_of_wans == 0){
                m->append(d);
                break;
            }
            
            int dif = 999;
            int wan_index = -1;
            
            //For the Wan
            for (int i = 0; i < number_of_wans; i++){
                //For the network
                for (int j = 0; j < MAX_MACHINES; j++){
                    //In this is the WAN in the network
                    if (network[j] != NULL){
                        if (network[j]->myAddress().sameAddress(WAN_list[i])){
                            if (dif > abs(WAN_list[i].firstOctad() - dst.firstOctad())){
                                dif = abs(WAN_list[i].firstOctad() - dst.firstOctad());
                                wan_index = j;
                            }
                        }
                    }
                }
            }

        if (number_of_wans != 0) network[wan_index]->receiveDatagram(d);
            
        }
        d = dlist->returnFront();
    }
    dlist = m;
}



















