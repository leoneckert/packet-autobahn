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




//__     ___    ____  ___    _    ____  _     _____ ____
//\ \   / / \  |  _ \|_ _|  / \  | __ )| |   | ____/ ___|
// \ \ / / _ \ | |_) || |  / _ \ |  _ \| |   |  _| \___ \
//  \ V / ___ \|  _ < | | / ___ \| |_) | |___| |___ ___) |
//   \_/_/   \_\_| \_\___/_/   \_\____/|_____|_____|____/

map<string, pair<int, long>> activeMACs;
vector<string> orderedMACs;
map<string, int> MACcolors;
int numpackets = 0;
int current_color = 1;
int activeThreshold = 10;
WINDOW *header_box;
WINDOW *graph_box;
WINDOW *footer_box;
WINDOW *graph_boxWrapper;
WINDOW *header_boxWrapper;
WINDOW *left_border;
WINDOW *right_border;
int lineForMACs = 0;



//____    _  _____  _      ____  _____ _____
//|  _ \  / \|_   _|/ \    / ___|| ____|_   _|
//| | | |/ _ \ | | / _ \   \___ \|  _|   | |
//| |_| / ___ \| |/ ___ \   ___) | |___  | |
//|____/_/   \_\_/_/   \_\ |____/|_____| |_|
//
//_____ _   _ _   _  ____ _____ ___ ___  _   _
//|  ___| | | | \ | |/ ___|_   _|_ _/ _ \| \ | |___
//| |_  | | | |  \| | |     | |  | | | | |  \| / __|
//|  _| | |_| | |\  | |___  | |  | | |_| | |\  \__ \
//|_|    \___/|_| \_|\____| |_| |___\___/|_| \_|___/


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
            //not found:
            pair <int,long> p (1,timestamp);
            activeMACs[MAC] = p;
            orderedMACs.push_back(MAC);
            MACcolors[MAC] = current_color++;
            if (current_color > 6) {
                current_color = 1;
            }
        }else{
            //found:
            activeMACs[MAC].first += 1;
            activeMACs[MAC].second = timestamp;
        }
        numpackets ++;
    }
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

//____  ____      ___        _____ _   _  ____
//|  _ \|  _ \    / \ \      / /_ _| \ | |/ ___|
//| | | | |_) |  / _ \ \ /\ / / | ||  \| | |  _
//| |_| |  _ <  / ___ \ V  V /  | || |\  | |_| |
//|____/|_| \_\/_/   \_\_/\_/  |___|_| \_|\____|

int resolution = 1;

int getNumOfMAClines(){
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    
    lineForMACs = 0;
    int numOf20sNeeded = getNumOfActive();
    while (numOf20sNeeded > 0) {
        lineForMACs++;
        numOf20sNeeded = numOf20sNeeded - floor(window_x/20);
    }
    if (lineForMACs < 2) {
        lineForMACs = 2;
    }
    return lineForMACs;
}

void drawHeader(){
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    int linesForMACsOnly = getNumOfMAClines();
    
    wclear(header_boxWrapper);
    wrefresh(header_boxWrapper);
    wresize(header_boxWrapper, linesForMACsOnly + 5, window_x);
    box(header_boxWrapper, 0 , 0);
    
    wborder(left_border, 1, 0, 1, 1, 1, 1, 1, 1);
    wborder(right_border, 1, 0, 1, 1, 1, 1, 1, 1);
    wrefresh(left_border);
    wrefresh(right_border);
    wrefresh(header_boxWrapper);
    box(footer_box, 0 , 0);
    mvwprintw(footer_box,1,( window_x - 29 )/2, "~ project by leoneckert.com ~");
    wrefresh(footer_box);
    
    wclear(header_box);
    wresize(header_box, linesForMACsOnly + 3, window_x-2);
    wrefresh(header_box);
    
//    wborder(header_box, 1, 1, 1, 0, 1, 1, 1, 1);
//    int window_x, window_y;
//    getmaxyx(stdscr, window_y, window_x);
    int MACsPerLine = floor((window_x-2)/20);
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
//    wprintw(header_box, "\n");
//    wprintw(header_box, "\nResolution: %d   (change with arrow keys)", resolution);
    mvwprintw(header_box, linesForMACsOnly + 2, 0, "Resolution: %d   (adjust w/ arrow keys)", resolution);
////    mvwprintw(header_box, columns_for_header - 2, 0, "\n\nResolution: \"|\" symbol represents %d packets. (change with arrow keys)", resolution);
//    
    wrefresh(header_box);
    
}

//void resize_windows(){
//    
//    int window_x, window_y;
//    getmaxyx(stdscr, window_y, window_x);
//    
//    columns_for_header = 0;
//    int numOf20sNeeded = getNumOfActive();
//    while (numOf20sNeeded > 0) {
//        columns_for_header++;
//        numOf20sNeeded = numOf20sNeeded - floor(window_x/20);
//    }
//    columns_for_header += 4;
//    
//    wclear(header_box);
////    wrefresh(header_box);
////    wclear(footer_box);
////    wrefresh(footer_box);
////    
////    delwin(header_box);
////    header_box = newwin(columns_for_header, window_x, 0, 0);
//    wresize(header_box, columns_for_header, window_x);
////    wborder(header_box, 1, 1, 1, 0, 1, 1, 1, 1);
//    
//    wrefresh(header_box);
//    
////    wborder(footer_box, 1, 1, 0, 1, 1, 1, 1, 1);
////    mvwprintw(footer_box,1,( window_x - 25 )/2, "project by leoneckert.com");
////    wrefresh(footer_box);		/* Show that box 		*/
////    
////    
//}




