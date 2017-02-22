#include "TrustLineTests.h"
#include "../../core/settings/Settings.h"


void TrustLineTests::create_trustlineCase() {

    TrustLineAmount incoming_amount = 350;
    TrustLineAmount outgoing_amount = 150;
    TrustLineBalance current_balance = -230;
//    Create trustline with balance bigger than range
    bool false_created_flag = true;
    TrustLine *trustLine_range_out = nullptr;
    TrustLine *trustline_range_inc = nullptr;
    try {
        trustLine_range_out = new TrustLine(
                contractor1,
                incoming_amount,
                outgoing_amount,
                current_balance);
        current_balance = 550;
        trustline_range_inc = new TrustLine(
                contractor1,
                incoming_amount,
                outgoing_amount,
                current_balance
        );
    }
    catch (ValueError){
        false_created_flag = false;
    };
//    Check zero trustline
    false_created_flag = true;
    TrustLine *trustLine_zero = nullptr;
    incoming_amount = 0;
    outgoing_amount = 0;
    current_balance = 0;
    try {
        trustLine_zero = new TrustLine(
                contractor1,
                incoming_amount,
                outgoing_amount,
                current_balance);
    }
    catch (ValueError){
        false_created_flag = false;
    };
    assert(false_created_flag && "Created trustline with zero balance");
}


void TrustLineTests::change_parametersCase(){
    //    Change change correct balance to range exctened value
    TrustLine *trustLine_correct = nullptr;
    TrustLineAmount incoming_amount = 100;
    TrustLineAmount outgoing_amount = 100;
    TrustLineBalance current_balance =  50;
    bool false_created_flag = false;
    trustLine_correct = new TrustLine(
            contractor1,
            incoming_amount,
            outgoing_amount,
            current_balance);
    try{
        trustLine_correct->setOutgoingTrustAmount(TrustLineAmount(20));
    }
    catch (...){
        false_created_flag = true;
    }
    assert(false_created_flag && "Change outgoing amount for value lesser than abs(balance)");
    false_created_flag = false;
    current_balance = -50;
    trustLine_correct->setBalance(current_balance);
    try {
        trustLine_correct->setIncomingTrustAmount(20);
    }
    catch (...){
        false_created_flag = true;
    }
    assert(false_created_flag && "Incoming amount lesser than  abs(negative balance)");
}


void TrustLineTests::check_balanceCase(){
    TrustLineAmount incoming_amount = 350;
    TrustLineAmount outgoing_amount = 150;
    TrustLineBalance current_balance = 100;
    ConstSharedTrustLineAmount available_amount;
    ConstSharedTrustLineBalance balance;
    TrustLine *trustLine_1 = nullptr;
    TrustLine *trustLine_2 = nullptr;

    trustLine_1 = new TrustLine(
            contractor1,
            incoming_amount,
            outgoing_amount,
            0);
//  check available amount with trustline without balance
    assert(*trustLine_1->availableAmount() == incoming_amount);
//  add balance to trustline
    trustLine_1->setBalance(current_balance);
    assert(*trustLine_1->availableAmount() == incoming_amount - current_balance );
//  increase incoming amount
    incoming_amount = incoming_amount + 100;
    trustLine_1->setIncomingTrustAmount(incoming_amount);
    assert(*trustLine_1->availableAmount() == incoming_amount - current_balance );
//    use negative balance
    current_balance  = -100;
    trustLine_1->setBalance(current_balance);
    assert(*trustLine_1->availableAmount() == incoming_amount - current_balance );

    trustLine_1->setBalance(current_balance);
    assert(*trustLine_1->availableAmount() == incoming_amount - current_balance );
}


void TrustLineTests::check_serializeCase(){
    TrustLineAmount incoming_amount = 350;
    TrustLineAmount outgoing_amount = 150;
    TrustLineBalance current_balance = 100;
    ConstSharedTrustLineAmount available_amount;
    ConstSharedTrustLineBalance balance;
//    vector<byte> buffer_byte;
    TrustLine *trustLine_1 = nullptr;

    trustLine_1 = new TrustLine(
            contractor1,
            incoming_amount,
            outgoing_amount,
            current_balance);

    available_amount = trustLine_1->availableAmount();
//    todo: finixh this moment
//    buffer_byte  = trustLine_1->serialize();
//    trustLine_1->deserialize(*buffer_byte);
    assert(*available_amount == *trustLine_1->availableAmount());
}


void TrustLineTests::run() {
//    uuid r_uuid = boost::uuids::random_generator()();
//    cout << r_uuid << endl;
//    create_trustlineCase();
//    change_parametersCase();
//    create_trustlineCase();
//    check_serializeCase();
}