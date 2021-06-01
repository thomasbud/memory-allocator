#include "BuddyAllocator.h"
#include <iostream>
using namespace std;

BuddyAllocator::BuddyAllocator(int _basic_block_size, int _total_memory_length)
{
  basic_block_size = _basic_block_size;
  total_memory_size = _total_memory_length;

  //allocate the memory please
  addr = new char[total_memory_size];

  int divisions = log2(total_memory_size / basic_block_size); //math to figure out how many times, added benefit of rounding down if it doesn't divide evenly
  for (int i = 0; i <= divisions; i++)                        //<= because I want to do it one extra time
  {
    FreeList.push_back(LinkedList());
  }

  BlockHeader *b = (BlockHeader *)addr; //sets the first (whole) free block
  b->block_size = total_memory_size;
  b->next = NULL;
  b->isFree = true;
  FreeList[FreeList.size() - 1].head = b;

  for (int i = 0; i < FreeList.size() - 1; i++) //sets the rest of the free blocks to null
  {
    FreeList[i].head = NULL;
  }
}

BuddyAllocator::~BuddyAllocator()
{
  /* UNNECESSARY CODE
  for (int i = 0; i < FreeList.size(); i++)
  {
    while (FreeList[i].head)
    {
      FreeList[i].remove(FreeList[i].head);
    }
  }
  */

  delete addr;
}

// PRIVATE FUNCTS
///////////////////////////////////////////////////////////////////////

BlockHeader *BuddyAllocator::getbuddy(BlockHeader *adr)
{
  //get the buffer and subtract by it
  int adjusted = (char *)adr - (char *)addr;
  //bitwise xor
  int temp_addr = adjusted xor adr->block_size;
  //re-add buffer
  char *ret = addr + temp_addr;
  return (BlockHeader *)ret;
}

bool BuddyAllocator::arebuddies(BlockHeader *block1, BlockHeader *block2)
{
  //check condition to see if they are free and if they're the same size
  return (block1->isFree && block2->isFree && (block1->block_size == block2->block_size));
  //that's it
}

BlockHeader *BuddyAllocator::merge(BlockHeader *block1, BlockHeader *block2)
{
  //remove both from the list below
  int lv = log2(block1->block_size / basic_block_size);
  FreeList[lv].remove(block1);
  FreeList[lv].remove(block2);
  //check which is the smaller address
  BlockHeader *ret;
  if (block1 > block2)
  {
    ret = block2;
    ret->next = block1->next;
  }
  else
  {
    ret = block1;
    ret->next = block2->next;
  }
  //change the initial pointer
  ret->block_size = (ret->block_size) * 2;
  //add it to the list above
  FreeList[lv + 1].insert(ret);
  //do we need to delete the middle address? recast it to char perhaps?
  return ret;
}

BlockHeader *BuddyAllocator::split(BlockHeader *block)
{
  //calculate an intermediate point
  char *new_address = (char *)block + block->block_size / 2;

  //put in a block header at intermediate point
  BlockHeader *new_split = (BlockHeader *)new_address;
  new_split->block_size = block->block_size / 2;
  new_split->isFree = true;

  //change block header at original pointer address
  block->block_size = block->block_size / 2;
  block->isFree = true;

  int lv = log2(block->block_size / basic_block_size);
  //edit the current vector
  FreeList[lv + 1].remove(block);
  //place into next vectors
  FreeList[lv].insert(block);
  FreeList[lv].insert(new_split);
  //return first address
  return block; //could also be return block I think
}

///////////////////////////////////////////////////////////////////////

char *BuddyAllocator::alloc(int _length)
{
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  double space_needed = _length + sizeof(BlockHeader);

  //look for the smallest vector that exists which is >= to BH+B
  int smallest = ceil(log2(space_needed / basic_block_size)); //smallest level possible for us to check
  if (smallest < 0)
    smallest = 0; //it can't be negative, otherwise it will be a seg fault because negative index access
  int chunksize = basic_block_size * pow(2, smallest);
  while (FreeList[smallest].head == NULL)
  {
    smallest++;
    chunksize = chunksize * 2;

    if (smallest >= FreeList.size()) //if we can't find suitable space
    {
      cout << "We cannot find suitable space." << endl
           << endl;
      return NULL;
    }
  }

  //split it into the smallest possible chunk
  BlockHeader *here = FreeList[smallest].head;
  while (chunksize / 2 >= space_needed && chunksize / 2 >= basic_block_size) //second condition to make sure we are not going past smallest
  {
    //here = split(here);
    split(here); //marginally improves performance
    smallest--;
    chunksize = chunksize / 2;
  }

  here->isFree = false;
  FreeList[smallest].remove(here);

  return (char *)(here + 1);
}

int BuddyAllocator::free(char *_a)
{
  /* Same here! */
  BlockHeader *ret;
  BlockHeader *buddy_addr;
  //you know the address - delete and clean up
  //first get rid of the header
  char *actual_char_addr = _a - sizeof(BlockHeader);
  BlockHeader *actual_addr = (BlockHeader *)actual_char_addr;
  //change it to free
  actual_addr->isFree = true;
  //add it to free list
  FreeList[log2(actual_addr->block_size / basic_block_size)].insert(actual_addr); ///////////////
  bool go_again = true;
  while (go_again) //we need to keep merging until we can't
  {
    go_again = false;
    //find the pair
    buddy_addr = getbuddy(actual_addr);
    //verify if they're both free
    if (arebuddies(actual_addr, buddy_addr))
    {
      //merge (merge)
      ret = merge(actual_addr, buddy_addr);                           //do I have to store this somewhere doe
      if (log2(ret->block_size / basic_block_size) < FreeList.size()) //<= is 20, < is 19
      {
        go_again = true;
      }
    }
    actual_addr = ret;
  }
  return 0;
}

void BuddyAllocator::printlist()
{
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  for (int i = 0; i < FreeList.size(); i++)
  {
    cout << "[" << i << "] (" << ((1 << i) * basic_block_size) << ") : "; // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader *b = FreeList[i].head;
    // go through the list from head to tail and count
    while (b)
    {
      count++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      if (b->block_size != (1 << i) * basic_block_size)
      {
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit(-1);
      }
      b = b->next;
    }
    cout << count << endl;
  }
}
