#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/NodeUUID.h"

#include "../../../../common/exceptions/ValueError.h"

class CloseTrustLineCommand: public BaseUserCommand {
public:
    typedef shared_ptr<CloseTrustLineCommand> Shared;

public:
    CloseTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    CloseTrustLineCommand(
        BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

    const CommandResult *resultOk() const;

    const CommandResult *trustLineIsAbsentResult() const;

    const CommandResult *resultConflict() const;

    const CommandResult *resultNoResponse() const;

    const CommandResult *resultTransactionConflict() const;

private:
    void deserialize(
        const string &command);

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
