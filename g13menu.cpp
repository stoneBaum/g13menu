#include <fstream>
#include <iostream>
#include <string>
#include <signal.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

std::string fromG13Path = "/tmp/g13-0_out";
std::string toG13Path = "/tmp/g13-0";
std::string profilePath = "./Profiles";

void signal_callback_handler(int signum) {
    std::cout << "Signal " << signum << " recived. Terminating."<< std::endl;
    exit(signum);
}

// returns true if file was written, false if file was not found
bool writeFileToG13(std::string filepath, bool isKeybind){

    std::ofstream toG13;
    std::ifstream readFile;
    unsigned char data;

    readFile.open(filepath, std::ios::in);

    if(readFile.is_open()){
        toG13.open(toG13Path, std::ofstream::out);
        if(!toG13)
            std::cout << "whats happening\n";
        /* TODO: remove - if sure it works bytewise
        if ( isKeybind ) {
            while(std::getline(readFile, data)) {
                toG13<<data<<'\n';
            }
        }else {
            while(std::getline(readFile, data)) {
                toG13<<data;
            }
        }*/
        while(readFile.read((char*)&data, 1)){
            toG13<<data;
        }
        readFile.close();
        // TODO: remove this wait and find a better solution
        // wait, so that G13 has enough time to empty the buffer
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        toG13.flush();
        toG13.close();

        return true;
    }
    return false;
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_callback_handler);


    // TODO: remove this wait and find a better solution
    // wait, so that g13 can start properly

            std::this_thread::sleep_for(std::chrono::seconds(5));

    // TODO: it doesnt work ... but why?
    if(argc >=1 &&(argv[1] == "-h" || argv[1] == "help")) {
        std::cout << "On device menu for Logitech G13" <<std::endl;
        std::cout << "Usage:" <<std::endl;
        std::cout << "g13menu <g13 output pipe> <g13 command input pipe> <g13 profiles folder>" <<std::endl;
        std::cout << "If not all arguments are given the default values will be used:" <<std::endl;
        std::cout << "default g13 output pipe:         /tmp/g13-0_out" <<std::endl;
        std::cout << "default g13 command input pipe:  /tmp/g13-0" <<std::endl;
        std::cout << "default g13 profiles folder:     ./Profiles" <<std::endl;
        return 0;
    }
    switch(argc) {
        case 4:
            profilePath = argv[3];
        case 3:
            toG13Path = argv[2];
        case 2:
            fromG13Path = argv[1];
    }

    std::vector<std::string> profiles {};
    std::ifstream fromG13{fromG13Path};
    std::ofstream toG13;
    std::string line;
    std::ifstream readFile;
    std::string currentProfile = "";
    std::string data = "";

    bool inMenu = false;
    int currentSelection = 0;

    // looking through the profile folder for profiles

    for (fs::directory_entry const & entry : fs::directory_iterator(profilePath)){
        if (entry.is_directory()){
            bool hasBind = false;
            for (fs::directory_entry const & subEntry : fs::directory_iterator(entry.path())){
                if (subEntry.is_regular_file()) {
                    // only count folder as profile if it has at least the Keybinds.bind file
                    if (subEntry.path().filename() == "Keybinds.bind")
                        hasBind = true;
                }

            }
            if (hasBind)
            {
                profiles.push_back(entry.path().filename());
            }
        }
    }

    std::cout << "Profiles found:" <<std::endl;
    for (auto&& i : profiles) std::cout << "└─" << i << std::endl;

    if (profiles.empty()){
        std::cout << "No Profiles found" << std::endl;
        return -1;
    }

    // make the default profile the selected one or if it does not exist the first one
    std::vector<std::string>::iterator iter = std::find(profiles.begin(), profiles.end(), "Default");
    if(iter != profiles.end()) {
        currentSelection = std::distance(profiles.begin(), iter);
        currentProfile = "Default";
    }
    else {
        currentProfile = profiles[0];
    }


    // check if toG13Path already exists
    fs::path f{ toG13Path };
    if (fs::exists(f))
        std::cout << "Pipe to g13 exists" << std::endl;
    else
        std::cout << "Pipe to g13 does not exist. Is g13d running?" << std::endl;

    // check if fromG13Path already exists
    fs::path f2{ fromG13Path };
    if (fs::exists(f2))
        std::cout << "Pipe from g13 exists" << std::endl;
    else
        std::cout << "Pipe from g13 does not exist. Is g13d running?" << std::endl;



    // activate the profile
    if(!writeFileToG13(profilePath + "/" + currentProfile + "/Keybinds.bind", true)){

        std::cerr << profilePath + "/" + currentProfile + "/Keybinds.bind could not be written to G13" << std::endl;

    }

    // bind the keybinds needed for the menu
    if(!writeFileToG13(profilePath + "/menu/menuBind.bind", true)) {

        std::cerr << profilePath + "/menu/menuBind.bind could not be written to G13" << std::endl;

    }


    // main loop
    while(true)
    {

        if (inMenu) {

            // write the menu to the screen
            if(writeFileToG13(profilePath + "/menu/menuLCD.bind", true)){

                // only 3 profiles can be shown at a time
                // figure out with which one we start
                // the selected line is always the second line unless the first or the last profile is selected
                int startingProfile = (currentSelection==0?0:((currentSelection==profiles.size()-1)&&profiles.size()>=3?profiles.size()-1-2:currentSelection-1));


                toG13.open(toG13Path, std::ofstream::out);
                // write the 3 shown profiles to the screen
                for (int i = 0; i < (profiles.size()<3?profiles.size():3); i ++) {

                    // currently selected profile must be visually obvious
                    if (startingProfile+i == currentSelection){
                        toG13<<"textmode 1"<<'\n';
                    }
                    // go to corresponding line
                    toG13<<"pos "<< i <<" 0"<<'\n';
                    // write a "checkbox" and the name of the profile
                    // check the checkbox if it is the current profile
                    toG13<<"out ["<< (currentProfile==profiles[startingProfile+i]?"x":" ") <<"] "<< profiles[startingProfile+i] <<'\n';
                    // only the currently selected profile must be visually obvious
                    if (startingProfile+i == currentSelection){
                        toG13<<"textmode 0"<<'\n';
                    }
                }
                toG13.flush();
                toG13.close();
            }else {

                std::cerr << profilePath + "/menu/menuLCD.bind could not be written to G13" << std::endl;

            }

        }else{

            // not in menu, therefore display art
            // TODO: fix: for some reason this sometimes does not change the display
            if(!writeFileToG13(profilePath + "/" + currentProfile + "/LCD.lpbm", false)){


                // No .lpbm cover art found so try to find a textual cover art
                if(writeFileToG13(profilePath + "/" + currentProfile + "/LCD.bind", true)){

                    // redo the menu buttonbind to ensure the menu still works even if the neccessary keys are overwritten
                    writeFileToG13(profilePath + "/menu/menuBind.bind", true);
                }
                else{

                    // No textual cover art found so just display the profile name
                    toG13.open(toG13Path, std::ofstream::out);

                    toG13<<"clear"<<'\n';
                    // 160 (pixles of screen) divided by 2 (because text should be in the middle) minus 1 (because counting starts at 0) (=79) minus (length of name times 5 (becase each character is 5 pixles wide) / 2 because the text should be not start in the middle but be centered
                    toG13<<"pos 2 "<< (79-(currentProfile.length()*5)/2)<< '\n';
                    toG13<<"out "<<currentProfile<<'\n';

                    toG13.close();


                }

            }
        }

        // wait for new input from the menu
        std::getline(fromG13, line);
        std::cout << line << std::endl;

        // handle accordingly
        if(line == "menuToggle") {
            // menu is toggled
            inMenu = !inMenu;
        }
        else if(line == "menuDown") {
            // menu is activated and selection downwards
            inMenu=true;
            currentSelection++;
            if (currentSelection>=profiles.size())
                currentSelection=profiles.size()-1;
        }
        else if(line == "menuUp") {
            // menu is activated and selection upwards
            inMenu=true;
            currentSelection--;
            if (currentSelection<0)
                currentSelection=0;
        }
        else if(line == "menuSelect") {
            // if in menu: currently selected profile will be activated
            if (inMenu){
                currentProfile = profiles[currentSelection];

                // write default profile to G13 first in case not all keys/stickzones/color are/is set
                writeFileToG13(profilePath + "/Default/Keybinds.bind", true);

                // write profile to G13
                if(writeFileToG13(profilePath + "/" + currentProfile + "/Keybinds.bind", true)){

                    // redo the menu buttonbind to ensure the menu still works even if the neccessary keys are overwritten
                    writeFileToG13(profilePath + "/menu/menuBind.bind", true);

                    // go out of menu to display cover art
                    inMenu = false;
                }else {

                std::cerr << profilePath + "/" + currentProfile + "/Keybinds.bind could not be written to G13" << std::endl;

                }

            }
        }
    }
    return EXIT_SUCCESS;
}


