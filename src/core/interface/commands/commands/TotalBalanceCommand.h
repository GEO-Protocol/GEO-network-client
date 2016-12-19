#ifndef GEO_NETWORK_CLIENT_TOTALBALANCE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCE_H

#include "BaseUserCommand.h"

class TotalBalanceCommand:
    public BaseUserCommand {

public:
    static const constexpr char *kIdentifier =
        "GET:stats/balances/total";

public:
    TotalBalanceCommand(
        const CommandUUID &uuid);
};

#endif //GEO_NETWORK_CLIENT_TOTALBALANCE_H
