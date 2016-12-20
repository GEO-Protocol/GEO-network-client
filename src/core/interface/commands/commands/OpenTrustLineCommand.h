#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H


#include "BaseUserCommand.h"
#include "../../../trust_lines/TrustLine.h"
#include "../../../common/exceptions/ValueError.h"

using namespace std;

class OpenTrustLineCommand:
    public BaseUserCommand {

protected:
    NodeUUID mContractorUUID;
    trust_amount mAmount;

public:
    OpenTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string identifier();

    const NodeUUID &contractorUUID() const;

    const trust_amount &amount() const;

    const CommandResult *resultOk() const;

    const CommandResult *trustLineAlreadyPresentResult() const;

protected:
    void deserialize(
        const string &command);
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
