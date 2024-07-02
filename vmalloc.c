#include "vm.h"
#include "vmlib.h"

void *vmalloc(size_t size)
{
    // Check validity of size
    if (size <= 0) {
        return NULL;
    }

    // Change allocation size
    size_t allocSz = ROUND_UP(size + 8, 16);

    // Initialize block to start of heap
    struct block_header *block = heapstart;

    // Stores current block size
    size_t blockSz;

    // Best fit block
    struct block_header *optmBlock = NULL;

    // Best fit size
    size_t optmSz = 9999999;

    // Traverses until end of heap
    while (block->size_status != VM_ENDMARK) {
        blockSz = BLKSZ(block);

        // Check: block size big enough + free
        if (blockSz >= allocSz && (block->size_status & VM_BUSY) == 0) {
            // Checks for best fit
            if (blockSz < optmSz) {
                optmSz = blockSz;
                optmBlock = block;
            }
        }

        // Moves the block pointer to next block
        block = (struct block_header *)((char *)block + blockSz);
    }

    // Check: Best fit block not found
    if (optmBlock == NULL) {
        return NULL;
    }

    // Splitting large blocks
    if (optmSz > allocSz && (optmSz - allocSz) % 16 == 0) {
        // Creates address of new block
        struct block_header *header =
            (struct block_header *)((char *)optmBlock + allocSz);
        // Updates size of newBlock as the free portion of the split, sets prev
        // to busy
        header->size_status = optmSz - allocSz + VM_PREVBUSY;

        struct block_footer *footer =
            (struct block_footer *)((char *)optmBlock + optmSz -
                                    sizeof(struct block_footer));
        footer->size = optmSz - allocSz;
    }

    // Updates block size adding allocSc + last bit as busy + second to last bit
    // as busy
    optmBlock->size_status =
        allocSz | (optmBlock->size_status & VM_PREVBUSY) | VM_BUSY;

    // Return: Address to payload after block header
    return (void *)(optmBlock + 1);
}
