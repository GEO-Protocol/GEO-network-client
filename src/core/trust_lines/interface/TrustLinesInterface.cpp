#include "TrustLinesInterface.h"

TrustLinesInterface::TrustLinesInterface(
        TrustLinesManager *manager) :

        mManager(manager){}

TrustLinesInterface::~TrustLinesInterface() {

}

void TrustLinesInterface::open(
        const NodeUUID &contractorUUID,
        const trust_amount &amount) {

    mManager->open(
            contractorUUID,
            amount);
}

void TrustLinesInterface::accept(
        const NodeUUID &contractorUUID,
        const trust_amount &amount) {

    mManager->accept(
            contractorUUID,
            amount);
}

void TrustLinesInterface::close(
        const NodeUUID &contractorUUID) {

    mManager->close(
            contractorUUID);
}

void TrustLinesInterface::reject(
        const NodeUUID &contractorUUID) {

    mManager->reject(
            contractorUUID);
}
