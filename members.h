//IDENTIFIER  = EEC50281EEC50281EEC50281EEC50281EEC50281

#include "csvstream.h"
#include <iostream>
#include <vector>
#include <string>
#include <getopt.h>

using namespace std;
//INTERFACE

//IMPLEMENTATION
struct member {
    vector<string> zoomAliases;
    string name;
    uint32_t timePresent = 0;
};

class nameLess {
    public: 
    bool operator() (member* a, member* b) const {
        return a->name > b->name;
    }
};

class Members {
    private:
        vector<member> attendanceList;
        vector<member*> sortedAttendanceList;
        string attendanceListFile;
        string zoomLogFile;
        string date;
        bool isHelp = false;
    public:
        //default ctor
        Members() {
            cerr << "should have an attendance list argument in the code, see implementation";
            exit(1);
        }

        //main ctor
        Members(int argc, char** argv) {
            getOptions(argc, argv);
        }

        void getOptions(int argc, char** argv) {
            int option_index = 0, option = 0;

            //check for required flags
            bool hasAttendanceList = false;
            bool hasZoomLog;

            // Don't display getopt error messages about options
            opterr = false;

            struct option longOpts[] = {{ "group", required_argument, nullptr, 'g' },
                                        { "zoom", required_argument, nullptr, 'z' },
                                        { "help", no_argument, nullptr, 'h' },
                                        { nullptr, 0, nullptr, '\0' }};
            //required has : after
            while ((option = getopt_long(argc, argv, "g:z:h", longOpts, &option_index)) != -1) {
                switch (option) {
                    case 'g':
                        hasAttendanceList = true;
                        attendanceListFile = string(optarg);
                        break;

                    case 'z':
                        hasZoomLog = true;
                        zoomLogFile = string(optarg);
                        break;

                    case 'h':
                        isHelp = true;
                        std::cout << "This program reads .csv files:\n"
                                << "-g specifying the file with the member names and\n"
                                <<  "-a specifying the file with the zoom attendance log\n\n"
                                <<  "Usage:  \'./attendance [--group | -g] <groupList file>"
                                <<                       " [--zoom | -z] <zoomLog file>' or \n"
                                <<        "\t\'./attendance [--help  | -h]'\n";
                        //exit(0);
                        return;
                }//switch
            }//while

            if (!hasAttendanceList || !hasZoomLog) {
                //doesn't have one of the required files
                cerr << "One of the flags/files aren't specified, do './attendance -h' for formatting" << endl;
                exit(1);
            }//if
        }

        bool getHelp() {return isHelp;}

        void readAttendanceList() {}
        void readZoomLogs() {
            string junk;
            csvstream csvin(zoomLogFile);

            //read first 2 lines (for the date)
            map<string, string> dateRow;
            csvin >> dateRow;
            date = dateRow["Start Time"];

            //read the rest
            csvin.set_new_header(); // empty line
            csvin.set_new_header(); // first row (new header) of attendance
            map<string, string> attendanceRow;
            while (csvin >> attendanceRow) {
                //find in attendance list, add alias and time
                date = attendanceRow["Start Time"];
            }
        }

        void printSummary() {
            //print only the date, not the time
            uint32_t i = 0;
            while (date[i] != ' ') {
                cout << date[i++];
            }
            cout << endl;

            //print the attendance

        }

};

