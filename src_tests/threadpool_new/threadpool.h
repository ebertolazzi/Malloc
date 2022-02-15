/** @file threadpool/threadpool.h
 *
 * Threadpool for C++11, header for thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H


#include <cstddef>
#include <memory>
#include <type_traits>  // For std::remove_reference()
#include <functional>		// For std::bind()
#include <cassert>
#include <iterator>
#include <utility>	    // For std::move(), std::forward()

#include <vector>
#include <limits>
#include <limits>

#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>


#include "threadpool_config.h"
#include "threadpool_utils.hxx"
#include "threadpool_virtuals.hxx"

namespace threadpool {

  /**
   * Store pointers into the queue. Decorate the pointers
   * with an operator() to make them callable as needed by
   * HomogenousThreadPool.
   *
   * I tried std::unique_ptr but at least with g++ it was
   * very slow. Seems to do some heavyweight locking. Using
   * raw pointers, we can just delete them in the tasks
   * operator() or in the destructor. This is even more
   * flexible than using std::unique_ptr, there may be use
   * cases where the task shall outlive the execution by the
   * thread pool, for example for cleanup work to be done.
   *
   * Delete copy-constructor and copy-assignment so `pimpl`
   * is not deleted twice. The move constructor makes sure
   * to leave an empty `pimpl` behind.
   */
  class QueueElement {
    VirtualTask * pimpl;

    QueueElement() = delete;
    QueueElement(const QueueElement&) = delete;
    QueueElement& operator=(const QueueElement&) = delete;
    QueueElement& operator=(QueueElement&&) = delete;

  public:

    QueueElement(VirtualTask* t)	: pimpl(t) { }
    QueueElement(QueueElement&& x) : pimpl(x.pimpl) { x.pimpl = nullptr; }
    void operator()() { (*pimpl)(); pimpl = nullptr; }
    ~QueueElement() { if (pimpl) delete pimpl; }
  };

  /**
   * ThreadPool
   *
   * Builds a compiler firewall around VirtualThreadPoolImpl so
   * that the thread pool can be used without seeing the
   * internals.This also speeds up compilation because the
   * compiler does not see the implementation.
   *
   * Defines only the implementation and not the usability
   * member functions that make it easy to run tasks. The
   * derived class ThreadPool defines these.
   *
   * This will only ever by used with template parameter 0. We
   * could define the class directly, but then it would not be
   * allowed to include the class definition in multiple
   * separately compiled files. By making it a class *template*,
   * we profit from the fact that multiple implicit
   * instantiations of a template are allowed. This means when
   * the user switches between header-only and library
   * configuration he does not need to recompile everything, and
   * the ODR is not violated.
   */
  class ThreadPool {

    std::unique_ptr<VirtualThreadPoolInterface> pimpl;

  public:

    explicit
    ThreadPool(
      int         thread_count = -1,
      std::size_t queue_size   = 0,
      std::size_t maxpart      = 1
    );

    void
    run(std::unique_ptr<VirtualTask>&& t) {
      pimpl->run(std::move(t));
    }

    void run(VirtualTask* t) { pimpl->run(t); }

    /**
     * Wrap void functions in a task and run them without
     * exception handling.
     */
    template<class Function>
    typename std::enable_if<!std::is_pointer<typename std::remove_reference<Function>::type>::value &&
           std::is_void<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>::value
    >::type
    run( Function&& f ) {
      typedef typename std::remove_reference<Function>::type function_type;
      class WrappedFunction : public VirtualTask {
        Function f;
      public:
        WrappedFunction(function_type&& f) : f(std::move(f)) { }
        virtual void operator()() override { f(); delete this; }
      };
      run(new WrappedFunction(std::forward<Function>(f)));
    }

    /**
     * For functions with nonvoid return type, catch exceptions
     * and return a future.
     */
    template<class Function>
    typename std::enable_if<!std::is_pointer<typename std::remove_reference<Function>::type>::value &&
       !std::is_void<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>::value,
       std::future<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>
    >::type
    run(Function&& f) {
      typedef typename std::remove_reference<Function>::type function_type;
      typedef typename std::result_of<Function()>::type return_type;

      class WrappedFunction : public VirtualTask {
        Function                  f;
        std::promise<return_type> promise;
      public:
        WrappedFunction(function_type&& f) : f(std::move(f)) { }
        WrappedFunction(function_type& f) : f(f) { }

        std::future<return_type> get_future() { return promise.get_future(); }

        virtual
        void
        operator()() override {
          try {
            promise.set_value(f());
          } catch (...) {
            promise.set_exception(std::current_exception());
          }
          delete this;
        }
      };

      WrappedFunction* task(new WrappedFunction(std::forward<Function>(f)));
      std::future<return_type> future(task->get_future());
      run(task);
      return future;
    }

    /**
     * Run a function on all objects in an iterator range
     *
     * @param first
     *			Start of the range
     *
     * @param last
     *			End of the range
     *
     * @param fun
     *			The function taking one parameter
     *			by reference and returning void.
     *
     * Does not wait for all tasks to finish! Caller is
     * responsible for wait()ing on the pool if necessary.
     */
    template <class Iterator, class Function>
    void
    for_each( Iterator first, const Iterator& last, Function&& fun );

    /**
     * Run a function on all members of a container
     *
     * @param container
     *			The container to process
     *
     * @param fun
     *			The function taking one parameter
     *			by reference and returning void.
     *
     * Does not wait for all tasks to finish! Caller is
     * responsible for wait()ing on the pool if necessary.
     */
    template<class Container, class Function>
    void
    for_each( Container&& container, Function&& fun ) {
      for ( auto & e: container ) run([&fun,&e](){ fun(e); });
    }



    /**
     * Wait for all active tasks to finish.
     *
     * Also throws an exception if one of the tasks has
     * encountered an uncatched exception.
     *
     * Leaves the pool in a valid state ready to run more
     * tasks, unless an exception has been thrown.
     */
    void wait() { pimpl->wait(); }



    /**
     * Discard all tasks from the queue that have not yet
     * started and wait for all threads to return.
     *
     * Also throws an exception if one of the tasks has
     * encountered an uncatched exception.
     *
     * Leaves the pool in a shutdown state not ready to run
     * tasks, but ready for destruction.
     */
    void join() { pimpl->join(); }


    /**
     * Destroy the thread pool.
     *
     * Does the equivalent of wait() and join() before the
     * thread pool is destructed. This means, the destructor
     * can hang a long time and can throw an exception (unless
     * wait() or join() have been called before the
     * destructor).
     */
    ~ThreadPool() {}

	};
}

