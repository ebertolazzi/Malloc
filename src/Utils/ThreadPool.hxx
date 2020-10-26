/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2020                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                |
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

///
/// eof: ThreadPool.hxx
///

#pragma once

#ifndef THREADPOOL_dot_HH
#define THREADPOOL_dot_HH

#include <algorithm>
#include <utility>
#include <vector>
//#include <type_traits>

#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>

#ifdef UTILS_OS_LINUX
  #include <pthread.h>
#endif

namespace Utils {

  /*\
   |   _____ _                        _
   |  |_   _| |__  _ __ ___  __ _  __| |___
   |    | | | '_ \| '__/ _ \/ _` |/ _` / __|
   |    | | | | | | | |  __/ (_| | (_| \__ \
   |    |_| |_| |_|_|  \___|\__,_|\__,_|___/
  \*/

  class SpinLock {
    // see https://geidav.wordpress.com/2016/03/23/test-and-set-spinlocks/
  private:
    std::atomic<bool> m_locked = {false};
  public:
    SpinLock() {}

    void
    wait() {
      while (m_locked.load(std::memory_order_relaxed) == true);
    }

    void
    lock() {
      do { wait(); } while (m_locked.exchange(true, std::memory_order_acquire) == true);
    }

    void
    unlock() {
      m_locked.store(false, std::memory_order_release);
    }
  };

  class WaitWorker {
  private:
    #ifdef UTILS_OS_WINDOWS
    std::atomic<int> n_worker;
    #else
    std::atomic<int> n_worker = {0};
    #endif
  public:
    #ifdef UTILS_OS_WINDOWS
    WaitWorker() { n_worker = 0; }
    #else
    WaitWorker() {}
    #endif

    void
    wait() {
      while (n_worker.load(std::memory_order_relaxed) != 0 );
    }

    void enter() { ++n_worker; }
    void leave() { --n_worker; }
  };

  template <typename DATA>
  class BinarySearch {
  private:
    typedef std::pair<std::thread::id,DATA*> DATA_TYPE;
    mutable std::vector<DATA_TYPE>           m_data;
    mutable SpinLock                         m_spin_write;
    mutable WaitWorker                       m_worker_read;

  public:

    BinarySearch() {
      m_data.clear();
      m_data.reserve(64);
    }

    ~BinarySearch() {
      m_spin_write.wait();
      for ( auto & a : m_data ) delete a.second;
      m_data.clear();
      m_spin_write.wait();
    }

    void
    clear() {
      m_spin_write.wait();
      for ( auto & a : m_data ) delete a.second;
      m_data.clear(); m_data.reserve(64);
      m_spin_write.wait();
    }

    DATA *
    search( std::thread::id const & id, bool & ok ) const {
      m_spin_write.wait(); // wait writing finished
      m_worker_read.enter();
      ok = true;
      size_t U = m_data.size();
      size_t L = 0;
      while ( U-L > 1 ) {
        size_t pos = (L+U)>>1;
        std::thread::id const & id_pos = m_data[pos].first;
        if ( id_pos < id ) L = pos; else U = pos;
      }
      DATA_TYPE & dL = m_data[L];
      if ( dL.first == id ) { m_worker_read.leave(); return dL.second; }
      DATA_TYPE & dU = m_data[U];
      if ( dU.first == id ) { m_worker_read.leave(); return dU.second; }
      m_worker_read.leave();
      // not found must insert
      m_spin_write.lock();
      m_worker_read.wait(); // wait all read finished
      ok = false;
      U  = m_data.size();
      m_data.resize(U+1);
      while ( U > L ) {
        --U;
        m_data[U+1].first  = m_data[U].first;
        m_data[U+1].second = m_data[U].second;
      }
      DATA_TYPE & dL1 = m_data[L+1];
      dL1.first = id;
      DATA * res = dL1.second = new DATA();
      m_spin_write.unlock();
      return res;
    }

  };

  class SpinLock_barrier {
  private:
    std::atomic<unsigned> m_count;
    std::atomic<unsigned> m_generation;
    unsigned int m_count_reset_value;
  public:
    SpinLock_barrier(const SpinLock_barrier&) = delete;
    SpinLock_barrier& operator=(const SpinLock_barrier&) = delete;

    explicit
    SpinLock_barrier()
    : m_generation(0)
    {}

    void
    setup( unsigned count ) {
      m_count_reset_value = m_count = count ;
    }

    void
    count_down() {
      unsigned gen = m_generation.load();
      if ( --m_count == 0 ) {
        if ( m_generation.compare_exchange_weak(gen, gen + 1) )
          m_count = m_count_reset_value;
        return;
      }
    }

    void
    wait() {
      unsigned gen = m_generation.load();
      while ((gen == m_generation) && (m_count != 0))
        std::this_thread::yield();
    }

    void
    count_down_and_wait() {
      unsigned gen = m_generation.load();
      if ( --m_count == 0 ) {
        if ( m_generation.compare_exchange_weak(gen, gen + 1) )
          m_count = m_count_reset_value;
        return;
      }
      while ((gen == m_generation) && (m_count != 0))
        std::this_thread::yield();
    }
  };

  class Barrier {
    int to_be_done, usedThread;
    std::mutex              mtx;
    std::condition_variable cond;
  public:
    Barrier() : to_be_done(0) {}

    void
    setup( int nthreads )
    { usedThread = to_be_done = nthreads ; }

    void
    count_down() {
      std::unique_lock<std::mutex> lck(mtx);
      if ( --to_be_done <= 0 ) cond.notify_all() ; // wake up all tread
    }

