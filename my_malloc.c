#include <stdio.h>
#include "my_malloc.h"
#include <stdio.h>
#include <time.h>

/*
 * First Fit malloc
 * find the first larger block in freelist. If cannot find, use sbrk
 */
void * ff_malloc(size_t size){
    // for corner case
    if(size == 0){
        return NULL;
    }
    //printf("ff_malloc\n");
    if(freeList_headPtr  == NULL){
        return request_newBlock(size);
    }
    meta * curPtr = freeList_headPtr;
    while(curPtr != NULL){
        if(curPtr -> size > size + metaSize){   //if(curPtr -> size > size + metaSize){
            return split_oldBlock(curPtr, size); //the after size can be split
        }else if(curPtr -> size >= size ){
            deleteFromFreeList(curPtr); //the after size cannot be split
            return curPtr -> afterMeta; //return ptr to after block
        }
        curPtr = curPtr -> next;
    }
    //cannot find big enough block
    return request_newBlock(size);
}

void *request_newBlock(size_t size) {
    data_segment_size  = data_segment_size + size + metaSize;
    //sbrk space: size + metaSize
    meta * newBlock_withMeta = sbrk(size + metaSize);
    newBlock_withMeta -> prev = NULL;
    newBlock_withMeta -> next = NULL;
    newBlock_withMeta -> size = size;
    newBlock_withMeta -> afterMeta = (char *)newBlock_withMeta  + metaSize;
    //printf("request_newBlock, sbrk new block, ptr =  %p\n", newBlock_withMeta);

    return newBlock_withMeta -> afterMeta;
}

void *split_oldBlock(meta * curPtr, size_t size) {
    void * mallocBlock_ptr = curPtr -> afterMeta;
    size_t leftSize = curPtr ->size - size - metaSize; // left size
    meta * newMeta_ptr = (meta *)((char *)mallocBlock_ptr + size); // point to new meta

    //information for left size
    void * newSize_ptr = (void *)((char *)newMeta_ptr + metaSize);
    newMeta_ptr -> afterMeta = newSize_ptr;
    newMeta_ptr -> size = leftSize;

    //update size of curPtr
    curPtr -> size = size;

    //delete curPtr from free list
    deleteFromFreeList(curPtr);

    //add back to free list
    addBackToFreeList(newMeta_ptr);

    //malloc point to return
    return mallocBlock_ptr;
}


void deleteFromFreeList(meta * curPtr){
    //printf("deleteFromFreeList\n");
    //curPtr is tail
    if(curPtr -> next == NULL && curPtr -> prev != NULL){
        freeList_tailPtr = curPtr -> prev; //update tail
        curPtr -> prev  = NULL;
        curPtr->next = NULL;
        freeList_tailPtr -> next = NULL;
    }
        //curPtr is head
    else if(curPtr -> prev == NULL &&  curPtr -> next != NULL){
        freeList_headPtr = curPtr -> next; //update head
        curPtr ->next = NULL;
        curPtr->prev = NULL;
        freeList_headPtr -> prev = NULL;
    }
        //free list only has curPtr
    else if(curPtr -> prev == NULL && curPtr -> next == NULL){
        freeList_tailPtr = NULL;
        freeList_headPtr = NULL; //update head and tail
        curPtr -> next = NULL;
        curPtr -> prev = NULL;
    }
        //curPtr is in the freeList
    else if(curPtr -> prev != NULL && curPtr -> next != NULL){
        curPtr -> prev -> next = curPtr -> next;
        curPtr -> next -> prev = curPtr -> prev;
        curPtr -> next = NULL;
        curPtr -> prev = NULL;
    }
}

