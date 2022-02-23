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
do_test( int n, int sz ) {
  Counter c;
  int nn = 1+((n*14)%sz);
  //int nn = 40;
  for ( int i = 0; i < nn; ++i ) {
    //Utils::sleep_for_nanoseconds(10);
    c.inc();
  }
  accumulator += c.get();
  //c.print();
}


template <class TP>
void
test_TP( int NN, int nt, int sz, char const * name ) {
  Utils::TicToc tm;

  double ti0, ti1;

  accumulator = 0;
  TP pool(nt);
  tm.tic();
  for ( int i = 0; i < NN; ++i) pool.run( do_test, i, sz );
  pool.wait();
  tm.toc();
  ti0 = tm.elapsed_ms();

  tm.tic();
  pool.join();
  tm.toc();
  ti1 = tm.elapsed_ms();

  fmt::print(
    "[{}] result {} [{}ms, JOIN {}ms]\n",
    name, accumulator, ti0, ti1
  );

  pool.info(std::cout);
}

int
main( int argc, char *argv[] ) {
  Utils::TicToc tm;

  int NN = 10000;
  int nt = 10;
  int sz = 200;

  if ( argc >= 2 ) nt = atoi( argv[1] );
  if ( argc >= 3 ) sz = atoi( argv[2] );
  if ( argc == 4 ) NN = atoi( argv[3] );

  fmt::print( "NT = {}\n", nt );

  accumulator = 0;

  double ttotal = 0;
  std::function<void()> fun;
  for ( int i = 0; i < NN; ++i) {
    fun = std::bind(do_test,i,sz);
    //auto  f = [&]() { do_test(i,sz); };
    tm.tic();
    fun(); // do_test(i,sz);
    tm.toc();
    ttotal += tm.elapsed_ms();
  }
  fmt::print(
    "[No Thread]   result {} [{:.6} ms, AVE = {:.6}]\n",
    accumulator, ttotal, ttotal/NN
  );

  test_TP<Utils::ThreadPool1>( NN, nt, sz, "ThreadPool1");

  test_TP<Utils::ThreadPool2>( NN, nt, sz, "ThreadPool2");

  test_TP<Utils::ThreadPool3>( NN, nt, sz, "ThreadPool3");

  test_TP<Utils::ThreadPool4>( NN, nt, sz, "ThreadPool4");

  test_TP<Utils::ThreadPool5>( NN, nt, sz, "ThreadPool5");

  fmt::print("All done folks!\n\n");

  //Utils::ThreadPool1 TP(16); // 0%
  //Utils::ThreadPool2 TP(16); // 0%
  //Utils::ThreadPool3 TP(16); // 100%
  Utils::ThreadPool4 TP(16); // 100%
  //Utils::ThreadPool5 TP(16); // 0%
  Utils::sleep_for_seconds(10);

  return 0;
}
