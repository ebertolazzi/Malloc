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

  /*
    The thread pool for arbitrary functions works fine, and can be
    used to process elements of a container. But this means queuing
    a task for each element, with each task executing the same
    function on another element of the container. Certainly there
    is the possibility for optimization.
    Define a queue calling the same function for each object in an
    iterator range. This means we do not need a true queue but just
    an object using incremental values of an iterator until the end
    of the range is reached.
  */

  /**
   * Queue calling the function on single objects.
   *
   * @relates ForEachThreadPoolImpl
   *	Conceptually ForEach_Queue is a member
   *	of class ForEachThreadPoolImpl, but the standard does
   *	not allow template specialization inside classes. I
   *	had to move it out of the class.
   */
  template<class Iterator, class Last, class Function, bool forward_iterator>
  class ForEach_Queue : public VirtualQueue {
  protected:
    Iterator   & m_current;
    Last const & m_last;
    Function   & m_fun;
    std::mutex   m_mutex; // Make sure threads do not access concurrently

  public:

    ForEach_Queue(
      Iterator   & first,
      Last const & last,
      Function   & fun,
      std::size_t /*ignored*/ = 0
    )
    : m_current(first)
    , m_last(last)
    , m_fun(fun)
    { }

    void
    work(bool /*ignored*/) override {
      typedef iterval_traits<Iterator> IT;
      Last const & l(m_last);
      for (;;) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_current == l) break;
        typename IT::type v(IT::copy(m_current));
        ++m_current;
        lock.unlock();
        m_fun(IT::pass(std::move(v)));
      }
    }

    /**
     * Shut the queue down, stop returning values
     */
    void
    shutdown() override {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_current = m_last;
    }
  };

  /*
    The queue just implemented would work fine. But if there are
    a lot of tasks with each task taking a very short time, it may
    cause a lot of overhead because each object is dequeued
    separately. Wouldn't it be nice if we could deliver larger
    tasks?
  */

  /**
   * Run a function on objects from a container.
   *
   * Queue with `forward_iterator` == false takes groups of
   * objects from the queue.
   *
   * This works only for random access iterators. The
   * specialization is selected with template parameter
   * forward_iterator = true. For all other iterators, use the
   * general case of the template above.
   *
   * @relates ForEachThreadPoolImpl
   *	Conceptually ForEach_Queue is a member
   *	of class ForEachThreadPoolImpl, but the standard does
   *	not allow template specialization inside classes. I
   *	had to move it out of the class.
   */
  template<class Iterator, class Last, class Function>
  class ForEach_Queue<Iterator, Last, Function, true> : public ForEach_Queue<Iterator, Last, Function, false> {
    typedef ForEach_Queue<Iterator, Last, Function, false> Base;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    std::size_t const m_maxpart;
    difference_type   m_remaining;
  public:

    ForEach_Queue(
      Iterator   & first,
      Last const & last,
      Function   & fun,
      std::size_t maxpart
    )
    : Base(first, last, fun)
    , m_maxpart(maxpart)
    , m_remaining(std::distance(first, last))
    { }

    void
    work(bool /*ignored*/) override {
      Last const & last = this->m_last; // Does never change
      for (;;) {
        Iterator c, l;
        {
          std::lock_guard<std::mutex> lock(this->m_mutex);
          if ((c = this->m_current) == last) break;
          difference_type stride = (m_maxpart == 0) ? 1 : m_remaining / m_maxpart;
          if (stride <= 0) stride = 1;
          l = c;
          std::advance(l, stride);
          this->m_current = l;
          m_remaining -= stride;
        }
        while (c != l) { this->m_fun(*c); ++c; }
      }
    }
  };

} // End of namespace threadpool
