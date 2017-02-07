#include "Exception.h"

Exception::Exception(const string &message):

    mMessage(message),
    msg_(mMessage.c_str()){}

const string Exception::message() const {

    return mMessage;
}
