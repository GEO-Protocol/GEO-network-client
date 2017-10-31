#ifndef GEO_NETWORK_CLIENT_EXCEPTIONS_H
#define GEO_NETWORK_CLIENT_EXCEPTIONS_H

#include <string>

using namespace std;

// todo: add exception parameters info (source code line, subsystem, etc)
class Exception : public std::exception {
public:
    virtual ~Exception() throw(){}

    explicit Exception(const std::string &message);
    explicit Exception(const char *message) :
        mMessage(message) {};

    const std::string message() const;

    virtual const char* what() const throw(){
        return mMessage.c_str();
    }

private:
    std::string mMessage;
};


#endif //GEO_NETWORK_CLIENT_EXCEPTIONS_H
