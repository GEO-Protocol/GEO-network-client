#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLIST_H

#include "BaseUserCommand.h"


class ContractorsListCommand:
    public BaseUserCommand {

public:
    ContractorsListCommand(
        const CommandUUID &uuid);

    static const string& identifier();

    void deserialize(const string& command){};
};

#endif //GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
