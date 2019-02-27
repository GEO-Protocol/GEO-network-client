#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H

#include "ConfirmationCachedResponseMessage.h"
#include "../../../../logger/LoggerMixin.hpp"
#include "../../internal/common/Types.h"

#include <map>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>

using namespace std;
namespace as = boost::asio;

class ConfirmationResponseMessagesHandler : LoggerMixin {

public:
    ConfirmationResponseMessagesHandler(
        IOService &ioService,
        Logger &logger);

    void addCachedMessage(
        ContractorID contractorID,
        TransactionMessage::Shared cachedMessage,
        Message::MessageType incomingMessageTypeFilter,
        uint32_t cacheLivingTime);

    Message::Shared getCachedMessage(
        TransactionMessage::Shared incomingMessage);

protected:
    const string logHeader() const
    noexcept;

    /**
     * @returns timestamp, when next timer awakeness must be performed.
     * This method checks all queues and returns the smallest one time duration,
     * between now and queue timeout.
     */
    const DateTime closestLegacyCacheTimestamp() const
    noexcept;

    /**
     * Schedules timer for next awakeness.
     */
    void rescheduleResending();

    /**
     * Clear legacy caches
     * This method would be called every time when some queue timeout would fire up.
     */
    void clearLegacyCacheMessages();

private:
    map<pair<SerializedEquivalent, ContractorID>, ConfirmationCachedResponseMessage::Shared> mCachedMessages;
    IOService &mIOService;
    as::steady_timer mCleaningTimer;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H
