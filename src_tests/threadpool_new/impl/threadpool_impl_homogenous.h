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

  namespace impl {

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
     * @param queue
     *			The queue managing the tasks.
     *
     * @param thread_count
     *			The number of threads to use. If the
     *			thread count is not specified it
     *			defaults to the number of available
     *			hardware threads
     *			std::thread::hardware_concurrency(),
     *			as read through
     *			hardware_concurrency().
     *
     */
    GenericThreadPoolTmpl(VirtualQueue& queue, int thread_count = -1);

    /**
     * Help with the work.
     *
     * @param return_if_idle
     *		Never wait for work, return instead.
     */
    void help(bool return_if_idle) override;

    /**
     * Rethrow a potentially pending exception from a worker
     * thread.
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
     * @param c
     *		The hardware concurrency to use
     */
    static unsigned int hardware_concurrency(int c = -1);

    /**
     * Determine thread count to use based on users
     * specifications.
     *
     * @param thread_count
     *		Runtime specified threadcount parameter.
     *
     * @returns
     *		The number of threads to use. Returns always a
     *		positive number, possibly 1 for
     *		single-processor systems.
     *
     * This policy function does just some
     * guesswork. Allocating a number of threads in the order
     * of the hardware threads may be a good bet for CPU-bound
     * work. For other tasks it depends.
     */
    static unsigned int determine_thread_count(int thread_count = -1);

    /**
     * Switch exception handling off
     */
    static bool ignore_thread_pool_exceptions(bool i = true);

	};



  /**
   * A thread pool reading its tasks from a generic queue.
   *
   * @tparam class Queue
   *			The queue delivering the tasks.
   *
   *	Class Queue must provide the following members:
   *
   *	- void work()
   *			Gets tasks and works until the end of
   *			the queue is reached.
   *
   *	- void shutdown()
   *			Causes the queue to tell the threads
   *			asking for work to return.
   *
   */
  template<class Queue>
  class GenericThreadPoolImpl : public GenericThreadPoolInterface {
    class Worker {
      std::thread thread;
    public:
      Worker & operator=(std::thread&& t)
      { thread = std::move(t); return *this; }

      void join() { if (thread.joinable()) thread.join(); }
    };

    std::mutex         mutex;
    std::exception_ptr pending_exception;
    Queue&             queue;

    unsigned const thread_count; /// The number of threads

    std::vector<Worker> workers;

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
      for (Worker& w: workers) w.join();
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
     * @param queue
     *			The queue delivering the tasks.
     *
     * @param thread_count The number of threads
     *			to use. If the thread count is not
     *			specified it defaults to the number of
     *			available hardware threads
     *			std::thread::hardware_concurrency(),
     *			as read through hardware_concurrency().
     *
     */
    GenericThreadPoolImpl(Queue& queue, int thread_count)
    : pending_exception(nullptr)
    , queue(queue)
    , thread_count(determine_thread_count(thread_count))
    , workers(this->thread_count)
    {
      for (Worker& w: workers)
        w = std::move(std::thread(std::bind(&GenericThreadPoolImpl::work, this)));
    }

    /**
     * Help with the work.
     *
     * @param return_if_idle
     *		Never wait for work, return instead.
     */
    void
    help(bool return_if_idle) override {
      if (ignore_thread_pool_exceptions()) {
        queue.work(return_if_idle);
      } else {
        try {
          queue.work(return_if_idle);
        } catch (...) {
          {
            std::exception_ptr e = std::current_exception();
            std::lock_guard<std::mutex> lock(mutex);
            if (!pending_exception) pending_exception = std::move(e);
          }
          queue.shutdown();
        }
      }
    }

    /**
     * Rethrow a potentially pending exception from a worker
     * thread.
     */
    void
    rethrow_exception() override {
      if (pending_exception && !std::uncaught_exception()) {
        queue.shutdown();
        join_workers();
        if (!std::uncaught_exception()) {
          std::exception_ptr e = pending_exception;
          pending_exception = nullptr;
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
      if (std::uncaught_exception()) queue.shutdown();
      join(); // Running threads would continue to access the destructed pool.
    }

    /**
     * Cache the hardware concurrency so we are sure that it
     * is cheap to get. Also this gives us a point to
     * cheat. The cached value can be modified by a parameter.
     *
     * @param c
     *		The hardware concurrency to use
     */
    static unsigned int hardware_concurrency(int c = -1) {
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
     * @param thread_count
     *		Runtime specified threadcount parameter.
     *
     * @returns
     *		The number of threads to use.
     *
     * This policy function does just some
     * guesswork. Allocating a number of threads in the order
     * of the hardware threads may be a good bet for CPU-bound
     * work. For other tasks it depends.
     */
    static unsigned int determine_thread_count(int thread_count = -1) {
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

  unsigned int
  GenericThreadPoolTmpl::hardware_concurrency(int c) {
    return GenericThreadPoolImpl<VirtualQueue>::hardware_concurrency(c);
  }

  unsigned int
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
   * @tparam Function
   *		The function type to be used for the
   *		queue. All tasks to be queued must be of this
   *		type. The class Function must support the
   *		following interface:
   *		- operator()(): the functions must be callable
   *		  with no parameters.
   */
  template<class Function>
  class HomogenousThreadPool {

    /**
     * Queue for functions with signature void()
     *
     * This queue is dependent on template parameter
     * Function. Only one type of functions can be queued. For
     * example if this queue is instantiated for a lambda
     * function, only this exact lambda function can be
     * queued, since each lambda function has its own type
     * separate from all other lambda functions.
     *
     * To make this queue more flexible, instantiate it either
     * with std::function<void()>, or use the virtual thread
     * pool VirtualThreadPool.
     *
     * @tparam Function
     *		The function type to queue.
     */
    class HQueue : public VirtualQueue {

		/*
		  If we would use a deque, we would have to protect
		  against overlapping accesses to the front and the
		  back. The standard containers do not allow this. Use a
		  vector instead.  With a vector it is possible to access
		  both ends of the queue at the same time, as push()ing
		  and pop()ing does not modify the container itself but
		  only its elements.
		*/
		class Queue {
		    union Fun {
#ifdef _MSC_VER // Work around Visual C++ bug, does not like constructable objects in unions
			alignas(Function) char fun[sizeof(Function)];
#else // Standard conforming, C++11 9.5
			Function fun; // Only used between pop_ptr and push_ptr
#endif
			Fun() noexcept { }
			Fun(const Fun&) noexcept { }
			Fun(Fun&&) noexcept { }
			~Fun() noexcept { }
		    };
		    std::vector<Fun> impl;
		    std::size_t push_ptr = 0, pop_ptr = 0;

		    Queue(const Queue&) = delete;
		    Queue(Queue&&) = delete;
		    Queue& operator=(const Queue&) = delete;
		    Queue& operator=(Queue&&) = delete;

		public:

		    Queue(std::size_t s) : impl(s + 1) { }

		    template<class F> void push(F&& f) {
			new (&impl[push_ptr].fun) Function(std::forward<F>(f));
			if (++push_ptr == impl.size()) push_ptr = 0;
		    }

		    Function pop() {
#ifdef _MSC_VER // Work around Visual C++ bug, does not like constructable objects in unions
			Function r = std::move(reinterpret_cast<Function&>(impl[pop_ptr].fun));
			reinterpret_cast<Function&>(impl[pop_ptr].fun).~Function();
#else
			Function r = std::move(impl[pop_ptr].fun);
			impl[pop_ptr].fun.~Function();
#endif
			if (++pop_ptr == impl.size()) pop_ptr = 0;
			return r;
		    }

		    std::size_t size() const {
			std::size_t r = push_ptr + impl.size() - pop_ptr;
			if (r >= impl.size()) r -= impl.size();
			return r;
		    }

		    bool empty() const {
			return push_ptr == pop_ptr;
		    }

		    std::size_t capacity() {
			return impl.size() - 1;
		    }

		    void reserve(std::size_t s) {
			assert(empty()); // Copying / moving of Fun not supported.
			if (s >= impl.size())
			    impl.resize(s + 1);
		    }

		    ~Queue() { while (!empty()) pop(); }
		};



		/*
		  This queue requires attention for protection against
		  concurrent access. Protect against:
		  - Concurrent access by two worker threads both
		    wanting to get() a task from the queue at the same
		    time.
		  - Concurrent access by two threads both wanting to
		    put() a task into the queue at the same time.
		  - A worker thread having determined that the queue
		    is empty, while at the same time a new task is put()
		    into the queue.
		  - A task wanting to put() a task into the queue
		    having found the queue full, while at the same time
		    the queues fill level decreases.
		*/

		const std::size_t queue_size;
		const std::size_t maxpart;
		bool shutting_down;
		unsigned int idle_workers;
		unsigned int total_workers;
		bool wakeup_is_pending;
		Queue queue;
		std::mutex pop_mutex;
		std::mutex push_mutex;
		std::condition_variable waiting_workers;
		std::condition_variable waiters;

		/**
		 * Get tasks and execute them. Return as soon as the queue
		 * shrinks to `return_if_idle` tasks.
		 */
		void help(std::ptrdiff_t return_if_idle) {

		    std::size_t min_queue_size = return_if_idle < 0 ? 0 : return_if_idle;

		    // Increment total worker count, decrement again on scope exit
		    { std::lock_guard<std::mutex> lock(push_mutex); ++total_workers; }
		    //std::cerr << " total_workers(" << this->total_workers << ")";
		    auto x1 = at_scope_exit([this](){
			    std::lock_guard<std::mutex> lock(push_mutex);
			    if (--this->total_workers == this->idle_workers)
				this->waiters.notify_all();;
			});

		    Queue functions(1);

		    for (;;) {
			std::unique_lock<std::mutex> lock(pop_mutex);

			std::size_t queue_size;

			// Try to get the next task(s)
			while ((queue_size = queue.size()) <= min_queue_size) {
			    if (static_cast<std::ptrdiff_t>(queue_size) <= return_if_idle)
				return;
			    if (queue_size)
				break;

			    // The queue is empty, wait for more tasks to be put()

			    lock.unlock();

			    {
				std::unique_lock<std::mutex> lock(push_mutex);
				while (queue.empty() && !shutting_down) {

				    if (++idle_workers == total_workers)
					waiters.notify_all();;

				    waiting_workers.wait(lock); // Wait for task to be queued
				    wakeup_is_pending = false;

				    --idle_workers;
				}

			    }

			    if (shutting_down)
				return;

			    lock.lock();
			}

			// There is at least one task in the queue and the back is locked.

			std::size_t stride =
			    (maxpart == 0) ? 1 : queue_size / maxpart;
			if (stride <= 0)
			    stride = 1;
			if (stride > functions.capacity())
			    functions.reserve(2 * stride);

			while (stride--)
			    functions.push(queue.pop());

			lock.unlock();

			if (idle_workers && !wakeup_is_pending && queue_size)
			    waiting_workers.notify_one();

			while (!functions.empty())
			    functions.pop()();
		    }
		}

		/**
		 * Help, and shut down if an exception escapes.
		 */
		void try_help(std::ptrdiff_t return_if_idle) {
		    try {
			help(return_if_idle);
		    } catch (...) {
			shutdown();
			throw;
		    }
		}



	    public:

		HQueue(std::size_t queue_size, std::size_t maxpart)
		    : queue_size(queue_size ? queue_size : 10000),
		      maxpart(maxpart),
		      shutting_down(false),
		      idle_workers(0),
		      total_workers(0),
		      wakeup_is_pending(false),
		      queue(this->queue_size)
		{ }

		/**
		 * Get tasks and execute them. If `return_if_idle`, return
		 * instead of idly waiting.
		 */
		void work(bool return_if_idle) override {
		    help(return_if_idle ? 0 : -1);
		}

		/**
		   Enqueue a task.
		*/
		template<class C>
		void put(C&& c) {

		    std::unique_lock<std::mutex> lock(push_mutex);

		    while (queue.size() >= queue_size) {
			// No space in the queue. Must wait for workers to advance.

			lock.unlock();

			try_help(queue_size / 2);

			lock.lock();
		    }

		    // Now there is space in the queue and we have locked the back.

		    // Enqueue function.
		    if (shutting_down) {

			Function fun(std::forward<C>(c)); // Run Function destructor

		    } else {
			/*
			  Here we have exclusive access to the head of the
			  queue.
			*/
			queue.push(std::forward<C>(c));

			if (idle_workers && !wakeup_is_pending) {
			    wakeup_is_pending = true;
			    waiting_workers.notify_one();
			}
		    }
		}

		void shutdown() override {
		    std::unique_lock<std::mutex> push_lock(push_mutex);
		    std::unique_lock<std::mutex> pop_lock(pop_mutex);
		    shutting_down = true;
		    while (!queue.empty())
			queue.pop();
		    waiting_workers.notify_all();
		    waiters.notify_all();
		}

		void wait() {
		    if (std::uncaught_exception())
			shutdown();
		    std::exception_ptr e;
		    std::unique_lock<std::mutex> lock(push_mutex);
		    while (!queue.empty() || idle_workers != total_workers) {
			while (!queue.empty()) {
			    lock.unlock();
			    try {
				try_help(0);
			    } catch (...) {
				if (e == nullptr)
				    e = std::current_exception();
			    }
			    lock.lock();
			}
			while (idle_workers != total_workers)
			    waiters.wait(lock);
		    }
		    if (e != nullptr && !std::uncaught_exception())
			std::rethrow_exception(std::move(e));
		}
	    };

	    HQueue queue;
	    GenericThreadPoolTmpl pool;

	public:

	    explicit HomogenousThreadPool(int thread_count = -1,
					  std::size_t queue_size = 0,
					  std::size_t maxpart = 1)
		: queue(queue_size,
			(maxpart != 1) ? maxpart
			: 3 * (GenericThreadPoolTmpl::determine_thread_count(thread_count)+ 1)),
		  pool(queue, thread_count)
	    { }

	    template<class F>
	    void run(F&& f) {
		queue.put(std::forward<F>(f));
	    }

	    void wait() {
		pool.help(true); 	// Help out instead of sitting around idly.
		queue.wait();
	    }

	    void join() {
		queue.shutdown();
		pool.join();
	    }

	    virtual ~HomogenousThreadPool() {
		wait();
		join();
	    }

	    /**
	     * Run a function on all members of a container
	     *
	     * @param container
	     *			The container to process
	     * @param fun
	     *			The function taking one parameter
	     *			by reference and returning void.
	     *
	     * Does not wait for all tasks to finish! Caller is
	     * responsible for wait()ing on the pool if necessary.
	     */
	    template<class Container, class F>
	    F run_for_each(Container&& container, F&& fun) {
		for (auto& e: container)
		    run([&fun,&e](){
			    fun(e);
			});
		return std::forward<F>(fun);
	    }

	};

    } // End of namespace impl

} // End of namespace threadpool

#endif // !defined(THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H)
