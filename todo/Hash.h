// -*-c++-*-

/**
 *  Copyright (C) 2011
 *  University of Rochester Department of Computer Science
 *    and
 *  Lehigh University Department of Computer Science and Engineering
 *
 * License: Modified BSD
 *          Please see the file LICENSE.RSTM for licensing information
 */

#ifndef HASH_HPP__
#define HASH_HPP__

#include "List.hpp"

// the Hash class is an array of N_BUCKETS LinkedLists
class HashTable
{
    static const int N_BUCKETS = 256;

    /**
     *  during a sanity check, we want to make sure that every element in a
     *  bucket actually hashes to that bucket; we do it by passing this
     *  method to the extendedSanityCheck for the bucket.
     */
    static bool verify_hash_function(uint32_t val, uint32_t bucket)
    {
        return ((val % N_BUCKETS) == bucket);
    }

  public:
    /**
     *  Templated type defines what kind of list we'll use at each bucket.
     */
    List bucket[N_BUCKETS];

    TM_CALLABLE
    void insert(TX_FIRST_PARAMETER int val)
    {
        bucket[val % N_BUCKETS].insert(TX_FIRST_ARG val);
    }

    TM_CALLABLE
    bool lookup(TX_FIRST_PARAMETER int val) const
    {
        return bucket[val % N_BUCKETS].lookup(TX_FIRST_ARG val);
    }

    TM_CALLABLE
    void remove(TX_FIRST_PARAMETER int val)
    {
        bucket[val % N_BUCKETS].remove(TX_FIRST_ARG val);
    }

    bool isSane() const
    {
        for (int i = 0; i < N_BUCKETS; i++)
            if (!bucket[i].extendedSanityCheck(verify_hash_function, i))
                return false;
        return true;
    }
};

#endif // HASH_HPP__
