#include "Block.h"

namespace db {
    namespace uuid_map_block_storage {

        Block::Block(byte *data, const size_t bytesCount) {
            mData = data;
            mBytesCount = bytesCount;
        }

        Block::~Block() {
            free(mData);
        }

        const byte *Block::data() const {
            return mData;
        }

        const size_t Block::bytesCount() const {
            return mBytesCount;
        }

    }
}