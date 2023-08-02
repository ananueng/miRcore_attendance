//IDENTIFIER  = EEC50281EEC50281EEC50281EEC50281EEC50281

#include "members.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    Members campers(argc, argv);

    //check for help flag
    if (campers.getHelp()) {
        return 0;
    }

    campers.readAttendanceList();
    campers.readZoomLogs();
    campers.printSummary();
    return 0;
}