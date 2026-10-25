#pragma once

#include <vector>

//! adds items in batches to a pool, which amortizes the costs of creating and queueing a job per item
//! if the queue runs empty, other workers are idle, so a job gives them the second half of its remaining items
//! \note works with all POOL types, they need add( Job ) and empty()
template<class Item, class Pool, class Func>
class Batcher {
    public:
        //! \param func is called for each item on a worker thread
        //! \param size max. items per job
        Batcher( Pool& pool, Func func, const size_t size ) : pool( pool ), func( std::move( func ) ), size( size ) {
            items.reserve( size );
        }
        ~Batcher() { flush(); }
        Batcher( const Batcher& ) = delete;
        Batcher& operator=( const Batcher& ) = delete;

        void add( Item item ) {
            items.emplace_back( std::move( item ) );

            if( items.size() >= size ) { flush(); }
        }

        //! adds the collected items as one job
        void flush() {
            if( items.empty() ) { return; }

            addJob( pool, func, std::move( items ) );
            items.clear();
            items.reserve( size );
        }

    private:
        static void addJob( Pool& pool, const Func& func, std::vector<Item> items ) {
            pool.add( [&pool, func, items{std::move( items )}]() mutable {
                for( size_t i = 0; i < items.size(); ++i ) {
                    const size_t rest = items.size() - i;

                    if( rest > 1 && pool.empty() ) {
                        const size_t half = i + ( rest + 1 ) / 2;
                        addJob( pool, func, std::vector<Item>( std::make_move_iterator( items.begin() + half ),
                                                               std::make_move_iterator( items.end() ) ) );
                        items.resize( half );
                    }

                    func( items[i] );
                }
            } );
        }

        Pool& pool;
        Func func;
        const size_t size;
        std::vector<Item> items;
};

//! \returns Batcher for items of type Item, deduces Pool and Func
template<class Item, class Pool, class Func>
Batcher<Item, Pool, Func> makeBatcher( Pool& pool, Func func, const size_t size = 128 ) {
    return Batcher<Item, Pool, Func>( pool, std::move( func ), size );
}
