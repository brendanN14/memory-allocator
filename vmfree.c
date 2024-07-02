#include "vm.h"
#include "vmlib.h"

/**
 * The vmfree() function frees the memory space pointed to by ptr,
 * which must have been returned by a previous call to vmmalloc().
 * Otherwise, or if free(ptr) has already been called before,
 * undefined behavior occurs.
 * If ptr is NULL, no operation is performed.
 */
void vmfree(void *ptr)
{
    // Check validity of pointer
    if (ptr == NULL) {
        return;
    }

    // Creates header pointer that is at ptr's header
    struct block_header *header =
        (struct block_header *)((char *)ptr - sizeof(struct block_header));
    // Gets size of the block
    size_t blockSz = BLKSZ(header);

    // Check if block already free
    if (!(header->size_status & VM_BUSY)) {
        return;
    }

    // Unset busy bit
    header->size_status &= ~VM_BUSY;

    // Get's previous block's footer
    struct block_footer *prevFooter =
        (struct block_footer *)((char *)header - sizeof(struct block_footer));
    // Declare previous pointer to previous header
    struct block_header *prevHeader = NULL;
    size_t prevSz;

    // Check is previous footer is valid
    if ((char *)prevFooter != (char *)heapstart && prevFooter->size > 0) {
        // Gets previous header and subsequent size
        prevHeader = (struct block_header *)((char *)header - prevFooter->size);
        prevSz = BLKSZ(prevHeader);
    }

    // Check if next block is free
    struct block_header *nextHeader =
        (struct block_header *)((char *)header + blockSz);
    int nextFree = !(nextHeader->size_status & VM_BUSY);

    // Check if previous block is free
    int prevFree = (prevHeader != NULL && !(prevHeader->size_status & VM_BUSY));

    // Get sizes of next block
    size_t nextSz = BLKSZ(nextHeader);

    // COALESCING
    if (prevFree && nextFree) {
        // CASE 1: Coalesce with both previous and next blocks
        size_t totalSz = prevSz + blockSz + prevSz;

        prevHeader->size_status = totalSz;
        struct block_footer *footer =
            (struct block_footer *)((char *)prevHeader + totalSz -
                                    sizeof(struct block_footer));
        footer->size = totalSz;
    } else if (prevFree) {
        // CASE 2: Coalesce with previous block
        size_t totalSz = prevSz + blockSz;
        struct block_footer *footer =
            (struct block_footer *)((char *)prevHeader + totalSz -
                                    sizeof(struct block_footer));

        prevHeader->size_status = totalSz;
        footer->size = totalSz;
    } else if (nextFree) {
        // CASE 3: Coalesce with next block
        size_t totalSz = blockSz + nextSz;
        struct block_footer *footer =
            (struct block_footer *)((char *)header + totalSz -
                                    sizeof(struct block_footer));

        header->size_status = totalSz;
        footer->size = totalSz;
    } else {
        // CASE 4: Just setting current block to free
        struct block_footer *footer =
            (struct block_footer *)((char *)header + blockSz -
                                    sizeof(struct block_footer));
        footer->size = blockSz;
    }
}
