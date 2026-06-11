#include <iostream>
#include <cstring>
#include <chrono>
#include <fstream> //file handling 
#define HEAP_MEMORY  1024*1024 //constant number heap defined here which is 1MB
using namespace std;
static char heap[HEAP_MEMORY]; //// raw 1MB memory pool  our simulated heap, managed entirely by us 
static bool initialised = false;
struct BlockHeader 
{
    size_t size; //specified for memory sizes could never be negative 
    bool is_free;
    BlockHeader *next; //pointer pointing towards the next block 
};
void init_heap() //heap initialised 
{
    BlockHeader* first = (BlockHeader*)heap; //pointer poitnting towards the first block that described the current situation of memory
    first->size = HEAP_MEMORY-sizeof(BlockHeader);
    first->is_free = true; //bool redeemed as true to show the memory allocated is free right now 
    first->next = nullptr;
}
void *my_malloc(size_t size) //memory allocated in this  fucntion 
{
    BlockHeader *current = (BlockHeader*)heap; //starting position 
    if(initialised == false) //want to setup the first free giant array || only be called once 
    {
        init_heap();
        initialised = true; //it is made true so that the condition only works once 
    }
        while(current != nullptr) //condition to check we reach the end of array or not
        {
            if(current->is_free == true && current->size >= size) //for each block as 2 questions 1.is it free | 2. is it big enough
            {
                if(current->size >= size + sizeof(BlockHeader) + 1) //can we split this block
                {
                    //Place a new header right after the user's requested bytes
                    //New block gets the leftover size, marked free
                    //Current block gets resized to exactly what was asked, marked used
                    //Return pointer to memory right after current's header
                BlockHeader* new_block = (BlockHeader*)((char*) current + sizeof(BlockHeader) + size);
                new_block->size = current->size - size - sizeof(BlockHeader);
                new_block->is_free = true;
                new_block->next = current->next;
                current->size = size;
                current->is_free = false;
                current->next = new_block;
                return (void*)(current+1);
                }
                else
                {
                    // just give them the whole block, no split
                    current->is_free = false;
                    return (void*)(current+1);
                }
            }
            else 
            {
                current = current->next;
            }
        }
        return nullptr; //if loop finishes , return null ptr because no space is left 
}
void my_free(void*ptr) //frees the used up memory and coalesing 
{
    BlockHeader *header = (BlockHeader*)(ptr)-1; // going back 1 place to free the memory as the memory freed is actually just behind the pointer being used 
    header->is_free = true;
    if (header->next != nullptr && header->next->is_free == true && (char*)header->next > (char*)header)
    {  //// coalescing if the next block is also free, merge both into one larger block
        header->size = header->size + sizeof(BlockHeader)+ header->next->size;
        header->next = header->next->next;
    }
}
void print_heap() //prints everything in the current memory blocks 
{
    cout << "--------------------------------------" << endl;
    BlockHeader *current = (BlockHeader*)heap;
    while(current != nullptr)
    {
        cout << "ADDRESS:  " << current << endl;
        cout << "SIZE:   " << current->size << endl;
        if (current->is_free == false)
        {
            cout << "THE CURRENT BLOCK IS NOT FREE AS OF YET" << endl;
        }
        else 
        {
            cout << "THE CURRENT MEMORY BLOCK IS FREE " << endl;
        }
        current=  current->next;
    }
    cout << "--------------------------------------" << endl;
}
void* my_realloc(void* ptr, size_t new_size)
{
    if (ptr == nullptr)
    {
        return my_malloc(new_size);
    }
    if(new_size == 0)
    {
        my_free(ptr);
        return nullptr;
    }
    void *newarray = my_malloc(new_size);//initialising the new array where every element has to be copied 
    BlockHeader* header = (BlockHeader*)ptr - 1;
    size_t copy_size;
    if (header->size < new_size) //a saefty precaution to copy which size is smaller and to copy the smaller one into a larger one 
    {
        copy_size = header->size;
    }
    else
    {
        copy_size = new_size;
    }
    memcpy(newarray, ptr,copy_size); //fucntion to copy everything from orignal array to new array for resizing 
    my_free(ptr); //freeing the previous memory the is of no use now 
    return newarray; //returning the new memory allocation 
}
void print_stats() //printing the number of free and used blocks as well as free and used bytes using a while loop until end of array
{
    cout << "--------------------------------------" << endl;
    BlockHeader *current = (BlockHeader*)heap;
    int free = 0 ;
    int used = 0;
    size_t free_memo = 0;
    size_t used_memo = 0;
    while(current != nullptr)
    {
        if(current->is_free == true)
        {
            free++;
            free_memo+=current->size;
        }
        else if(current->is_free == false)
        {
            used++;
            used_memo+= current->size;
        }
        current=  current->next;
    }
    cout << endl;
    cout << "the number of free bytes is:  " << free_memo << endl; //printing everything 
    cout << "the number of used bytes is:  " << used_memo << endl;
    cout << "number of free blocks:  " << free << endl;
    cout << "the number of used blocks:  " << used << endl;
    cout << "total memory:  " << HEAP_MEMORY << endl;
    cout << "--------------------------------------" << endl;
}
void benchmark() //measures the time taken for my memory allocator to run 
{
    auto start = chrono::high_resolution_clock::now(); //starting clock now 
    for (int i= 0; i< 10000 ; i++)
    {
        void* ptr = my_malloc(64); //ptr made and calling my malloc for memory allocation 
        my_free(ptr); // freeeing memory
    }
    auto end = chrono::high_resolution_clock::now(); //ending clock after the loop ends 
    cout << endl;
    cout << "time for my allocator to run is:  " << chrono::duration<double>(end - start).count() << endl;
    //calculating time for actual malloc and free to start and end 
    auto start2 = chrono::high_resolution_clock::now();
    for (int i = 0; i< 10000; i++)
    {
        void* ptr = malloc(64); //actual malloc and free working in C++ under the hood 
        free(ptr);
    }
    auto end2 = chrono::high_resolution_clock::now(); //ending clock 
    cout << "time for actual c++ allocator to run is:  " << chrono::duration<double>(end2 - start2).count() << endl;
    ofstream file("results.csv"); //file made to store the times into file so that python could read this 
    file << "allocator,time" << endl;
    file << "my_malloc," << chrono::duration<double>(end - start).count() << endl;
    file << "standard_malloc," << chrono::duration<double>(end2 - start2).count() << endl;
    file.close();
}
int main () //test everything 
{
    int* ptr = (int*)my_malloc(100); //pointer is made here 
    my_free(ptr); //pointer memory of ptr is freed 
    cout << "Allocated at address: " << ptr << endl; //address printed 
    int *ptr1 = (int*)my_malloc(50); //ptr1 and ptr2 allocating memory fo 50 Bytes  (new)
    int *ptr2 = (int*)my_malloc(50);
    cout << "first:  " << ptr1 << endl; //adress printed 
    cout << "second:  " << ptr2 << endl;
    my_free(ptr1); //memory freed (delete)
    my_free(ptr2);
    int *ptr3 = (int*)my_malloc(80); //memory of 80 bytes is taken from char array
    cout << "third:  " << ptr3<< endl; //address
    cout << "adress of ptr1:  " << ptr1;
    int* ptr4 = (int*)my_malloc(50); //memory of 50 bytes is taken 
    cout << "before realloc: " << ptr4 << endl;
    int* ptr5 = (int*)my_realloc(ptr4, 200); //the block of 50 bytes expanded useing realloc 
    cout << "after realloc: " << ptr5 << endl;
    print_heap(); //prints everything 
    print_stats();
    benchmark();
}