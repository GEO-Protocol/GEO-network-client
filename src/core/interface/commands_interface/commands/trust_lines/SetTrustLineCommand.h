#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../CommandUUID.h"
#include "../../../results_interface/result/CommandResult.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/exceptions/ValueError.h"

#include <memory>
#include <utility>

using namespace std;

class SetTrustLineCommand: public BaseUserCommand {
public:
    typedef shared_ptr<SetTrustLineCommand> Shared;

public:
    SetTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    SetTrustLineCommand(
        BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const TrustLineAmount &newAmount() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

    CommandResult::SharedConst resultOk() const;

    CommandResult::SharedConst trustLineAbsentResult() const;

    CommandResult::SharedConst resultConflict() const;

    CommandResult::SharedConst resultNoResponse() const;

    CommandResult::SharedConst resultTransactionConflict() const;

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mNewAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
