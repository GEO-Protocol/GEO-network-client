#ifndef GEO_NETWORK_CLIENT_EXCEPTIONS_H
#define GEO_NETWORK_CLIENT_EXCEPTIONS_H

#include <string>

// todo: add exception parameters info (source code line, subsystem, etc)
class Exception {
public:
    Exception(const std::string &message);
    Exception(const char *message);

    const std::string message() const;

private:
    std::string mMessage;
};


#endif //GEO_NETWORK_CLIENT_EXCEPTIONS_H
