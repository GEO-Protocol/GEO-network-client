#ifndef GEO_NETWORK_CLIENT_MEMORYERROR_H
#define GEO_NETWORK_CLIENT_MEMORYERROR_H

#include "Exception.h"


class MemoryError: public Exception {
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_MEMORYERROR_H
