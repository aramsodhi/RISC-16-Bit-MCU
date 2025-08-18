#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

// node constructor
void setNode(struct node* node, char* key, int value) {
    node->key = strdup(key); // Duplicate the key to avoid pointer issues
    node->value = value;
    node->next = NULL;
}

void initializeHashTable(struct hashTable* tp) {
    // default capacity
    tp->capacity = DEFAULT_CAPACITY;
    tp->numElements = 0;

    // dynamically allocate array of node pointers
    tp->arr = (struct node**)malloc(sizeof(struct node*) * tp->capacity);
    for (int i = 0; i < tp->capacity; i++) {
        tp->arr[i] = NULL;
    }
}

int hashFunction(struct hashTable* tp, char* key) {
    int sum = 0;
    int factor = 31;

    for (int i = 0; i < strlen(key); i++) {
        sum = ((sum % tp->capacity) + (((int)key[i]) * factor) % tp->capacity) % tp->capacity;
        factor = ((factor % __INT16_MAX__) * (31 % __INT16_MAX__)) % __INT16_MAX__;
    }

    return sum;
}

void insert(struct hashTable* tp, char* key, int value) {
    // get bucket index for key/value pair
    int bucketIndex = hashFunction(tp, key);

    // create new node
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node.\n");
        exit(EXIT_FAILURE);
    }
    setNode(newNode, key, value);

    // if bucket is empty, there is no collision
    if (tp->arr[bucketIndex] == NULL) {
        tp->arr[bucketIndex] = newNode;
    } else {
        // adding new node at head of linked list at bucket index
        newNode->next = tp->arr[bucketIndex];
        tp->arr[bucketIndex] = newNode;
    }

    tp->numElements++;
}

void delete(struct hashTable* tp, char* key) {
    // get bucket index for given key
    int bucketIndex = hashFunction(tp, key);

    struct node* prevNode = NULL;
    struct node* currNode = tp->arr[bucketIndex];

    while (currNode != NULL) {
        // delete if key is matched
        if (strcmp(key, currNode->key) == 0) {
            if (prevNode == NULL) {
                // head node deletion
                tp->arr[bucketIndex] = currNode->next;
            } else {
                // middle or last node deletion
                prevNode->next = currNode->next;
            }

            free(currNode->key); // Free the duplicated key
            free(currNode);
            tp->numElements--;
            return;
        }

        prevNode = currNode;
        currNode = currNode->next;
    }
}

int search(struct hashTable* tp, char* key) {
    // get bucket index for key/value pair
    int bucketIndex = hashFunction(tp, key);

    struct node* bucketHead = tp->arr[bucketIndex];

    while (bucketHead != NULL) {
        // key is found in the hash table
        if (strcmp(bucketHead->key, key) == 0) {
            return bucketHead->value;
        }

        bucketHead = bucketHead->next;
    }

    return -1; // Return -1 if the key is not found
}

void freeHashTable(struct hashTable* tp) {
    for (int i = 0; i < tp->capacity; i++) {
        struct node* currNode = tp->arr[i];
        while (currNode != NULL) {
            struct node* temp = currNode;
            currNode = currNode->next;
            free(temp->key); // Free the duplicated key
            free(temp);
        }
    }

    free(tp->arr);
}
