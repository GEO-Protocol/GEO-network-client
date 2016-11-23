#ifndef GEO_NETWORK_CLIENT_RESULT_H
#define GEO_NETWORK_CLIENT_RESULT_H

#include <string>
#include <boost/lexical_cast.hpp>
#include "../commands/Command.h"

using namespace std;

class Result {
private:
    uint16_t mCode;
    string mTimestampExcepted;
    string mTimestampCompleted;

protected:
    Result();

    Result(Command *command,
           uint16_t resultCode,
           string timestampExcepted,
           string timestampCompleted);

    ~Result();

    uint16_t getResCode();

    string getExceptedTimestamp();

    string getCompletedTimestamp();

    virtual string serialize() = 0;
};

#endif //GEO_NETWORK_CLIENT_RESULT_H
