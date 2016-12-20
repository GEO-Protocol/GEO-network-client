#ifndef GEO_NETWORK_CLIENT_USECREDITCOMMAND_H
#define GEO_NETWORK_CLIENT_USECREDITCOMMAND_H

#include "BaseUserCommand.h"
#include "../../../trust_lines/TrustLine.h"
#include "../../../common/exceptions/ValueError.h"


class UseCreditCommand:
    public BaseUserCommand {

public:
    static const constexpr char *kIdentifier =
        "CREATE:contractors/transactions";

public:
    UseCreditCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    const NodeUUID& contractorUUID() const;
    const trust_amount& amount() const;
    const string& purpose() const;

protected:
    NodeUUID mContractorUUID;
    trust_amount mAmount;
    string mPurpose;

protected:
    void deserialize(
        const string &command);
};

#endif //GEO_NETWORK_CLIENT_USECREDITCOMMAND_H
