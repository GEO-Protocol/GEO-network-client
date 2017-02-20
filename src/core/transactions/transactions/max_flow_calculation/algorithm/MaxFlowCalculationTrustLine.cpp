//
// Created by mc on 19.02.17.
//

#include "MaxFlowCalculationTrustLine.h"

MaxFlowCalculationTrustLine::MaxFlowCalculationTrustLine(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    const TrustLineAmount &amount):

    mSourceUUID(sourceUUID),
    mTargetUUID(targetUUID),
    mAmount(amount),
    mUsedAmount(0) {

    if (amount < TrustLine::kZeroAmount()) {
        throw ValueError("MaxFlowCalculationTrustLine::MaxFlowCalculationTrustLine: "
                             "Amount can't be negative value.");
    }
}

NodeUUID MaxFlowCalculationTrustLine::getSourceUUID() {
    return mSourceUUID;
}

NodeUUID MaxFlowCalculationTrustLine::getTargetUUID() {
    return mTargetUUID;
}

TrustLineAmount MaxFlowCalculationTrustLine::getAmount() {
    return mAmount;
}

TrustLineAmount MaxFlowCalculationTrustLine::getFreeAmount() {
    return mAmount - mUsedAmount;
}

void MaxFlowCalculationTrustLine::addUsedAmount(TrustLineAmount amount) {
    mUsedAmount = mUsedAmount + amount;
}

void MaxFlowCalculationTrustLine::setUsedAmount(TrustLineAmount amount) {
    mUsedAmount = amount;
}
