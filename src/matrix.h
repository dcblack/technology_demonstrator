#ifndef MATRIX_H
#define MATRIX_H

#include "dev_hls.h"
#include "memory.h"
#include <string>
#include <cassert>
#ifdef CXX11
#include <random>
#endif

struct Matrix
{
  enum Pattern_t
  { RANDALL, FILL0, FILL1, FILL2, FILL3, RANDOM, RAND10, RAND100, RAND1000
  , PLUSMINUS, MINUS2, PLUS1, MINUS1, PLUS3
  , NONE
  };

  static int    next;
#ifdef CXX11
  typedef std::uniform_int_distribution<Matrix::Pattern_t> Pattern_distribution;
  static std::default_random_engine gen;
  static Pattern_distribution       distr;
#endif

  // Attributes
  Addr_t  m_rows, m_cols;
  Data_t* m;
  std::string  m_name;

  // Methods
  Matrix(size_t r, size_t c, Pattern_t patt=NONE, std::string name=""); //< Constructor
  Matrix(size_t r, size_t c, std::string name); //< Constructor
  Matrix(const Matrix& rhs); // Copy constructor
  Matrix& operator=(const Matrix& rhs); //< Assignment
  ~Matrix(void) { delete[] m; }
  Addr_t rows(void)  const   { return m_rows; }
  Addr_t cols(void)  const   { return m_cols; }
  Addr_t size(void)  const   { return m_rows*m_cols; }
  Addr_t space(void) const   { return m_rows*m_cols+1; }
  Data_t& operator[](int i)  { return m[i]; }
  const Data_t& operator[](int i) const { return m[i]; }
  int xy(size_t r, size_t c) const { return r*m_cols+c; }
  int yx(size_t r, size_t c) const { return c*m_rows+r; }
  Data_t&       rc(size_t r, size_t c)       { return m[xy(r,c)]; }
  const Data_t& rc(size_t r, size_t c) const { return m[xy(r,c)]; }
  bool check(size_t r, size_t c) const { return r<m_rows && c<m_cols; }
  Data_t&       at(size_t r, size_t c)       { assert(check(r,c)); return rc(r,c); }
  const Data_t& at(size_t r, size_t c) const { assert(check(r,c)); return rc(r,c); }
  Addr_t shape(void) const { return Mshape(m_rows, m_cols); }
  Addr_t begin(void) const { return 0; }
  Addr_t end(void)   const { return size(); }
  int id(void)       const { return m_id; }
  std::string name(void) const;
  void set_name(std::string name) { m_name = name; }
  bool operator== (const Matrix& rhs); //< Compare
  bool is_square (void) const { return m_rows == m_cols; }
  size_t zeroes(void) const;
  void identity  (Data_t value=1);
  void randomize (int mod=0);
  void fill      (Data_t value=0);
  void fill_patt (Pattern_t patt=PLUS1);
  void load(const Memory& m, Addr_t from);
  void store(Memory& m, Addr_t to) const;
  std::string to_string(const std::string& nm = std::string()) const;
  void print(const std::string& nm = std::string()) const;
  Matrix& operator+= (Data_t rhs);
  Matrix& operator-= (Data_t rhs);
  Matrix& operator*= (Data_t rhs);
  Matrix& operator+= (const Matrix& rhs);
  Matrix& operator-= (const Matrix& rhs);
  Matrix  operator+  (Data_t rhs) const;
  Matrix  operator-  (Data_t rhs) const;
  Matrix  operator*  (Data_t rhs) const;
  Matrix  operator+  (const Matrix& rhs) const;
  Matrix  operator-  (const Matrix& rhs) const;
  Matrix  operator*  (const Matrix& rhs) const; //< matrix cross-product
  Matrix  transpose(void) const;
  Data_t  sum(void) const;
  // Data_t determinant(void);
  // typedef std::pair<Addr_t,Addr_t> Rc;
  // typedef std::pair<Rc,Rc> Submatrix; // first < second
  // void copy(Submatrix dst, const Matrix& rhs, Submatrix src);

private:
  int m_id;
};//endclass Matrix


#endif /*MATRIX_H*/
