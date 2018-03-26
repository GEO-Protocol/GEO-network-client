/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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

private:
    CommandUUID mPaymentTransactionCommandUUID;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDCOMMAND_H
