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

//#include "Utils/threadpool/threadpool.h"
#include "threadpool_new/threadpool.h"

static std::atomic_int accumulatore;

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
  for ( int i = 0; i < nn; ++i ) {
    //Utils::sleep_for_milliseconds(1);
    c.inc();
  }
  accumulatore += c.get();
  //c.print();
}

int
main( int argc, char *argv[] ) {
  Utils::TicToc tm;

  int NN = 50000;
  int nt = 10;
  double ti0, ti1;

  if ( argc == 2 ) nt = atoi( argv[1] );

  fmt::print( "NT = {}\n", nt);

  {
    accumulatore = 0;
    quickpool::ThreadPool pool(nt);
    tm.tic();
    for ( int i = 0; i < NN; ++i)
      pool.push( do_test, i );
    tm.toc();
    ti0 = tm.elapsed_ms();
    pool.wait();
    tm.toc();
    ti1 = tm.elapsed_ms();
  }

  fmt::print(
    "Elapsed [quickpool] {} ms\n"
    "                    {} ms\n"
    "                    {}\n\n",
    ti0, ti1, accumulatore
  );

  {
    accumulatore = 0;
    threadpool::ThreadPool pool2(nt);
    tm.tic();
    for ( int i = 0; i < NN; ++i)
      pool2.run( [i]{ do_test(i); } );
    tm.toc();
    ti0 = tm.elapsed_ms();
    pool2.wait();
    tm.toc();
    ti1 = tm.elapsed_ms();
  }
  fmt::print(
    "Elapsed [threadpool] {} ms\n"
    "                     {} ms\n"
    "                     {}\n\n",
    ti0, ti1, accumulatore
  );

  // Multithreaded for_each, std::vector
  std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
  threadpool::parallel::for_each(a, [](int&e){ e *= 2; });
  for ( int i = 0; i < a.size(); ++i )
    fmt::print( "a[{}] = {}\n", i, a[i] );

  accumulatore = 0;

  Utils::ThreadPool TP(nt);

  tm.tic();
  for ( int i = 0; i < NN; ++i ) TP.run( i % nt, do_test, i );
  tm.toc();
  ti0 = tm.elapsed_ms();
  TP.wait_all();
  tm.toc();
  ti1 = tm.elapsed_ms();

  fmt::print(
    "Elapsed {} ms\n"
    "        {} ms\n"
    "        {}\n\n",
    ti0, ti1, accumulatore
  );

  fmt::print("All done folks!\n\n");
  return 0;
}
