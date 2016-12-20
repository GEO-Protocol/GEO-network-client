#ifndef GEO_NETWORK_CLIENT_TOTALBALANCE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCE_H

#include "BaseUserCommand.h"

class TotalBalanceCommand:
    public BaseUserCommand {

public:
    TotalBalanceCommand(
        const CommandUUID &uuid);

    static const string &identifier();

    void deserialize(const string &buffer){};
};

#endif //GEO_NETWORK_CLIENT_TOTALBALANCE_H
