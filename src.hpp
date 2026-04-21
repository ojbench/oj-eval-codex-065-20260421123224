// Problem 065 - A Naïve but Block-based Allocator
// Submission header implementing Allocator with 4096-byte block strategy.

#include <cstddef>
#include <vector>

// Provided by judge
int* getNewBlock(int n);
void freeBlock(const int* block, int n);

class Allocator {
public:
    Allocator() = default;

    ~Allocator() {
        // Free all blocks we ever acquired
        for (const auto& b : blocks_) {
            freeBlock(b.base, b.n_blocks);
        }
    }

    /**
     * Allocate a sequence of memory space of n int.
     */
    int* allocate(int n) {
        if (n <= 0) return nullptr;
        const int ints_per_block = 4096 / static_cast<int>(sizeof(int));

        // Use remaining tail of the last block if sufficient
        if (!blocks_.empty()) {
            Block& last = blocks_.back();
            if (last.used + n <= last.capacity) {
                int* p = last.base + last.used;
                last.used += n;
                return p;
            }
        }

        // Reuse any completely free block that is large enough
        for (Block& b : blocks_) {
            if (b.used == 0 && b.capacity >= n) {
                b.used = n;
                return b.base;
            }
        }

        // Acquire a new block sized to fit n ints (rounded up to 4096-byte multiples)
        int need_blocks = (n + ints_per_block - 1) / ints_per_block;
        int cap = need_blocks * ints_per_block;
        int* base = getNewBlock(need_blocks);
        Block nb{base, need_blocks, cap, n};
        blocks_.push_back(nb);
        return base;
    }

    /**
     * Deallocate the memory that is allocated by the allocate member function.
     */
    void deallocate(int* pointer, int n) {
        if (pointer == nullptr || n <= 0) return;
        // Locate which block this pointer belongs to and try tail-pop if possible
        for (Block& b : blocks_) {
            if (pointer >= b.base && pointer < b.base + b.capacity) {
                if (pointer + n == b.base + b.used) {
                    // Tail deallocation enables reuse
                    b.used -= n;
                }
                return;
            }
        }
        // If not found, undefined behavior per spec; ignore.
    }

private:
    struct Block {
        int* base = nullptr;
        int n_blocks = 0;   // number of 4096-byte blocks backing this block
        int capacity = 0;   // ints capacity in this block
        int used = 0;       // ints currently used from the start (simple bump pointer)
    };

    std::vector<Block> blocks_;
};

