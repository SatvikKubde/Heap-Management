#include <stdio.h>
#include <stdlib.h>

// Structure for memory blocks
typedef struct MemoryBlock {
    int address;  // Starting address of the memory block
    int size;     // Size of the block in bytes
    void* memory; // Pointer to allocated memory
    struct MemoryBlock* next; // Pointer to next block in the list
} MemoryBlock;

void* heapMemory;          // Simulated heap memory
MemoryBlock* freeBlocks;    // Linked list of free memory blocks
MemoryBlock* allocatedBlocks; // Linked list of allocated memory blocks

// Function declarations
void displayList(MemoryBlock* list);
void displayMemoryStatus();
void initializeHeap();
void cleanHeap();
void* allocateMemory(int size);
void mergeFreeBlocks();
void freeMemory(void* memory);

// Function to display memory blocks in a given list (either free or allocated)
void displayList(MemoryBlock* list) {
    MemoryBlock* ptr = list;
    printf("Address\tSize\n");
    while (ptr != NULL) {
        printf("%d\t%d\n", ptr->address, ptr->size);
        ptr = ptr->next;
    }
    printf("\n");
}

// Function to display both free and allocated memory status
void displayMemoryStatus() {
    printf("\nFree Memory Blocks\n");
    displayList(freeBlocks);
    printf("Allocated Memory Blocks\n");
    displayList(allocatedBlocks);
}

// Initialize heap memory and create a single free memory block
void initializeHeap() {
    heapMemory = malloc(1024); // Simulating a heap of 1024 bytes
    if (!heapMemory) {
        printf("Error: Failed to allocate heap memory!\n");
        exit(1);
    }

    // Creating the initial free memory block
    freeBlocks = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!freeBlocks) {
        printf("Error: Failed to allocate memory block!\n");
        free(heapMemory);
        exit(1);
    }

    allocatedBlocks = NULL;    // Initially, no memory is allocated
    freeBlocks->next = NULL;   // Single free block initially
    freeBlocks->address = 0;   // Starting address
    freeBlocks->size = 1024;   // Full heap size
    freeBlocks->memory = NULL; // No actual memory assigned yet
}

// Function to clean up memory before exiting
void cleanHeap() {
    free(heapMemory);  // Free the simulated heap memory

    // Free all memory blocks in the free list
    MemoryBlock* ptr = freeBlocks;
    while (ptr != NULL) {
        MemoryBlock* temp = ptr;
        ptr = ptr->next;
        free(temp);
    }

    // Free all memory blocks in the allocated list
    ptr = allocatedBlocks;
    while (ptr != NULL) {
        MemoryBlock* temp = ptr;
        ptr = ptr->next;
        free(temp);
    }
}

// Function to allocate memory using First-Fit strategy
void* allocateMemory(int size) {
    if (size <= 0) {
        printf("Error: Cannot allocate zero or negative bytes!\n");
        return NULL;
    }
    
    MemoryBlock* ptr = freeBlocks, * prev = NULL;

    // Traverse the free list to find a block that is large enough
    while (ptr != NULL && size > ptr->size) {
        prev = ptr;
        ptr = ptr->next;
    }

    // If no suitable block is found, return NULL
    if (ptr == NULL) return NULL;

    // Allocate a new memory block for the requested size
    MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!newBlock) {
        printf("Error: Memory allocation failed for new block!\n");
        return NULL;
    }
    
    // Assign properties to the new block
    newBlock->size = size;
    newBlock->memory = (char*)heapMemory + ptr->address; // Assign memory from heap
    newBlock->address = ptr->address;
    newBlock->next = NULL;

    // Adjust the free block's address and size
    ptr->address += size;
    ptr->size -= size;

    // If the free block has been completely used, remove it from the list
    if (ptr->size == 0) {
        if (prev == NULL) {
            freeBlocks = ptr->next;
        } else {
            prev->next = ptr->next;
        }
        free(ptr);
    }

    // Add the newly allocated block to the allocatedBlocks list
    if (allocatedBlocks == NULL) {
        allocatedBlocks = newBlock;
    } else {
        MemoryBlock* temp = allocatedBlocks;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newBlock;
    }

    return newBlock->memory;
}

// Function to merge adjacent free memory blocks to reduce fragmentation
void mergeFreeBlocks() {
    MemoryBlock* ptr = freeBlocks;
    while (ptr != NULL && ptr->next != NULL) {
        // If two consecutive blocks are adjacent in memory, merge them
        if (ptr->address + ptr->size == ptr->next->address) {
            ptr->size += ptr->next->size; // Extend the current block size
            MemoryBlock* temp = ptr->next;
            ptr->next = temp->next;
            free(temp); // Free the merged block
        } else {
            ptr = ptr->next;
        }
    }
}

// Function to deallocate previously allocated memory
void freeMemory(void* memory) {
    if (!memory) {
        printf("Error: Attempted to free a NULL pointer!\n");
        return;
    }

    MemoryBlock* ptr1 = allocatedBlocks, *prev1 = NULL;

    // Find the allocated block in the allocated list
    while (ptr1 != NULL && ptr1->memory != memory) {
        prev1 = ptr1;
        ptr1 = ptr1->next;
    }

    // If the memory is not found in the allocated list, return an error
    if (ptr1 == NULL) {
        printf("Error: Attempted to free unallocated memory!\n");
        return;
    }

    // Remove the block from the allocated list
    if (prev1 == NULL) {
        allocatedBlocks = ptr1->next;
    } else {
        prev1->next = ptr1->next;
    }

    // Insert the freed block back into the free list in sorted order
    MemoryBlock* prev = NULL, *curr = freeBlocks;

    while (curr != NULL && curr->address < ptr1->address) {
        prev = curr;
        curr = curr->next;
    }

    // Check if the block is already in the free list (to prevent double-free)
    if (curr != NULL && curr->address == ptr1->address) {
        printf("Warning: Memory block already freed!\n");
        free(ptr1);
        return;
    }

    // Insert the block into the free list at the correct position
    ptr1->next = curr;
    if (prev == NULL) {
        freeBlocks = ptr1;
    } else {
        prev->next = ptr1;
    }

    // Merge adjacent free blocks to avoid fragmentation
    mergeFreeBlocks();
}

// Main function to run the memory management system
int main() {
    initializeHeap();
    void* allocatedMemory[20]; // Array to keep track of allocated memory pointers
    int allocationCount = 0;
    int choice;

    do {
        printf("\nChoose an option:\n");
        printf("1. Allocate Memory\n2. Free Memory\n3. Display Memory Status\n4. Exit\n");
        scanf("%d", &choice);

        if (choice == 1) {
            int size;
            printf("Enter memory size to allocate:\n");
            scanf("%d", &size);
            allocatedMemory[allocationCount] = allocateMemory(size);
            if (allocatedMemory[allocationCount] != NULL) {
                printf("Memory allocated at index %d\n", allocationCount);
                ++allocationCount;
            } else {
                printf("Allocation failed. Insufficient memory.\n");
            }
        } else if (choice == 2) {
            int index;
            printf("Enter index of memory to free:\n");
            scanf("%d", &index);
            if (index >= 0 && index < allocationCount) {
                freeMemory(allocatedMemory[index]);
                allocatedMemory[index] = NULL;  // Mark as freed
            } else {
                printf("Invalid index.\n");
            }            
        } else if (choice == 3) {
            displayMemoryStatus();
        }
    } while (choice != 4);

    cleanHeap();
    return 0;
}
