#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLIST_H

#include "BaseUserCommand.h"


class ContractorsListCommand:
    public BaseUserCommand {
public:
    typedef shared_ptr<ContractorsListCommand> Shared;

public:
    ContractorsListCommand(
        const CommandUUID &uuid);

    static const string& identifier();

    const CommandResult *resultOk(vector<NodeUUID> contractors) const;

    const CommandResult *noContractorsResult() const;

    void deserialize(const string& command){};
};

#endif //GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
