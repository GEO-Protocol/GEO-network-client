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

const NodeUUID& MaxFlowCalculationTrustLine::sourceUUID() const {
    return mSourceUUID;
}

const NodeUUID& MaxFlowCalculationTrustLine::targetUUID() const {
    return mTargetUUID;
}

const TrustLineAmount& MaxFlowCalculationTrustLine::amount() const {
    return mAmount;
}

ConstSharedTrustLineAmount MaxFlowCalculationTrustLine::freeAmount() const {
    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            mAmount - mUsedAmount));
}

void MaxFlowCalculationTrustLine::addUsedAmount(const TrustLineAmount &amount) {
    mUsedAmount = mUsedAmount + amount;
}

void MaxFlowCalculationTrustLine::setUsedAmount(const TrustLineAmount &amount) {
    mUsedAmount = amount;
}

void MaxFlowCalculationTrustLine::setAmount(const TrustLineAmount& amount) {
    mAmount = amount;
}