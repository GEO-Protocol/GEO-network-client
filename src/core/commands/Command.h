#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include <string>


using namespace std;

class Command {
public:
    Command(const string &identifier);

    Command(const string &identifier, const string &timestampExcepted);

    const string &identifier() const;

    const string &timeStampExcepted() const;

private:
    string mIdentifier;

    string mTimestampExcepted;
};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
