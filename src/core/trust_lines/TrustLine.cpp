#include "TrustLine.h"

TrustLine::TrustLine(uuids::uuid nodeUUID,
                     trust_amount incomingAmount,
                     trust_amount outgoingAmount,
                     balance_value nodeBalance){
    contractor_node_uuid = nodeUUID;
    incoming_trust_amount = incomingAmount;
    outgoing_trust_amount = outgoingAmount;
    balance = nodeBalance;
}

void TrustLine::setContractorNodeUUID(uuids::uuid nodeUUID) {
    contractor_node_uuid = nodeUUID;
}
void TrustLine::setIncomingTrustAmount(callback managersCallback, trust_amount incomingAmount) {
    incoming_trust_amount = incomingAmount;
    managerCallback = managersCallback;
    managersCallback();
}
void TrustLine::setOutgoingTrustAmount(callback managersCallback, trust_amount outgoingAmount) {
    outgoing_trust_amount = outgoingAmount;
    managerCallback = managersCallback;
    managersCallback();
}
void TrustLine::setBalance(callback managersCallback, balance_value nodeBalance) {
    balance = nodeBalance;
    managerCallback = managersCallback;
    managersCallback();
}
uuids::uuid TrustLine::getContractorNodeUUID() {
    return contractor_node_uuid;
}
trust_amount TrustLine::getIncomingTrustAmount() {
    return incoming_trust_amount;
}
trust_amount TrustLine::getOutgoingTrustAmount() {
    return outgoing_trust_amount;
}
balance_value TrustLine::getBalance() {
    return balance;
}

