#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "TrustLinesManagerTest.h"

void TrustLinesManagerTest::run() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();
    cout << trustLinesManager->open(uuid, 500) << endl;
    cout << trustLinesManager->accept(uuid, 1000) << endl;
    cout << trustLinesManager->close(uuid) << endl;
    cout << trustLinesManager->reject(uuid) << endl;
}
