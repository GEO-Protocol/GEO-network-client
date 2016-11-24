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
           const uint16_t &resultCode,
           const string &timestampCompleted);

    const uint16_t &resCode() const;

    const string &exceptedTimestamp() const;

    const string &completedTimestamp() const;

    virtual string serialize() = 0;
};

#endif //GEO_NETWORK_CLIENT_RESULT_H
