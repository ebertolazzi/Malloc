// ██████╗ ███████╗ ██████╗ ██╗   ██╗███████╗███████╗████████╗
// ██╔══██╗██╔════╝██╔═══██╗██║   ██║██╔════╝██╔════╝╚══██╔══╝
// ██████╔╝█████╗  ██║   ██║██║   ██║█████╗  ███████╗   ██║
// ██╔══██╗██╔══╝  ██║▄▄ ██║██║   ██║██╔══╝  ╚════██║   ██║
// ██║  ██║███████╗╚██████╔╝╚██████╔╝███████╗███████║   ██║
// ╚═╝  ╚═╝╚══════╝ ╚══▀▀═╝  ╚═════╝ ╚══════╝╚══════╝   ╚═╝
//
// Write a C++11 or later program that utilizes the Eigen3 library to perform matrix-matrix multiplication using block partitioning.
// Given matrices A and B, compute the matrix C = A * B. Matrices A and B must be compatible for multiplication. Given the integers N, P, M, partition the matrices as follows:
//  - Matrix A into N x P blocks
//  - Matrix B into P x M blocks
//  - Matrix C into N x M blocks
// Ensure that the partitioning is compatible. If matrices A and B are incompatible, or if the required partitioning (N, P, M) is not possible, throw an exception.
// Each (i, j) block of matrix C must be computed on a separate thread if available, enabling parallel code execution.
// Finally, compare the execution speed of your block partitioning matrix multiplication with the timing of the standard Eigen3 matrix multiplication command.
// Use the proposed ThreadPool to find a better one to perform parallel tasks.

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "Utils_eigen.hh"

using mat     = Eigen::MatrixXd;
using integer = Eigen::Index;

class BlockMult {

  std::mutex mtx;
  Utils::ThreadPool0 Pool0{12};
  Utils::ThreadPool1 Pool1{12};
  Utils::ThreadPool2 Pool2{12};
  Utils::ThreadPool3 Pool3{12};
  Utils::ThreadPool4 Pool4{12};
  Utils::ThreadPool5 Pool5{12};

  std::vector<integer> i_block;
  std::vector<integer> j_block;
  std::vector<integer> k_block;

  // (n x m) * (m x p)
  void
  Compute_C_block(
    mat const & A,
    mat const & B,
    mat       & C,
    integer     i,
    integer     j
  );

public:
  BlockMult() {}

  bool
  multiply(
    integer     nthp,
    mat const & A,
    mat const & B,
    mat       & C,
    integer     n,
    integer     m,
    integer     p
  );

  ~BlockMult() {}

};

//-----------------------------------------------------

void
BlockMult::Compute_C_block(
  mat const & A,
  mat const & B,
  mat       & C,
  integer     i,
  integer     j
) {
  auto II = Eigen::seqN( i_block[i-1], i_block[i]-i_block[i-1] );
  auto JJ = Eigen::seqN( j_block[j-1], j_block[j]-j_block[j-1] );
  for ( size_t k{1}; k < k_block.size(); ++k ) {
    auto KK = Eigen::seqN( k_block[k-1], k_block[k]-k_block[k-1] );
    C(II,JJ) += A(II,KK)*B(KK,JJ);
  }
}

//-----------------------------------------------------

