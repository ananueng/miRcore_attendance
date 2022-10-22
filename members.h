//IDENTIFIER  = EEC50281EEC50281EEC50281EEC50281EEC50281

#include "csvstream.h"
#include "ctype.h"
#include <iostream>
#include <vector>
#include <string>
#include <getopt.h>
#include <algorithm>

using namespace std;
//INTERFACE

//IMPLEMENTATION

struct zoomAlias {
    string alias;
    uint32_t time;
};

struct member {
    member() {}
    member(const string &first, const string &last) {
        firstName = first;
        lastName = last;
    }
    vector<zoomAlias> zoomAliases;
    string firstName;
    string lastName;
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
        class firstNameLess {
            public: 
            bool operator() (member* a, member* b) const {
                return a->firstName < b->firstName;
            }
        };

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

        template<typename COMPARE = firstNameLess>
        void readAttendanceList() {
            string junk;
            csvstream csvin(attendanceListFile);
            COMPARE less;

            map<string, string> row;
            csvin >> row;
            while (csvin >> row) {
                if (row["Name (First Last)"] != "") {
                    string first;
                    string last;
                    uint32_t i = 0;

                    //read firstName until space or comma
                    while (row["Name (First Last)"][i] != ' ' && 
                        row["Name (First Last)"][i] != ',' && 
                        i < row["Name (First Last)"].size()) {

                        first.push_back(row["Name (First Last)"][i++]);
                    }

                    //skip the space or comma in between
                    while ((row["Name (First Last)"][i] == ' ' || 
                            row["Name (First Last)"][i] == ',') &&
                            i < row["Name (First Last)"].size()) {

                        i++;
                    }

                    //read lastName until space or end
                    while (row["Name (First Last)"][i] == ' ' &&
                           i < row["Name (First Last)"].size()) {

                        last.push_back(row["Name (First Last)"][i++]);
                    }

                    //add name (all lowercase) to master list
                    transform(first.begin(), first.end(), first.begin(), ::tolower);
                    transform(last.begin(), last.end(), last.begin(), ::tolower);
                    attendanceList.push_back(member(first, last));
                }
            }

            //create sorted attendance list for faster search
            for (auto &i:attendanceList) {
                sortedAttendanceList.push_back(&i);
            }
            sort(sortedAttendanceList.begin(), sortedAttendanceList.end(), less);
        }

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
                //bin search (lower case) in sorted attendance list, add alias and time
                //date = attendanceRow["Start Time"];
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

