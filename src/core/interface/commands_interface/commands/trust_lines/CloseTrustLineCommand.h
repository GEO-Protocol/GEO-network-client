#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../CommandUUID.h"
#include "../../../results_interface/result/CommandResult.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../common/exceptions/ValueError.h"

#include <memory>
#include <utility>

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

    CommandResult::SharedConst resultOk() const;

    CommandResult::SharedConst trustLineIsAbsentResult() const;

    CommandResult::SharedConst resultConflict() const;

    CommandResult::SharedConst resultNoResponse() const;

    CommandResult::SharedConst resultTransactionConflict() const;

private:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
