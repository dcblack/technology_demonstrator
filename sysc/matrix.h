#ifndef MATRIX_H
#define MATRIX_H

// DESCRIPTION
//   Matrix class creates/manages a two-dimension matrix in memory as defined
//   by the example 'dev' implementation to hardware augment matrix operations.
//   Uses `malloc` rather than `new` in order to better accomodate embedded
//   style that might prefer C-style coding.

#include <stdint.h>
#include <assert.h>
#include <iostream>

#ifdef UNIT_TEST
#define VERBOSITY_ENABLED
#endif

#define check(expr) do { if (!(expr)) printf("ERROR: " #expr " failed on line %d of file %s\n",__LINE__,__FILE__); } while(0)

// DEFINITIONS
//   shape    = rows << 16 + cols
//   size     = rows x cols
//   capacity = original size
//   alloc    = capacity + BASE
//   space    = size + 1 (doesn't include capacity)
//   index    = offset to mtx.at(row,col) aka      mem[index]
//   mem[]    = { capacity                         mem[CAPACITY]
//              , shape      ____________          mem[SHAPE]
//              , mtx.at(0,0)            \         mem[BASE+0]
//              , mtx.at(0,1)             |        mem[BASE+1]
//              , mtx.at(0,2)             | row 0  mem[BASE+2]
//              , ...                     |
//              , mtx.at(0,cols-1)_______/         mem[BASE+cols-1]
//              , mtx.at(1,0)            \         mem[BASE+cols+0]
//              , mtx.at(1,1)             |        mem[BASE+cols+1]
//              , mtx.at(1,2)             | row 1  mem[BASE+cols+2]
//              , ...                     |
//              , mtx.at(1,cols-1)_______/         mem[BASE+2*cols-1]
//              , mtx.at(2,0)            \         mem[BASE+2*cols+0]
//              , mtx.at(2,1)             |        mem[BASE+2*cols+1]
//              , mtx.at(2,2)             |        mem[BASE+2*cols+2]
//              , ...                     :
//              , mtx.at(x,y)             :
//              , ...                     :
//              , ...                     :
//              , mtx.at(rows-2,cols-1)__/         
//              , mtx.at(rows-1,0)       \         
//              , mtx.at(rows-1,1)        |        
//              , mtx.at(rows-1,2)        | row N  
//              , ...                     |
//              , mtx.at(rows-1,cols-2)   |        
//              , mtx.at(rows-1,cols-1)__/         
//              }
//   mtx = mem + 2;

typedef int32_t Mdata_t;

