#ifndef GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H
#define GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H

#include "BaseUserCommand.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../trust_lines/TrustLine.h"


class UpdateTrustLineCommand:
    public BaseUserCommand {

public:
    static const constexpr char *kIdentifier =
        "SET:contractors/trust-lines";

public:
    UpdateTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    const NodeUUID &contractorUUID() const;
    const trust_amount &amount() const;

protected:
    NodeUUID mContractorUUID;
    trust_amount mAmount;

protected:
    void deserialize(
        const string &command);
};

#endif //GEO_NETWORK_CLIENT_UPDATEOUTGOINGTRUSTAMOUNTCOMMAND_H
