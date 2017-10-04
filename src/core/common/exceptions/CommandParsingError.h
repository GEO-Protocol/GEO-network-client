#ifndef GEO_NETWORK_CLIENT_COMMANDPARSINGERROR_H
#define GEO_NETWORK_CLIENT_COMMANDPARSINGERROR_H

#include "Exception.h"

// todo: replace with ValueError
class CommandParsingError : public Exception {
    using Exception::Exception;
};

#endif //GEO_NETWORK_CLIENT_COMMANDPARSINGERROR_H