//bool first_Graph = true;
void drawGraph(){
    bool first = true;
    int old_color = 0;
    
    int pipecount = 0;
    
    for (auto& x : orderedMACs){
        if (isActive(x, activeThreshold)) {
            wattron(graph_box, COLOR_PAIR(MACcolors[x]));
            wattron(graph_box, A_BOLD);
            
            //to distunguish two devices with the same colout if they come right after wach other
            if(!first && activeMACs[x].first > 0){
                if(MACcolors[x] == old_color){
                    wprintw(graph_box, " ");
                }
            }
            first = false;
            old_color = MACcolors[x];
            
            
            int c = 0;
            for(int j = 0; j < activeMACs[x].first; j++){
                
                c++;
                if (c >= resolution) {
                    c = 0;
                    wprintw(graph_box, "|");
                    pipecount += 1;
                }
                
            }
            
            wattroff(graph_box, A_BOLD);
            wattroff(graph_box, COLOR_PAIR(MACcolors[x]));
        }
        
    }
    wprintw(graph_box, "\n");
    
    //making it easier to disginguish if there are several lines of packets
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    if (pipecount > window_x - 2) {
        wprintw(graph_box, "\n");
    }
    
    wrefresh(graph_box);
}

void printOut_data(){
//    resize_windows();
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

int kbhit(){
    int ch = wgetch(graph_box);
    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

void checkForKeypress(){
    if (kbhit()) {
        int g = wgetch(graph_box);
        if(g == 65){
            if (resolution < 5) resolution += 1;
            else if (resolution == 5) resolution += 5;
            else if (resolution >= 10) resolution += 5;
            wprintw(graph_box, "[NOTE] new resolution: %d\n\n", resolution);
            wrefresh(graph_box);
            drawHeader();
        }else if(g == 66){
            if (resolution > 10) resolution -= 10;
            else if (resolution == 10) resolution -= 5;
            else if (resolution <= 5 && resolution > 1) resolution -= 1;
            else if (resolution == 1) resolution = 1;
            wprintw(graph_box, "[NOTE] new resolution: %d\n\n", resolution);
            wrefresh(graph_box);
            drawHeader();
        }
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
            
            checkForKeypress();
            
            if (currentTime() - timeTrack > print_interval){
                printOut_data();
                wipeData();
                timeTrack = currentTime();
            }

            
            
        }
    }catch(...){}
    
    

}

int main(){
//  NCURSES:
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
    curs_set(0);
    
    int window_x, window_y;
    getmaxyx(stdscr, window_y, window_x);
    
    

//    GRAPH BOX WRAPPER
    graph_boxWrapper = newwin(window_y - 2, window_x, 0, 0);
//    box(graph_boxWrapper, 0 , 0);
//    wborder(graph_boxWrapper, '|', '|', 1, 0, 1, 1, 0, 0);
    wrefresh(graph_boxWrapper);
    
    
    
//    GRAPH BOX 
    graph_box = newwin(window_y - 3, window_x-2, 0, 1);
    scrollok(graph_box, TRUE);
    
//    for key control:
    cbreak();
    noecho();
    nodelay(graph_box, TRUE);
    
//    welcome text:
    init_pair(7, COLOR_BLACK, COLOR_WHITE);
    wattron(graph_box, COLOR_PAIR(7));
 
    mvwprintw(graph_box,8,0, "                                     \n");
//    wmove(graph_box, 15, 0);
    wprintw(graph_box, " Willkommen auf der Packet Autobahn! \n");
    wprintw(graph_box, "                                     \n\n");
    wattroff(graph_box, COLOR_PAIR(7));
    wprintw(graph_box, "\nA visual representation of the Wifi Data traffic in the air surrounding you.");
    wprintw(graph_box, "\n\n\nA few quick things:");
    wprintw(graph_box, "\n\nThe bigger your command line window the better. Resizing during runtime will lead to bad results.");
    wprintw(graph_box, "\n\nThe graph is updated (roughly) every second and visualises data packets as they \nare received by individual devices via the wifi channel you are currently on.");
    wprintw(graph_box, "\n\nThe 'Resolution' value defines how many packets are represented by a \"|\" symbol.\nThis can be adjusted with [arrow down] and [arrow up].");
    wprintw(graph_box, "\n\n[Press any key to start]\n");
    
    wrefresh(graph_box);
 
//    LEFT AND RIGHT BORDER
    left_border = newwin(window_y - 2, 1, 0, 0);
    wborder(left_border, 1, 0, 1, 1, 1, 1, 1, 1);
    wrefresh(left_border);
    right_border = newwin(window_y - 2, 1, 0, window_x-1);
    wborder(right_border, 1, 0, 1, 1, 1, 1, 1, 1);
    wrefresh(right_border);
    
//    HEADER BOX WRAPPER
    header_boxWrapper = newwin(7, window_x, 0, 0);
    box(header_boxWrapper, 0 , 0);
    wrefresh(header_boxWrapper);
    
    
//    HEADER BOX
    header_box = newwin(5, window_x - 2, 1, 1);
//    box(header_box, 0 , 0);
    wrefresh(header_box);
    
//    FOOTER BOX
    footer_box = newwin(3, window_x, window_y-3, 0);
//    wborder(footer_box, 0, 0, 0, 1, 0, 0, 1, 1);
    box(footer_box, 0 , 0);
    mvwprintw(footer_box,1,( window_x - 29 )/2, "~ project by leoneckert.com ~");
    wrefresh(footer_box);

    
    wgetch(header_box);
    startSniffing("en0", true);
    

    endwin();
    return 0;
}