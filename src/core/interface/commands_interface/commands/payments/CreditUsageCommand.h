#ifndef GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
#define GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../CommandUUID.h"
#include "../../../results_interface/result/CommandResult.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/MemoryError.h"

#include <memory>
#include <utility>

using namespace std;

class CreditUsageCommand: public BaseUserCommand {

public:
    typedef shared_ptr<CreditUsageCommand> Shared;

public:
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    CreditUsageCommand(
        BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const TrustLineAmount &amount() const;

    const string &reason() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kMinRequestedBufferSize();

    CommandResult::SharedConst resultOk() const;

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
    string mReason;
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
