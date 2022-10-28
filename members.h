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
    string time;
};

struct member {
    member() {}
    member(const string &output, const string &full, const string &first, const string &last) {
        outName = output;
        fullName = full;
        firstName = first;
        lastName = last;
    }
    deque<zoomAlias> zoomAliases;
    string outName;
    string fullName;
    string firstName;
    string lastName;
    bool fullMatch = false;
    bool partialMatch = false;
    bool filler = false;
};

class Members {
    private:
        vector<member> attendanceList;
        vector<member*> sortedAttendanceList;
        vector<string> notMatched;
        string attendanceListFile;
        string zoomLogFile;
        string date;
        bool isHelp = false;
        bool find(string full, string &time) {
            bool foundMatch = false;
            string fullOnlyLetters;
            for (int j = 0; j < full.length(); j++) {
                if ((int(full[j]) >= 'a' && int(full[j]) <= 'z') || full[j] == ' ') {
                    fullOnlyLetters.push_back(full[j]);
                }
            }
            full = fullOnlyLetters;
            for (size_t i = 0; i < attendanceList.size(); i++) {
                size_t foundFirst = full.find(attendanceList[i].firstName);
                size_t foundLast = full.find(attendanceList[i].lastName);
                if (full == attendanceList[i].fullName || 
                    (foundFirst != std::string::npos && foundLast != std::string::npos)) {
                    zoomAlias toPush = {full, time};
                    attendanceList[i].zoomAliases.push_front(toPush);
                    attendanceList[i].fullMatch = true;
                    foundMatch = true;
                }
                else {
                    string temp;
                    stringstream ss(full);
                    while (getline(ss, temp, ' ')) {
                        if (temp == attendanceList[i].lastName || 
                            temp == attendanceList[i].firstName) {
                        zoomAlias toPush = {full, time};
                        attendanceList[i].zoomAliases.push_back(toPush);
                        attendanceList[i].partialMatch = true;   
                        foundMatch = true; 
                        }
                    }
                } 
            }
            return foundMatch;
        }
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

        //template<typename COMPARE = firstNameLess>
        void readAttendanceList() {
            string junk;
            csvstream csvin(attendanceListFile);
            //COMPARE less;

            map<string, string> row;
            csvin >> row;
            while (csvin >> row) {
                string full = row["Name (First Last)"];
                transform(full.begin(), full.end(), full.begin(), ::tolower);
                if (full != "") {
                    string first;
                    string last;
                    uint32_t i = 0;

                    //read firstName until space or comma
                    while (full[i] != ' ' && full[i] != ',' && i < full.size()) {
                        first.push_back(full[i++]);
                    }

                    //skip the space or comma in between
                    while ((full[i] == ' ' || full[i] == ',') && i < full.size()) {
                        i++;
                    }

                    //read lastName until space or end
                    while (full[i] != ' ' && i < full.size()) {
                        last.push_back(full[i++]);
                    }

                    //add name (all lowercase) to master list
                    attendanceList.push_back(member(row["Name (First Last)"], full, first, last));
                }
                else { //pushback empty for formatting
                    attendanceList.push_back(member());
                }
            }

            //create sorted attendance list for faster search
            // for (auto &i:attendanceList) {
            //     sortedAttendanceList.push_back(&i);
            // }
            // sort(sortedAttendanceList.begin(), sortedAttendanceList.end(), less);
        }
        // template<typename COMPARE = firstNameLess>
        void readZoomLogs() {
            //COMPARE less;
            string junk;
            csvstream csvin(zoomLogFile);

            //read first 2 lines (for the date)
            map<string, string> dateRow;
            csvin >> dateRow;
            date = dateRow["Start Time"];

            //read the rest
            csvin.set_new_header(); // empty line in zoomLogs
            csvin.set_new_header(); // first row (new header) of attendance

            map<string, string> attendanceRow;
            while (csvin >> attendanceRow) {
                string temp = attendanceRow["Name (Original Name)"];
                string time = attendanceRow["Total Duration (Minutes)"];
                //vector<member>::iterator it;
                transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
                if (!find(temp, time)) {
                    notMatched.push_back(attendanceRow["Name (Original Name)"]);
                };
                //bin search (lower case) in sorted attendance list, add alias and time
                // auto it = attendanceList;
                // it = find(attendanceList.begin(), attendanceList.end(), temp);
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
            for (size_t i = 0; i < attendanceList.size(); i++) {
                cout << attendanceList[i].outName << ": ";
                if (attendanceList[i].fullMatch) {
                    //only cout the first zoomAlias, which will be the full name
                    cout << attendanceList[i].zoomAliases.front().alias << "|" <<
                    attendanceList[i].zoomAliases.front().time << "|,";
                }
                else if (attendanceList[i].partialMatch) {//partial match
                    for (size_t j = 0; j < attendanceList[i].zoomAliases.size(); i++) {
                    cout << attendanceList[i].zoomAliases[j].alias << "|" <<
                    attendanceList[i].zoomAliases[j].time << "|, ";
                    }
                }
                else {
                    if (attendanceList[i].filler) {
                        cout << "this is filler";
                    }
                    else {
                        cout << "ABSENT";
                    }
                }
                cout << endl;
            }
        //print unmatched
            if (!notMatched.empty()) {
                cout << "Names in zoomLogs that weren't matched: ";
                for (auto &i: notMatched){
                    cout << i << endl;
                }
            }
        }
};

