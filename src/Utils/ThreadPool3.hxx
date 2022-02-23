/*!
 */

///
/// file: ThreadPool3.hxx
///

namespace Utils {

  class ThreadPool3 : public ThreadPoolBase {

    typedef double real_type;

    typedef tp::Queue::TaskData TaskData;

    std::atomic<bool>        m_done;
    std::atomic<unsigned>    m_running_task;
    std::atomic<unsigned>    m_running_thread;
    std::vector<std::thread> m_worker_threads;
    tp::Queue                m_work_queue; // not thread safe
    SpinLock                 m_spin_queue;
    // -----------------------------------------
    std::mutex               m_push_queue_mutex;
    std::condition_variable  m_push_queue_cv;
    //std::atomic<unsigned>    m_push_waiting;
    unsigned                 m_push_waiting;
    // -----------------------------------------
    std::mutex               m_pop_queue_mutex;
    std::condition_variable  m_pop_queue_cv;
    //std::atomic<unsigned>    m_pop_waiting;
    unsigned                 m_pop_waiting;

    TicToc                   m_tm;
    std::vector<real_type>   m_job_ms;
    std::vector<real_type>   m_pop_ms;
    std::vector<unsigned>    m_n_job;
    real_type                m_push_ms;

    //
    // https://stackoverflow.com/questions/48936591/is-the-performance-of-notify-one-really-this-bad
    //

    TaskData *
    pop_task() {
      ++m_pop_waiting;
      std::unique_lock<std::mutex> lock(m_pop_queue_mutex);
      while ( m_work_queue.empty() ) m_pop_queue_cv.wait( lock );
      --m_pop_waiting;
      //-----------------
      m_spin_queue.lock();
      TaskData * task = m_work_queue.pop();
      m_spin_queue.unlock();
      //-----------------
      ++m_running_task; // must be incremented in the locked part
      if ( m_push_waiting > 0 ) m_push_queue_cv.notify_one();
      return task;
    }

    void
    push_task( TaskData * task ) {
      ++m_push_waiting;
      std::unique_lock<std::mutex> lock(m_push_queue_mutex);
      while ( m_work_queue.is_full() ) m_push_queue_cv.wait( lock );
      --m_push_waiting;
      //-----------------
      m_spin_queue.lock();
      m_work_queue.push( task );
      m_spin_queue.unlock();
      //-----------------
      if ( m_pop_waiting > 0 ) m_pop_queue_cv.notify_one();
    }

    void
    worker_thread(
      real_type & pop_ms,
      real_type & job_ms,
      unsigned  & n_job
    ) {
      TicToc tm;
      ++m_running_thread;
      while ( !m_done ) {
        // ---------------------------- POP
        tm.tic();
        TaskData * task = pop_task();
        tm.toc();
        pop_ms += tm.elapsed_ms();
        // ---------------------------- RUN
        tm.tic();
        (*task)(); // run and delete task;
        tm.toc();
        job_ms += tm.elapsed_ms();
        // ---------------------------- UPDATE
        --m_running_task; ++n_job;
      }
      --m_running_thread;
    }

    void
    create_workers( unsigned thread_count ) {
      m_worker_threads.clear();
      m_worker_threads.reserve(thread_count);
      m_job_ms.resize( std::size_t(thread_count) );
      m_pop_ms.resize( std::size_t(thread_count) );
      m_n_job.resize( std::size_t(thread_count) );
      std::fill( m_job_ms.begin(), m_job_ms.end(), 0 );
      std::fill( m_pop_ms.begin(), m_pop_ms.end(), 0 );
      std::fill( m_n_job.begin(), m_n_job.end(), 0 );
      m_push_ms      = 0;
      m_done         = false;
      m_push_waiting = 0;
      m_pop_waiting  = 0;
      try {
        for ( unsigned i=0; i<thread_count; ++i )
          m_worker_threads.push_back(
            std::thread(
              &ThreadPool3::worker_thread, this,
              std::ref(m_pop_ms[i]),
              std::ref(m_job_ms[i]),
              std::ref(m_n_job[i])
            )
          );
      } catch(...) {
        m_done = true;
        throw;
      }
    }

  public:

    explicit
    ThreadPool3(
      unsigned thread_count   = std::thread::hardware_concurrency(),
      unsigned queue_capacity = 0
    )
    : m_done(false)
    , m_running_task(0)
    , m_running_thread(0)
    , m_work_queue( queue_capacity == 0 ? std::max( 10 * (thread_count+1), unsigned(4096) ) : queue_capacity )
    {
      create_workers( thread_count );
    }

    virtual ~ThreadPool3() { join(); }

    void
    exec( std::function<void()> && fun ) override {
      m_tm.tic();
      push_task( new TaskData(std::move(fun)) );
      m_tm.toc();
      m_push_ms += m_tm.elapsed_ms();
    }

    void
    wait() override
    { while ( !m_work_queue.empty() || m_running_task > 0 ) std::this_thread::yield(); }

    void
    join() {
      wait();
      m_done = true;
      { // send null task until all the workers stopped
        std::function<void()> null_job = [](){};
        for ( unsigned i = m_running_thread; i > 0; --i )
          push_task( new TaskData(null_job) );
        while ( m_running_thread > 0 ) std::this_thread::yield();
      }
      m_work_queue.clear();
      for ( std::thread & w : m_worker_threads ) { if (w.joinable()) w.join(); }
      m_worker_threads.clear();
    }

    void resize( unsigned thread_count ) override { resize( thread_count, 0 ); }

    void
    resize( unsigned thread_count, unsigned queue_capacity = 0 ) {
      join();
      if ( queue_capacity == 0 ) queue_capacity = 4 * (thread_count+1);
      m_work_queue.resize( queue_capacity );
      create_workers( thread_count );
    }

    void
    info( ostream_type & s ) const override {
      unsigned nw = unsigned(m_pop_ms.size());
      for ( unsigned i = 0; i < nw; ++i )
        fmt::print( s,
          "Worker {:2}, #job = {:4}, [job {:.6} ms, POP {:.6} ms] AVE = {:.6} ms\n",
          i, m_n_job[i], m_job_ms[i], m_pop_ms[i], m_job_ms[i]/m_n_job[i]
        );
      fmt::print( s, "PUSH {:10.6} ms\n\n", m_push_ms );
    }

    unsigned thread_count()   const override { return unsigned(m_worker_threads.size()); }
    unsigned queue_capacity() const          { return m_work_queue.capacity(); }

    char const * name() const override { return "ThreadPool3"; }
  };

}

///
/// eof: ThreadPool3.hxx
///
