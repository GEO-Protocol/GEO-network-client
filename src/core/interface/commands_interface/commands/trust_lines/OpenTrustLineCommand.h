#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H

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

class  OpenTrustLineCommand: public BaseUserCommand {
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

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
