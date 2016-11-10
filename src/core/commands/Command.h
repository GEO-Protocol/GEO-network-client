#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include <string>


using namespace std;

class Command {
public:
    Command(const string &identifier);

    const string& identifier() const;

private:
    string mIdentifier;
};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
