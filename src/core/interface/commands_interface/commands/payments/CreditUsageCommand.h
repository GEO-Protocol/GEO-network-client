#ifndef GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
#define GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/memory/memory.h"

#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/MemoryError.h"


class CreditUsageCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CreditUsageCommand> Shared;
    typedef shared_ptr<const CreditUsageCommand> ConstShared;

public:
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    CreditUsageCommand(
        BytesShared buffer);

    static const string &identifier();

    const NodeUUID& contractorUUID() const;

    const TrustLineAmount& amount() const;

    const string& reason() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

    const CommandResult::Shared resultOk() const;
//
//    const CommandResult *trustLineAlreadyPresentResult() const;
//
//    const CommandResult *resultConflict() const;
//
//    const CommandResult *resultNoResponse() const;
//
//    const CommandResult *resultTransactionConflict() const;

protected:
    // todo: (DM) rename "deserialize()" -> "parse()"
    void deserialize(
        const string &command);

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;

    // todo: utf-8 bugs are comming!!1
    string mReason; // text description why this operation is performed.
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
