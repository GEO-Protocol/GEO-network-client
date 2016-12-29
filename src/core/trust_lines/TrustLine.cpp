#include "TrustLine.h"

TrustLine::TrustLine(const NodeUUID &nodeUUID,
                     const trust_amount &incomingAmount,
                     const trust_amount &outgoingAmount,
                     const balance_value &nodeBalance) {
    mContractorNodeUuid = nodeUUID;
    mIncomingTrustAmount = incomingAmount;
    mOutgoingTrustAmount = outgoingAmount;
    mBalance = nodeBalance;
}

void TrustLine::setContractorNodeUUID(
        const NodeUUID &nodeUUID) {
    mContractorNodeUuid = nodeUUID;
}

void TrustLine::setIncomingTrustAmount(
        callback managersCallback,
        const trust_amount &incomingAmount) {
    mIncomingTrustAmount = incomingAmount;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

void TrustLine::setOutgoingTrustAmount(
        callback managersCallback,
        const trust_amount &outgoingAmount) {
    mOutgoingTrustAmount = outgoingAmount;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

void TrustLine::setBalance(callback managersCallback, const balance_value &nodeBalance) {
    mBalance = nodeBalance;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

const NodeUUID &TrustLine::getContractorNodeUUID() const{
    return mContractorNodeUuid;
}

const trust_amount &TrustLine::getIncomingTrustAmount() const{
    return mIncomingTrustAmount;
}

const trust_amount &TrustLine::getOutgoingTrustAmount() const{
    return mOutgoingTrustAmount;
}

const balance_value &TrustLine::getBalance() const{
    return mBalance;
}

