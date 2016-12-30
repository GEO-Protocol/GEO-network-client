#ifndef GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
#define GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H

#include "BaseUserCommand.h"
#include "../../../trust_lines/TrustLine.h"
#include "../../../common/exceptions/ValueError.h"


class MaximalTransactionAmountCommand:
    public BaseUserCommand {
public:
    typedef shared_ptr<MaximalTransactionAmountCommand> Shared;

protected:
    NodeUUID mContractorUUID;

public:
    MaximalTransactionAmountCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const CommandResult *resultOk(TrustLineAmount amount) const;


protected:
    void deserialize(
            const string &command);
};

#endif //GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
