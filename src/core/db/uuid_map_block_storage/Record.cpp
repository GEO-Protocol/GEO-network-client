#include "Record.h"

namespace db {
    namespace uuid_map_block_storage {

        Record::Record(byte *data, const size_t bytesCount) {
            mData = data;
            mBytesCount = bytesCount;
        }

        Record::~Record() {
            free(mData);
        }

        const byte *Record::data() const {
            return mData;
        }

        const size_t Record::bytesCount() const {
            return mBytesCount;
        }

    }
}