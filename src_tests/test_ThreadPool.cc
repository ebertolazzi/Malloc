/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2017                                                      |
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

#include "Utils.hh"
#include "threadpool/threadpool.hh"

static std::atomic<unsigned> accumulator;

class Counter {
  Utils::BinarySearch<int> bs;
public:
  Counter() {
    bool ok ;
    int * pdata = bs.search( std::this_thread::get_id(), ok );
    *pdata = 0;
  }

  void
  inc() {
    bool ok;
    int * pdata = bs.search( std::this_thread::get_id(), ok );
    if ( !ok ) fmt::print("Counter::inc failed thread\n");
    ++(*pdata);
  }

  int
  get() {
    bool ok;
    int * pdata = bs.search( std::this_thread::get_id(), ok );
    if ( !ok ) fmt::print("Counter::inc failed thread\n");
    return *pdata;
  }

  void
  print() {
    bool ok;
    int * pdata = bs.search( std::this_thread::get_id(), ok );
    if ( !ok ) fmt::print("Counter::inc failed thread\n");
    fmt::print(
      "thread {}, counter = {}\n",
      std::this_thread::get_id(),
      *pdata
    );
  }
};

static
void
do_test( int n ) {
  Counter c;
  int nn = 1+((n*14)%157);
  //int nn = 40;
  for ( int i = 0; i < nn; ++i ) {
    //Utils::sleep_for_milliseconds(1);
    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    c.inc();
  }
  accumulator += c.get();
  //c.print();
}


template <class TP>
void
test_TP( int NN, int nt, char const * name ) {
  Utils::TicToc tm;

  double ti0, ti1;

  accumulator = 0;
  TP pool(nt);
  tm.tic();
  for ( int i = 0; i < NN; ++i) pool.run( do_test, i );
  tm.toc();
  ti0 = tm.elapsed_ms();

  //pool.wait();
  pool.join();
  tm.toc();
  ti1 = tm.elapsed_ms();

  pool.info(std::cout);

  fmt::print(
    "[{}] result {} [{}ms,{}ms]\n",
    name, accumulator, ti0, ti1
  );
}

int
main( int argc, char *argv[] ) {
  Utils::TicToc tm;

  int NN = 1000;
  int nt = 10;
  double ti0, ti1;

  if ( argc == 2 ) nt = atoi( argv[1] );

  fmt::print( "NT = {}\n", nt);

  accumulator = 0;
  for ( int i = 0; i < NN; ++i) do_test(i);
  fmt::print("[No Thread]   result {}\n", accumulator );

  test_TP<Utils::ThreadPool1>( NN, nt, "ThreadPool1");

  test_TP<Utils::ThreadPool2>( NN, nt, "ThreadPool2");

  test_TP<Utils::ThreadPool3>( NN, nt, "ThreadPool3");

  test_TP<Utils::ThreadPool4>( NN, nt, "ThreadPool4");

  test_TP<Utils::ThreadPool5>( NN, nt, "ThreadPool5");

  fmt::print("All done folks!\n\n");
  return 0;
}