#include "impl/threadpool_impl_homogenous.h"

namespace threadpool {

  /**
   * Implementation of virtual thread pool.
   *
   * Implements the functionality of the virtual thread
   * pool. Only provides an interface to run a generic
   * VirtualTask. The convenience functions to run
   * different types of callable objects should be implemented
   * in a subclass.
   *
   * The template parameter is not used, only serves to make
   * this a class template which can be instantiated in multiple
   * compilation units without giving multiply defined symbol
   * errors.
   */
  class VirtualThreadPoolImpl : public VirtualThreadPoolInterface {
    HomogenousThreadPool<QueueElement> impl;
  public:
    explicit
    VirtualThreadPoolImpl(
      int         thread_count = -1,
      std::size_t queue_size   = 0,
      std::size_t maxpart      = 1
    )
    : impl(thread_count, queue_size, maxpart)
    { }

    void
    run(std::unique_ptr<VirtualTask>&& t)
    { impl.run(t.release()); }

    void
    run(VirtualTask* t)
    { impl.run(t); }

    void wait() { impl.wait(); }
    void join() { impl.join(); }
  };


  ThreadPool::ThreadPool(
    int         thread_count,
    std::size_t queue_size,
    std::size_t maxpart
  )
  : pimpl(new VirtualThreadPoolImpl(thread_count, queue_size, maxpart))
  {  }

  template <class Iterator, class Function>
  void
  ThreadPool::for_each( Iterator first, Iterator const & last, Function && fun ) {
    while (first != last) {
      typedef iterval_traits<Iterator> INTERVAL_TRAITS;
      Wrap<typename INTERVAL_TRAITS::type> e(INTERVAL_TRAITS::copy(first));
      ++first;
      run([&fun,e](){ fun(INTERVAL_TRAITS::pass(std::move(e.value))); });
    }
  }

} // End of namespace threadpool

#include "parallel_for_each.h"
#include "parallel_transform.h"
#include "make_iterator.h"

#endif // !defined(THREADPOOL_THREADPOOL_H)
