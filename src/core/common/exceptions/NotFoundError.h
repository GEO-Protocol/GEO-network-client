#ifndef GEO_NETWORK_CLIENT_NOTFOUNDERROR_H
#define GEO_NETWORK_CLIENT_NOTFOUNDERROR_H

#include "Exception.h"


class NotFoundError: public Exception {
    using Exception::Exception;
};


#endif //GEO_NETWORK_CLIENT_NOTFOUNDERROR_H
