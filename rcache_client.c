// Client-side test program for the rCache system

#include <stdio.h>
#include "rcache.h"

int main(int argc, char** argv) {

  char response[1024];
  
  rInsert("a", "one");
  rInsert("b", "two");
  rInsert("c", "three");
  rPrint();
  printf("\n");
  
  //Add more tests here...
  
  //Testing all Lookups
  rLookup("a", response);
  printf("Found: %s\n", response);
  rLookup("b", response);
  printf("Found: %s\n", response);
  rLookup("c", response);
  printf("Found: %s\n", response);
  
  //Testing all Removes
  rRemove("a", response);
  printf("Removed: %s\n", response);
  rRemove("b", response);
  printf("Removed: %s\n", response);
  rRemove("c", response);
  printf("Removed: %s\n", response);
  
  printf("\n");
  printf("Values removed.\n");
  
  rPrint();
  
  //Testing NULL checks...
  rLookup("a", response);
  printf("Found: %s\n", response);
  
  rLookup("b", response);
  printf("Found: %s\n", response);
  
  rRemove("a", response);
  printf("Removed: %s\n", response);
  
  //Testing multiple removes on one bucket
  rInsert("a", "one");
  rInsert("a", "two");
  rInsert("a", "three");
  rPrint();
  
  rLookup("a", response);
  printf("Found: %s\n", response);
  
  rRemove("a", response);
  printf("Removed: %s\n", response);
  rRemove("a", response);
  printf("Removed: %s\n", response);
  rRemove("a", response);
  printf("Removed: %s\n", response);
  
  rPrint();
  
  
  return 0;
}
