#include "TrustLinesManagerTests.h"

TrustLinesManagerTest::TrustLinesManagerTest() {
    mTrustLinesManager = new TrustLinesManager();
}

void TrustLinesManagerTest::openSuccessTestCase() {
    mTrustLinesManager->open(contractor1, 10543);
    mTrustLinesManager->open(contractor2, 29841);

}

void TrustLinesManagerTest::acceptSuccessTestCase() {
    mTrustLinesManager->accept(contractor2, 50000);
    mTrustLinesManager->accept(contractor3, 7854);
    mTrustLinesManager->accept(contractor4, 80);
}


void TrustLinesManagerTest::closeSuccessTestCase() {
    mTrustLinesManager->close(contractor1);
}

void TrustLinesManagerTest::rejectSuccessTestCase() {
    mTrustLinesManager->reject(contractor2);
    mTrustLinesManager->reject(contractor4);
}

void TrustLinesManagerTest::deletePointer() {
    delete mTrustLinesManager;
}

void TrustLinesManagerTest::showAllTrustLines() {
    deletePointer();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();
    for(auto const &it : trustLinesManager->mTrustLines){
        TrustLine::Shared tl = trustLinesManager->getTrustLineByContractorUUID(it.first);
        cout << "Contractor " << tl.get()->getContractorNodeUUID().stringUUID() << endl;
        cout << "Outgoing trust amount " << tl.get()->getOutgoingTrustAmount() << endl;
        cout << "Incoming trust amount " << tl.get()->getIncomingTrustAmount() << endl;
        cout << "Balance " << tl.get()->getBalance() << endl;
        cout << "-------------------------------------------------------" <<endl;
    }
    delete trustLinesManager;
}


void TrustLinesManagerTest::run() {
    //openSuccessTestCase();
    //acceptSuccessTestCase();
    //closeSuccessTestCase();
    //rejectSuccessTestCase();
    showAllTrustLines();
}
