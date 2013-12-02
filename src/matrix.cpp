////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "matrix.h"
#ifdef USING_HARDWARE
#include "dev.h"
#include "xdev_hls_hw.h"
#else
class Dev //< when not using hardware, this allows compile to happen
{
  void* dev;
};
#endif
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#ifdef CXX11
#include <random>
#endif
#include <cassert>
using namespace std;

//------------------------------------------------------------------------------
// Static data
size_t Matrix::next_id = 0;
Dev*   Matrix::dev     = 0;
Reg_t  Matrix::t_reg   = UNUSED;

#ifdef CXX11
std::default_random_engine Matrix::gen;
Matrix::Pattern_distribution       Matrix::distr(Matrix::FILL0,Matrix::RANDOM);
#endif

//------------------------------------------------------------------------------
Matrix::Matrix(size_t r, size_t c, Pattern_t patt, string name, bool prefer_hard) //< constructor
: m_rows(r)
, m_cols(c)
, m(0)
, m_reg(UNUSED)
, m_addr(~0U)
, m_hard(prefer_hard)
, m_name(name)
, m_id(next_id++)
{
  assert(m_rows!=0 & m_cols!=0);
  assert(m_rows*m_cols <= MAX_MATRIX_SIZE);
#ifdef USING_HARDWARE
  if (m_hard && dev != 0)
  {
    dev->alloc_mem(*this);
    if (t_reg == UNUSED) t_reg = dev->alloc_reg();
    dev->set_reg(m_reg,shape());
    dev->set_mem(m_addr,shape());
    dev->set_reg(Reg_t(m_reg+1),m_addr+sizeof(Data_t));
    m = dev->mem_ptr(m_addr+sizeof(Data_t));
    m_hard = true;
  }
  else
#endif
  {
    m_hard = false;
    m = new Data_t[m_rows*m_cols];
  }
  fill_patt(patt);
}//end constructor

//------------------------------------------------------------------------------
Matrix::Matrix(size_t r, size_t c, string name, bool prefer_hard) //< constructor
: m_rows(r)
, m_cols(c)
, m(0)
, m_reg(UNUSED)
, m_addr(~0U)
, m_hard(prefer_hard)
, m_name(name)
, m_id(next_id++)
{
  assert(m_rows!=0 & m_cols!=0);
  assert(m_rows*m_cols <= MAX_MATRIX_SIZE);
#ifdef USING_HARDWARE
  if (m_hard && dev != 0)
  {
    dev->alloc_mem(*this);
    if (t_reg == UNUSED) t_reg = dev->alloc_reg();
    dev->set_reg(m_reg,shape());
    dev->set_mem(m_addr,shape());
    dev->set_reg(Reg_t(m_reg+1),m_addr+sizeof(Data_t));
    m = dev->mem_ptr(m_addr+sizeof(Data_t));
    m_hard = true;
  }
  else
#endif
  {
    m_hard = false;
    m = new Data_t[m_rows*m_cols];
  }
}//end constructor

//------------------------------------------------------------------------------
Matrix::Matrix(const Matrix& rhs) //< copy constructor
: m_rows(rhs.m_rows)
, m_cols(rhs.m_cols)
, m(0)
, m_reg(UNUSED)
, m_addr(~0U)
, m_hard(false)
, m_name(rhs.m_name)
, m_id(next_id++)
{
#ifdef USING_HARDWARE
  if (m_hard)
  {
    dev->alloc_mem(*this);
    if (t_reg == UNUSED) t_reg = dev->alloc_reg();
    dev->set_reg(m_reg,shape());
    dev->set_mem(m_addr,shape());
    dev->set_reg(Reg_t(m_reg+1),m_addr+1);
    m = dev->mem_ptr(m_addr+sizeof(Data_t));
    m_hard = true;
  }
  else
#endif
  {
    m = new Data_t[m_rows*m_cols];
  }
  for (Addr_t i=begin(); i!=end(); ++i) m[i] = rhs.m[i];
}//end copy constructor

//------------------------------------------------------------------------------
Matrix::~Matrix(void) //< Destructor
{
  if (m && !m_hard) {
    delete[] m;
    m=0;
  }
#ifdef USING_HARDWARE
  if (using_reg()) dev->free_mreg(*this);
  if (using_mem()) dev->free_mem(*this);
#endif
}//end destructor

