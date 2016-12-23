#include "Operation.h"


namespace io {
namespace routing_tables {


Operation::Operation(Operation::OperationType type):
    mType(type){}

const Operation::OperationType Operation::type() const {
    return mType;
}


} // routing_tables
} // io