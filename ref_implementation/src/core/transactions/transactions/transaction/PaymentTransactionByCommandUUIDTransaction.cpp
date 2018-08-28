/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "PaymentTransactionByCommandUUIDTransaction.h"

PaymentTransactionByCommandUUIDTransaction::PaymentTransactionByCommandUUIDTransaction(
    NodeUUID &nodeUUID,
    PaymentTransactionByCommandUUIDCommand::Shared command,
    BaseTransaction::Shared requestedTransaction,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::TransactionByCommandUUIDType,
        nodeUUID,
        logger),
    mCommand(command),
    mRequestedPaymentTransaction(requestedTransaction)
{}

PaymentTransactionByCommandUUIDCommand::Shared PaymentTransactionByCommandUUIDTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst PaymentTransactionByCommandUUIDTransaction::run()
{
    stringstream stream;
    if (mRequestedPaymentTransaction == nullptr) {
        stream << "0";
        info() << "Requested transaction not found";
    } else {
        stream << "1" << kTokensSeparator
               << mRequestedPaymentTransaction->currentTransactionUUID().stringUUID();
        info() << "Requested transaction found";
    }
    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(result));
}

const string PaymentTransactionByCommandUUIDTransaction::logHeader() const
{
    stringstream s;
    s << "[PaymentTransactionByCommandUUIDTA: " << currentTransactionUUID() << "]";
    return s.str();
}