//------------------------------------------------------------------------------
Matrix& Matrix::operator=(const Matrix& rhs) { //< Assignment
  if (this == &rhs) return *this; //< self-assignment
  m_id = next_id++;
  m_name = rhs.m_name;
  bool bigger = size() < rhs.size();
  m_rows = rhs.m_rows;
  m_cols = rhs.m_cols;
  m_reg  = UNUSED;
  m_addr = ~0U;
  m_hard = false;
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard)
  {
    if (bigger) {
      dev->free_mem(*this);
      dev->alloc_mem(*this);
      dev->set_reg(Reg_t(m_reg+1),m_addr+1);
      m = dev->mem_ptr(m_addr+sizeof(Data_t));
    }
    dev->set_reg(m_reg,shape());
    dev->set_mem(m_addr,shape());
    m_hard = true;
  }
  else
#endif
  {
    delete[] m;
    m = new Data_t[m_rows*m_cols];
  }
  for (Addr_t i=begin(); i!=end(); ++i) m[i] = rhs.m[i];
  return *this;
}//end operator=

//------------------------------------------------------------------------------
bool Matrix::operator== (const Matrix& rhs) { //< Compare
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    CmdState_t retcode = dev->do_command(EQUAL,t_reg,m_reg,rhs.m_reg);
    bool result = (dev->get_reg(t_reg) != 0);
    return result;
  }
  else
#endif
  {
    if (m_rows != rhs.m_rows || m_cols != rhs.m_cols) return false;
    for (Addr_t i=begin(); i!=end(); ++i) {
      if (m[i] != rhs.m[i]) return false;
    }
    return true;
  }
}//end operator==

//------------------------------------------------------------------------------
size_t Matrix::zeroes(void) const {
  size_t result = 0;
#ifdef USING_HARDWARE
  if (m_hard) {
    CmdState_t retcode = dev->do_command(MZERO,t_reg,m_reg);
    bool result = (dev->get_reg(t_reg) != 0);
    return result;
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) if (m[i] == 0) ++result;
  }
  return result;
}

//------------------------------------------------------------------------------
void Matrix::fill(Data_t value) {
#ifdef USING_HARDWARE
  if (m_hard) {
    dev->set_reg(t_reg,value);
    CmdState_t retcode = dev->do_command(FILL,m_reg,t_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) m[i] = value;
  }
}

//------------------------------------------------------------------------------
// Fill matrix with various patterns
void Matrix::fill_patt(Pattern_t patt) {
  if (patt == NONE) return;
  if (FILL0 <= patt && patt < RANDOM) {
    switch (patt) {
      case    FILL0: fill(1);          break;
      case    FILL1: fill(0xC0EDBABE); break;
      case    FILL2: fill(42);         break;
      case    FILL3: fill(0xFACE);     break;
      default      :                   break;
    }
  } else if (RANDOM <= patt && patt < PLUSMINUS) {
    switch (patt) {
      case   RAND10: randomize(  10); break;
      case  RAND100: randomize( 100); break;
      case RAND1000: randomize(1000); break;
      default      : randomize();   break;
    }
  } else if (PLUSMINUS <= patt && patt < NONE) {
    Data_t value = 0;
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
  } else /*RANDALL*/ {
#ifdef CXX11
    // Use C++11 random distribution engine
    fill_patt(distr(gen));
#else
    fill_patt(Pattern_t(random()%(NONE+1)));
#endif
  }
}

