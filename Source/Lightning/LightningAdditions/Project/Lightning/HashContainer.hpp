#pragma once
#ifndef LIGHTNING_HASH_CONTAINER_HPP
#define LIGHTNING_HASH_CONTAINER_HPP

namespace Lightning
{
  typedef Pair<Any, Any> AnyKeyValue;

  class PlasmaShared AnyHashMap : public HashMap<Any, Any>
  {
  public:
    // Constructor
    AnyHashMap();

    // Invalidates all active ranges via incrementing a modification id
    void Modified();

    // A special counter that we use to denote whenever the container has been modified
    Integer ModifyId;
  };

  class PlasmaShared AnyHashMapRange
  {
  public:

    AnyHashMapRange();

    // The hash map range that we contain
    AnyHashMap::range Range;

    // The original range allows us to revert when Reset is called
    AnyHashMap::range OriginalRange;

    // A handle back to the source container that our data belongs to
    Handle HashMap;

    // The id that the container had when we were created from it
    Integer ModifyId;

    // Moves to the next element in the range
    void MoveNext();

    // Resets the range back to the original position
    void Reset();

    // Checks if there are no more elements left within the range
    bool IsEmpty();

    // Checks if there are still elements left within the range
    bool IsNotEmpty();
  };
  
  // The user-data we attach to the hash map class (every template instantiation)
  class PlasmaShared HashMapUserData
  {
  public:
    HashMapUserData();

    Type* KeyType;
    Type* ValueType;
    BoundType* PairRangeType;
    BoundType* ValueRangeType;
    BoundType* KeyRangeType;
  };

  namespace HashMapRangeMode
  {
    enum Enum
    {
      Pair,
      Value,
      Key
    };
  }
}

#endif
