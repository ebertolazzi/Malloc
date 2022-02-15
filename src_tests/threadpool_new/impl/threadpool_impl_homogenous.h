/** @file threadpool/impl/threadpool_impl_homogenous.h
 *
 * Threadpool for C++11, homogenous thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H
#define THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H

namespace threadpool {

  /**
   * A thread pool reading its tasks from a VirtualQueue.
   *
   * The template parameter is ignored. This will only ever by
   * used with template parameter 0. We could define a straight
   * non-template class, but then it would not be possible to
   * include the class definition in multiple separately
   * compiled files. By making it a class *template*, we profit
   * from the fact that multiple implicit instantiations of a
   * template are allowed. This means when the user switches
   * between header-only and library configuration he does not
   * need to recompile everything, and the ODR is not violated.
   */

  class GenericThreadPoolTmpl : public GenericThreadPoolInterface {
    std::unique_ptr<GenericThreadPoolInterface> pimpl;
	public:

    /**
     * Create a generic thread pool.
     *
     * As soon as the pool is created the threads start
     * running tasks from the queue until the queue is
     * empty. When the queue is empty the threads return and
     * are ready to be collected by join() or by the
     * destructor.
     *
     * \param queue
     *        The queue managing the tasks.
     *
     * \param thread_count
     *        The number of threads to use.
     *        If the thread count is not specified it
     *        defaults to the number of available hardware
     *        threads std::thread::hardware_concurrency(),
     *        as read through hardware_concurrency().
     *
     */
    GenericThreadPoolTmpl(VirtualQueue& queue, int thread_count = -1);

    /**
     * Help with the work.
     *
     * \param return_if_idle
     *        Never wait for work, return instead.
     */
    void help(bool return_if_idle) override;

    /**
     * Rethrow a potentially pending exception from a worker thread.
     */
    void rethrow_exception() override;

    /**
     * Wait for all threads to finish and collect them.
     *
     * Leaves the thread pool ready for destruction.
     */
    void join() override;

    /**
     * Destroy the thread pool.
     *
     * Generally a destructor should not wait for a long time,
     * and it should not throw any exceptions. Unfortunately
     * threads are not abortable in C++.  The only way to make
     * sure the threads have terminated is to wait for them to
     * return by themselves.  If we would allow the
     * destruction of the thread pool before the threads have
     * returned, the threads would continue to access the
     * memory of the destroyed thread pool, potentially
     * clobbering other objects residing in the recycled
     * memory.  We could allocate parts of the memory with
     * new, and leave it behind for the threads after the
     * thread pool is destructed.  But even then, the user
     * supplied functions run by the threads might access
     * memory that gets destroyed if the function that
     * constructed the thread pool terminates.  The danger of
     * undetected and undebuggable memory corruption is just
     * too big.
     *
     * With regard to the exceptions rethrown in the destructor,
     * it is better to signal the exception than to ignore it
     * silently.
     *
     * If it is not acceptable for the destructor to wait or
     * to throw an exception, just call join() before the pool
     * is destructed.  After join() returns the destructor is
     * guaranteed to run fast and without exceptions.
     *
     * If it should really be necessary to keep threads
     * running after the function that created the thread pool
     * returns, just create the thread pool on the heap with
     * new. And if you want to make sure nobody destroys the
     * thread pool, feel free to throw away the handle.
     */
    virtual ~GenericThreadPoolTmpl();

    /**
     * Cache the hardware concurrency so we are sure that it
     * is cheap to get. Also this gives us a point to
     * cheat. The cached value can be modified by a parameter.
     *
     * \param c
     *        The hardware concurrency to use
     */
    static unsigned hardware_concurrency(int c = -1);

    /**
     * Determine thread count to use based on users
     * specifications.
     *
     * \param thread_count
     *        Runtime specified threadcount parameter.
     *
     * \returns
     *           The number of threads to use. Returns always a
     *           positive number, possibly 1 for
     *           single-processor systems.
     *
     * This policy function does just some
     * guesswork. Allocating a number of threads in the order
     * of the hardware threads may be a good bet for CPU-bound
     * work. For other tasks it depends.
     */
    static unsigned determine_thread_count(int thread_count = -1);

    /**
     * Switch exception handling off
     */
    static bool ignore_thread_pool_exceptions(bool i = true);

	};

  /**
   * A thread pool reading its tasks from a generic queue.
   *
   * \tparam class Queue
   *         The queue delivering the tasks.
   *
   *	Class Queue must provide the following members:
   *
   *	- void work()
   *    Gets tasks and works until the end of
   *    the queue is reached.
   *
   *	- void shutdown()
   *    Causes the queue to tell the threads
   *    asking for work to return.
   *
   */
  template<class Queue>
  class GenericThreadPoolImpl : public GenericThreadPoolInterface {
    class Worker {
      std::thread m_thread;
    public:
      Worker & operator=(std::thread&& t) { m_thread = std::move(t); return *this; }
      void join() { if (m_thread.joinable()) m_thread.join(); }
    };

    std::mutex          m_mutex;
    std::exception_ptr  m_pending_exception;
    Queue&              m_queue;
    unsigned const      m_thread_count; /// The number of threads
    std::vector<Worker> m_workers;

    /**
     * The main function of the thread.
     */
    void work() { help(false); }

    /**
     * Wait for all workers to finish.
     */
    void
    join_workers() {
      work();		// Instead of hanging around, help the workers!
      for ( Worker & w : m_workers ) w.join();
    }

    // Copying and moving are not supported.
    GenericThreadPoolImpl(const GenericThreadPoolImpl&) = delete;
    GenericThreadPoolImpl(GenericThreadPoolImpl&&) = delete;
    GenericThreadPoolImpl& operator=(const GenericThreadPoolImpl&) = delete;
    GenericThreadPoolImpl& operator=(GenericThreadPoolImpl&&) = delete;

	public:

    /**
     * Generic thread pool.
     *
     * As soon as the pool is created the threads start running
     * tasks from the queue until the queue is empty. When the
     * queue is empty the threads return and are ready to be
     * collected by join() or by the destructor.
     *
     * \param queue
     *        The queue delivering the tasks.
     *
     * \param thread_count The number of threads
     *                     to use. If the thread count is not
     *                     specified it defaults to the number of
     *                     available hardware threads
     *                     std::thread::hardware_concurrency(),
     *                     as read through hardware_concurrency().
     *
     */
    GenericThreadPoolImpl(Queue& queue, int thread_count)
    : m_pending_exception(nullptr)
    , m_queue(queue)
    , m_thread_count(determine_thread_count(thread_count))
    , m_workers(this->m_thread_count)
    {
      for (Worker& w: m_workers)
        w = std::move(std::thread(std::bind(&GenericThreadPoolImpl::work, this)));
    }

    /**
     * Help with the work.
     *
     * \param return_if_idle
     *        Never wait for work, return instead.
     */
    void
    help(bool return_if_idle) override {
      if (ignore_thread_pool_exceptions()) {
        m_queue.work(return_if_idle);
      } else {
        try {
          m_queue.work(return_if_idle);
        } catch (...) {
          {
            std::exception_ptr e = std::current_exception();
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_pending_exception) m_pending_exception = std::move(e);
          }
          m_queue.shutdown();
        }
      }
    }

    /**
     * Rethrow a potentially pending exception from a worker thread.
     */
    void
    rethrow_exception() override {
      if ( m_pending_exception && !std::uncaught_exception() ) {
        m_queue.shutdown();
        join_workers();
        if (!std::uncaught_exception()) {
          std::exception_ptr e = m_pending_exception;
          m_pending_exception = nullptr;
          std::rethrow_exception(std::move(e));
        }
      }
    }

    /**
     * Wait for all threads to finish and collect them.
     *
     * Leaves the thread pool ready for destruction.
     */
    void
    join() override {
      join_workers();
      rethrow_exception();
    }

    /**
     * Destroy the thread pool.
     *
     * Generally a destructor should not wait for a long time,
     * and it should not throw any exceptions. Unfortunately
     * threads are not abortable in C++.  The only way to make
     * sure the threads have terminated is to wait for them to
     * return by themselves.  If we would allow the
     * destruction of the thread pool before the threads have
     * returned, the threads would continue to access the
     * memory of the destroyed thread pool, potentially
     * clobbering other objects residing in the recycled
     * memory.  We could allocate parts of the memory with
     * new, and leave it behind for the threads after the
     * thread pool is destructed.  But even then, the user
     * supplied functions run by the threads might access
     * memory that gets destroyed if the function that
     * constructed the thread pool terminates.  The danger of
     * undetected and undebuggable memory corruption is just
     * too big.
     *
     * With regard to the exceptions rethrown in the
     * destructor, it is better to signal the exception than
     * to ignore it silently.
     *
     * If it is not acceptable for the destructor to wait or
     * to throw an exception, just call join() before the pool
     * is destructed.  After join() the destructor is
     * guaranteed to run fast and without exceptions.
     *
     * If it should really be necessary to keep threads
     * running after the function that created the thread pool
     * returns, just create the thread pool on the heap with
     * new. And if you want to make sure nobody destroys the
     * thread pool, feel free to throw away the handle.
     */
    virtual
    ~GenericThreadPoolImpl() {
      // Abort processing if destructor runs during exception handling.
      if (std::uncaught_exception()) m_queue.shutdown();
      join(); // Running threads would continue to access the destructed pool.
    }

    /**
     * Cache the hardware concurrency so we are sure that it
     * is cheap to get. Also this gives us a point to
     * cheat. The cached value can be modified by a parameter.
     *
     * \param c
     *        The hardware concurrency to use
     */
    static unsigned hardware_concurrency(int c = -1) {
      static int cached_concurrency = -1;
      if (c != -1) cached_concurrency = c;
      if (cached_concurrency == -1)
        cached_concurrency = std::thread::hardware_concurrency();
      return cached_concurrency;
    }

    /**
     * Determine thread count to use based on users
     * specifications.
     *
     * \param thread_count
     *        Runtime specified threadcount parameter.
     *
     * \returns
     *        The number of threads to use.
     *
     * This policy function does just some
     * guesswork. Allocating a number of threads in the order
     * of the hardware threads may be a good bet for CPU-bound
     * work. For other tasks it depends.
     */
    static unsigned determine_thread_count(int thread_count = -1) {
      if (thread_count == -1 && !(thread_count = hardware_concurrency())) thread_count = 8;
      return thread_count;
    }

    /**
     * Switch exception handling off
     */
    static bool ignore_thread_pool_exceptions(bool i = true) {
      static bool do_ignore_exceptions = false;
      if (i) do_ignore_exceptions = i;
      return do_ignore_exceptions;
    }
	};

  GenericThreadPoolTmpl::GenericThreadPoolTmpl(
    VirtualQueue& queue, int thread_count
  ) : pimpl(new GenericThreadPoolImpl<VirtualQueue>(queue, thread_count))
  { }

  void
  GenericThreadPoolTmpl::help(bool return_if_idle) {
    pimpl->help(return_if_idle);
  }

  void
  GenericThreadPoolTmpl::rethrow_exception() {
    pimpl->rethrow_exception();
  }

  void
  GenericThreadPoolTmpl::join() {
    pimpl->join();
  }

  GenericThreadPoolTmpl::~GenericThreadPoolTmpl() { }

  unsigned
  GenericThreadPoolTmpl::hardware_concurrency(int c) {
    return GenericThreadPoolImpl<VirtualQueue>::hardware_concurrency(c);
  }

  unsigned
  GenericThreadPoolTmpl::determine_thread_count(int c) {
    return GenericThreadPoolImpl<VirtualQueue>::determine_thread_count(c);
  }

  bool
  GenericThreadPoolTmpl::ignore_thread_pool_exceptions(bool i) {
    return GenericThreadPoolImpl<VirtualQueue>::ignore_thread_pool_exceptions(i);
  }

  /**
   * Thread pool with homogenous functions
   *
   * This thread pool is dependent on template parameter
   * Function. Only one type of functions can be queued. For
   * example if this thread pool is instantiated for a lambda
   * function, only this exact lambda function can be run,
   * since each lambda function has its own type separate from
   * all other lambda functions.
   *
   * To make this thread pool more universally usable,
   * instantiate it either with std::function<void()>, or use
   * the virtual thread pool VirtualThreadPool.
   *
   * \tparam Function
   *         The function type to be used for the
   *         queue. All tasks to be queued must be of this
   *         type. The class Function must support the
   *         following interface:
   *         - operator()(): the functions must be callable
   *           with no parameters.
   */
  template<class Function>
  class HomogenousThreadPool {

    HQueue<Function>      queue;
    GenericThreadPoolTmpl pool;

  public:

    explicit
    HomogenousThreadPool(
      int         thread_count = -1,
      std::size_t queue_size   = 0,
      std::size_t maxpart      = 1
    )
    : queue(queue_size, (maxpart != 1) ? maxpart : 3 * (GenericThreadPoolTmpl::determine_thread_count(thread_count)+ 1))
    , pool(queue, thread_count)
    { }

    template<class F>
    void
    run(F&& f) { queue.put(std::forward<F>(f)); }

    void
    wait() {
      pool.help(true); 	// Help out instead of sitting around idly.
      queue.wait();
    }

    void
    join() {
      queue.shutdown();
      pool.join();
    }

    virtual
    ~HomogenousThreadPool() { wait(); join(); }

    /**
     * Run a function on all members of a container
     *
     * \param container
     *        The container to process
     * \param fun
     *        The function taking one parameter by reference and returning void.
     *
     * Does not wait for all tasks to finish! Caller is
     * responsible for wait()ing on the pool if necessary.
     */
    template<class Container, class F>
    F
    run_for_each(Container&& container, F&& fun) {
      for (auto& e: container) run([&fun,&e](){ fun(e); });
      return std::forward<F>(fun);
    }
  };

} // End of namespace threadpool

#endif // !defined(THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H)
