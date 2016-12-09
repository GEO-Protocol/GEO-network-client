#include "TrustLinesManagerTest.h"

void TrustLinesManagerTest::openSuccessTestCase() {
    NodeUUID nodeUUID;
    NodeUUID nodeUUID1;
    TrustLinesManager *trustLinesManager = new TrustLinesManager();
    trustLinesManager->open(nodeUUID, 100);
    trustLinesManager->open(nodeUUID1, 200);
    trustLinesManager->accept(nodeUUID1, 300);
    delete trustLinesManager;
}

void TrustLinesManagerTest::acceptSuccessTestCase() {
    TrustLinesManager *trustLinesManager = new TrustLinesManager();
    for(auto const &it : trustLinesManager->mTrustLines){
        TrustLine *tl = trustLinesManager->getTrustLineByContractorUUID(it.first);
        cout << tl->getContractorNodeUUID().stringUUID() << endl;
        cout << tl->getOutgoingTrustAmount() << endl;
        cout << tl->getIncomingTrustAmount() << endl;
        cout << tl->getBalance() << endl;
        cout << "-------------------------------------------------------" <<endl;
    }
    delete trustLinesManager;
}


void TrustLinesManagerTest::closeSuccessTestCase() {

}

void TrustLinesManagerTest::rejectSuccessTestCase() {

}


void TrustLinesManagerTest::run() {
    openSuccessTestCase();
    acceptSuccessTestCase();
}


