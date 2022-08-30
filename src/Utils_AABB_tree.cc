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

///
/// file: Utils_AABB_tree.cc
///

#include "Utils_AABB_tree.hh"
#include <algorithm>
#include <utility>

namespace Utils {

  using std::max;
  using std::min;
  using std::swap;
  using std::copy_n;

  #define CHECK_OVERLAP(OLAP,A,B)                       \
    bool OLAP = true;                                   \
    for ( integer iii = 0; OLAP && iii < m_dim; ++iii ) \
      OLAP = ! ( A##_min[iii] > B##_max[iii] ||         \
                 B##_min[iii] > A##_max[iii] )

  #define GET_BB(NAME,FROM,ID)                       \
  Real const * NAME##_min = FROM##_min + ID * m_dim; \
  Real const * NAME##_max = FROM##_max + ID * m_dim

  #define SET_BB(NAME,FROM,ID)                 \
  Real * NAME##_min = FROM##_min + ID * m_dim; \
  Real * NAME##_max = FROM##_max + ID * m_dim

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  AABBtree<Real>::AABBtree( AABBtree<Real> const & T )
  : m_rmem("AABBtree")
  , m_imem("AABBtree")
  {

    m_dim            = T.m_dim;
    m_num_bb         = T.m_num_bb;
    m_num_tree_nodes = T.m_num_tree_nodes;

    integer nmax = 2*m_num_tree_nodes;

    m_rmem.free();
    m_imem.free();

    m_rmem.allocate( size_t(2*nmax*m_dim) );
    m_imem.allocate( size_t(6*nmax+m_num_bb) );

    m_bb_min    = m_rmem( size_t(nmax*m_dim) );
    m_bb_max    = m_rmem( size_t(nmax*m_dim) );

    m_father    = m_imem( size_t(nmax) );
    m_child     = m_imem( size_t(nmax) );
    m_ptr_nodes = m_imem( size_t(nmax) );
    m_num_nodes = m_imem( size_t(nmax) );
    m_id_nodes  = m_imem( size_t(m_num_bb) );
    m_stack     = m_imem( size_t(2*nmax) );

    std::copy_n( T.m_father,    m_num_tree_nodes, m_father );
    std::copy_n( T.m_child,     m_num_tree_nodes, m_child );
    std::copy_n( T.m_ptr_nodes, m_num_tree_nodes, m_ptr_nodes );
    std::copy_n( T.m_num_nodes, m_num_tree_nodes, m_num_nodes );
    std::copy_n( T.m_id_nodes,  m_num_bb,         m_id_nodes );
    std::copy_n( T.m_bb_min,    nmax*m_dim,       m_bb_min );
    std::copy_n( T.m_bb_max,    nmax*m_dim,       m_bb_max );

    m_max_object_per_node = T.m_max_object_per_node;
    m_long_bbox_tolerance = T.m_long_bbox_tolerance;
    m_volume_tolerance    = T.m_volume_tolerance;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  string
  AABBtree<Real>::info() const {
    integer nleaf = 0;
    integer nlong = 0;
    for ( integer i = 0; i < m_num_tree_nodes; ++i ) {
      if      ( m_child[i] < 0     ) ++nleaf;
      else if ( m_num_nodes[i] > 0 ) ++nlong;
    }
    string res = "-------- AABB tree info --------\n";
    res += fmt::format( "  Dimension           {}\n", m_dim );
    res += fmt::format( "  Number of nodes     {}\n", m_num_tree_nodes );
    res += fmt::format( "  Number of leaf      {}\n", nleaf );
    res += fmt::format( "  Number of long node {}\n", nlong );
    res += fmt::format( "  Number of objects   {}\n", m_num_bb );
    res += fmt::format( "  max_object_per_node {}\n", m_max_object_per_node );
    res += fmt::format( "  long_bbox_tolerance {}\n", m_long_bbox_tolerance );
    res += fmt::format( "  volume_tolerance    {}\n", m_volume_tolerance );
    res += "--------------------------------\n";
    return res;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::set_max_object_per_node( integer n ) {
    UTILS_ASSERT(
      n > 0 && n <= 4096,
      "AABBtree::set_max_object_per_node( nobj = {} )\n"
      "nobj must be > 0 and <= 4096\n",
      n
    );
    m_max_object_per_node = n;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::set_long_bbox_tolerance( Real tol ) {
    UTILS_ASSERT(
      tol > 0 && tol < 1,
      "AABBtree::set_long_bbox_tolerance( tol = {} )\n"
      "tol must be > 0 and < 1\n",
      tol
    );
    m_long_bbox_tolerance = tol;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::set_volume_tolerance( Real tol ) {
    UTILS_ASSERT(
      tol > 0 && tol < 1,
      "AABBtree::set_volume_tolerance( tol = {} )\n"
      "tol must be > 0 and < 1\n",
      tol
    );
    m_volume_tolerance = tol;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::build(
    Real const * bbox_obj_min, integer ldim0, integer ncol0,
    Real const * bbox_obj_max, integer ldim1, integer ncol1,
    integer      nbox,
    integer      dim
  ) {

    UTILS_ASSERT(
      ldim0 >= dim && ldim1 >= dim && ncol0 >= nbox && ncol1 >= nbox,
      "AABBtree::build( bb_min, ldim0={}, ncol0={},\n"
      "                 bb_max, ldim1={}, ncol1={},\n"
      "                 nbox={}, dim={} )\n"
      "must be ldim0, ldim1 >= dim and ncol0, ncol1 >= nbox\n",
      ldim0, ncol0, ldim1, ncol1, nbox, dim
    );

    UTILS_WARNING(
      dim <= 10,
      "AABBtree::build( bbox_min, ldim0, ncol0, bbox_max, ldim1, ncol1, nbox, dim={})\n"
      "dim is greather that 10!!!",
      dim
    );

    m_dim    = dim;
    m_num_bb = nbox;

    integer nmax = 2*m_num_bb; // estimate max memory usage

    m_rmem.allocate( size_t(2*nmax*dim) );
    m_imem.allocate( size_t(6*nmax+m_num_bb) );

    m_bb_min    = m_rmem( size_t(nmax*dim) );
    m_bb_max    = m_rmem( size_t(nmax*dim) );

    m_father    = m_imem( size_t(nmax) );
    m_child     = m_imem( size_t(nmax) );
    m_ptr_nodes = m_imem( size_t(nmax) );
    m_num_nodes = m_imem( size_t(nmax) );
    m_id_nodes  = m_imem( size_t(m_num_bb) );
    m_stack     = m_imem( size_t(2*nmax) );

    // initialize id nodes, will be reordered during the tree build
    for ( integer i = 0; i < m_num_bb; ++i ) m_id_nodes[i] = i;

    // setup root node
    m_father[0]    = -1;
    m_child[0]     = -1;
    m_ptr_nodes[0] = 0;
    m_num_nodes[0] = m_num_bb;

    // root contains all rectangles, build its bbox
    for ( integer j = 0; j < m_dim; ++j ) {
      Real & minj = m_bb_min[j];
      Real & maxj = m_bb_max[j];
      Real const * pmin = bbox_obj_min+j;
      Real const * pmax = bbox_obj_max+j;
      minj = *pmin;
      maxj = *pmax;
      UTILS_ASSERT0( maxj >= minj, "AABBtree::build, bad bbox N.0 max < min" );
      for ( integer i = 1; i < m_num_bb; ++i ) {
        pmin += ldim0;
        pmax += ldim1;
        if ( minj > *pmin ) minj = *pmin;
        if ( maxj < *pmax ) maxj = *pmax;
        UTILS_ASSERT( *pmax >= *pmin, "AABBtree::build, bad bbox N.{} max < min", i );
      }
    }

    // main loop: divide nodes until all constraints satisfied
    m_stack[0] = 0;
    integer n_stack = 1;
    m_num_tree_nodes = 1;

    while ( n_stack > 0 ) {

      // pop node from stack
      integer id_father = m_stack[--n_stack];

      // set no childer for the moment
      m_child[id_father] = -1;

      // get rectangles id in parent
      integer num = m_num_nodes[id_father];

      // if few bbox stop splitting
      if ( num < m_max_object_per_node ) continue;

      integer  iptr = m_ptr_nodes[id_father];
      integer * ptr = m_id_nodes + iptr;

      // split plane on longest axis, use euristic
      GET_BB(father,m_bb,id_father);

      integer idim = 0;
      Real    mx = father_max[0] - father_min[0];
      for ( integer i = 1; i < m_dim; ++i ) {
        Real mx1 = father_max[i] - father_min[i];
        if ( mx < mx1 ) { mx = mx1; idim = i; }
      }
      Real tol_len = m_long_bbox_tolerance * mx;
      Real sp      = 0;

      // separate short/long and accumulate short baricenter
      integer n_long  = 0;
      integer n_short = 0;
      while ( n_long + n_short < num ) {
        integer id = ptr[n_long];
        Real const * id_min = bbox_obj_min + id * ldim0;
        Real const * id_max = bbox_obj_max + id * ldim1;
        Real id_len = id_max[idim] - id_min[idim];
        if ( id_len > tol_len ) {
          // found long BBOX, increment n_long and update position
          ++n_long;
        } else {
          // found short BBOX, increment n_short and exchange with bottom
          ++n_short;
          swap( ptr[n_long], ptr[num-n_short] );
          sp += id_max[idim] + id_min[idim];
        }
      }

      // if split rectangles do not improve search, stop split at this level
      if ( n_short < 2 ) continue;

      // select the split position: take the mean of the set of
      // (non-"long") rectangle centers along axis idim
      sp /= 2*n_short;

      // partition based on centers
      integer n_left  = 0;
      integer n_right = 0;

      while ( n_long + n_left + n_right < num ) {
        integer id = ptr[n_long+n_left];
        Real const * id_min = bbox_obj_min + id * ldim0;
        Real const * id_max = bbox_obj_max + id * ldim1;
        Real id_mid = (id_max[idim] + id_min[idim])/2;
        if ( id_mid < sp ) {
          ++n_left; // in right position do nothing
        } else {
          ++n_right;
          swap( ptr[n_long+n_left], ptr[num-n_right] );
        }
      }

      // if cannot improve bbox, stop split at this level!
      if ( n_left == 0 || n_right == 0 ) continue;

      // child indexing
      integer id_left  = m_num_tree_nodes + 0;
      integer id_right = m_num_tree_nodes + 1;

      // compute bbox of left and right child
      SET_BB(left,m_bb,id_left);
      for ( integer i = 0; i < n_left; ++i ) {
        integer id = ptr[n_long+i];
        Real const * id_min = bbox_obj_min + id * ldim0;
        Real const * id_max = bbox_obj_max + id * ldim1;
        if ( i == 0 ) {
          copy_n( id_min, m_dim, left_min );
          copy_n( id_max, m_dim, left_max );
        } else {
          for ( integer j = 0; j < m_dim; ++j ) {
            if ( left_min[j] > id_min[j] ) left_min[j] = id_min[j];
            if ( left_max[j] < id_max[j] ) left_max[j] = id_max[j];
          }
        }
      }

      SET_BB(right,m_bb,id_right);
      for ( integer i = 0; i < n_right; ++i ) {
        integer id = ptr[n_long+n_left+i];
        Real const * id_min = bbox_obj_min + id * ldim0;
        Real const * id_max = bbox_obj_max + id * ldim1;
        if ( i == 0 ) {
          copy_n( id_min, m_dim, right_min );
          copy_n( id_max, m_dim, right_max );
        } else {
          for ( integer j = 0; j < m_dim; ++j ) {
            if ( right_min[j] > id_min[j] ) right_min[j] = id_min[j];
            if ( right_max[j] < id_max[j] ) right_max[j] = id_max[j];
          }
        }
      }

      // check again if split improve the AABBtree otherwise stop exploration
      if ( n_left < m_max_object_per_node || n_right < m_max_object_per_node ) {
        // few nodes, check if improve volume
        Real vo = 1;
        Real vL = 1;
        Real vR = 1;
        for ( integer j = 0l; j < m_dim; ++j ) {
          Real Lmin = left_min[j];
          Real Lmax = left_max[j];
          Real Rmin = right_min[j];
          Real Rmax = right_max[j];
          vo *= max(min(Lmax,Rmax) - max(Lmin,Rmin), Real(0));
          vL *= Lmax - Lmin;
          vR *= Rmax - Rmin;
        }
        // if do not improve volume, stop split at this level!
        if ( vo > (vL+vR-vo)*m_volume_tolerance ) continue;
      }

      // push child nodes onto stack
      m_father[id_left]  = id_father;
      m_father[id_right] = id_father;
      m_child[id_father] = id_left;

      m_num_nodes[id_father] = n_long;

      m_ptr_nodes[id_left]  = iptr + n_long;
      m_num_nodes[id_left]  = n_left;

      m_ptr_nodes[id_right] = iptr + n_long + n_left;
      m_num_nodes[id_right] = n_right;

      // push on stack children
      m_stack[n_stack++] = id_left;
      m_stack[n_stack++] = id_right;
      m_num_tree_nodes += 2;
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect_with_one_point(
    Real const * pnt,
    SET        & bb_index
  ) const {
    Real const * pnt_min = pnt;
    Real const * pnt_max = pnt;

    m_num_check = 0;

    // quick return on empty inputs
    if ( m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    integer n_stack = 1;
    while ( n_stack > 0 ) {
      // pop node from stack
      integer id_father = m_stack[--n_stack];

      // get BBOX
      GET_BB(father,m_bb,id_father);

      ++m_num_check;
      CHECK_OVERLAP( overlap, father, pnt );
      // if do not overlap skip
      if ( !overlap ) continue;

      // get rectangles id in parent
      this->get_bbox_indexes_of_a_node( id_father, bb_index );

      integer nn = m_child[id_father];
      if ( nn > 0 ) { // root == 0, children > 0
        // push on stack children
        m_stack[n_stack++] = nn;
        m_stack[n_stack++] = nn+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect_with_one_point_and_refine(
    Real const * pnt,
    Real const * bbox_min, integer ldim0, integer ncol0,
    Real const * bbox_max, integer ldim1, integer ncol1,
    SET        & bb_index
  ) const {

    UTILS_ASSERT(
      ldim0 >= m_dim && ldim1 >= m_dim && ncol0 >= m_num_bb && ncol1 >= m_num_bb,
      "AABBtree::intersect_with_one_point_and_refine(\n"
      " pnt, bb_min, ldim0={}, ncol0={},\n"
      "      bb_max, ldim1={}, ncol1={},\n"
      "      bb_index )\n"
      "must be ldim0, ldim1 >= {} and ncol0, ncol1 >= {}\n",
      ldim0, ncol0, ldim1, ncol1, m_dim, m_num_bb
    );

    // BBOX
    Real const * pnt_min = pnt;
    Real const * pnt_max = pnt;

    m_num_check = 0;

    // quick return on empty inputs
    if ( m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    integer n_stack = 1;
    while ( n_stack > 0 ) {
      // pop node from stack
      integer id_father = m_stack[--n_stack];

      // get BBOX
      GET_BB(father,m_bb,id_father);

      ++m_num_check;
      CHECK_OVERLAP( overlap, father, pnt );
      // if do not overlap skip
      if ( !overlap ) continue;

      // refine candidate
      integer num = this->m_num_nodes[id_father];
      integer const * ptr = this->m_id_nodes + this->m_ptr_nodes[id_father];
      for ( integer ii = 0; ii < num; ++ii ) {
        integer s = ptr[ii];
        Real const * s_min = bbox_min + s * ldim0;
        Real const * s_max = bbox_max + s * ldim1;
        ++m_num_check;
        CHECK_OVERLAP( olap, s, pnt );
        if ( olap ) bb_index.insert(s);
      }

      integer nn = m_child[id_father];
      if ( nn > 0 ) { // root == 0, children > 0
        // push on stack children
        m_stack[n_stack++] = nn;
        m_stack[n_stack++] = nn+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect_with_one_bbox(
    Real const * bb_min,
    Real const * bb_max,
    SET        & bb_index
  ) const {
    m_num_check = 0;

    // quick return on empty inputs
    if ( m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    integer n_stack = 1;
    while ( n_stack > 0 ) {
      // pop node from stack
      integer id_father = m_stack[--n_stack];

      // get BBOX
      GET_BB(father,m_bb,id_father);

      ++m_num_check;
      CHECK_OVERLAP( overlap, father, bb );
      // if do not overlap skip
      if ( !overlap ) continue;

      // get rectangles id in parent
      this->get_bbox_indexes_of_a_node( id_father, bb_index );

      integer nn = m_child[id_father];
      if ( nn > 0 ) { // root == 0, children > 0
        // push on stack children
        m_stack[n_stack++] = nn;
        m_stack[n_stack++] = nn+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect_with_one_bbox_and_refine(
    Real const * bb_min,
    Real const * bb_max,
    Real const * bbox_min, integer ldim0, integer ncol0,
    Real const * bbox_max, integer ldim1, integer ncol1,
    SET        & bb_index
  ) const {

    UTILS_ASSERT(
      ldim0 >= m_dim && ldim1 >= m_dim && ncol0 >= m_num_bb && ncol1 >= m_num_bb,
      "AABBtree::intersect_with_one_bbox_and_refine(\n"
      " bb_min, bb_max, \n"
      " bbox_min, ldim0={}, ncol0={},\n"
      " bbox_max, ldim1={}, ncol1={},\n"
      " bb_index )\n"
      "must be ldim0, ldim1 >= {} and ncol0, ncol1 >= {}\n",
      ldim0, ncol0, ldim1, ncol1, m_dim, m_num_bb
    );

    m_num_check = 0;

    // quick return on empty inputs
    if ( m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    integer n_stack = 1;
    while ( n_stack > 0 ) {
      // pop node from stack
      integer id_father = m_stack[--n_stack];

      // get BBOX
      GET_BB(father,m_bb,id_father);

      ++m_num_check;
      CHECK_OVERLAP( overlap, father, bb );
      // if do not overlap skip
      if ( !overlap ) continue;

      // refine candidate
      integer num = this->m_num_nodes[id_father];
      integer const * ptr = this->m_id_nodes + this->m_ptr_nodes[id_father];
      for ( integer ii = 0; ii < num; ++ii ) {
        integer s = ptr[ii];
        Real const * s_min = bbox_min + s * ldim0;
        Real const * s_max = bbox_max + s * ldim1;
        ++m_num_check;
        CHECK_OVERLAP( olap, s, bb );
        if ( olap ) bb_index.insert(s);
      }

      integer nn = m_child[id_father];
      if ( nn > 0 ) { // root == 0, children > 0
        // push on stack children
        m_stack[n_stack++] = nn;
        m_stack[n_stack++] = nn+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect(
    AABBtree<Real> const & aabb,
    MAP                  & bb_index
  ) const {

    m_num_check = 0;

    // quick return on empty inputs
    if ( this->m_num_tree_nodes == 0 || aabb.m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    m_stack[1] = 0;
    integer n_stack = 2;
    while ( n_stack > 1 ) {
      // pop node from stack
      integer root2  = m_stack[--n_stack];
      integer sroot1 = m_stack[--n_stack];
      integer root1  = sroot1 >= 0 ? sroot1 : -1-sroot1;

      // check for intersection
      GET_BB(root1,this->m_bb,root1);
      GET_BB(root2,aabb.m_bb,root2);

      ++m_num_check;
      CHECK_OVERLAP( overlap, root1, root2 );
      // if do not overlap skip
      if ( !overlap ) continue;

      // check if there are elements to check
      integer nn1 = this->m_num_nodes[root1];
      integer nn2 = aabb.m_num_nodes[root2];
      if ( nn1 > 0 && nn2 > 0 ) aabb.get_bbox_indexes_of_a_node( root2, bb_index[root1] );

      integer id_lr1 = sroot1 >= 0 ? m_child[root1] : -1;
      integer id_lr2 = aabb.m_child[root2];

      if ( id_lr1 >= 0 ) {
        m_stack[n_stack++] = id_lr1;   m_stack[n_stack++] = root2;
        m_stack[n_stack++] = id_lr1+1; m_stack[n_stack++] = root2;
        if ( nn1 > 0 ) {
          m_stack[n_stack++] = -1-root1; m_stack[n_stack++] = root2;
        }
      } else if ( id_lr2 >= 0 ) {
        m_stack[n_stack++] = sroot1; m_stack[n_stack++] = id_lr2;
        m_stack[n_stack++] = sroot1; m_stack[n_stack++] = id_lr2+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::intersect_and_refine(
    AABBtree<Real> const & aabb,
    Real const * bbox1_min, integer ldim0, integer ncol0,
    Real const * bbox1_max, integer ldim1, integer ncol1,
    Real const * bbox2_min, integer ldim2, integer ncol2,
    Real const * bbox2_max, integer ldim3, integer ncol3,
    MAP        & bb_index
  ) const {

    UTILS_ASSERT(
      ldim0 >= m_dim && ldim1 >= m_dim && ncol0 >= m_num_bb && ncol1 >= m_num_bb,
      "AABBtree::intersect_and_refine(\n"
      " aabb, \n"
      " bbox1_min, ldim0={}, ncol0={},\n"
      " bbox1_max, ldim1={}, ncol1={},\n"
      " bbox2_min, ldim2={}, ncol2={},\n"
      " bbox2_max, ldim3={}, ncol3={},\n"
      " bb_index )\n"
      "must be ldim0:3 >= {} and ncol0:3 >= {}\n",
      ldim0, ncol0, ldim1, ncol1, ldim2, ncol2, ldim3, ncol3,
      m_dim, m_num_bb
    );

    m_num_check = 0;

    // quick return on empty inputs
    if ( this->m_num_tree_nodes == 0 || aabb.m_num_tree_nodes == 0 ) return;

    // descend tree from root
    m_stack[0] = 0;
    m_stack[1] = 0;
    integer n_stack = 2;
    while ( n_stack > 1 ) {
      // pop node from stack
      integer root2  = m_stack[--n_stack];
      integer sroot1 = m_stack[--n_stack];
      integer root1  = sroot1 >= 0 ? sroot1 : -1-sroot1;

      // check for intersection
      GET_BB(root1,this->m_bb,root1);
      GET_BB(root2,aabb.m_bb,root2);

      ++m_num_check;
      CHECK_OVERLAP( overlap, root1, root2 );
      // if do not overlap skip
      if ( !overlap ) continue;

      // check if there are elements to check
      integer nn1 = this->m_num_nodes[root1];
      integer nn2 = aabb.m_num_nodes[root2];
      if ( nn1 > 0 && nn2 > 0 ) {
        // construct list of intersecting candidated
        integer const * ptr1 = this->m_id_nodes + this->m_ptr_nodes[root1];
        integer const * ptr2 = aabb.m_id_nodes + aabb.m_ptr_nodes[root2];
        for ( integer ii = 0; ii < nn1; ++ii ) {
          integer s1 = ptr1[ii];
          Real const * s1_min = bbox1_min + s1 * ldim0;
          Real const * s1_max = bbox1_max + s1 * ldim1;
          for ( integer jj = 0; jj < nn2; ++jj ) {
            integer s2 = ptr2[jj];
            Real const * s2_min = bbox2_min + s2 * ldim2;
            Real const * s2_max = bbox2_max + s2 * ldim3;
            ++m_num_check;
            CHECK_OVERLAP( olap, s1, s2 );
            if ( olap ) bb_index[s1].insert(s2);
          }
        }
      }

      integer id_lr1 = sroot1 >= 0 ? m_child[root1] : -1;
      integer id_lr2 = aabb.m_child[root2];

      if ( id_lr1 >= 0 ) {
        m_stack[n_stack++] = id_lr1;   m_stack[n_stack++] = root2;
        m_stack[n_stack++] = id_lr1+1; m_stack[n_stack++] = root2;
        if ( nn1 > 0 ) {
          m_stack[n_stack++] = -1-root1; m_stack[n_stack++] = root2;
        }
      } else if ( id_lr2 >= 0 ) {
        m_stack[n_stack++] = sroot1; m_stack[n_stack++] = id_lr2;
        m_stack[n_stack++] = sroot1; m_stack[n_stack++] = id_lr2+1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::get_bbox_indexes_of_a_node( integer i_pos, SET & bb_index ) const {
    UTILS_ASSERT(
      i_pos >= 0 && i_pos < m_num_tree_nodes,
      "AABBtree::get_bbox_indexes_of_a_node( i_pos={}, bb_index ) i_pos must be >= 0 and < {}\n",
      i_pos, m_num_tree_nodes
    );
    integer num = m_num_nodes[i_pos];
    integer ptr = m_ptr_nodes[i_pos];
    while ( num-- > 0 ) bb_index.insert( m_id_nodes[ptr++] );
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  typename AABBtree<Real>::integer
  AABBtree<Real>::num_tree_nodes( integer nmin ) const {
    integer n = 0;
    for ( integer i = 0; i < m_num_tree_nodes; ++i )
      if ( m_num_nodes[i] >= nmin ) ++n;
    return n;
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template <typename Real>
  void
  AABBtree<Real>::get_bboxes_of_the_tree(
    Real * bbox_min, integer ldim0, integer ncol0,
    Real * bbox_max, integer ldim1, integer ncol1,
    integer nmin
  ) const {
    UTILS_ASSERT(
      ldim0 >= m_dim && ldim1 >= m_dim,
      "AABBtree::get_bboxes_of_the_tree(\n"
      "  bbox_min, ldim0={}, ncol0={},\n"
      "  bbox_max, ldim1={}, ncol1={},\n"
      "  nmin={} )\n"
      "must be nmin >= 0 and ldim0:1 >= {}\n",
      ldim0, ncol0, ldim1, ncol1, nmin, m_dim
    );

    integer nt = num_tree_nodes( nmin );
    UTILS_ASSERT(
      ncol0 >= nt && ncol1 >= nt,
      "AABBtree::get_bboxes_of_the_tree(\n"
      "  bbox_min, ldim0={}, ncol0={},\n"
      "  bbox_max, ldim1={}, ncol1={},\n"
      "  nmin={} )\n"
      "must be ncol0:1 >= {}\n",
      ldim0, ncol0, ldim1, ncol1, nmin, nt
    );
    for ( integer i = 0; i < m_num_tree_nodes; ++i ) {
      if ( m_num_nodes[i] >= nmin ) {
        std::copy_n( m_bb_min+i*m_dim, m_dim, bbox_min ); bbox_min += ldim0;
        std::copy_n( m_bb_max+i*m_dim, m_dim, bbox_max ); bbox_max += ldim1;
      }
    }
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  template class AABBtree<float>;
  template class AABBtree<double>;

}

///
/// eof: Utils_AABB_tree.cc
///
