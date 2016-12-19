#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLIST_H

#include "BaseUserCommand.h"


class ContractorsListCommand:
    public BaseUserCommand {

public:
    static const constexpr char *kIdentifier = "GET:contractors";

public:
    ContractorsListCommand(
        const CommandUUID &uuid);
};

#endif //GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