    void
    wait() {
      std::unique_lock<std::mutex> lck(mtx);
      cond.wait(lck);
    }

    void
    count_down_and_wait() {
      std::unique_lock<std::mutex> lck(mtx);
      if ( --to_be_done <= 0 ) {
        cond.notify_all() ; // wake up all tread
        to_be_done = usedThread ;
      } else {
        cond.wait(lck);
      }
    }
  };

  class SimpleSemaphore {
  private:
    bool                    m_go;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
  public:
    SimpleSemaphore() noexcept : m_go(true) {}

    void
    green() noexcept {
      { std::unique_lock<std::mutex> lock(m_mutex); m_go = true; }
      m_cv.notify_one();
    }

    void
    red() noexcept {
      { std::unique_lock<std::mutex> lock(m_mutex); m_go = false; }
      m_cv.notify_one();
    }

    void
    wait() noexcept {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this]()->bool { return this->m_go; });
    }

  };

  /*\
   |  __        __         _
   |  \ \      / /__  _ __| | _____ _ __
   |   \ \ /\ / / _ \| '__| |/ / _ \ '__|
   |    \ V  V / (_) | |  |   <  __/ |
   |     \_/\_/ \___/|_|  |_|\_\___|_|
  \*/

  class Worker {
    friend class ThreadPool;

    bool                  active;
    SimpleSemaphore       is_running, job_done;
    std::thread           running_thread;
    std::function<void()> job;

    //disable copy
    Worker( Worker const & ) = delete;
    Worker& operator = ( Worker const & ) = delete;

    void
    loop() {
      while ( active ) {
        is_running.wait();
        if ( active ) job();
        is_running.red();
        job_done.green();
      }
    }

  public:

    Worker() : active(false) { start(); }
    ~Worker() { stop(); }

    Worker( Worker && rhs ) {
      active         = rhs.active;
      job            = rhs.job;
      running_thread = std::move(rhs.running_thread);
    }

    void
    start() {
      if ( !active ) {
        active = true;
        is_running.red();
        job_done.green();
        running_thread = std::thread( [this] () -> void { this->loop(); } );
      }
    }

    void
    stop() {
      if ( active ) {
        active = false;        // deactivate computation
        is_running.green();    // for exiting from the loop
        running_thread.join(); // wait thread for exiting
      }
    }

    void wait() { job_done.wait(); }

    template < class Func, class... Args >
    void
    run( Func && func, Args && ... args ) {
      //launch( std::bind(std::forward<Func>(func), std::forward<Args>(args)...) );
      job_done.wait(); // se gia occupato in task aspetta
      job = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
      job_done.red();
      is_running.green(); // activate computation
    }

  };

  /*\
   |   _____ _                        _ ____             _
   |  |_   _| |__  _ __ ___  __ _  __| |  _ \ ___   ___ | |
   |    | | | '_ \| '__/ _ \/ _` |/ _` | |_) / _ \ / _ \| |
   |    | | | | | | | |  __/ (_| | (_| |  __/ (_) | (_) | |
   |    |_| |_| |_|_|  \___|\__,_|\__,_|_|   \___/ \___/|_|
  \*/

  class ThreadPool {
    // need to keep track of threads so we can join them
    std::vector<Worker> workers;

    //disable copy
    ThreadPool() = delete;
    ThreadPool( ThreadPool const & ) = delete;
    ThreadPool& operator = ( ThreadPool const & ) = delete;

    #if defined(UTILS_OS_WINDOWS)
    void
    setup() {
      for ( auto & w: workers ) {
        w.start();
        std::thread & t = w.running_thread;
        SetThreadPriority( t.native_handle(), THREAD_PRIORITY_HIGHEST );
      }
    }
    #elif defined(UTILS_OS_LINUX)
    void
    setup() {
      sched_param sch;
      int         policy;
      for ( auto & w: workers ) {
        w.start();
        std::thread & t = w.running_thread;
        pthread_getschedparam( t.native_handle(), &policy, &sch );
        sch.sched_priority = sched_get_priority_max( SCHED_RR );
        pthread_setschedparam( t.native_handle(), SCHED_RR, &sch );
      }
    }
    #else
    void
    setup() {
      for ( auto & w: workers ) w.start();
    }
    #endif

  public:

    ThreadPool(
      unsigned nthread = std::max(
        unsigned(1),
        unsigned(std::thread::hardware_concurrency()-1)
      )
    ) {
      workers.resize( size_t( nthread ) );
      setup();
    }

    //! Submit a job to be run by the thread pool.
    template <typename Func, typename... Args>
    void
    run( unsigned nt, Func && func, Args && ... args ) {
      workers[size_t(nt)].run( func, args...);
    }

    void wait_all()  { for ( auto && w : workers ) w.wait(); }
    void start_all() { for ( auto && w : workers ) w.start(); }
    void stop_all()  { for ( auto && w : workers ) w.stop(); }

    unsigned size() const { return unsigned(workers.size()); }

    std::thread::id
    get_id( unsigned i ) const
    { return workers[size_t(i)].running_thread.get_id(); }

    std::thread const &
    get_thread( unsigned i ) const
    { return workers[size_t(i)].running_thread; }

    std::thread &
    get_thread( unsigned i )
    { return workers[size_t(i)].running_thread; }

    void
    resize( unsigned numThreads ) {
      wait_all();
      stop_all();
      workers.resize( size_t(numThreads) );
      setup();
    }

  };

}

#endif

///
/// eof: ThreadPool.hxx
///
