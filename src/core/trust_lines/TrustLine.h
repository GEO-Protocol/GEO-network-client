#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/NodeUUID.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/multiprecision/cpp_int.hpp>


using namespace std;
using boost::multiprecision::cpp_int;

namespace multiprecision = boost::multiprecision; // todo: one line above imports the type from this namespace too. why not to merge them?

typedef multiprecision::checked_uint256_t trust_amount; // todo: rename to TrustLineAmount. All non-primitive types should be named in CamelCase
typedef multiprecision::int256_t balance_value; // todo: rename to TrustLineBalance

// todo: how to know what this callback is doing by the name?
// see the methods, that accept this callback.
// it should be possible to guess by the callback type what is it's purpose.
typedef boost::function<void()> callback;

struct TrustLine {
    // todo: tests?
    // todo: hsc: review the tests.

    // todo: trust line should be usable noy only from the TrustLinesManager.
    // TrustLine - it's an object, that has it's own logic, and purpose, and must have public methods.
    // Otherwise, how transactions should update the trust lines data?
    friend class TrustLinesManager; // todo: remove this.

public:
    typedef shared_ptr<TrustLine> Shared;
    typedef shared_ptr<const TrustLine> SharedConst;

    // todo: public methods must be at the top, and then private
private:
    NodeUUID mContractorNodeUuid;
    trust_amount mIncomingTrustAmount;
    trust_amount mOutgoingTrustAmount;
    balance_value mBalance;
    callback mManagerCallback; // todo: what this callback is doing? please, comment or rename
                                // todo: why is it stored here?

private: // todo: make it public
    TrustLine(
        const NodeUUID &nodeUUID,
        const trust_amount &incomingAmount,
        const trust_amount &outgoingAmount,
        const balance_value &nodeBalance);

    // todo: remove this. trust line can't change it's contractor in runtime
//    void setContractorNodeUUID(
//        const NodeUUID &nodeUUID);

    void setIncomingTrustAmount(
        const trust_amount &amount,
        callback callback);

    void setOutgoingTrustAmount(
        const trust_amount &amount,
        callback callback);

    void setBalance(
        const balance_value &balance,
        callback callback);

public: // todo: make it public
    const NodeUUID &getContractorNodeUUID() const; // todo: rename to contractorNodeUUID()
                                                    // todo: try make inline
    const trust_amount &getIncomingTrustAmount() const; // todo: rename to incomingTrustAmount()
                                                        // todo: try make inline
    const trust_amount &getOutgoingTrustAmount() const; // todo: rename to outgoingTrustAmount()
                                                        // todo: try make inline
    const balance_value &getBalance() const; // todo: rename to balance()
                                            // todo: try make inline
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
