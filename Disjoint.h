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

#ifndef DISJOINT_HPP__
#define DISJOINT_HPP__

extern thread_local int thread_id;

// this is a benchmark for evaluating the overhead that TM induces for a
// variety of read/write ratios when there are no conflicts.  The benchmark
// is templated so that we can (at compile time) specify the number of
// locations that a transaction will touch, and the percentage of those
// locations that are to be written.  We don't actually care about the values
// read or written, since it's a microbenchmark.
struct Disjoint
  {
      // const for keeping things from being 'too regular'
      static const unsigned DJBUFFER_SIZE = 1009;
      static const unsigned BUFFER_COUNT = 256;

      // each transaction will work on special buffers, where entries in the
      // buffer are pretty far apart
      struct PaddedBufferEntry
      {
          uint32_t value;
          char padding[64-sizeof(uint32_t)];
          PaddedBufferEntry() : value(0), padding() { }
      };

      // array of padded entries
      struct PaddedBuffer
      {
          PaddedBufferEntry buffer[DJBUFFER_SIZE];
          PaddedBuffer() : buffer() { }
      };

      // private buffers for upto BUFFER_COUNT transactions
      PaddedBuffer privateBuffers[BUFFER_COUNT];

      // public buffer (in case we need it)
      PaddedBuffer publicBuffer;

      unsigned reads_per_ten;
      unsigned writes_per_ten;
      unsigned locations_per_transaction;
      bool use_shared_read_buffer;

      // constructor just sets up the execution parameters
      // R : reads_per_10
      // W : writes_per_10
      // L : locations_per_transaction
      // S : use_shared_read_buffer
      Disjoint(unsigned R, unsigned W, unsigned L, bool S)
          : privateBuffers(), publicBuffer(),
            reads_per_ten(R), writes_per_ten(W),
            locations_per_transaction(L),
            use_shared_read_buffer(S)
      {
          unsigned int s = W;
          for (uint32_t i = 0; i < BUFFER_COUNT; ++i)
              for (unsigned j = 0; j < DJBUFFER_SIZE; ++j)
                  privateBuffers[i].buffer[j].value = rand_r_32(&s);
          for (unsigned j = 0; j < DJBUFFER_SIZE; ++j)
              publicBuffer.buffer[j].value = rand_r_32(&s);
      }

    private:
      /**
       *  do some reads only... bool return type to keep it from being
       *  optimized out
       */
      __attribute__((transaction_safe))
      bool ro_transaction(uint32_t id, uint32_t startpoint) {
          unsigned sum = 0;
          unsigned index = startpoint;
          PaddedBuffer& rBuffer =
              use_shared_read_buffer ? publicBuffer : privateBuffers[id];

          for (unsigned i = 0; i < locations_per_transaction; i++) {
              sum += rBuffer.buffer[index].value;
              // compute the next index
              index = (index + 1) % DJBUFFER_SIZE;
          }
          return (sum == 0);
      }

      /**
       *  do some reads, do some rmw's
       */
      __attribute__((transaction_safe))
      bool r_rw_transaction(uint32_t id, uint32_t startpoint)
      {
          PaddedBuffer& rBuffer =
              use_shared_read_buffer ? publicBuffer : privateBuffers[id];
          PaddedBuffer& wBuffer = privateBuffers[id];
          unsigned index = startpoint, writes = 0, reads = 0, buff = 0;

          for (unsigned i = 0; i < locations_per_transaction; i++) {
              // if we've done ten things, then reset the read and write
              // counters
              if ((writes + reads) == 10)
                  writes = reads = 0;
              // should_write determies action to do on this loop iteration
              bool should_write = false;
              // set should_write based on /i/:
              // on odd, do a read/write if there are writes left to do
              // on even, do a read if there are reads left to do
              if (i && 0x1)
                  should_write = (writes < writes_per_ten);
              else
                  should_write = !(reads < reads_per_ten);

              // perform the selected action
              if (should_write) {
                  // increment the item (read and write it)
                  unsigned oldval = (wBuffer.buffer[index].value);
                  wBuffer.buffer[index].value =  (uint32_t)(oldval + 1);
                  writes++;
              }
              else {
                  buff += rBuffer.buffer[index].value;
                  reads++;
              }

              // compute the next index
              index = (index + 1) % DJBUFFER_SIZE;
          }
          return buff & 1;
      }

    public:

      __attribute__((transaction_safe))
      bool lookup(int val) {
          return ro_transaction(thread_id, val);
      }


      __attribute__((transaction_safe))
      bool insert(int val) {
          return r_rw_transaction(thread_id, val);
      }


      __attribute__((transaction_safe))
      bool remove(int val) {
          return r_rw_transaction(thread_id, val);
      }

      /**
       * Not in use
       */
      bool isSane() {
          return true;
      }
};

#endif // DISJOINT_HPP__
