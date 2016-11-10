#ifndef GEO_NETWORK_CLIENT_CONFLICTERROR_H
#define GEO_NETWORK_CLIENT_CONFLICTERROR_H

#include "Exception.h"


class ConflictError: public Exception {
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_CONFLICTERROR_H
