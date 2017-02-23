#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../Message.hpp"

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

//TODO:: (D.V.) do you really need this import ?
#include "../../../settings/Settings.h"

#include "../base/transaction/TransactionMessage.h"

//TODO:: Class file structure:
//TODO:: first - public methods, then protected and then private;
//TODO:: fields-members order like methods order, but members declaring after methods.
class InBetweenNodeTopologyMessage: public Message {
public:
    typedef shared_ptr<InBetweenNodeTopologyMessage> Shared; // TODO:: (D.V.) This must be after class declaration.

public:
    InBetweenNodeTopologyMessage(
            const TrustLineBalance maxFlow, //TODO:: (D.V.) TrustLineBalance is non primitive type, use address, don't copies tmp value in constructor anymore.
            const byte max_depth,
            vector<NodeUUID> &path); //TODO:: (D.V.) (Recommendation) Try to use move semantic.

    // TODO:: Constructor's or function's parameters starts from new line
    InBetweenNodeTopologyMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    //TODO:: (D.V.) In most cases, "getter" returns const value, and address, not copy.
    //TODO:: Also, getter must be declared as a const function. It's deny to modify inner object state.
    TrustLineBalance getMaxFlow();

    //TODO:: (D.V.) In most cases, "getter" returns const value, and address, not copy.
    //TODO:: Also, getter must be declared as a const function. It's deny to modify inner object state.
    vector<NodeUUID> getPath();

protected:
    //TODO:: This method must be protected, 'cause he will invokes only from abstract(base) type.
    //TODO:: So in base class, he is declared as public.
    pair<BytesShared, size_t> serializeToBytes();

    //TODO:: This method must be protected, 'cause he will invokes only from inherit class.
    //TODO:: So in base class, he is also declared as protected.
    void deserializeFromBytes(
            BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

protected:
    vector<NodeUUID> mPath;
    TrustLineBalance mMaxFlow;
    uint8_t mMaxDepth;
    static uint8_t mNodesInPath;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
