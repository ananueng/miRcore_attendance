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
    member(bool x) {
        filler = x;
    }
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
        string outputFile;
        string date;
        bool isHelp = false;
        bool outputMode = false;
        bool find(string full, string &time) {
            bool foundMatch = false;
            string fullOnlyLetters;
            for (size_t j = 0; j < full.length(); j++) {
                if ((int(full[j]) >= 'a' && int(full[j]) <= 'z') || full[j] == ' ') {
                    fullOnlyLetters.push_back(full[j]);
                }
            }
            full = fullOnlyLetters;
            for (size_t i = 0; i < attendanceList.size(); i++) {
                
                member &forDEBUG = attendanceList[i];
                if (!forDEBUG.filler) {}
                if (time == "47") {
                    //i - 2 = roll call row, time == zoom time
                    time = "47";
                }

                size_t foundFirst = full.find(attendanceList[i].firstName);
                size_t foundLast = full.find(attendanceList[i].lastName);
                //full match
                if ((full == attendanceList[i].fullName || 
                    (foundFirst != std::string::npos && foundLast != std::string::npos)) &&
                    !attendanceList[i].firstName.empty() && !attendanceList[i].lastName.empty()) {
                    //fullmatch found
                    zoomAlias toPush = {full, time};
                    attendanceList[i].zoomAliases.push_front(toPush);
                    attendanceList[i].fullMatch = true;
                    foundMatch = true;
                    break;
                }
            }
            if (foundMatch) {
                return foundMatch;
            }
            //attempt partial match
            for (size_t i = 0; i < attendanceList.size(); i++) {
                string temp;
                stringstream ss(full);
                while (getline(ss, temp, ' ')) {
                    if ((temp == attendanceList[i].lastName && !attendanceList[i].lastName.empty()) || 
                        (temp == attendanceList[i].firstName && !attendanceList[i].firstName.empty())) {
                    zoomAlias toPush = {full, time};
                    attendanceList[i].zoomAliases.push_back(toPush);
                    attendanceList[i].partialMatch = true;   
                    foundMatch = true; 
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
                                        { "output", required_argument, nullptr, 'o' },
                                        { "help", no_argument, nullptr, 'h' },
                                        { nullptr, 0, nullptr, '\0' }};
            //required has : after
            while ((option = getopt_long(argc, argv, "g:z:o:h", longOpts, &option_index)) != -1) {
                switch (option) {
                    case 'g':
                        hasAttendanceList = true;
                        attendanceListFile = string(optarg);
                        break;

                    case 'z':
                        hasZoomLog = true;
                        zoomLogFile = string(optarg);
                        break;

                    case 'o':
                        outputMode = true;
                        outputFile = string(optarg);
                        break;

                    case 'h':
                        isHelp = true;
                        std::cout << "This program reads .csv files:\n"
                                << "-g specifying the file with the member names and\n"
                                <<  "-z specifying the file with the zoom attendance log\n"
                                <<  "-o file specifying the .txt file to import into google sheets\n"
                                <<  "< filename specifying the file to check for non-matched names\n"
                                <<  "Usage:  \'./attendance [--group  | -g] <groupList file> \n"
                                <<   "                      [--zoom   | -z] <zoomLog file>\n"
                                <<   "                      [--output | -o] <output file> > <usercheck file>\n"
                                <<   "                      [--help   | -h]'\n";
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
                else { //pushback filler for formatting
                    attendanceList.push_back(member(true));
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
            cout << "numParticipants: " << dateRow["Participants"] << endl;

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

            //print to outputFile
            if (outputMode) {
                ofstream os;
                os.open(outputFile);
                for (size_t n = 0; n < attendanceList.size(); n++) {
                    if (attendanceList[n].fullMatch) {
                        os << "TRUE" << endl;
                    }
                    else if (attendanceList[n].partialMatch) {
                        os << "***" << endl;
                    }
                    else if (attendanceList[n].filler) {
                        os << "---" << endl;
                    }
                    else {
                        os << "FALSE" << endl;
                    }
                }
                os.close();
            }
            //print the attendance
            for (size_t j = 0; j < attendanceList.size(); j++) {
                if (!attendanceList[j].filler) {
                    cout << attendanceList[j].outName << ": ";
                }
                if (attendanceList[j].fullMatch) {
                    //only cout the first zoomAlias, which will be the full name
                    cout << attendanceList[j].zoomAliases.front().alias << "|" <<
                    attendanceList[j].zoomAliases.front().time << "|,";
                }
                else if (attendanceList[j].partialMatch) {//partial match
                    cout << "***";
                    for (size_t k = 0; k < attendanceList[j].zoomAliases.size(); k++) {
                    cout << attendanceList[j].zoomAliases[k].alias << "|" <<
                    attendanceList[j].zoomAliases[k].time << "|, ";
                    }
                }
                else {
                    if (attendanceList[j].filler) {
                        cout << "FILLER (no person in role call here)";
                    }
                    else {
                        cout << "ABSENT";
                    }
                }
                cout << endl;
            }
        //print unmatched
            if (!notMatched.empty()) {
                cout << "Names in zoomLogs that weren't matched: " << endl;
                for (auto &x: notMatched){
                    cout << x << endl;
                }
            }
        }
};

