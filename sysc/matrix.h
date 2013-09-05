#ifndef MATRIX_H
#define MATRIX_H

// DESCRIPTION
//   Matrix class creates/manages a two-dimension matrix in memory as defined
//   by the example 'dev' implementation to hardware augment matrix operations.
//   Uses `malloc` rather than `new` in order to better accomodate embedded
//   style that might prefer C-style coding.

#include <assert.h>

#define check(expr) do { if (!(expr)) printf("ERROR: " #expr " failed\n"); } while(0)

struct Matrix
{
  typedef int Data_t;
  typedef int Shape_t;
  enum { CAPACITY, SHAPE, BASE, BITS=16, MASK=0xFFFF };
  Matrix(size_t rows, size_t cols) //< Constructor (from new rows x cols)
  : mtx(nullptr)
  {
    assert(sizeof(Data_t)>=4);
    check(is_valid(rows,cols));
    mtx = (Data_t*) malloc(sizeof(Data_t)*space(rows,cols));
    mtx[SHAPE] = shape(rows,cols);
    mtx[CAPACITY] = size();
  }
  Matrix(Data_t* ptr, size_t bytes) //< Constructor (from remotely constructed)
  {
    check(size(ptr[SHAPE]) > 0 && size(ptr[SHAPE])<ptr[CAPACITY] && ptr[CAPACITY]+BASE < bytes);
    mtx = ptr;
  }
  Matrix(Data_t* ptr, size_t rows, size_t cols) //< Constructor (from existing memory, but not initialized)
  {
    mtx = ptr;
    mtx[SHAPE] = shape(rows,cols);
    mtx[CAPACITY] = size();
  }
  Matrix(const Matrix& rhs) //< Copy constructor
  : mtx(nullptr)
  {
    mtx = (Data_t*) malloc(sizeof(Data_t)*rhs.alloc());
    memcpy(mtx,rhs.mtx,sizeof(Data_t)*(rhs.space()));
  }
  ~Matrix(void) { free(mtx); mtx = nullptr; } //< Destructor
  Matrix& operator=(const Matrix& rhs) { //< Assignment
    check(space() >= rhs.space());
    memcpy(&mtx[SHAPE],&rhs.mtx[SHAPE],sizeof(Data_t)*(rhs.space()-SHAPE));
    return *this;
  }
  bool operator==(const Matrix& rhs) { return shape() == rhs.shape() && memcmp(mtx,rhs.mtx,sizeof(Data_t)*size()) == 0; }
  Data_t& at(size_t x, size_t y) { check((x+1)*(y+1)<=size()); return mtx[index(x,y)]; }
  static bool    is_valid(size_t rows, size_t cols) { return (0<rows) && (rows<=MASK) && (0<cols) && (cols<=MASK); }
  static bool    is_valid(Shape_t shape) { return is_valid(rows(shape),cols(shape)); }
  static size_t  rows(Shape_t shape) { return ((shape >> BITS) & MASK); }
  static size_t  cols(Shape_t shape) { return ( shape          & MASK); }
  static size_t  size(Shape_t shape) { return rows(shape) * cols(shape); }
  static size_t  space(size_t rows, size_t cols) { return rows*cols+BASE; }
  static Shape_t shape(size_t rows, size_t cols) { return ((rows<<BITS) & MASK)|(cols & MASK); }
  size_t  rows()  const { return rows(mtx[SHAPE]); }
  size_t  cols()  const { return cols(mtx[SHAPE]); }
  size_t  size()  const { return rows()*cols(); }
  size_t  alloc() const { return mtx[CAPACITY] + BASE; }
  size_t  space() const { return size() + BASE; }
  Shape_t shape() const { return mtx[SHAPE]; }
  size_t  index(size_t x, size_t y) { return x*cols()+y+BASE; }
  size_t  row(size_t i) { return (i-BASE)/cols(); }
  size_t  col(size_t i) { return (i-BASE)%cols(); }
private:
  Data_t* mtx;
};

#endif /*MATRIX_H*/
