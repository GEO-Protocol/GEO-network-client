#ifndef GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
#define GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H

#include "BaseUserCommand.h"
#include "../../../common/exceptions/ValueError.h"


class MaximalTransactionAmountCommand:
    public BaseUserCommand {

public:
    MaximalTransactionAmountCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();
    const NodeUUID &contractorUUID() const;

protected:
    NodeUUID mContractorUUID;

protected:
    void deserialize(const string &command);
};

#endif //GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
