#ifndef GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H
#define GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H

#include "Command.h"

class UpdateOutgoingTrustAmountCommand : public Command {
private:
    string mCommandBuffer;
    NodeUUID mContractorUUID;
    trust_amount mAmount;

public:
    UpdateOutgoingTrustAmountCommand(const uuids::uuid &commandUUID, const string &identifier,
                                     const string &timestampExcepted, const string &commandBuffer);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const string &exceptedTimestamp() const;

    const NodeUUID &contractorUUID() const;

    const trust_amount &amount() const;

private:
    void deserialize();
};

#endif //GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H