bool
BlockMult::multiply(
  integer     ntp,
  mat const & A,
  mat const & B,
  mat       & C,
  integer     n,
  integer     m,
  integer     p
) {

  if ( A.cols() != B.rows())  {
    fmt::print(
      "Invalid matrix multiplication. Found {} x {} Times {} x {} ",
      A.cols(), A.rows(), B.cols(), B.rows()
    );
    return false;
  }

  // get dimensions
  integer N{A.rows()};
  integer M{B.cols()};
  integer P{A.cols()};

  if ( n > N )  {
    fmt::print( "Invalid matrix multiplication. Found n = {} > N = {}\n", n, N );
    return false;
  }
  if ( m > M )  {
    fmt::print( "Invalid matrix multiplication. Found m = {} > M = \n", m, M );
    return false;
  }
  if ( p > P )  {
    fmt::print( "Invalid matrix multiplication. Found p = {} > P = {}\n", p, P );
    return false;
  }

  i_block.clear(); i_block.reserve(n+1); i_block.emplace_back(0);
  k_block.clear(); k_block.reserve(p+1); k_block.emplace_back(0);
  j_block.clear(); j_block.reserve(m+1); j_block.emplace_back(0);

  {
    integer dn{int(N/n)};
    while ( i_block.back() < N ) i_block.emplace_back( i_block.back() + dn );
    i_block.back() = N;
  }

  {
    integer dm{int(M/m)};
    while ( j_block.back() < M ) j_block.emplace_back( j_block.back() + dm );
    j_block.back() = M;
  }

  {
    integer dp{int(P/p)};
    while ( k_block.back() < P ) k_block.emplace_back( k_block.back() + dp );
    k_block.back() = P;
  }

  C.setZero();

  switch ( ntp ) {
  case 0:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool0.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool0.exec( fun );
        #endif
      }
    }
    Pool0.wait();
    break;
  case 1:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool1.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool1.exec( fun );
        #endif
      }
    }
    Pool1.wait();
    break;
  case 2:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool2.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool2.exec( fun );
        #endif
      }
    }
    Pool2.wait();
    break;
  case 3:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool3.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool3.exec( fun );
        #endif
      }
    }
    Pool3.wait();
    break;
  case 4:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool4.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool4.exec( fun );
        #endif
      }
    }
    Pool4.wait();
    break;
  case 5:
    for ( integer i{1}; i < integer(i_block.size()); ++i ) {
      for ( integer j{1}; j < integer(j_block.size()); ++j ) {
        #if 0
        Pool5.run( &BlockMult::Compute_C_block, this, std::ref(A), std::ref(B), std::ref(C), i, j );
        #else
        auto fun = [this, &A, &B, &C, i, j]() -> void { this->Compute_C_block( A, B, C, i, j ); };
        Pool5.exec( fun );
        #endif
      }
    }
    Pool5.wait();
    break;
  default:
    fmt::print("ERROR\n\n\n");
  }
  return true;
}

int
main() {

  Eigen::initParallel();
  std::cout << "Eigen Test" << std::endl;
  double mean   = 0.0;
  double stdDev = 0.0;
  Eigen::MatrixXd M1, M2, M3a, M3b;
  int n_runs = 100;
  Eigen::VectorXd times(n_runs);
  Eigen::VectorXd stdDev_vec(n_runs);
  int n_size = 300;
  int p_size = 200;
  int m_size = 500;
  int N = 10;
  int P = 10;
  int M = 10;
  M1.resize(n_size,p_size);
  M2.resize(p_size,m_size);
  M3a.resize(n_size,m_size);
  M3b.resize(n_size,m_size);

  M1 = Eigen::MatrixXd::Random(n_size,p_size);
  M2 = Eigen::MatrixXd::Random(p_size,m_size);

  fmt::print("Standard Product\n");
  for ( int i{0}; i < n_runs; i++) {
    Utils::TicToc tm;
    tm.tic();
    M3a = M1 * M2;
    tm.toc();
    times(i) = tm.elapsed_ms();
  }
  mean   = times.mean();
  stdDev = (((times.array() - mean) * (times.array() - mean)).sqrt()).sum()/((double)(n_runs-1));
  fmt::print( "time: {}ms {}ms (sdev)\n\n\n", mean, stdDev );

  for ( int nptp{0}; nptp <= 5; ++nptp ) {
    fmt::print("Block Product #{}\n",nptp);
    BlockMult BM;
    for ( int i{0}; i < n_runs; ++i ) {
      Utils::TicToc tm;
      tm.tic();
      BM.multiply( nptp, M1, M2, M3b, N, P, M );
      tm.toc();
      times(i) = tm.elapsed_ms();
    }
    mean   = times.mean();
    stdDev = (((times.array() - mean) * (times.array() - mean)).sqrt()).sum()/((double)(n_runs-1));
    fmt::print( "time (#{}): {}ms {}ms (sdev)\n", nptp, mean, stdDev );
    fmt::print(
      "Check if the results are the same\n"
      "M3a - M3b: {}\n\n\n",
      (M3a- M3b).norm()
    );
  }

  return 0;
}
