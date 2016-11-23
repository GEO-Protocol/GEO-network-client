#ifndef GEO_NETWORK_CLIENT_RESULT_H
#define GEO_NETWORK_CLIENT_RESULT_H

#include <string>
#include <boost/lexical_cast.hpp>
#include "../commands/Command.h"

using namespace std;

class Result {
private:
    Command mCommand;
    uint16_t mCode;
    string mTimestampExcepted;
    string mTimestampCompleted;

protected:
    Result(Command *command,
           uint16_t resultCode,
           string timestampExcepted,
           string timestampCompleted
    );

    ~Result();

    Command getCommand();

    uint16_t getResultCode();

    string getTimestampExcepted();

    string getTimestampCompleted();

    virtual string serialize() = 0;
};

#endif //GEO_NETWORK_CLIENT_RESULT_H
