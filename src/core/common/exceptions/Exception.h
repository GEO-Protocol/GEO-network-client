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
            msg_(message) {};

    const std::string message() const;

    virtual const char* what() const throw(){
        if (!mMessage.empty()) {
            return mMessage.c_str();
        }
        return msg_.c_str();
    }

protected:
    std::string msg_;

private:
    std::string mMessage;
};


#endif //GEO_NETWORK_CLIENT_EXCEPTIONS_H
