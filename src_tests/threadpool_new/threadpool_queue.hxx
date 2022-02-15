namespace threadpool {

  /**
   * A Queue interface for the generic thread pool.
   */
  class VirtualQueue {
  public:

    /**
     * Work on items in the queue. If there are no more items to
     * work on, but more items could possible be added, then wait
     * for new items unless parameter return_if_idle is true.
     *
     * @param return_if_idle
     *		Return if there is no more work in the moment.
     */
    virtual void work(bool return_if_idle) = 0;

    /**
     * Shut the queue down.
     *
     * Irreversably terminates processing. All idle workers should be
     * woken up and return from work() as soon as the current task
     * is processed.
     */
    virtual void shutdown() = 0;
    virtual ~VirtualQueue() { };
  };

} // End of namespace threadpool
