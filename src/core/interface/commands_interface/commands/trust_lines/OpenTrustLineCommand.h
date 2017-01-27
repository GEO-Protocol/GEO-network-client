#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include "../../../../common/exceptions/ValueError.h"

using namespace std;

class OpenTrustLineCommand: public BaseUserCommand {
public:
    typedef shared_ptr<OpenTrustLineCommand> Shared;

public:
    OpenTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    OpenTrustLineCommand(
        BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const TrustLineAmount &amount() const;

    const CommandResult *resultOk() const;

    const CommandResult *trustLineAlreadyPresentResult() const;

    const CommandResult *resultConflict() const;

    const CommandResult *resultNoResponse() const;

    const CommandResult *resultTransactionConflict() const;

protected:
    void deserialize(
        const string &command);

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    const size_t kTrustLineAmountSize = 32;

    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
