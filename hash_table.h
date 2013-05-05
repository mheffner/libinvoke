#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

struct hash_table_type
{
  void *key;
  void *data;
  struct hash_table_type *next; 
  struct hash_table_type *prev;
};


/* function definitions */

struct hash_table_type **create_hash_table(unsigned long hash_table_size);
int  hash_insert(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		 unsigned char *key, int key_len, void *data);
int hash_delete(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		void *key, int key_len);
void *hash_find(struct hash_table_type **hash_table, unsigned long hash_table_size, 
		void *key, int key_len);
void destroy_hash_table(struct hash_table_type **hash_table, unsigned long hash_table_size);

#endif
