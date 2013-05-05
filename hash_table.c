#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include "hash_function.h"

/* 

   Hash Table functions 
   Created on 6.23.1996
   By Srinidhi Varadarajan
   
   Virginia Tech

*/


/*

Creates a hash table and returns a pointer to it

   Parameters:
   hash_table_size : size of the hash table to create

   Returns: Pointer to created hash table or NULL

*/

struct hash_table_type **create_hash_table(unsigned long hash_table_size)
{
  int t;
  struct hash_table_type **hash_table;

  hash_table = (struct hash_table_type **) 
    malloc(sizeof(struct hash_table_type *) * (hash_table_size + 2));
  if (hash_table == NULL) return(NULL);
  
  for (t=0; t<=hash_table_size; t++)
    hash_table[t] = NULL;

  return(hash_table);
}

/* 
   Inserts a structure into the hash table. 
   
   Parameters : 
   hash_table : Hash table to use
   hash_table_size : Size of the hash table.
   key : key to index the hash table
   key_len: length of the hash key in bytes
   data : Structure to insert
      
   Returns: 0 if key exists in the hash table  
            1 if key was inserted into the table
	   -1 if key could not be inserted into the table;
				  
*/

   
int  hash_insert(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		 unsigned char *key, int key_len, void *data)
{
  unsigned long hash_key;
  struct hash_table_type *ptr, *new_entry, *prev_ptr;

  hash_key  = hash(key, key_len, 7);
  hash_key %= hash_table_size;
 
  ptr = hash_table[hash_key];
  prev_ptr = NULL;
  while (ptr != NULL)
    {
      if (memcmp(ptr->key, key, key_len) == 0) return(0);
      prev_ptr = ptr;
      ptr = ptr->next;
    }
  
  new_entry = (struct hash_table_type *) malloc(sizeof(struct hash_table_type));
  if (new_entry == NULL) return(-1);
  
  new_entry->next = NULL;
  new_entry->prev = prev_ptr;
  if (prev_ptr == NULL)
    hash_table[hash_key] = new_entry;
  else
    prev_ptr->next = new_entry;
  
  new_entry->key = (char *) malloc(key_len);
  if (new_entry->key == NULL)
    {
      printf("Warning: Unable to allocate memory for hash key. \n");
      free(new_entry);
      return(-1);
    }
  
  memcpy(new_entry->key, key, key_len);
  new_entry->data = data;
  return(1);
}

/* 
   Deletes a hash table entry. 
   
   Parameters : 
   hash_table : Hash table to use
   hash_table_size : Size of the hash table.
   key : Key to index the hash table
   key_len: Length of the key in bytes
   
   Returns: 1 if key was successfully deleted  
	   -1 if key could not be deleted (key was not found)
				  
*/
int hash_delete(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		void *key, int key_len)
{
  unsigned long hash_key;
  struct hash_table_type *ptr, *prev_ptr;
  
  hash_key  = hash(key, key_len, 7);
  hash_key %= hash_table_size;
 
  ptr = hash_table[hash_key];
  prev_ptr = NULL;

  while (ptr != NULL)
    {
      if (memcmp(ptr->key, key, key_len) == 0) 
	{
	  if (prev_ptr == NULL) // First entry
	    hash_table[hash_key] = ptr->next;
	  else
	    prev_ptr->next = ptr->next;

	  free(ptr->key);
	  free(ptr);
	  return(1);
	}
      prev_ptr = ptr;
      ptr = ptr->next;
    }

  return(-1);
}


/*
  Finds the entry corrsponding to key in the hash table

  Parameters:
  hash_table : Pointer to the hash table to use
  hash_table_size : Size of the hash table
  key : Key to index the hash table.
  key_len: Length of the key in bytes
  
  Returns: 
  Pointer to the data field in the hash table on success
  NULL on failure
*/

void *hash_find(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		void *key, int key_len)
{
  unsigned long hash_key;
  struct hash_table_type *ptr;

  hash_key  = hash(key, key_len, 7);
  hash_key %= hash_table_size;
 
  ptr = hash_table[hash_key];
  while (ptr != NULL)
    {
      if (memcmp(ptr->key, key, key_len) == 0) return(ptr->data);
      ptr = ptr->next;
    }

  return(NULL);
}

/*
  Destroy hash table and free all allocated memory

  Parameters:
  hash_table : Pointer to the hash table to use
  hash_table_size : Size of the hash table
  
  Returns : nothing
*/

inline void destroy_hash_table(struct hash_table_type **hash_table, unsigned long hash_table_size)
{
  unsigned long t, count, max_count=0, tot_count = 0;
  struct hash_table_type *cur_ptr, *next_ptr;

  for (t=0; t<=hash_table_size; t++) 
    {
      if (hash_table[t] != NULL)
	{
	  cur_ptr = hash_table[t];
	  next_ptr = cur_ptr->next;
	  count = 1;
	  while (next_ptr != NULL)
	    {
	      free(cur_ptr->key);
	      free(cur_ptr);
	      cur_ptr = next_ptr;
	      next_ptr = cur_ptr->next;
	      count++;
	    }
	  free(cur_ptr->key);
	  free(cur_ptr);
	  hash_table[t] = NULL;
	  tot_count += count;
	  if (count > max_count) max_count = count;
	}
    }
  printf("Max collision list entries: %lu. Total: %lu\n", max_count, tot_count);
  free(hash_table);
}






