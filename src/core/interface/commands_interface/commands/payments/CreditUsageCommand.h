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

class CreditUsageCommand:
    public BaseUserCommand {

public:
    enum ResultCodes {
        OK = 200,
        ProtocolError = 401,
        InsufficientFunds = 412,
        RemoteNodeIsInaccessible = 444,
        NoPaths = 462,
    };

public:
    typedef shared_ptr<CreditUsageCommand> Shared;

public:
    static const string &identifier();
    static const size_t kMinRequestedBufferSize();

public:
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    CreditUsageCommand(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();
    const TrustLineAmount &amount() const;
    const NodeUUID &contractorUUID() const;

public:
    // Results handlers
    CommandResult::SharedConst resultOK() const;
    CommandResult::SharedConst resultNoPaths() const;
    CommandResult::SharedConst resultNoResponse() const;
    CommandResult::SharedConst resultProtocolError() const;
    CommandResult::SharedConst resultInsufficientFundsError() const;

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
