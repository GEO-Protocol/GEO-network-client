#ifndef GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H
#define GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H

#include "Exception.h"


class PreconditionFaultError: public Exception {
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_PRECONDITIONFAULTERROR_H
