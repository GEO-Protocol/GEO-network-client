#ifndef GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
#define GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H

#include "Command.h"

class MaximalTransactionAmountCommand : public Command {
private:
    string mCommandBuffer;
    NodeUUID mContractorUUID;

public:
    MaximalTransactionAmountCommand(const uuids::uuid &commandUUID, const string &identifier,
                                    const string &timestampExcepted, const string &commandBuffer);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const string &exceptedTimestamp() const;

    const NodeUUID &contractorUUID() const;

private:
    void deserialize();
};

#endif //GEO_NETWORK_CLIENT_MAXIMALTRANSACTIONAMOUNTCOMMAND_H
