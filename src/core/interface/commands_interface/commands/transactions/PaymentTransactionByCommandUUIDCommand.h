#ifndef GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDCOMMAND_H
#define GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class PaymentTransactionByCommandUUIDCommand : public BaseUserCommand {

public:
    typedef shared_ptr<PaymentTransactionByCommandUUIDCommand> Shared;

public:
    PaymentTransactionByCommandUUIDCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &transactionUUIDStr) const;

    const CommandUUID &paymentTransactionCommandUUID() const;

protected:
    [[deprecated("Remove it when parent class would be updated")]]
    void parse(
        const string &_){}

private:
    CommandUUID mPaymentTransactionCommandUUID;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDCOMMAND_H
