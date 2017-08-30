#include "src/core/Core.h"

//#define TESTS


#ifdef UNIT_TESTS
#define TESTS__DB__UUID_COLUMN
#define TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#define TESTS__ROUTING_TABLE
#define TESTS__TRUSTLINES
#endif

// todo: move this into separate if..def
//#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
//#include "src/tests/parseInterface/OperationsLogTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockRecordTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockTests.cpp"
//#include "src/tests/db/uuid_column/UUIDMapColumnTests.cpp"


#ifdef TESTS__DB__UUID_COLUMN
#include "src/tests/db/fields/uuid_column/UUIDColumnTests.cpp"
#endif

#ifdef TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#include "src/tests/db/fields/tl_direction_column/TrustLineDirectionColumnTests.cpp"
#endif


#ifdef TESTS__ROUTING_TABLE
#include "src/tests/io/routing_tables/OperationsLogTests.cpp"
#include "src/tests/io/routing_tables/operations/SetOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/RemoveOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/DirectionUpdateOperationTests.cpp"
#include "src/tests/io/routing_tables/RoutingTableTests.cpp"
#endif

#ifdef TESTS__TRUSTLINES
#include "src/tests/trust_lines/TrustLineTests.cpp"
#endif


//#include <boost/endian/arithmetic.hpp>

//vector<byte> TLAmountToBytes(
//    const TrustLineAmount &amount) {

//    vector<byte> rawExportedBytesBuffer, resultBytesBuffer;

//    // Exporting bytes of the "amount".
//    rawExportedBytesBuffer.reserve(kTrustLineAmountBytesCount);
//    export_bits(amount, back_inserter(rawExportedBytesBuffer), 8);

//    // Prepending received bytes by zeroes until 32 bytes would be used.
//    resultBytesBuffer.reserve(kTrustLineAmountBytesCount);

//    size_t unusedBytesCount = kTrustLineAmountBytesCount - rawExportedBytesBuffer.size();
//    for (size_t i = 0; i < unusedBytesCount; ++i) {
//        resultBytesBuffer.push_back(0);
//    }

//    size_t usedBytesCount = rawExportedBytesBuffer.size();
//    for (size_t i = 0; i < usedBytesCount; ++i) {
//        resultBytesBuffer.push_back(
//            // Casting each byte to big endian makes the deserializer independent
//            // from current machine architecture, and, as result - platfrom portable.
//            boost::endian::native_to_big(
//                rawExportedBytesBuffer[i]));
//    }

//    return resultBytesBuffer;
//}

//vector<byte> TLBalanceToBytes(
//    const TrustLineBalance &balance) {

//    vector<byte> rawExportedBytesBuffer, resultBytesBuffer;

//    // Exporting bytes of the "balance".
//    rawExportedBytesBuffer.reserve(kTrustLineAmountBytesCount);
//    export_bits(balance, back_inserter(rawExportedBytesBuffer), 8);

//    // Prepending received bytes by zeroes until 32 bytes would be used.
//    resultBytesBuffer.reserve(kTrustLineAmountBytesCount + 1);

//    size_t unusedBytesCount = kTrustLineAmountBytesCount - rawExportedBytesBuffer.size();
//    for (size_t i = 0; i < unusedBytesCount; ++i) {
//        resultBytesBuffer.push_back(0);
//    }

//    size_t usedBytesCount = rawExportedBytesBuffer.size();
//    for (size_t i = 0; i < usedBytesCount; ++i) {
//        resultBytesBuffer.push_back(
//            // Casting each byte to big endian makes the deserializer independent
//            // from current machine architecture, and, as result - platfrom portable.
//            boost::endian::native_to_big(
//                rawExportedBytesBuffer[i]));
//    }

//    // Process sign
//    resultBytesBuffer.insert(
//        resultBytesBuffer.begin(),
//        boost::endian::native_to_big(
//            byte(balance > 0)));

//    return resultBytesBuffer;
//}

//TrustLineAmount bytesToTLAmount(
//    const vector<byte> &amountBytes) {

//    vector<byte> internalBytesBuffer;
//    internalBytesBuffer.reserve(kTrustLineAmountBytesCount);

//    for (size_t i=0; i<kTrustLineAmountBytesCount; ++i) {
//        internalBytesBuffer.push_back(
//            boost::endian::big_to_native(
//                amountBytes[i]));
//    }

//    TrustLineAmount amount;
//    import_bits(
//        amount,
//        internalBytesBuffer.begin(),
//        internalBytesBuffer.end());

//    return amount;
//}

//TrustLineBalance bytesToTLBalance(
//    const vector<byte> &balanceBytes) {

//    vector<byte> internalBytesBuffer;
//    internalBytesBuffer.reserve(kTrustLineAmountBytesCount);

//    // Note: sign byte must be skipped, so the cycle is starting from 1.
//    for (size_t i=1; i<kTrustLineAmountBytesCount+1; ++i) {
//        internalBytesBuffer.push_back(
//            boost::endian::big_to_native(
//                balanceBytes[i]));
//    }

//    TrustLineBalance balance;
//    import_bits(
//        balance,
//        internalBytesBuffer.begin(),
//        internalBytesBuffer.end());

//    // Sign must be processed only in case if balance != 0.
//    // By default, after deserialization, balance is always positive,
//    // so it must be only checked for > 0, and not != 0.
//    if (balance > 0) {
//        byte sign = boost::endian::big_to_native(balanceBytes[0]);
//        if (sign == 0) {
//            balance = balance * -1;
//        }
//    }

//    return balance;
//}


int main(int argc, char** argv) {
    // todo: include other tests here

/*#ifdef TESTS__DB__UUID_COLUMN
    UUIDColumnTests tests;
    tests.run();
#endif

#ifdef TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
    TrustLineDirectionColumnTests tests;
    tests.run();
#endif

#ifdef TESTS__ROUTING_TABLE
    {
        OperationsLogTests tests;
        tests.run();
    }
    {
        SetOperationsTests tests;
        tests.run();
    }
    {
        RemoveOperationsTests tests;
        tests.run();
    }
    {
        DirectionUpdateOperationsTests tests;
        tests.run();
    }

    {
        AbstractRoutingTableTests tests;
        tests.run();
    }

#endif



#ifndef TESTS

#endif*/

/*#ifdef TESTS__TRUSTLINES
    TrustLineTests tests;
    tests.run();
#endif*/

////    TrustLineAmount a("1111222233334444555566667777888899990000");
//    TrustLineBalance a("+0");
//    auto exported = TLBalanceToBytes(a);
//    auto imported = bytesToTLBalance(exported);

//    cout << a << endl << imported << endl;

    return Core(argv[0]).run();
}
