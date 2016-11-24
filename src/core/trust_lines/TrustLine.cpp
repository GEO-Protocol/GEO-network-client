#include "TrustLine.h"

TrustLine::TrustLine(const uuids::uuid &nodeUUID,
                     const trust_amount &incomingAmount,
                     const trust_amount &outgoingAmount,
                     const balance_value &nodeBalance) {
    mContractorNodeUuid = nodeUUID;
    mIncomingTrustAmount = incomingAmount;
    mOutgoingTrustAmount = outgoingAmount;
    mBalance = nodeBalance;
}

void TrustLine::setContractorNodeUUID(const uuids::uuid &nodeUUID) {
    mContractorNodeUuid = nodeUUID;
}

void TrustLine::setIncomingTrustAmount(callback managersCallback, const trust_amount &incomingAmount) {
    mIncomingTrustAmount = incomingAmount;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

void TrustLine::setOutgoingTrustAmount(callback managersCallback, const trust_amount &outgoingAmount) {
    mOutgoingTrustAmount = outgoingAmount;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

void TrustLine::setBalance(callback managersCallback, const balance_value &nodeBalance) {
    mBalance = nodeBalance;
    mManagerCallback = managersCallback;
    mManagerCallback();
}

uuids::uuid &TrustLine::getContractorNodeUUID() {
    return mContractorNodeUuid;
}

trust_amount &TrustLine::getIncomingTrustAmount() {
    return mIncomingTrustAmount;
}

trust_amount &TrustLine::getOutgoingTrustAmount() {
    return mOutgoingTrustAmount;
}

balance_value &TrustLine::getBalance() {
    return mBalance;
}