void addBackToFreeList(meta * newMeta_ptr) {//compare, then add, then merge
    if (freeList_headPtr == NULL) {   //freeList_headPtr == NULL
        freeList_headPtr = newMeta_ptr;
        freeList_tailPtr = newMeta_ptr;
        newMeta_ptr->prev = NULL;
        newMeta_ptr->next = NULL;
        return;
    }
    if (newMeta_ptr < freeList_headPtr) {  //freeList_headPtr != NULL, make newMeta_ptr to be the new head
        newMeta_ptr->next = freeList_headPtr;
        freeList_headPtr->prev = newMeta_ptr;
        newMeta_ptr->prev = NULL;
        freeList_headPtr = newMeta_ptr;
        return;
    }

    if (newMeta_ptr > freeList_tailPtr) {  //freeList_headPtr != NULL, make newMeta_ptr to be the new tail
        newMeta_ptr->prev = freeList_tailPtr;
        freeList_tailPtr->next = newMeta_ptr;
        newMeta_ptr->next = NULL;
        freeList_tailPtr = newMeta_ptr;
        return;
    }

    //   head  < curPtr < tail
    meta * curPtr = freeList_headPtr;

    while (curPtr != NULL) {
        if (curPtr == newMeta_ptr) {
            printf("double free\n");
        }
        if (curPtr < newMeta_ptr) {  //lager than head, head-> next, head next next;
            curPtr = curPtr -> next;
        }else{ //head > new
            // printf("break here\n");
            break;
        }
    }

    meta *prevPtr = curPtr->prev;
    prevPtr -> next = newMeta_ptr;
    newMeta_ptr -> next = curPtr;
    curPtr -> prev = newMeta_ptr;
    newMeta_ptr -> prev = prevPtr;

}



void mergeBlock(meta * metaPtr){
    if ((metaPtr->next != NULL) && ((char *)metaPtr + metaPtr->size + metaSize == (char *)metaPtr->next)) {
        metaPtr->size = metaPtr->size + metaSize + metaPtr->next->size;
        deleteFromFreeList(metaPtr->next);
    }

    if ((metaPtr->prev != NULL) &&((char *)metaPtr->prev + metaPtr->prev->size + metaSize == (char *)metaPtr)) {
        metaPtr->prev->size = metaPtr->prev->size + metaSize + metaPtr->size;
        deleteFromFreeList(metaPtr);
    }

}

void ff_free(void *ptr) {
    if(ptr == NULL){
        return;
    }

    meta *metaPtr = (meta *) ((char *) ptr - metaSize);  //find the metaPtr address

    addBackToFreeList(metaPtr);

    mergeBlock(metaPtr);

}


//Best Fit malloc：find a suitable size block in freelist. If cannot find, use sbrk
void *bf_malloc(size_t size){
    if(size == 0){
        return NULL;
    }

    if(freeList_headPtr  == NULL){
        return request_newBlock(size);
    }
    int bestFitExist = 0;
    meta * curPtr = freeList_headPtr;
    meta * bestPtr = freeList_headPtr;
    unsigned int minDiff = UINT_MAX;
    while(curPtr != NULL){
        if(curPtr -> size >= size){
            bestFitExist = 1;
            size_t curDiff = curPtr -> size - size;
            if(curDiff == 0){
                bestPtr = curPtr;
                break;
            }
            if(curDiff < minDiff){
                minDiff = curDiff;
                bestPtr = curPtr;
            }
        }
        curPtr = curPtr -> next;
    }

    if(bestFitExist == 0){
        return request_newBlock(size);//cannot find larger block in free list
    }

    if(bestPtr -> size > size + metaSize){
        return split_oldBlock(bestPtr, size); //size after meta ptr can be split
    }else{  //bestPtr -> size >= size
        deleteFromFreeList(bestPtr); //size after meta ptr can not be split
        data_segment_free_space_size = data_segment_free_space_size - bestPtr -> size - metaSize; //after malloc，delete space from free data
        return bestPtr -> afterMeta; //return ptr to block
    }
}

//bf free
void bf_free(void *ptr){
    return ff_free(ptr);  //same with bf_free
}

// Performance study
unsigned long get_data_segment_size(){
    return data_segment_size;
}

//search the whole free list to find the data_segment_free_space_size
unsigned long get_data_segment_free_space_size() {
    if(freeList_headPtr == NULL){
        return 0;
    }

    size_t freeSize = 0;
    meta * curPtr = freeList_headPtr;
    while(curPtr != NULL) {
        freeSize += curPtr->size + metaSize;
        curPtr = curPtr -> next;
    }
    return freeSize;
}


