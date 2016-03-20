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
#include <math.h>



using namespace Tins;
using namespace std;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////////////[ Get Dst ]///////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
string get_dst_from_pdu(const PDU &pdu){
    const Dot11Data &dot11Data = pdu.rfind_pdu<Dot11Data>();
    string dst = dot11Data.dst_addr().to_string();
    return dst;
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
        
        string MAC = get_dst_from_pdu(pdu);
        output.push_back(MAC);
        string timestamp = get_timestamp_from_pkt(pkt);
        output.push_back(timestamp);
        
    }catch(...){}
    
    return output;
    
}





map<string, pair<int, long>> activeMACs;
vector<string> orderedMACs;
map<string, int> MACcolors;
int numpackets = 0;
int current_color = 1;
int activeThreshold = 10;
WINDOW *header_box;
WINDOW *graph_box;
WINDOW *footer_box;
int columns_for_header = 0;

void wipeData(){
    for(auto& x : activeMACs){
        x.second.first = 0;
    }
    numpackets = 0;
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
int getNumOfActive(){
    int n = 0;
    for (auto& x : orderedMACs) {
        if (isActive(x, activeThreshold)) {
            n++;
        }
    }
    return n;
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
            if (current_color > 6) {
                current_color = 1;
            }
            
            // perhaps also put on ordered array
        }else{
            //found:
            activeMACs[MAC].first += 1;
            activeMACs[MAC].second = timestamp;
        }
        
        numpackets ++;
    }
}

int resolution = 1;


void drawHeader(){
    wclear(header_box);
    
    wborder(header_box, 1, 1, 1, 0, 1, 1, 1, 1);
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    int MACsPerLine = floor(window_x/20);
    int MACcount = 0;
    wprintw(header_box, "Active Devices (last %d seconds):\n", activeThreshold);
    for (auto& x : orderedMACs){
        if (isActive(x, activeThreshold)) {
            wattron(header_box, COLOR_PAIR(MACcolors[x]));
            wprintw(header_box, x.c_str());
            wattroff(header_box, COLOR_PAIR(MACcolors[x]));
            
            wprintw(header_box, "   ");
            MACcount++;
            if (MACcount > MACsPerLine-1) {
                MACcount = 0;
                wprintw(header_box, "\n");
            }
            
        }
    }
    mvwprintw(header_box, columns_for_header - 2, 0, "Resolution: %d   (change with arrow keys)", resolution);
//    mvwprintw(header_box, columns_for_header - 2, 0, "\n\nResolution: \"|\" symbol represents %d packets. (change with arrow keys)", resolution);
    
    wrefresh(header_box);
}

bool first_Graph = true;

void drawGraph(){
    if (first_Graph) {
        init_pair(7, COLOR_BLACK, COLOR_WHITE);
        wattron(graph_box, COLOR_PAIR(7));
        wprintw(graph_box, "\n\n\n\n\n\n\n\n\n\n\n                                    \n");
//        wprintw(graph_box, "\n\n\n\n\n\n\n\n\n\n\n----------------------------------\n");
        wprintw(graph_box, " Wilkommen auf der Packet Autobahn! \n");
        wprintw(graph_box, "                                    \n\n");
        wattroff(graph_box, COLOR_PAIR(7));
        wprintw(graph_box, "A little explanation:");
        wprintw(graph_box, "\n\n\"Resolution\" defines how many packets are represented by a \"|\" symbol.");
        wprintw(graph_box, "\n\nThe Graph is updated every second and visualises data packets as they \nare received by individual devices on the Wifi channel you are curretnly on.");
        
        wprintw(graph_box, "\n\n\n\n\n\n\n\n\n\n");
        first_Graph = false;
    }
    for (auto& x : orderedMACs){
        if (isActive(x, activeThreshold)) {
            wattron(graph_box, COLOR_PAIR(MACcolors[x]));
            wattron(graph_box, A_BOLD);
            
            int c = 0;
            for(int j = 0; j < activeMACs[x].first; j++){
                c++;
                if (c >= resolution) {
                    c = 0;
                    wprintw(graph_box, "|");
                }
            }
            wattroff(graph_box, A_BOLD);
            wattroff(graph_box, COLOR_PAIR(MACcolors[x]));
        }
        
    }
    wprintw(graph_box, "\n");
    wrefresh(graph_box);
}

