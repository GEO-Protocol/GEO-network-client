/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../trust_lines/TrustLine.h"
#include "../common/time/TimeUtils.h"

class MaxFlowCalculationTrustLine {

public:
    typedef shared_ptr<MaxFlowCalculationTrustLine> Shared;
    typedef shared_ptr<const MaxFlowCalculationTrustLine> SharedConst;

public:
    MaxFlowCalculationTrustLine(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        ConstSharedTrustLineAmount amount);

    const NodeUUID& sourceUUID() const;

    const NodeUUID& targetUUID() const;

    ConstSharedTrustLineAmount amount() const;

    void setAmount(ConstSharedTrustLineAmount amount);

    ConstSharedTrustLineAmount freeAmount();

    void addUsedAmount(const TrustLineAmount &amount);

    void setUsedAmount(const TrustLineAmount &amount);

private:
    NodeUUID mSourceUUID;
    NodeUUID mTargetUUID;
    ConstSharedTrustLineAmount mAmount;
    SharedTrustLineAmount mUsedAmount;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
