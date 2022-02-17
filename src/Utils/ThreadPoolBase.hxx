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
/// eof: ThreadPoolBase.hxx
///

namespace Utils {

  /*\
   |   _____ _                    _ ___          _ ___
   |  |_   _| |_  _ _ ___ __ _ __| | _ \___  ___| | _ ) __ _ ___ ___
   |    | | | ' \| '_/ -_) _` / _` |  _/ _ \/ _ \ | _ \/ _` (_-</ -_)
   |    |_| |_||_|_| \___\__,_\__,_|_| \___/\___/_|___/\__,_/__/\___|
  \*/

  class ThreadPoolBase {

    //disable copy
    ThreadPoolBase( ThreadPoolBase const & )               = delete;
    ThreadPoolBase( ThreadPoolBase && )                    = delete;
    ThreadPoolBase & operator = ( ThreadPoolBase const & ) = delete;
    ThreadPoolBase & operator = ( ThreadPoolBase && )      = delete;

  public:

    ThreadPoolBase() {}

    virtual
    void
    exec( std::function<void()> && ) = 0;

    template <typename Func, typename... Args>
    void
    run( Func && func, Args && ... args ) {
      this->exec(
        std::bind(
          std::forward<Func>(func),
          std::forward<Args>(args)...
        )
      );
    }

    virtual void         wait() = 0;
    virtual unsigned     thread_count() const = 0;
    virtual void         resize( unsigned numThreads ) = 0;
    virtual char const * name() const = 0;
  };

  /**
   * Call some function at the end of the current block
   */
  template <class Destructor>
  class at_scope_exit_impl {
    Destructor m_destructor;
    bool       m_active;
    at_scope_exit_impl( at_scope_exit_impl const & ) = delete;
    at_scope_exit_impl & operator=( at_scope_exit_impl const & ) = delete;
  public:
    at_scope_exit_impl() : m_active(false) { }

    explicit
    at_scope_exit_impl( Destructor&& destructor )
    : m_destructor(std::forward<Destructor>(destructor))
    , m_active(true)
    { }

    explicit
    at_scope_exit_impl( Destructor const & destructor )
    : m_destructor(destructor)
    , m_active(true)
    { }

    at_scope_exit_impl(at_scope_exit_impl&& x)
    : m_destructor(std::move(x.m_destructor))
    , m_active(x.m_active)
    { x.m_active = false; }

    at_scope_exit_impl&
    operator=(at_scope_exit_impl&& x) {
      m_destructor = std::move(x.m_destructor);
      m_active     = x.m_active;
      x.m_active   = false;
    }

    ~at_scope_exit_impl() { if (m_active) m_destructor(); }
  };

  /**
   * Create a variable that when destructed at the end of the scope
   * executes a destructor function.
   *
   * \tparam Destructor&& destructor
   *         The destructor function, maybe a lambda function.
   *
   * Use like this:
   *
   * static int a = 0;
   *
   * { // Enter scope
   *     ++a;
   *     auto x1 = at_scope_exit([&](){ --a; }
   *     // Do something, possibly throwing an exception
   * } // x1 goes out of scope, 'delete a' is called.
   */
  template<class Function>
  auto at_scope_exit(Function&& fun) -> at_scope_exit_impl<Function>
  { return at_scope_exit_impl<Function>(std::forward<Function>(fun)); }

  template<class Function>
  auto at_scope_exit(Function const & fun) -> at_scope_exit_impl<Function const &>
  { return at_scope_exit_impl<Function const &>(fun); }

}

///
/// eof: ThreadPoolBase.hxx
///
