# malloc_library

## Motivation
Deep the knowledge in computer system. 

## Thoughts
Train of this project is use a Linked List recording all the pointers pointing to a structure.The
pointers in the linked list order by an address ascending order.The structure has three pointers
and an integer.For more convenience to do operate such as merge, delete, add and so on, the
Linked List should be a bidirectional Linked list. It stores the free block information and the
pointers before or after it. Another pointer in it record the really address of the free block. Because
the size of a block combination comprised by size of the structure and the real free size. Thus,
there are two pointers. The first one is the pointers which pointing to the structure which is in the
Linked list and the second one is the pointers which points to the real free size start address.