void resize_windows(){
    
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    
    columns_for_header = 0;
    int numOf20sNeeded = getNumOfActive();
    while (numOf20sNeeded > 0) {
        columns_for_header++;
        numOf20sNeeded = numOf20sNeeded - floor(window_x/20);
    }
    columns_for_header += 4;
    
    wclear(header_box);
    wrefresh(header_box);
    wclear(footer_box);
    wrefresh(footer_box);
    
    delwin(header_box);
    header_box = newwin(columns_for_header, window_x, 0, 0);
//    wborder(header_box, 1, 1, 1, 0, 1, 1, 1, 1);
    wrefresh(header_box);
    
    wborder(footer_box, 1, 1, 0, 1, 1, 1, 1, 1);
    mvwprintw(footer_box,1,( window_x - 25 )/2, "project by leoneckert.com");
    wrefresh(footer_box);		/* Show that box 		*/
    
    
}


void printDataNCtest(){
    resize_windows();
    drawGraph();
    drawHeader();
    
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
int kbhit()
{
  
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

void startSniffing(std::string _interface, bool monitorMode){
    
    // configure the sniffer:
    std::string interface = _interface;
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_rfmon(monitorMode);
    Sniffer sniffer(interface, config);
    
    //timeing:
    int print_interval = 1; // in seconds
    long timeTrack = currentTime();
    
    
    // now sniff:
    try {
        while(Packet pkt = sniffer.next_packet()) {
            
            vector<string> pktData = returnFromData11(pkt);
            processData(pktData); //creates map with times and timestamp
            

            
            

            if (kbhit()) {
                int g = getch();
                if(g == 65){
                    resolution += 10;
                    wprintw(graph_box, "new resolution: %d\n", resolution);
                    wrefresh(graph_box);
                }else if(g == 66){
                    resolution -= 10;
                    if (resolution < 11) resolution = 1;
                    wprintw(graph_box, "new resolution: %d\n", resolution);
                    wrefresh(graph_box);
                }
            }else{
                if (currentTime() - timeTrack > print_interval){
                    printDataNCtest();
                    wipeData();
                    timeTrack = currentTime();
                }
            }
            
        }
    }catch(...){}
    
    

}

int main(){
    //ncurses:
    initscr();
    
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    
    if(has_colors() == FALSE)
    {	endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    if(can_change_color() == FALSE)
    {	endwin();
        printf("Your terminal does not support change of colors\n");
        exit(1);
    } start_color();
    init_pair(1,1, COLOR_BLACK);
    init_pair(2,2, COLOR_BLACK);
    init_pair(3,3, COLOR_BLACK);
    init_pair(4,4, COLOR_BLACK);
    init_pair(5,5, COLOR_BLACK);
    init_pair(6,6, COLOR_BLACK);
    curs_set(0);
    
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    

//    MAIN WINDOW:
    graph_box = newwin(window_y - 5, window_x, 0, 0);
    scrollok(graph_box, TRUE);
//    mvwprintw(graph_box, window_y/2, 0, "Wilkommen auf der Packet Autobahn!\n\n");
//    box(graph_box, 0 , 0);
    wrefresh(graph_box);
    
    footer_box = newwin(2, window_x, window_y-2, 0);
//    wborder(footer_box, 1, 1, 0, 1, 1, 1, 1, 1);
//    mvwprintw(footer_box,1,( window_x - 25 )/2, "project by leoneckert.com");
    wrefresh(footer_box);		/* Show that box 		*/

    
    startSniffing("en0", true);
    

    endwin();
    return 0;
}