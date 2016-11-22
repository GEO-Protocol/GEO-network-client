#include "TrustLinesManagerTest.h"

/*void TrustLinesManagerTest::openSuccessTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->open(contractor1, 50));  //кейс на відкриття лінії в сторону контрагента 1

    assert(trustLinesManager->open(contractor2, 100)); //кейс на відкриття лінії в сторону контрагента 2

    delete trustLinesManager;
}

void TrustLinesManagerTest::openFailureTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    //assert(!trustLinesManager->open(contractor1, -50)); //В змінну типу checked_uint256_t неможливо присвоїти від’ємне значення

    assert(!trustLinesManager->open(contractor1, 0)); //кейс на нульове значення вихідної лінії

    assert(trustLinesManager->open(contractor1, 50)); //кейс на перевідкриття лінії
    assert(!trustLinesManager->open(contractor1, 60));

    delete trustLinesManager;
}

void TrustLinesManagerTest::acceptSuccessTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->accept(contractor1, 50));  //кейс на прийняття лінії від контрагента 1

    assert(trustLinesManager->accept(contractor2, 100)); //кейс на прийняття лінії від контрагента 2

    delete trustLinesManager;
}

void TrustLinesManagerTest::acceptFailureTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    //assert(!trustLinesManager->open(contractor1, -50)); //В змінну типу checked_uint256_t неможливо присвоїти від’ємне значення

    assert(!trustLinesManager->accept(contractor1, 0)); //кейс на нульове значення вхідної лінії

    assert(trustLinesManager->accept(contractor1, 50)); //кейс на прийняття лінії ще раз
    assert(!trustLinesManager->accept(contractor1, 60));

    delete trustLinesManager;
}

void TrustLinesManagerTest::closeSuccessTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->open(contractor1, 50)); //кейс на закриття лінії довіри до контрагента
    assert(trustLinesManager->open(contractor2, 100));
    assert(trustLinesManager->close(contractor1));
    assert(trustLinesManager->close(contractor2));

    delete trustLinesManager;
}

void TrustLinesManagerTest::closeFailureTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->open(contractor1, 50)); //кейс на подвійне закриття лінії довіри до контрагента
    assert(trustLinesManager->close(contractor1));
    assert(!trustLinesManager->close(contractor1));

    assert(!trustLinesManager->close(contractor2));//кейс на закриття лінії довіри до неіснуючого контрагента

    delete trustLinesManager;
}

void TrustLinesManagerTest::rejectSuccessTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->accept(contractor1, 50)); //кейс на відхилення лінії довіри від контрагента
    assert(trustLinesManager->accept(contractor2, 100));
    assert(trustLinesManager->reject(contractor1));
    assert(trustLinesManager->reject(contractor2));

    delete trustLinesManager;
}

void TrustLinesManagerTest::rejectFailureTestCase() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();

    assert(trustLinesManager->accept(contractor1, 50)); //кейс на подвійне відхилення лінії довіри від контрагента
    assert(trustLinesManager->reject(contractor1));
    assert(!trustLinesManager->reject(contractor1));

    assert(!trustLinesManager->reject(contractor2));//кейс на відхилення лінії довіри від неіснуючого контрагента

    delete trustLinesManager;
}*/

void TrustLinesManagerTest::run() {
    boost::uuids::uuid contractor1 = boost::uuids::random_generator()();
    boost::uuids::uuid contractor2 = boost::uuids::random_generator()();
    TrustLinesManager *trustLinesManager = new TrustLinesManager();
    trustLinesManager->open(contractor1, 50);
    trustLinesManager->open(contractor2, 100);
    trustLinesManager->accept(contractor2, 200);
    trustLinesManager->close(contractor1);
    trustLinesManager->reject(contractor2);
    delete trustLinesManager;

}