//------------------------------------------------------------------------------
void Matrix::identity(Data_t value) {
#ifdef USING_HARDWARE
  if (m_hard) {
    dev->set_reg(t_reg,value);
    CmdState_t retcode = dev->do_command(IDENT,m_reg,t_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    assert(is_square());
    for (Addr_t r=0; r!=m_rows; ++r) {
      for (Addr_t c=0; c!=m_cols; ++c) {
        if (r == c) m[r*m_cols+c] = value;
        else        m[r*m_cols+c] = 0;
      }
    }//endfor
  }
}

//------------------------------------------------------------------------------
void Matrix::randomize(int mod) {
  Data_t value;
  for (Addr_t i=begin(); i!=end(); ++i) {
#ifdef CXX11
    // Use C++11 random distribution engine
    value = gen();
#else
    value = random();
#endif
    if (mod != 0) value %= mod;
    m[i] = value;
  }
}

//------------------------------------------------------------------------------
string Matrix::name(void) const  {
  if (m_name.length() != 0) return m_name;
  ostringstream sout("");
  sout << "m" << m_id;
  return sout.str();
}

//------------------------------------------------------------------------------
void Matrix::load(const Memory& mem, Addr_t from) {
  // Load from device into normal memory
  Addr_t shape = mem.iget(from++);
  assert(shape != 0 && Msize(shape) < MAX_MATRIX_SIZE);
  m_rows = Mrows(shape);
  m_cols = Mcols(shape);
  m_hard = false;
  delete[] m;
  m = new Data_t[m_rows*m_cols];
  for (Addr_t i=begin(); i!=end(); ++i) {
    m[i] = mem.iget(from++);
  }
}

//------------------------------------------------------------------------------
void Matrix::store(Memory& mem, Addr_t to) const {
  mem.iset(to++, Mshape(m_rows,m_cols));
  for (Addr_t i=begin(); i!=end(); ++i) {
    mem.iset(to++,m[i]);
  }
}

//------------------------------------------------------------------------------
string Matrix::to_string(const string& nm) const {
  ostringstream sout("");
  static const int aw = 3;
  static const int dw = 4;
  // Separator
  string hdr;
  hdr += string(aw,'=') + "==";
  for (Addr_t col=0; col!=m_cols; ++col) {
    if (col != 0) hdr += "==";
    hdr+= string(dw,'=');
  }
  if (nm.size() > 0) {
    if (nm.size() > hdr.size()-5) {
      hdr = "== ";
      hdr += nm;
      hdr += " ==";
    } else {
      hdr[2] = ' ';
      hdr[2+nm.size()+1] = ' ';
      hdr.replace(3,nm.size(),nm);
    }
  }
  sout << hdr << "\n";

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

//------------------------------------------------------------------------------
void Matrix::print(const string& nm) const {
  cout << to_string(nm) << flush;
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator+=(Data_t rhs)
{
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    dev->set_reg(t_reg,rhs);
    CmdState_t retcode = dev->do_command(KADD,m_reg,m_reg,t_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) m[i] += rhs;
  }
  return *this;
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator-=(Data_t rhs)
{
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    dev->set_reg(t_reg,-rhs);
    CmdState_t retcode = dev->do_command(KADD,m_reg,m_reg,t_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) m[i] -= rhs;
  }
  return *this;
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator*=(Data_t rhs)
{
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    dev->set_reg(t_reg,rhs);
    CmdState_t retcode = dev->do_command(KMUL,m_reg,m_reg,t_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) m[i] *= rhs;
  }
  return *this;
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator+=(const Matrix& rhs)
{
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    CmdState_t retcode = dev->do_command(MADD,m_reg,m_reg,rhs.m_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) m[i] += rhs.m[i];
  }
  return *this;
}

//------------------------------------------------------------------------------
Matrix& Matrix::operator-=(const Matrix& rhs)
{
  return *this;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator+(Data_t rhs) const
{
  Matrix result(*this);
  return result += rhs;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator-(Data_t rhs) const
{
  Matrix result(*this);
  return result -= rhs;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator*(Data_t rhs) const
{
  Matrix result(*this);
  return result *= rhs;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator+(const Matrix& rhs) const
{
  Matrix result(*this);
  return result += rhs;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator-(const Matrix& rhs) const
{
  Matrix result(*this);
  return result -= rhs;
}

//------------------------------------------------------------------------------
Matrix Matrix::operator*(const Matrix& rhs) const //< matrix cross-product
{
  assert(m_cols == rhs.m_rows);
  Matrix result(m_rows, rhs.m_cols, m_name, m_hard);
#ifdef USING_HARDWARE
  if (m_hard && rhs.m_hard) {
    CmdState_t retcode = dev->do_command(MMUL,result.m_reg,m_reg,rhs.m_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t row=0; row!=m_rows; ++row) {
      for (Addr_t col=0; col!=rhs.m_cols; ++col) {
        Data_t sum = 0;
        for (Addr_t i=0; i!=m_cols; ++i) {
          sum += rc(row,i)*rhs.rc(i,col);
        }//endfor
        result.rc(row,col) = sum;
      }//endfor
    }//endfor
  }
  return result;
}

//------------------------------------------------------------------------------
Matrix Matrix::transpose(void) const
{
  Matrix t_mtx(m_cols, m_rows, m_name, m_hard);
#ifdef USING_HARDWARE
  if (m_hard) {
    CmdState_t retcode = dev->do_command(TRANS,t_mtx.m_reg,m_reg);
    assert(IS_OK(retcode));
  }
  else
#endif
  {
    for (Addr_t row=0; row!=m_rows; ++row) {
      for (Addr_t col=0; col!=m_cols; ++col) {
        t_mtx.rc(col,row) = rc(row,col);
      }//endfor
    }//endfor
  }
  return t_mtx;
}

//------------------------------------------------------------------------------
Data_t Matrix::sum(void) const
{
  Data_t result = 0;
#ifdef USING_HARDWARE
  if (m_hard) {
    CmdState_t retcode = dev->do_command(MSUM,t_reg,m_reg);
    result = dev->get_reg(t_reg);
  }
  else
#endif
  {
    for (Addr_t i=begin(); i!=end(); ++i) result += m[i];
  }
  return result;
}

//------------------------------------------------------------------------------
// The end
