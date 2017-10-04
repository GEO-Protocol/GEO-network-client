#ifndef GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H
#define GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H

#include "Exception.h"


class PreconditionFailedError: public Exception {
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H
