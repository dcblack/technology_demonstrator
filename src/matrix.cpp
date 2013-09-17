////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "matrix.h"
#include <cstdlib>
#include <iomanip>
#include <sstream>
#ifdef CXX11
#include <random>
#endif
#include <cassert>
using namespace std;

//------------------------------------------------------------------------------
int    Matrix::next = 0;
#ifdef CXX11
std::default_random_engine Matrix::gen;
Matrix::Pattern_distribution       Matrix::distr(Matrix::FILL0,Matrix::RANDOM);
#endif

//------------------------------------------------------------------------------
Matrix::Matrix(size_t r, size_t c, Pattern_t patt, string name) //< Constructor
: m_rows(r)
, m_cols(c)
, m(0)
, m_name(name)
, m_id(next++)
{
  assert(m_rows!=0 & m_cols!=0);
  assert(m_rows*m_cols <= MAX_MATRIX_SIZE);
  m = new Data_t[m_rows*m_cols];
  fill_patt(patt);
}

//------------------------------------------------------------------------------
Matrix::Matrix(const Matrix& rhs) // Copy constructor
: m_rows(rhs.m_rows)
, m_cols(rhs.m_cols)
, m(new Data_t[m_rows*m_cols])
, m_name(rhs.m_name)
, m_id(next++)
{
  for (Addr_t i=begin(); i!=end(); ++i) m[i] = rhs.m[i];
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator=(const Matrix& rhs) { //< Assignment
  m_id = next++;
  m_name = rhs.m_name;
  m_rows = rhs.m_rows;
  m_cols = rhs.m_cols;
  delete[] m;
  m = new Data_t[m_rows*m_cols];
  for (Addr_t i=begin(); i!=end(); ++i) m[i] = rhs.m[i];
  return *this;
}

//------------------------------------------------------------------------------
bool Matrix::operator== (const Matrix& rhs) { //< Compare
  bool result = true;
  for (Addr_t i=begin(); i!=end(); result && ++i) {
    result &= (m[i] == rhs.m[i]);
  }
  return result;
}

//------------------------------------------------------------------------------
size_t Matrix::zeroes(void) const {
  size_t result = 0;
  for (Addr_t i=begin(); i!=end(); ++i) if (m[i] == 0) ++result;
  return result;
}

//------------------------------------------------------------------------------
void Matrix::fill(Data_t value) {
  for (Addr_t i=begin(); i!=end(); ++i) m[i] = value;
}

//------------------------------------------------------------------------------
// Fill matrix with various patterns
void Matrix::fill_patt(Pattern_t patt) {
  if (patt == NONE) return;
  Data_t value = 0;
  if (RANDOM <patt && patt<NONE) {
    // Incrementing/decrementing patterns
    for (Addr_t r=0; r!=m_rows; ++r) {
      for (Addr_t c=0; c!=m_cols; ++c) {
        switch (patt) {
          case    MINUS2: m[c*m_rows+r] = value-=2     ; break;
          case     PLUS1: m[r*m_cols+c] = value+=1     ; break;
          case    MINUS1: m[r*m_cols+c] = value-=1     ; break;
          case     PLUS3: m[r*m_cols+c] = value+=3     ; break;
          default       : m[r*m_cols+c] = value=-value ; break;
        }
      }
    }
  } else if (FILL0 <= patt && patt < RANDOM) {
    switch (patt) {
      case    FILL0: fill(1);          break;
      case    FILL1: fill(0xC0EDBABE); break;
      case    FILL2: fill(42);         break;
      case    FILL3: fill(0xFACE);     break;
      default      :                   break;
    }
  } else if (patt == RANDOM) {
    randomize();
  } else /*RANDALL*/ {
#ifdef CXX11
    fill_patt(distr(gen));
#else
    fill_patt(Pattern_t(random()%(NONE+1)));
#endif
  }
}

//------------------------------------------------------------------------------
void Matrix::identity(Data_t value) {
  assert(is_square());
  for (Addr_t r=0; r!=m_rows; ++r) {
    for (Addr_t c=0; c!=m_cols; ++c) {
      if (r == c) m[r*m_cols+c] = value;
      else        m[r*m_cols+c] = 0;
    }
  }
}

//------------------------------------------------------------------------------
void Matrix::randomize(void) {
  for (Addr_t i=begin(); i!=end(); ++i) {
#ifdef CXX11
    m[i] = gen();
#else
    m[i] = random();
#endif
  }
}

//------------------------------------------------------------------------------
string Matrix::name(void) const  {
  ostringstream sout("");
  if (m_name.length() == 0) sout << "m" << m_id;
  else                      sout << m_name;
  return sout.str();
}

//------------------------------------------------------------------------------
void Matrix::load(const Memory& mem, Addr_t from) {
  Addr_t shape = mem.iget(from++);
  assert(shape != 0 && Msize(shape) < MAX_MATRIX_SIZE);
  delete[] m;
  m_rows = Mrows(shape);
  m_cols = Mcols(shape);
  m = new Data_t[m_rows*m_cols];
  for (Addr_t i=begin(); i!=end(); ++i) {
    m[i] = mem.iget(from++);
  }
}

//------------------------------------------------------------------------------
void Matrix::store(Memory& mem, Addr_t to) {
  mem.iset(to++, Mshape(m_rows,m_cols));
  for (Addr_t i=begin(); i!=end(); ++i) {
    mem.iset(to++,m[i]);
  }
}

//------------------------------------------------------------------------------
string Matrix::dump(void) {
  ostringstream sout("");
  static const int aw = 3;
  static const int dw = 4;
  // Separator
  sout << string(aw,'=') << "==";
  for (Addr_t col=0; col!=m_cols; ++col) {
    if (col != 0) sout << "==";
    sout << string(dw,'=');
  }
  sout << "\n";

  // Heading
  sout << setw(aw) << setfill(' ') << name() << "/ ";
  for (Addr_t col=0; col!=m_cols; ++col) {
    if (col != 0) sout << ", ";
    sout << setw(dw) << dec << col;
  }
  sout << "\n";

  // Separator
  sout << string(aw,'-') << "--";
  for (Addr_t col=0; col!=m_cols; ++col) {
    if (col != 0) sout << "--";
    sout << string(dw,'-');
  }
  sout << "\n";

  // Out data
  for (Addr_t row=0; row!=m_rows; ++row) {
    sout << setw(aw) << dec << row << ": ";
    // Output a row
    for (Addr_t col=0; col!=m_cols; ++col) {
      sout << setw(dw) << dec << m[row*m_cols+col];
      if (col != m_cols-1) sout << ", ";
    }
    sout << "\n";
  }
  sout << "\n";
  return sout.str();
}

// The end
