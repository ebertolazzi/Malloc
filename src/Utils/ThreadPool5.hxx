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
/// file: ThreadPool5.hxx
///

#ifdef UTILS_OS_LINUX
  #include <pthread.h>
#endif

namespace Utils {

  /*\
   |   _____ _                        _ ____             _
   |  |_   _| |__  _ __ ___  __ _  __| |  _ \ ___   ___ | |
   |    | | | '_ \| '__/ _ \/ _` |/ _` | |_) / _ \ / _ \| |
   |    | | | | | | | |  __/ (_| | (_| |  __/ (_) | (_) | |
   |    |_| |_| |_|_|  \___|\__,_|\__,_|_|   \___/ \___/|_|
  \*/

  class ThreadPool5 : public ThreadPoolBase {

    typedef double real_type;

    /*\
     |  __        __         _
     |  \ \      / /__  _ __| | _____ _ __
     |   \ \ /\ / / _ \| '__| |/ / _ \ '__|
     |    \ V  V / (_) | |  |   <  __/ |
     |     \_/\_/ \___/|_|  |_|\_\___|_|
    \*/

    class Worker {

      bool                  m_active           = false;
      unsigned              m_job_done_counter = 0;
      unsigned              m_worker_id        = 0;
      ThreadPool5 *         m_tp               = nullptr;
      UTILS_SEMAPHORE       m_is_running;
      std::thread           m_running_thread;
      std::function<void()> m_job;
      TicToc                m_tm;
      real_type             m_job_ms  = 0;
      real_type             m_sync_ms = 0;
      real_type             m_wait_ms = 0;

      void
      worker_loop() {
        m_is_running.red();     // block computation
        while ( m_active ) {
          m_tm.tic();
          m_is_running.wait();    // wait signal to start computation
          m_tm.toc();
          m_wait_ms += m_tm.elapsed_ms();
          // ----------------------------------------
          if ( !m_active ) break; // if finished exit
          m_tm.tic();
          m_job();
          m_tm.toc();
          m_job_ms += m_tm.elapsed_ms();
          // ----------------------------------------
          m_tm.tic();
          m_is_running.red();     // block computation
          ++m_job_done_counter;
          m_tp->push_worker( m_worker_id ); // worker ready for a new computation
          m_tm.toc();
          m_sync_ms += m_tm.elapsed_ms();
          std::this_thread::yield();
        }
      }

    public:

      explicit Worker() { start(); }
      ~Worker() { stop(); }

      // dummy copy constructor for resize
      Worker( Worker && ) {}

      void
      setup( ThreadPool5 * tp, unsigned id ) {
        m_worker_id        = id;
        m_job_done_counter = 0;
        m_tp               = tp;
      }

      void
      start() {
        if ( !m_active ) {
          m_active = true;
          m_running_thread = std::thread( &Worker::worker_loop, this );
        }
      }

      //!
      //! wait task is done
      //!
      void wait() { m_is_running.wait_red(); }

      void
      stop() {
        if ( m_active ) {
          wait();               // if running task wait it terminate
          m_active = false;     // deactivate computation
          m_job = [](){};       // dummy task
          m_is_running.green(); // start computation (exiting loop)
          if ( m_running_thread.joinable() ) m_running_thread.join(); // wait thread for exiting
          m_is_running.red();   // end of computation (for double stop);
        }
        //fmt::print( "worker_loop {} stopped\n", m_worker_id );
      }

      void
      exec( std::function<void()> & fun ) {
        m_is_running.wait_red();
        m_job = fun;             // cambia funzione da eseguire
        m_is_running.green();    // activate computation
      }

      unsigned job_done_counter() const { return m_job_done_counter; }

      real_type elapsed_job_ms()  const { return m_job_ms; }
      real_type elapsed_sync_ms() const { return m_sync_ms; }
      real_type elapsed_wait_ms() const { return m_wait_ms; }

      void
      info( ostream_type & s ) const {
        fmt::print(
          s,"Worker {:2}, #job = {:5}, "
          "[job {:.6} mus, sync {:.6} mus, wait {:.6} mus]\n",
          m_worker_id, m_job_done_counter,
          1000*elapsed_job_ms()/m_job_done_counter,
          1000*elapsed_sync_ms()/m_job_done_counter,
          1000*elapsed_wait_ms()/m_job_done_counter
        );
      }
    };

    // =========================================================================
    // =========================================================================
    // =========================================================================

    // need to keep track of threads so we can join them
    std::vector<Worker>     m_workers;
    // stack of available workers
    std::vector<unsigned>   m_stack;
    std::mutex              m_stack_mutex;
    std::condition_variable m_stack_cond;
    TicToc                  m_tm;
    real_type               m_exec_ms = 0;
    real_type               m_pop_ms  = 0;

    void setup() { for ( auto & w: m_workers ) w.start(); }

    void
    resize_workers( unsigned numThreads ) {
      m_stack.clear(); // empty stack
      m_stack.reserve( size_t(numThreads) );
      m_workers.resize( size_t(numThreads) );
      unsigned id = 0;
      for ( Worker & w : m_workers ) { w.setup( this, id ); ++id; }
      while ( id-- > 0 ) push_worker( id );
      setup();
    }

    void
    push_worker( unsigned id ) {
      std::unique_lock<std::mutex> lock(m_stack_mutex);
      m_stack.push_back(id);
      m_stack_cond.notify_one();
    }

    unsigned
    pop_worker() {
      std::unique_lock<std::mutex> lock(m_stack_mutex);
      m_stack_cond.wait( lock, [&]()->bool { return !m_stack.empty(); } );
      unsigned id = m_stack.back(); m_stack.pop_back();
      return id;
    }

  public:

    ThreadPool5(
      unsigned nthread = std::max(
        unsigned(1),
        unsigned(std::thread::hardware_concurrency()-1)
      )
    )
    : ThreadPoolBase()
    {
      resize_workers( nthread );
      //info( std::cout );
    }

    virtual
    ~ThreadPool5() {
      join();
      m_workers.clear();
      m_stack.clear();
    }

    void
    exec( std::function<void()> && fun ) override {
      // cerca prima thread libera

      m_tm.tic();
      unsigned id = pop_worker();
      Worker & w = m_workers[id];
      m_tm.toc();
      m_pop_ms += m_tm.elapsed_ms();

      m_tm.tic();
      w.exec( fun );
      m_tm.toc();
      m_exec_ms += m_tm.elapsed_ms();
    }

    void
    wait() override
    { for ( auto & w : m_workers ) w.wait(); }

    unsigned
    thread_count() const override
    { return unsigned(m_workers.size()); }

    void
    resize( unsigned numThreads ) override
    { wait(); stop(); resize_workers( numThreads ); }

    char const * name() const override { return "ThreadPool5"; }

    void start() { for ( auto && w : m_workers ) w.start(); }
    void stop()  { for ( auto && w : m_workers ) w.stop(); }
    void join()  { stop(); }

    void
    info_stack( ostream_type & s ) const {
      fmt::print( s, "STACK[{}]: ", m_stack.size() );
      for ( unsigned const & id : m_stack )
        fmt::print( s, "{}, ", id );
      s << '\n';
    }

    void
    info( ostream_type & s ) const override {
      for ( Worker const & w : m_workers ) w.info(s);
      fmt::print( s, "LAUNCH {} ms\n", m_exec_ms );
      fmt::print( s, "POP    {} ms\n", m_pop_ms );
      info_stack( s );
      fmt::print( s, "\n" );
    }
  };
}

///
/// eof: ThreadPool5.hxx
///