struct Matrix
{
  typedef int32_t Shape_t;
  enum Constant
  { CAPACITY
  , SHAPE
  , BASE
  , BITS=32
  , MAXDIM=(1<<(BITS/2))-1
  };
  Matrix(size_t _rows, size_t _cols) //< Constructor (from new rows x cols)
  : mtx(nullptr)
#ifdef VERBOSITY_ENABLED
  , m_id(next_id++)
#endif
  {
    assert(sizeof(Mdata_t)>=BITS/8);
    check(is_valid(_rows,_cols));
    mtx = (Mdata_t*) malloc(sizeof(Mdata_t)*space(_rows,_cols));
    mtx[SHAPE] = shape(_rows,_cols);
    mtx[CAPACITY] = size();
#ifdef VERBOSITY_ENABLED
    if (s_verbosity) { std::cout << "Constructed #" << id() << "(" << _rows << "x" << _cols << ")" << std::endl; }
    ++matrix_count;
#endif
  }
  Matrix(size_t max); // Construct random matrix
  Matrix(Mdata_t* ptr, size_t bytes) //< Constructor (from remotely constructed)
  : mtx(nullptr)
#ifdef VERBOSITY_ENABLED
  , m_id(next_id++)
#endif
  {
    check(size(ptr[SHAPE]) > 0 && size(ptr[SHAPE])<ptr[CAPACITY] && ptr[CAPACITY]+BASE < bytes);
    mtx = ptr;
#ifdef VERBOSITY_ENABLED
    if (s_verbosity) { std::cout << "Constructed remotely #" << id() << "(" << rows() << "x" << cols() << ") from ptr" << std::endl; }
    ++matrix_count;
#endif
  }
  Matrix(Mdata_t* ptr, size_t _rows, size_t _cols) //< Constructor (from existing memory, but not initialized)
  : mtx(nullptr)
#ifdef VERBOSITY_ENABLED
  , m_id(next_id++)
#endif
  {
    mtx = ptr;
    mtx[SHAPE] = shape(_rows,_cols);
    mtx[CAPACITY] = size();
#ifdef VERBOSITY_ENABLED
    if (s_verbosity) { std::cout << "Constructed #" << id() << "(" << _rows << "x" << _cols << ") from uninitialized" << std::endl; }
    ++matrix_count;
#endif
  }
  Matrix(const Matrix& rhs) //< Copy constructor
  : mtx(nullptr)
#ifdef VERBOSITY_ENABLED
  , m_id(next_id++)
#endif
  {
#ifdef VERBOSITY_ENABLED
    if (s_verbosity) { std::cout << "Copy constructing #" << id() << "(" << rhs.rows() << "x" << rhs.cols() << ")" << " from #" << rhs.id() << std::endl; }
    ++matrix_count;
#endif
    mtx = (Mdata_t*) malloc(sizeof(Mdata_t)*rhs.alloc());
    memcpy(mtx,rhs.mtx,sizeof(Mdata_t)*(rhs.alloc()));
  }
  virtual ~Matrix(void); //< Destructor
  Matrix& operator=(const Matrix& rhs) { //< Assignment
#ifdef VERBOSITY_ENABLED
    if (s_verbosity) { std::cout << "Assigning #" << id() << std::endl; }
#endif
    if (this == &rhs) return *this; //< self-assignment
    check(space() >= rhs.space());
    memcpy(&mtx[SHAPE],&rhs.mtx[SHAPE],sizeof(Mdata_t)*(rhs.space()));
    return *this;
  }
  // Compare
  bool operator==(const Matrix& rhs) const { return shape() == rhs.shape() && memcmp(mtx,rhs.mtx,sizeof(Mdata_t)*size()) == 0; }
  bool operator!=(const Matrix& rhs) const { return !(*this == rhs); }
  // Indexing/sizing operations
  Mdata_t&       at       (size_t x, size_t y)         { check((x+1)*(y+1)<=size()); return mtx[index(x,y)]; }
  const Mdata_t& at       (size_t x, size_t y) const   { check((x+1)*(y+1)<=size()); return mtx[index(x,y)]; }
  Mdata_t&       at       (size_t i)                   { check(i<size()); return mtx[index(i)]; }
  const Mdata_t& at       (size_t i) const             { check(i<size()); return mtx[index(i)]; }
  Mdata_t&       last     (void)                       { return at(size()-1); }
  const Mdata_t& last     (void) const                 { return at(size()-1); }
  Mdata_t&       first    (void)                       { return at(begin()); }
  const Mdata_t& first    (void) const                 { return at(begin()); }
  static bool    is_valid (size_t _rows, size_t _cols) { return (0<_rows) && (_rows<=MAXDIM) && (0<_cols) && (_cols<=MAXDIM); }
  static bool    is_valid (Shape_t _shape)             { return is_valid(rows(_shape),cols(_shape)); }
  static bool    is_valid (Mdata_t* _ptr)              { return is_valid(_ptr[SHAPE]) && _ptr[CAPACITY] <= size(_ptr[SHAPE]); }
  static size_t  rows     (Shape_t _shape)             { return ((_shape >> (BITS/2)) & MAXDIM); }
  static size_t  cols     (Shape_t _shape)             { return ( _shape          & MAXDIM); }
  static size_t  size     (Shape_t _shape)             { return rows(_shape) * cols(_shape); }
  static size_t  space    (size_t _rows, size_t _cols) { return _rows*_cols+BASE-SHAPE; }
  static size_t  space    (Shape_t _shape)             { return size(_shape)+BASE-SHAPE; }
  static Shape_t shape    (size_t _rows, size_t _cols) { return ((_rows & MAXDIM)<<(BITS/2))|(_cols & MAXDIM); }
  size_t         rows     (void)  const                { return rows(mtx[SHAPE]); }
  size_t         cols     (void)  const                { return cols(mtx[SHAPE]); }
  size_t         size     (void)  const                { return rows()*cols(); }
  size_t         alloc    (void) const                 { return mtx[CAPACITY] + BASE; }
  size_t         space    (void) const                 { return size() + BASE - SHAPE; }
  Shape_t        shape    (void) const                 { return mtx[SHAPE]; }
  size_t         index    (size_t x, size_t y) const   { return x*cols()+y+BASE; }
  size_t         index    (size_t i) const             { return i+BASE; }
  size_t         row      (size_t i) const             { return (i-BASE)/cols(); }
  size_t         col      (size_t i) const             { return (i-BASE)%cols(); }
  size_t         begin    (void) const                 { return 0; }
  size_t         end      (void) const                 { return size(); }
  // Scalar operations
  Matrix& operator= (const Mdata_t& rhs);        // Fill
  Matrix& operator*=(const Mdata_t& rhs);        // *=Kmul
  Matrix  operator* (const Mdata_t& rhs) const;  // * Kmul
  Matrix& operator/=(const Mdata_t& rhs);        // /=Kdiv
  Matrix  operator/ (const Mdata_t& rhs) const;  // / Kdiv
  Matrix& operator+=(const Mdata_t& rhs);        // +=Kadd
  Matrix  operator+ (const Mdata_t& rhs) const;  // + Kadd
  Matrix& operator-=(const Mdata_t& rhs);        // -=Ksub
  Matrix  operator- (const Mdata_t& rhs) const;  // - Ksub
  // Unary operations
  Matrix operator+(void) const { return *this; }      // Unary +
  Matrix operator-(void) const { return *this * -1; } // Unary -
  Matrix abs(void) const;
  // Matrix operations
  Matrix& operator+=(const Matrix& rhs);        // +=Madd
  Matrix  operator+ (const Matrix& rhs) const;  // + Madd
  Matrix& operator-=(const Matrix& rhs);        // -=Msub
  Matrix  operator- (const Matrix& rhs) const;  // - Msub
  Matrix& operator*=(const Matrix& rhs);        // *=Mmul
  Matrix  operator* (const Matrix& rhs) const;  // * Mmul
  Matrix transpose(void);
  // Matrix tests
  size_t count(Mdata_t value) const;
  Mdata_t max(void) const;
  Mdata_t min(void) const;
  bool is_square(void) const { return rows() == cols(); }
  bool is_triangle(bool above) const;
  // Special
  enum class Kind { identity, lower, upper, shape };
  void fill(Kind kind = Kind::shape); // Use for directed testing
  static Matrix identity(size_t dim) { Matrix sq(dim,dim); sq.fill(Kind::identity); return sq; }
  void randomize(size_t zeroes=0, size_t negative=0);
  static void zap(Mdata_t* mtx, Mdata_t rhs);
private:
  Mdata_t* mtx;
#ifdef VERBOSITY_ENABLED
  static bool    s_verbosity;
  static size_t  next_id;
  static ssize_t matrix_count;
  size_t         m_id;
public:
  static void verbosity(bool flag) { s_verbosity = flag; }
  static bool verbosity(void) { return s_verbosity; }
  size_t id(void) const { return m_id; }
  static ssize_t existing(void) { return matrix_count; }
#endif
};

// External binary operations
Matrix operator*(const Mdata_t& lhs,const Matrix& rhs); // Kmul*
std::ostream& operator<<(std::ostream& os, const Matrix& rhs);

#endif /*MATRIX_H*/
