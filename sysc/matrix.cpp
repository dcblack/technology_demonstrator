#include "matrix.h"
#include <random>
#include <iomanip>
using namespace std;

#ifdef VERBOSITY_ENABLED
bool    Matrix::s_verbosity = true;
size_t  Matrix::next_id     = 0;
ssize_t Matrix::matrix_count     = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructors

Matrix::Matrix(size_t max) //  Random constructor
: mtx(nullptr)
#ifdef VERBOSITY_ENABLED
, m_id(next_id++)
#endif
{
  max &= MAXDIM;
  if (max == 0) max = MAXDIM;
  static default_random_engine gen;
  static uniform_int_distribution<size_t> distr(1,max);
  size_t _rows = distr(gen);
  size_t _cols = distr(gen);
  mtx = (Mdata_t*) malloc(sizeof(Mdata_t)*space(_rows,_cols));
  mtx[SHAPE] = shape(_rows,_cols);
  mtx[CAPACITY] = size();
  randomize();
#ifdef VERBOSITY_ENABLED
  if (s_verbosity) { std::cout << "Constructed random #" << id() << "(" << rows() << "x" << cols() << ")" << std::endl; }
  ++matrix_count;
#endif
}

Matrix::~Matrix(void) //< Destructor
{
  if (mtx != nullptr) {
    check(is_valid(mtx)); // minimal safety check
    free(mtx);
    mtx = nullptr;
  }
#ifdef VERBOSITY_ENABLED
  if (s_verbosity) { std::cout << "Destroyed #" << id() << std::endl; }
  --matrix_count;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Scalar operations
Matrix& Matrix::operator=(const Mdata_t& rhs) // Fill
{
  for (int i=0; i!=size(); ++i) mtx[BASE+i] = rhs;
  return *this;
}

Matrix& Matrix::operator*=(const Mdata_t& rhs) // *Kmul
{
  for (int i=index(0); i!=index(size()); ++i) mtx[i] *= rhs;
  return *this;
}

Matrix Matrix::operator*(const Mdata_t& rhs) const // *Kmul
{
  Matrix result(*this);
  return result *= rhs;
}

Matrix& Matrix::operator/=(const Mdata_t& rhs) // /Kdiv
{
  for (int i=index(0); i!=index(size()); ++i) mtx[i] /= rhs;
  return *this;
}

Matrix Matrix::operator/(const Mdata_t& rhs) const // /Kdiv
{
  Matrix result(*this);
  return result /= rhs;
}

Matrix& Matrix::operator+=(const Mdata_t& rhs) // +Kadd
{
  for (int i=index(0); i!=index(size()); ++i) mtx[i] += rhs;
  return *this;
}

Matrix Matrix::operator+(const Mdata_t& rhs) const // +Kadd
{
  Matrix result(*this);
  return result += rhs;
}

Matrix& Matrix::operator-=(const Mdata_t& rhs) // -Ksub
{
  for (int i=index(0); i!=index(size()); ++i) mtx[i] -= rhs;
  return *this;
}

Matrix Matrix::operator-(const Mdata_t& rhs) const // -Ksub
{
  Matrix result(*this);
  return result -= rhs;
}

Matrix Matrix::abs(void) const
{
  Matrix result(rows(),cols());
  for (int i=index(0); i!=index(size()); ++i) result.mtx[i] = (mtx[i]<0)?-mtx[i]:mtx[i];
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Matrix operations

Matrix& Matrix::operator+=(const Matrix& rhs) // +=Madd
{
  check(size() == rhs.size());
  for (int i=index(0); i!=index(size()); ++i) mtx[i] += rhs.mtx[i];
  return *this;
}

Matrix Matrix::operator+(const Matrix& rhs) const // +Madd
{
  Matrix result(*this);
  return result += rhs;
}

Matrix& Matrix::operator-=(const Matrix& rhs) // -=Msub
{
  check(size() == rhs.size());
  for (int i=index(0); i!=index(size()); ++i) mtx[i] -= rhs.mtx[i];
  return *this;
}

Matrix Matrix::operator-(const Matrix& rhs) const // -Msub
{
  Matrix result(*this);
  return result -= rhs;
}

Matrix Matrix::operator*(const Matrix& rhs) const // *Mmul
{
  check(cols()==rhs.rows());
  Matrix result(rows(),rhs.cols());
  result = 0; // initialize
  for (size_t i=0; i!=rows(); ++i) {
    for (size_t j=0; j!=rhs.cols(); ++j) {
      for (size_t k=0; k!=cols(); ++k) {
        result.at(i,j) += at(i,k) * rhs.at(k,j);
      }//endfor
    }//endfor
  }//endfor
  return result;
}

Matrix& Matrix::operator*=(const Matrix& rhs) // *=Msub
{
  *this = *this * rhs;
  return *this;
}

Matrix Matrix::transpose(void)
{
  Matrix t(cols(),rows());
  for (size_t x=0; x!=cols(); ++x) {
    for (size_t y=0; y!=rows(); ++y) {
      t.at(y,x) = at(y,x);
    }
  }
  return t;
}

////////////////////////////////////////////////////////////////////////////////
// External binary operations

Matrix operator*(const Mdata_t& lhs,const Matrix& rhs) // Kmul*
{
  return rhs * lhs;
}
ostream& operator<<(ostream& os, const Matrix& rhs)
{
#ifdef VERBOSITY_ENABLED
  os << "#" << rhs.id() << ":";
#endif
  os << "\n";
  for (int r=0; r!=rhs.rows(); ++r) {
    os << setw(4) << r << ":";
    for (int c=0; c!=rhs.cols(); ++c) {
      os << " " << setw(4) << rhs.at(r,c);
    }//endfor
    os << "\n";
  }//endfor
  return os;
}

////////////////////////////////////////////////////////////////////////////////
// Matrix tests
size_t Matrix::count(Mdata_t value) const
{
  size_t result = 0;
  for (int i=index(0); i!=index(size()); ++i) result += (mtx[i] == value) ? 1 : 0;
  return result;
}

Mdata_t Matrix::max(void) const {
  Mdata_t result = mtx[index(0)];
  for (int i=index(0); i!=index(size()); ++i) if (result < mtx[i]) result = mtx[i];
  return result;
}

Mdata_t Matrix::min(void) const {
  Mdata_t result = mtx[index(0)];
  for (int i=index(0); i!=index(size()); ++i) if (result > mtx[i]) result = mtx[i];
  return result;
}

bool Matrix::is_triangle(bool above) const
{
  bool result = true;
  if (!is_square()) return false;
  for (size_t x=0; x!=cols(); ++x) {
    for (size_t y=0; y!=rows(); ++y) {
      if ( above && x<y && at(x,y) != 0) return false;
      if (!above && x>y && at(x,y) != 0) return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Special
void Matrix::fill(Kind kind)
{
  switch (kind) {
    case Kind::identity: {
      *this = 0;
      size_t min = rows() < cols() ? rows() : cols();
      for (size_t i=0; i!=min; ++i) at(i,i) = 1;
      break;
    }
    case Kind::lower:
    case Kind::upper: {
      for (size_t x=0; x!=cols(); ++x) {
        for (size_t y=0; y!=rows(); ++y) {
          if (kind == Kind::upper) {
            if (x<y) at(x,y) = 0;
            else     at(x,y) = 1;
          } else {
            if (x>y) at(x,y) = 0;
            else     at(x,y) = 1;
          }
        }//endfor
      }//endfor
      break;
    }
    default: /* includes Kind::shape */ {
      for (size_t x=0; x!=cols(); ++x) {
        for (size_t y=0; y!=rows(); ++y) {
          at(x,y) = (y<<(BITS/2))|x; // shape masks
        }//endfor
      }//endfor
      break;
    }
  }//endswitch
}

void Matrix::randomize(size_t zeroes,size_t negative)
{
  static std::default_random_engine gen;
  static std::uniform_int_distribution<Mdata_t> distr;
  for (size_t x=0; x!=cols(); ++x) {
    for (size_t y=0; y!=rows(); ++y) {
      at(x,y) = distr(gen);
    }//endfor
  }//endfor
  if (zeroes || negative) {
    zeroes %= size();
    negative %= size();
    std::uniform_int_distribution<size_t> row_distr(0,rows()-1);
    std::uniform_int_distribution<size_t> col_distr(0,cols()-1);
    while (zeroes--  ) at(row_distr(gen),col_distr(gen)) *=  0;
    while (negative--) at(row_distr(gen),col_distr(gen)) *= -1;
  }
}

// zap entire data structure including capacity (DANGEROUS - assumes not malloc'd)
void Matrix::zap(Mdata_t* mtx, Mdata_t rhs)
{
  check(is_valid(mtx)); // minimal safety check
  size_t sz = mtx[CAPACITY];
  for (int i=0; i!=BASE+sz; ++i) mtx[i] = rhs;
}

#ifdef UNIT_TEST

#include <string>

namespace {
  size_t here_count = 1;
  size_t errors = 0;
}
#define DEBUG(stream) cout << "DEBUG(" << __LINE__ << "): " << stream << endl
#define ERROR(stream) do { ++errors; cout << "ERROR(" << __LINE__ << "): " << stream << endl; here(0);} while (0)
#define EXPECT(expr)  do { if (!(expr)) ERROR( "NOT " << #expr ); else DEBUG("Got expected " << #expr); } while (0)
#define DUMP(matrix)  DEBUG( #matrix << "= " << matrix )
#define DO(matrix,expr) do { DEBUG(#expr); expr; DUMP(matrix);} while(0)
void here(size_t lineno) { if (lineno) ++here_count; } //< {:ATTENTION: Breakpoint on this when debugging :}
#define HERE          do { DEBUG( " **HERE" << here_count << "**" ); here(__LINE__); } while (0)
void dump(Matrix* m) {
  DUMP(*m);
}

int main(void)
{{
  cout << string(80,'#') << endl;
  cout << string(80,'#') << endl;
  cout << "Testing Matrix class" << endl << endl;

  Matrix::verbosity(true);
  // Test primitives
  cout << "BITS=" << Matrix::BITS << " SHAPE=" << Matrix::SHAPE << " MAXDIM=" << Matrix::MAXDIM << endl;
  DEBUG("shape(2,3) = 0x" << hex << Matrix::shape(2,3)) << dec;
  {
    // Construct, fill, count and destroy a simple matrix
    Matrix m0(2,3);
    DEBUG( "m0.rows()  = " << m0.rows()         );
    EXPECT(m0.rows() == 2);
    DEBUG( "m0.cols()  = " << m0.cols()         );
    EXPECT(m0.cols() == 3);
    DEBUG( "m0.size()  = " << dec << m0.size()  );
    EXPECT(m0.size() == 6);
    DEBUG( "m0.shape() = " << hex << m0.shape() ) << dec;
    m0 = 0; DUMP(m0);
    DEBUG( "m0 has " << m0.count(0) << " zeroes" );
  }
  Matrix m0(1,1);
  Matrix m1(3,4);
  Matrix m2(3,4);
  Matrix m3(3,4);
  Matrix mr(3,4);
  Mdata_t k1, k2, k3;
  m1 = 1; m1.first() *= 10; m1.last() *= 100; DUMP(m1);
  m2 = 2; m2.first() *= 10; m2.last() *= 100; DUMP(m2);
  m3 = 3; m3.first() *= 10; m3.last() *= 100; DUMP(m3);
  // Check comparison
  mr = m1; EXPECT(mr == m1);
  EXPECT(m1 != m2);
  // Check scalar arith operations
  DO(mr,mr = m1;);
  DO(mr,mr *= 2);
  DO(mr,mr /= 2);
  DO(mr,mr += 2);
  DO(mr,mr -= 2);
  DO(mr,mr = m1 * 2);  EXPECT(mr == m2);
  DO(mr,mr = 3 * m1);  EXPECT(mr == m3);
  DO(mr,mr = m1 - 1);  EXPECT(mr.count(0) == m1.size()-2);
  DO(mr,mr = m2 / 2);  EXPECT(mr == m1);
  cout << hex; DO(mr,mr.fill()); cout << dec;
  cout << hex; DO(mr,mr += 1);   cout << dec;
  // Check matrix arith operations
  DO(mr,mr = m2);
  DO(mr,mr += m1);
  HERE;
  DO(mr,mr -= m2);
  HERE;
  DO(mr,mr = m1 + m2); EXPECT(mr == m3);
  HERE;
  DO(mr,mr = m3 - m2); EXPECT(mr == m1);
  Matrix mt(4,3);
  DO(mt,mt = m2.transpose());
  // Create identity
  DEBUG("Identity");
  Matrix m4(Matrix::identity(4)); DUMP(m4);
  Matrix ms(4,4);
  DEBUG( "ms is " << (ms.is_square()? "square" : "rectangular") );
  // Create random
  DEBUG( "Random construction" );
  Matrix mr1(8);
  Matrix mr2(4);
  DO(ms,ms.randomize());
  DO(ms,ms.randomize(4,4));
  // Check count
  ms.at(1,2) = 0;
  ms.at(1,3) = 0;
  DEBUG( "ms has " << ms.count(0) << " zeroes" );
  }
  EXPECT(Matrix::existing() == 0);
  return (errors == 0) ? 0 : 1;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  L<http://www.apache.org/licenses/LICENSE-2.0>
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//------------------------------------------------------------------------------
// The end!
////////////////////////////////////////////////////////////////////////////////
