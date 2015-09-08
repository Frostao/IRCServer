
//
// Implementation of a HashTable that stores void *
//
#include "HashTableVoid.h"
#include <stdio.h>
// Obtain the hash code of a key
int HashTableVoid::hash(const char * key)
{
  // Add implementation here
  int sum = 0;
  while(*key) {
    sum += *key;
    key++;
  }
  return sum%TableSize;
}

// Constructor for hash table. Initializes hash table
HashTableVoid::HashTableVoid()
{
  _buckets = (HashTableVoidEntry **)malloc(TableSize*sizeof(HashTableVoidEntry *));
  for (int i = 0;i < TableSize; i++)
  {
    _buckets[i] = NULL;
    /* code */
  }
  // Add implementation here
       
}

// Add a record to the hash table. Returns true if key already exists.
// Substitute content if key already exists.
bool HashTableVoid::insertItem( const char * key, void * data)
{
  int h = hash(key);
  HashTableVoidEntry *e = _buckets[h];
  while(e!= NULL) {
    if (!strcmp(e->_key,key))
    {
      e->_data = data;
      return true;
    }
    e = e->_next;
  }
  e = new HashTableVoidEntry;
  e->_key = strdup(key);
  e->_data = data;
  e->_next = _buckets[h];
  _buckets[h] = e;
  // Add implementation here
  return false;
}

// Find a key in the dictionary and place in "data" the corresponding record
// Returns false if key is does not exist
bool HashTableVoid::find( const char * key, void ** data)
{
  int h = hash(key);
  HashTableVoidEntry *e = _buckets[h];
  while(e!=NULL) {
    if (!strcmp(e->_key,key))
    {
      *data = e->_data;
      return true;
    }
  }
  // Add implementation here
  return false;
}

// Removes an element in the hash table. Return false if key does not exist.
bool HashTableVoid::removeElement(const char * key)
{
  // Add implementation here
  int h = hash(key);
  HashTableVoidEntry * e = _buckets[h];
  HashTableVoidEntry * prev = NULL;
  while (e!=NULL) {
    if (!strcmp(e->_key, key)) {
      if (prev != NULL) {
        prev->_next = e->_next;
      }
      else {
        _buckets[h] = e->_next;
      }
      //free(e->_key);
      delete e;
      return true;
    }
    prev = e;
    e = e->_next;
  }
  return false;
}

// Creates an iterator object for this hash table
HashTableVoidIterator::HashTableVoidIterator(HashTableVoid * hashTable)
{
  // Add implementation here
  _hashTable = hashTable;
  _currentBucket = 0;
  _currentEntry = _hashTable->_buckets[_currentBucket];
}

// Returns true if there is a next element. Stores data value in data.
bool HashTableVoidIterator::next(const char * & key, void * & data)
{
  // Add implementation here
  // for (; _currentBucket < _hashTable->TableSize; _currentBucket++)
  // {
  //   _currentEntry = _hashTable->_buckets[_currentBucket];
  //   while(_currentEntry!=NULL){
  //     if (_currentEntry->_data != NULL)
  //     {
  if(_currentEntry != NULL){

    data = _currentEntry->_data;
    key = _currentEntry->_key;
    //printf("%s\n", (char*)data);
    //_currentBucket++;
    _currentEntry = _currentEntry->_next;
    return true;
  } else {
    //_currentEntry is null
    //printf("%d\n", _currentBucket);
    if (_currentBucket == _hashTable->TableSize)
    {
      return false;
    }
    while(_currentEntry == NULL && _currentBucket < _hashTable->TableSize) {
      
      _currentEntry = _hashTable->_buckets[_currentBucket];
      _currentBucket++;
    }
    if (_currentEntry == NULL)
    {
        return false;
    }
    data = _currentEntry->_data;
    key = _currentEntry->_key;
    //printf("%ld\n", (long)data);
    _currentEntry = _currentEntry->_next;
    return true;

    // while(_currentEntry !=NULL) {
    //   data = _currentEntry->_data;
    //   key = _currentEntry->_key;
    //   _currentBucket++;
    //   _currentEntry = _hashTable->_buckets[_currentBucket];
    //   return true;
    // }
  }
  
  
  
}

