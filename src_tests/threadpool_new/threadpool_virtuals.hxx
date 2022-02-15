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

} // End of namespace threadpool
