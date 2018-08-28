/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../common/memory/MemoryUtils.h"
#include "../common/multiprecision/MultiprecisionUtils.h"

#include "../common/exceptions/RuntimeError.h"
#include "../common/exceptions/MemoryError.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/OverflowError.h"

#include <vector>


using namespace std;

// todo: tests?
// todo: hsc: review the tests.
class TrustLine {
public:
    typedef shared_ptr<TrustLine> Shared;
    typedef shared_ptr<const TrustLine> ConstShared;

public:
    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &nodeBalance,
        bool isContractorGateway);

    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount);

    void setIncomingTrustAmount(
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const TrustLineAmount &amount);

    void pay(
        const TrustLineAmount &amount);

    void acceptPayment(
        const TrustLineAmount &amount);

    const NodeUUID& contractorNodeUUID() const;

    const TrustLineAmount& incomingTrustAmount() const;

    const TrustLineAmount& outgoingTrustAmount() const;

    const TrustLineBalance& balance() const;

    ConstSharedTrustLineAmount availableOutgoingAmount() const;

    ConstSharedTrustLineAmount availableIncomingAmount() const;

    ConstSharedTrustLineAmount usedAmountByContractor() const;

    ConstSharedTrustLineAmount usedAmountBySelf() const;

    bool isContractorGateway() const;

    void setContractorAsGateway(
        bool contractorAsGateway);

    static const TrustLineBalance& kZeroBalance();

    static const TrustLineAmount& kZeroAmount();

    friend bool operator== (
        const TrustLine::Shared contractor1,
        const TrustLine::Shared contractor2);

    friend bool operator== (
        const TrustLine &contractor1,
        const TrustLine &contractor2);

private:
    NodeUUID mContractorNodeUUID;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;
    bool mIsContractorGateway;

};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
