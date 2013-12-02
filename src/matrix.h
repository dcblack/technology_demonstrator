#ifndef MATRIX_H
#define MATRIX_H

// DESCRIPTION
//   This class provides a convenience layer for matrix
//   operations.

#include "dev_hls.h"
#include "memory.h"
#include <string>
#include <cassert>
#include <map>
#ifdef CXX11
#include <random>
#endif

#ifdef USING_HARDWARE
#define VOLATILE volatile
#else
#define VOLATILE
#endif

class Dev;

struct Matrix
{
  enum Pattern_t
  { RANDALL, FILL0, FILL1, FILL2, FILL3, RANDOM, RAND10, RAND100, RAND1000
  , PLUSMINUS, MINUS2, PLUS1, MINUS1, PLUS3
  , NONE
  };

  // Methods
  Matrix(size_t r, size_t c, Pattern_t patt=NONE, std::string name="", bool prefer_Hard = true); //< Constructor
  Matrix(size_t r, size_t c, std::string name, bool prefer_hard = true); //< Constructor
  Matrix(const Matrix& rhs); //< Copy constructor
  Matrix& operator=(const Matrix& rhs); //< Assignment
  ~Matrix(void); //< Destructor
  Addr_t rows(void)  const   { return m_rows; }
  Addr_t cols(void)  const   { return m_cols; }
  Addr_t size(void)  const   { return m_rows*m_cols; }
  Addr_t space(void) const   { return m_rows*m_cols+1; }
  VOLATILE Data_t& operator[](int i)  { return m[i]; }
  VOLATILE const Data_t& operator[](int i) const { return m[i]; }
  int xy(size_t r, size_t c) const { return r*m_cols+c; }
  int yx(size_t r, size_t c) const { return c*m_rows+r; }
  VOLATILE Data_t&       rc(size_t r, size_t c)       { return m[xy(r,c)]; }
  VOLATILE const Data_t& rc(size_t r, size_t c) const { return m[xy(r,c)]; }
  bool check(size_t r, size_t c) const { return r<m_rows && c<m_cols; }
  VOLATILE Data_t&       at(size_t r, size_t c)       { assert(check(r,c)); return rc(r,c); }
  VOLATILE const Data_t& at(size_t r, size_t c) const { assert(check(r,c)); return rc(r,c); }
  Addr_t shape(void) const { return Mshape(m_rows, m_cols); }
  Addr_t begin(void) const { return 0; }
  Addr_t end(void)   const { return size(); }
  size_t id(void)    const { return m_id; }
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
  bool is_valid(void) const { return ( (!using_mem() && !using_reg())
                                     ||( using_mem() &&  using_reg())
                                     );
  }
  friend class Dev;

#ifdef CXX11
  typedef std::uniform_int_distribution<Matrix::Pattern_t> Pattern_distribution;
  static std::default_random_engine gen;
  static Pattern_distribution       distr;
#endif

private:
  // Attributes
  Addr_t          m_rows;
  Addr_t          m_cols;
  VOLATILE Data_t* m;
  Reg_t            m_reg;
  size_t           m_addr; //< offset for memory
  bool             m_hard;
  std::string      m_name;
  size_t           m_id;
  static size_t    next_id;
  static Dev*      dev;
  static Reg_t     t_reg;
  bool using_reg(void) const { return m_reg != UNUSED; }
  bool using_mem(void) const { return (m_addr != ~0U); }

};//endclass Matrix


#endif /*MATRIX_H*/
