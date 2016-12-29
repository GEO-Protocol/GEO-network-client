#ifndef GEO_NETWORK_CLIENT_RUNTIMEERROR_H
#define GEO_NETWORK_CLIENT_RUNTIMEERROR_H

#include "Exception.h"


class RuntimeError:
    public Exception {

public:
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_RUNTIMEERROR_H
