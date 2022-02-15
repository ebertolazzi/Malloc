namespace threadpool {

  /**
   * VirtualTask type for thread pool
   *
   * The thread pool wraps functions into object of type
   * VirtualTask and enqueues them. The actual tasks can be
   * heterogenous and must only support the VirtualTask interface.
   */
  class VirtualTask {
  public:
    /**
     * The payload, users function to be run.
     *
     * Operator() is run from the thread pool.
     * Is responsible for deleting the task object once it is done.
     */
    virtual void operator()() = 0;

    /**
     * Destroy the task object
     */
    virtual ~VirtualTask() {};
  };

  /**
   * Interface of virtual thread pool.
   *
   * Implements the functionality of the virtual thread pool. Only
   * provides an interface to run a generic VirtualTask.
   * The convenience functions to run
   * different types of callable objects must be implemented in a
   * subclass.
   *
   */
  class ThreadPoolInterface {
	public:

    virtual void run(std::unique_ptr<VirtualTask>&& c) = 0;
    virtual void run(VirtualTask* c) = 0;
    /**
     * Wait for all active tasks to finish.
     *
     * Also throws an exception if one of the tasks has encountered
     * an uncatched exception.
     *
     * Leaves the pool in a valid state ready to run more tasks, unless
     * an exception has been thrown.
     */
    virtual void wait() = 0;
    /**
     * Discard all tasks from the queue that have not yet started
     * and wait for all threads to return.
     *
     * Also throws an exception if one of the tasks has encountered
     * an uncatched exception.
     *
     * Leaves the pool in a shutdown state not ready to run tasks,
     * but ready for destruction.
     */
    virtual void join() = 0;

    /**
     * Destroy the thread pool.
     *
     * Does the equivalent of wait() and join() before the thread pool
     * is destructed. This means, the destructor can hang a long time
     * and can throw an exception (unless wait() or join() have been
     * called before the destructor).
     */
    virtual ~ThreadPoolInterface() { };
  };

  /**
   * A generic thread pool interface.
   *
   * @tparam class Queue
   *			The queue delivering the tasks.
   *
   *	Class Queue must provide the following members:
   *
   *	- void work()
   *			Gets tasks and works until the end of the
   *			queue is reached.
   *	- void shutdown()
   *			Causes the queue to return invalid tasks,
   *			which will cause all threads to return.
   *
   *
   * As soon as the pool is created the threads start running tasks
   * from the queue until the queue is empty. When the queue is
   * empty the threads return and are ready to be collected by
   * join() or by the destructor. The following constructor arguments
   * should be supported:
   *
   * - Queue& queue
   *			The queue delivering the tasks.
   *
   * - int thread_count
   *			The number of threads to use. If the
   *			thread count is not specified (default
   *			value -1) it defaults to the number of
   *			available hardware threads
   *			std::thread::hardware_concurrency(),
   *			as read through
   *			hardware_concurrency().
   */

  class GenericThreadPoolInterface {
  public:
    GenericThreadPoolInterface() { }

    /**
     * Help with the work.
     *
     * @param return_if_idle
     *		Never wait for work, return instead.
     *
     * This function is called by all threads wanting to help.
     */
    virtual void help(bool return_if_idle) = 0;

    /**
     * Rethrow a potentially pending exception from a worker
     * thread.
     */
    virtual void rethrow_exception() = 0;

    /**
     * Wait for all threads to finish and collect them.
     *
     * Leaves the thread pool ready for destruction.
     */
    virtual void join() = 0;

    /**
     * Destroy the thread pool.
     *
     * Generally a destructor should not wait for a long time, and
     * it should not throw any exceptions. Unfortunately threads
     * are not abortable in C++.  The only way to make sure the
     * threads have terminated is to wait for them to return by
     * themselves.  If we would allow the destruction of the thread
     * pool before the threads have returned, the threads would
     * continue to access the memory of the destroyed thread pool,
     * potentially clobbering other objects residing in the
     * recycled memory.  We could allocate parts of the memory
     * with new, and leave it behind for the threads after the
     * thread pool is destructed.  But even then, the user supplied
     * functions run by the threads might access memory that gets
     * destroyed if the function that constructed the thread pool
     * terminates.  The danger of undetected and undebuggable
     * memory corruption is just too big.
     *
     * With regard to the exceptions rethrown in the destructor,
     * it is better to signal the exception than to ignore it
     * silently.
     *
     * If it is not acceptable for the destructor to wait or to
     * throw an exception, just call join() before the pool is
     * destructed.  After join() the destructor is guaranteed to
     * run fast and without exceptions.
     *
     * If it should really be necessary to keep threads running
     * after the function that created the thread pool returns,
     * just create the thread pool on the heap with new. And if
     * you want to make sure nobody destroys the thread pool, feel
     * free to throw away the handle.
     *
     * It is not possible to use the default nothrow version.
     */
    virtual ~GenericThreadPoolInterface() { }
  };

} // End of namespace threadpool
