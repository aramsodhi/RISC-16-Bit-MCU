#ifndef HASHTABLE_H
#define HASHTABLE_H

#define DEFAULT_CAPACITY 100

// struct definitions
struct node {
    char* key;          // Key is a string
    int value;          // Value is an integer
    struct node* next;  // Pointer to the next node (for collision handling)
};

struct hashTable {
    int numElements;    // Number of elements in the hash table
    int capacity;       // Capacity of the hash table
    struct node** arr;  // Array of pointers to nodes (buckets)
};

// function prototypes
void setNode(struct node* node, char* key, int value);
void initializeHashTable(struct hashTable*  tp);
int hashFunction(struct hashTable*  tp, char* key);
void insert(struct hashTable*  tp, char* key, int value);
void delete(struct hashTable*  tp, char* key);  // Renamed `_delete` to `delete` for consistency
int search(struct hashTable*  tp, char* key);
void freeHashTable(struct hashTable*  tp);

#endif
