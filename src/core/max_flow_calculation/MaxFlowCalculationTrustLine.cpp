#include "MaxFlowCalculationTrustLine.h"

MaxFlowCalculationTrustLine::MaxFlowCalculationTrustLine(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    ConstSharedTrustLineAmount amount):

    mSourceUUID(sourceUUID),
    mTargetUUID(targetUUID),
    mAmount(amount),
    mUsedAmount(make_shared<TrustLineAmount>(0)) {

    if (*amount.get() < TrustLine::kZeroAmount()) {
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

ConstSharedTrustLineAmount MaxFlowCalculationTrustLine::amount() const {
    return mAmount;
}

ConstSharedTrustLineAmount MaxFlowCalculationTrustLine::freeAmount() {
    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            *mAmount.get() - *mUsedAmount.get()));
}

void MaxFlowCalculationTrustLine::addUsedAmount(const TrustLineAmount &amount) {
    *mUsedAmount.get() = *mUsedAmount.get() + amount;
}

void MaxFlowCalculationTrustLine::setUsedAmount(const TrustLineAmount &amount) {
    *mUsedAmount.get() = amount;
}

void MaxFlowCalculationTrustLine::setAmount(ConstSharedTrustLineAmount amount) {
    mAmount = amount;
}