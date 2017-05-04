#ifndef GEO_NETWORK_CLIENT_NOTIFICATIONTRUSTLINECREATEDMESSAGE_H
#define GEO_NETWORK_CLIENT_NOTIFICATIONTRUSTLINECREATEDMESSAGE_H

#include "NotificationTrustLineRemovedMessage.h"


class NotificationTrustLineCreatedMessage:
    public NotificationTrustLineRemovedMessage {

public:
    typedef shared_ptr<NotificationTrustLineCreatedMessage> Shared;

public:
    using NotificationTrustLineRemovedMessage::NotificationTrustLineRemovedMessage;

    virtual const MessageType typeID() const
        noexcept;
};

#endif //GEO_NETWORK_CLIENT_NOTIFICATIONTRUSTLINECREATEDMESSAGE_H
