//
//  main.cpp
//  packet_autobahn
//
//  Created by Leon Eckert on 11/03/2016.
//  Copyright © 2016 Leon Eckert. All rights reserved.
//


#include <iostream>
#include <tins/tins.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <ncurses.h>



using namespace Tins;
using namespace std;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////////////[ Get Src ]///////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
string get_src_from_pdu(const PDU &pdu){
    const Dot11Data &dot11Data = pdu.rfind_pdu<Dot11Data>();
    string src = dot11Data.src_addr().to_string();
    return src;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////////[ Get Timestamp ]/////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

string get_timestamp_from_pkt(const Packet& pkt){
//    long timestamp = pkt.timestamp().seconds();
    string ts;
    stringstream strstream;
    strstream << pkt.timestamp().seconds();
    strstream >> ts;
    return ts;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////[ Get Requested Info from Data11 pkt ]//////
////////////////////////////////////////////////
////////////////////////////////////////////////
vector<string> returnFromData11(const Packet& pkt){
    vector<string> output;
    // here is where we do stuff with packets:
    const PDU& pdu = *pkt.pdu();
    
    // if yes, get the rawPDU and do something with it
    try{
        
        string MAC = get_src_from_pdu(pdu);
        output.push_back(MAC);
        string timestamp = get_timestamp_from_pkt(pkt);
        output.push_back(timestamp);
        
    }catch(...){}
    
    return output;
    
}





map<string, pair<int, long>> activeMACs;
vector<string> orderedMACs;
map<string, int> MACcolors;
int current_color = 1;

void wipeData(){
    for(auto& x : activeMACs){
        x.second.first = 0;
    }
}
long currentTime(){
    time_t  timev;
    return time(&timev);
}

bool isActive(const string& MAC, long KillThreshold){
    if (currentTime() - activeMACs[MAC].second >= KillThreshold){
        return false;
    }else{
        return true;
    }
}

void processData(vector<string> data){
    if (data.size() == 2) {
        string MAC = data[0];
        long timestamp = stol( data[1]);
        
        if (activeMACs.find(MAC) == activeMACs.end()) {
//            generate a color here and assign to thing:
            //not found:
            pair <int,long> p (1,timestamp);
            activeMACs[MAC] = p;
            orderedMACs.push_back(MAC);
            MACcolors[MAC] = current_color++;
            if (current_color > 7) {
                current_color = 1;
            }
            
            
            // perhaps also put on ordered array
        }else{
            //found:
            activeMACs[MAC].first += 1;
            activeMACs[MAC].second = timestamp;
        }
    }
}


void printData(){
    string s = ":";
    for(auto x : activeMACs){
        //        cout << x.first << ": ";
        if (s == ":") {
            s = "|";
        }else if (s == "|"){
            s = ":";
        }
        for(int j = 0; j < x.second.first/3; j++){
            //            cout << "|";
            cout << s;
        }
    }
    cout << endl;
}


void printDataNC(){
    start_color();
    

    
    init_pair(1,COLOR_RED, COLOR_BLACK);
    init_pair(2,COLOR_CYAN, COLOR_BLACK);
    init_pair(3,COLOR_YELLOW, COLOR_BLACK);
    
    int a = 1;
    for(auto x : activeMACs){
        if (a == 1) {
            attron(COLOR_PAIR(1));
            a++;
        }else if (a == 2){
            attron(COLOR_PAIR(2));
            a++;
        }else if (a == 3){
            attron(COLOR_PAIR(3));
            a = 1;
        }
        attron(A_BOLD);
        
        
        for(int j = 0; j < x.second.first/3; j++){
            
            printw("|");
            
            
        }
        attroff(COLOR_PAIR(1));
        attroff(A_BOLD);
        refresh();
    }
    printw("\n");
    //    cout << endl;
    
    
}


void printDataNCtest(){

    
    for (auto& x : orderedMACs){
        if (isActive(x, 10)) {
            attron(COLOR_PAIR(MACcolors[x]));
            attron(A_BOLD);
            
            for(int j = 0; j < activeMACs[x].first; j++){
                printw("|");
            }
            attroff(A_BOLD);
            
            if (activeMACs[x].first < 1) {
                printw("|");
            }
            
            attroff(COLOR_PAIR(MACcolors[x]));
            
            refresh();

        }
        
    }
    printw("\n");
    
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void startSniffing(std::string _interface, bool monitorMode){
    
    // configure the sniffer:
    std::string interface = _interface;
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_rfmon(monitorMode);
    Sniffer sniffer(interface, config);
    
    
    
    // now sniff:
    int print_interval = 1; // in seconds
    long timeTrack = currentTime();
    
    try {
        int c = 0;
        while(Packet pkt = sniffer.next_packet()) {
            
            vector<string> pktData = returnFromData11(pkt);
            processData(pktData); //creates map with times and timestamp
            
            //print processed data:
//            for (auto& s : activeMACs){
//                cout << s.first << " || last active: " << setw(3) << currentTime() - s.second.second << " seconds ago" << " || active: " << isActive(s.first, 10) << " || colorpair: " << MACcolors[s.first] << endl;
//            }
//            cout << "---" << endl;
//            for (auto& s : orderedMACs){
//                cout << s << " | ";
//            }
//            cout << endl;
//            cout << "-----------" << endl;
            
            
//
//            
//            
//            if (c > 600){
//                printDataNC();
//                c = 0;
//                wipeData();
//            }
//            c ++;
            
            if (currentTime() - timeTrack > print_interval){
//                printDataNC();
                printDataNCtest();
                wipeData();
                timeTrack = currentTime();
            }

            
        }
    }catch(...){}
    
    

}

int main(){
    //ncurses:
    initscr();
    if(has_colors() == FALSE)
    {	endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    if(can_change_color() == FALSE)
    {	endwin();
        printf("Your terminal does not support change of colors\n");
        exit(1);
    }
    start_color();
    init_pair(1,1, COLOR_BLACK);
    init_pair(2,2, COLOR_BLACK);
    init_pair(3,3, COLOR_BLACK);
    init_pair(4,4, COLOR_BLACK);
    init_pair(5,5, COLOR_BLACK);
    init_pair(6,6, COLOR_BLACK);
    init_pair(7,7, COLOR_BLACK);
    curs_set(0);
    scrollok(stdscr, TRUE);
    
    
    
    startSniffing("en0", true);
    
//    getch();
//    endwin();
//    return 0;
}