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

  /*
    Now write a parallel version of std::transform().
    We could reuse a few parts from the parallel for_each
    implementation, but this would not save much and create a
    dependence. Better to keep parallel_transform self-standing.
  */

  /**
   * Run a function on objects from a container, store the return
   * values in a result container.
   *
   * General case for arbitrary iterators. This is the difficult
   * case to implement, because the worker threads must synchronize
   * after they have done their work, and must make sure they write
   * the results in the order of the input objects and not in the
   * order the threads are finished. But this is the thing that
   * makes parallel_transform interesting for the users. They don't
   * need to synchronize, the algorithm makes it for them.
   *
   * \tparam InputIterator
   *         Type of the input iterator. In this
   *         specialization with forward_iterator = false,
   *         an arbitrary input iterator type.
   *
   * \tparam OutputIterator
   *         Type of the result iterator. In this
   *         specialization with forward_iterator = false,
   *         an arbitrary output iterator type.
   * \tparam Function
   *         Type of the function to be called with
   *         successive elements from the input iterator.
   *         The function must return a result
   *         which is stored through the result iterator.
   *
   * \tparam forward_iterator
   *         A bool selecting the specialization.
   *         The general case for arbitrary input and output
   *         iterators which is implemented here is selected
   *         with forward_iterator = false. The specialization
   *         for forward iterators follows below.
   *
   * \relates TransformThreadPoolImpl
   *          Transform_Queue is conceptually a member of class
   *          TransformThreadPoolImpl, but the standard does not
   *          allow template specialization inside classes.
   *          I had to move it out of the class.
   */
  template<
    class InputIterator,
    class Last,
    class OutputIterator,
    class Function,
    bool forward_iterator
  >
  class Transform_Queue : public VirtualQueue {
    struct Results {
      typename std::remove_reference<decltype(std::declval<Function&>()(*std::declval<InputIterator>()))>::type result;
      std::unique_ptr<Results> next;
    };

    InputIterator  & m_current;
    Last const     & m_last;
    OutputIterator & m_result;
    Function       & m_fun;
    std::mutex       m_mutex;
    bool             m_do_shutdown = false;

    typedef unsigned long long int counter_type;
    counter_type            m_input_counter    = 1; // Counter of objects got from the queue
    counter_type            m_output_counter   = 1; // Counter of objects written
    Results *               m_previous_results = nullptr;
    counter_type            m_max_output_queue_length = 1000; // This should be configurable
    std::mutex              m_output_mutex;
    std::condition_variable m_output_queue;
    std::size_t             m_output_queue_waiters = 0;

  public:

    Transform_Queue(
      InputIterator  & first,
      Last const     & last,
      OutputIterator & result,
      Function       & fun,
      std::size_t
    )
    : m_current(first)
    , m_last(last)
    , m_result(result)
    , m_fun(fun)
    { }

    void
    work(bool return_if_idle) override {
      typedef iterval_traits<InputIterator> IT;

      std::unique_ptr<Results> results;
      Last const & last = m_last; // Does never change.
      for (;;) {
        if (!results) results = std::unique_ptr<Results>(new Results);
        counter_type ctr;
        Results* prvres;
        {
          std::unique_lock<std::mutex> lock(m_mutex);
          if (m_current == last) break;
          ctr                = m_input_counter;
          prvres             = m_previous_results;
          m_previous_results = &*results;
          typename IT::type v(IT::copy(m_current));
          ++m_current;
          m_input_counter = ctr + 1;
          lock.unlock();
          results->result = fun(IT::pass(std::move(v)));
        }
        {
          /*
            We must store the results in the order they had
            in the input sequence, not in the order the
            tasks finish. Just work together: whoever is
            ready before his predecessor just leaves his
            work for the predecessor to clean up.
          */
          std::unique_lock<std::mutex> lock(m_output_mutex);
          while (ctr - m_output_counter > m_max_output_queue_length) {
            if (m_do_shutdown) return;
            if (return_if_idle) {
              prvres->next = std::move(results);
              return;
            }
            ++m_output_queue_waiters;
            m_output_queue.wait(lock);
            --m_output_queue_waiters;
          }
          if (m_output_counter == ctr) {
            // Predecessor is done, we can store our things.
            lock.unlock();
            *m_result = std::move(results->result);
            ++m_result;
            ++ctr;
            lock.lock();
            // Now look whether our successors have left us their work.
            while (results->next) {
              results = std::move(results->next);
              lock.unlock();
              *m_result = std::move(results->result);
              ++m_result;
              ++ctr;
              lock.lock();
            }
            m_output_counter = ctr;
            if (m_output_queue_waiters) m_output_queue.notify_all(); // All because we do not know who is the right one.
          } else {
            // Predecessor still running, let him clean up.
            prvres->next = std::move(results);
          }
        }
      }
    }

    /**
     * Shut the queue down, stop returning values
     */
    void
    shutdown() override {
      std::lock_guard<std::mutex> lock(m_mutex);
      std::lock_guard<std::mutex> olock(m_output_mutex);
      m_current     = m_last;
      m_do_shutdown = true;
      m_output_queue.notify_all();
    }
  };

  /**
   * Run a function on objects from a container, store the return
   * values in a result container.
   *
   * Specialization for forward iterators. It is used when
   * template argument forward_iterator is true. For all other
   * iterators, use the generic version above.
   *
   * \tparam InputIterator
   * Type of the input iterator.
   * In this specialization with forward_iterator = false,
   * an arbitrary input iterator type.
   *
   * \tparam OutputIterator
   * Type of the result iterator.
   * In this specialization with forward_iterator = false,
   * an arbitrary output iterator type.
   *
   * \tparam Function
   * Type of the function to be called with
   * successive elements from the input iterator.
   * The function must return a result
   * which is stored through the result iterator.
   *
   * \relates TransformThreadPoolImpl
   * Transform_Queue is conceptually a member
   * of class TransformThreadPoolImpl, but the standard
   * does not allow template specialization inside classes.
   * I had to move it out of the class.
   */
  template<class InputIterator, class Last,class OutputIterator, class Function>
  class Transform_Queue<InputIterator, Last, OutputIterator, Function, true>: public VirtualQueue {
    typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;
    InputIterator   & m_current;
    Last const      & m_last;
    OutputIterator  & m_result;
    Function        & m_fun;
    std::mutex        m_mutex;
    std::size_t const m_maxpart;
    difference_type   m_remaining;
  public:
    Transform_Queue(
      InputIterator  & first,
      Last const     & last,
      OutputIterator & result,
      Function       & fun,
      std::size_t      maxpart
    )
    : m_current(first)
    , m_last(last)
    , m_result(result)
    , m_fun(fun)
    , m_maxpart(maxpart)
    , m_remaining(std::distance(first, last))
    { }

    void
    work(bool) override {
      Last const & last = m_last; // Does never change
      for (;;) {
        InputIterator  c, l;
        OutputIterator r;
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          if ((c = m_current) == last) break;
          difference_type stride = (m_maxpart == 0) ? 1 : m_remaining / m_maxpart;
          if (stride <= 0) stride = 1;
          l = c;
          std::advance(l, stride);
          r = m_result;
          m_current = l;
          std::advance(m_result, stride);
          m_remaining -= stride;
        }
        while (c != l) { *r = m_fun(*c); ++c; ++r; }
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

} // End of namespace threadpool
