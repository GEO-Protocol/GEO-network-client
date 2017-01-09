#include "Exception.h"

Exception::Exception(const std::string &message):
    mMessage(message),
    msg_(mMessage.c_str()){}

const std::string Exception::message() const {
    return mMessage;
}
