#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void easytest(BuddyAllocator *ba)
{
  // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte
  char *mem = ba->alloc(1);
  char *meme = ba->alloc(10);
  char *memem = ba->alloc(190);
  // now print again, how should the list look now
  ba->printlist();

  ba->free(meme); // give back the memory you just allocated
  ba->free(memem);
  ba->free(mem);
  ba->printlist(); // shouldn't the list now look like as in the beginning
}

int main(int argc, char **argv)
{

  int basic_block_size = 128, memory_length = 512;
  int opt;
  int pass_in;

  while ((opt = getopt(argc, argv, "b:s:")) != -1)
    switch (opt)
    {
    case 'b':
      pass_in = atoi(optarg);
      if (ceil(log2(pass_in)) == floor(log2(pass_in)) && pass_in > 0)
        basic_block_size = pass_in;
      else
        cout << "Invalid basic block size, reverting to default block size (128)" << endl
             << endl;
      break;
    case 's':
      pass_in = atoi(optarg);
      if (ceil(log2(pass_in)) == floor(log2(pass_in)) && pass_in > 0)
        memory_length = pass_in;
      else
        cout << "Invalid total memory size, reverting to default memory size (512)" << endl
             << endl;
      break;
    }

  if (basic_block_size > memory_length)
  {
    cout << "Total memory length not a multiple of basic block size" << endl;
    cout << "Reverting to default memory sizes" << endl
         << endl;
    basic_block_size = 128;
    memory_length = 512;
  }

  // create memory manager
  BuddyAllocator *allocator = new BuddyAllocator(basic_block_size, memory_length);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest(allocator);

  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman *am = new Ackerman();
  am->test(allocator); // this is the full-fledged test.

  // destroy memory manager
  delete allocator;
}
