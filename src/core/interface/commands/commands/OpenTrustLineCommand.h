#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H


#include "BaseUserCommand.h"
#include "../../../trust_lines/TrustLine.h"
#include "../../../common/exceptions/ValueError.h"


class OpenTrustLineCommand:
    public BaseUserCommand {

public:
    static const constexpr char *kIdentifier =
        "CREATE:contractors/trust-lines";

public:
    OpenTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    const NodeUUID& contractorUUID() const;
    const trust_amount& amount() const;

protected:
    NodeUUID mContractorUUID;
    trust_amount mAmount;

protected:
    void deserialize(
        const string &command);
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
