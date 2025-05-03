#include "task_pool.h"

TaskPool::TaskPool() :
    threads_( std::thread::hardware_concurrency() ) 
{
    for ( auto& t : threads_ ) {
    	
        t = std::thread( [this]() {                                
        	
                while ( !stop_ ) {
                	std::unique_lock<std::mutex> lock( queue_mutex_ );
                	
                    cond_queue_not_empty.wait( lock, [this]() {
                        return !queue_.empty() || stop_;
		} );
		
		if ( stop_ ) {
		    return;
		}
		
		auto task = std::move( queue_.front() );
		queue_.pop_front();
		lock.unlock();
		
		if ( task ) {
		    task();
		} // end if
	      } // end while
	      
            });
            
    }
}

TaskPool::~TaskPool() {
    stop_ = true;
    cond_queue_not_empty.notify_all();
    for ( auto& t : threads_ ) {
        if ( t.joinable() ) {
            t.join();
        }
    }
}

void TaskPool::AddTask( const task_type& task ) {
    {
        std::lock_guard<std::mutex> lock( queue_mutex_ );
        queue_.push_back( task );
    }
    
    cond_queue_not_empty.notify_one();
}
