#include "Exception.h"

Exception::Exception(const std::string &message)
    :mMessage(message){}

Exception::Exception(const char *message)
    :mMessage(message){}

const std::string Exception::message() const {
    return mMessage;
}
