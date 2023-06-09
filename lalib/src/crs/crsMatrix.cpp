#include "crsMatrix.hpp"
#include "../declare_lalib.hpp"


using namespace lalib;

// -------------------CONSTRUCTORS AND DESTRUCTORS--------------------------


// Constructor that doesn't allocate memory
CRSMatrix::CRSMatrix(void) {}

// Constructor that copies the contents of a given matrix
CRSMatrix::CRSMatrix(const CRSMatrix& that) {
  if (that._ncols > 0 && that._nrows > 0) {
    _ncols = that._ncols;
    _nrows = that._nrows;

    vals = that.vals;
    colInds = that.colInds;
    rowPtrs = that.rowPtrs;
  }
}

// Constructor that 'allocates' memory for wanted sized matrix and initializes
// the values as zeros
CRSMatrix::CRSMatrix(int rows, int cols) {
  if (cols < 1 || rows < 1) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must be positive!", __FILE__, __func__, __LINE__));
  }

  _ncols = cols;
  _nrows = rows;

  rowPtrs = std::vector<int>(rows + 1, 0);
}

// Constructor that allocates memory for wanted sized matrix and initializes
// the values as zeros
CRSMatrix::CRSMatrix(int rows, int cols, double init_val) {

  if (cols < 1 || rows < 1) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must be positive!", __FILE__, __func__, __LINE__));
  }

  if (init_val == 0.0) {
    _ncols = cols;
    _nrows = rows;

    rowPtrs = std::vector<int>(rows + 1, 0);
  }
  else {
    std::cout << "\nWARNING: Full matrix allocation is not memory efficient! Consider using DenseMatrix class instead." << "\n\n";
  
    _ncols = cols;
    _nrows = rows;

    vals.reserve(_ncols * _nrows);
    colInds.reserve(_ncols * _nrows);
    rowPtrs.reserve(_nrows + 1);

    for (int row = 0; row < _nrows; row++) {
      for (int col = 0; col < _ncols; col++) {
	vals.push_back(init_val);
	colInds.push_back(col);
      }
      rowPtrs.push_back(row * _ncols);
    }
    rowPtrs.push_back(_nrows * _ncols);
  }
}

// Constructor that copies the contents of double array into a matrix.
// NOTE! SHOULD NOT BE USED UNLESS ABSOLUTELY NECESSARY! This function will
// read the needed amount of elements from the array independent of the size
// of the array (which can not be verified) and thus might read unwanted memory.
// Assumes that array elems is in dense format
CRSMatrix::CRSMatrix(int rows, int cols, double* elems) {
  std::cout << "\nWARNING: Initializing a matrix with double array might lead to undefined behaviour!" << "\n\n";

  if (cols < 1 || rows < 1) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must be positive!", __FILE__, __func__, __LINE__));
  }

  _ncols = cols;
  _nrows = rows;
  rowPtrs.push_back(0);

  for (int row = 0; row < _nrows; row++) {
    int elems_on_row = 0;
    for (int col = 0; col < _ncols; col++) {
      double elem = elems[row * _ncols + col];
      if (elem != 0.0) {
	vals.push_back(elem);
	colInds.push_back(col);
	elems_on_row++;
      }
    }
    rowPtrs.push_back(rowPtrs.back() + elems_on_row);
  }
}

// Constructor that copies the contents of a std::vector into a matrix.
// NOTE! If the number of elements in the std::vector doesn't match the 
// dimensions of the matrix either the extra elements are ignored or 
// the matrix is padded with zeros at the last rows. In either case a 
// warning is printed.
CRSMatrix::CRSMatrix(int rows, int cols, std::vector<double> elems) {
  if (cols < 1 || rows < 1) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must be positive!", __FILE__, __func__, __LINE__));
  }

  if (rows * cols != (int)elems.size()) {
    std::cout << "\nWARNING: Given dimensions don't match with the size of the std::vector!" << "\n\n";
  } 

  _ncols = cols;
  _nrows = rows;
  rowPtrs.push_back(0);

  for (int row = 0; row < _nrows; row++) {
    int elems_on_row = 0;
    for (int col = 0; col < _ncols; col++) {
      double elem = (int)elems.size() > row * _ncols + col ? elems[row * _ncols + col] : 0.0;
      if (elem != 0.0) {
	vals.push_back(elem);
	colInds.push_back(col);
	elems_on_row++;
      }
    }
    rowPtrs.push_back(rowPtrs.back() + elems_on_row);
  }
}

// Constructor that copies the contents of given std::vectors and assigns them as
// the value and index arrays
CRSMatrix::CRSMatrix(int rows, int cols, std::vector<double> new_vals, std::vector<int> new_colInds, std::vector<int> new_rowPtrs) {
  if (*std::max_element(new_colInds.begin(), new_colInds.end()) > cols || *std::max_element(new_rowPtrs.begin(), new_rowPtrs.end()) || *std::min_element(new_colInds.begin(), new_colInds.end()) <= 0 || *std::min_element(new_rowPtrs.begin(), new_rowPtrs.end()) <= 0.0) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions out of bounds!", __FILE__, __func__, __LINE__));
  }
  
  _ncols = cols;
  _nrows = rows;

  vals = new_vals;
  colInds = new_colInds;
  rowPtrs = new_rowPtrs;
}

// Constructor that reads the contents of a given file and stores them in a matrix.
// The file should consist of three whitespace separated columns s.t. the first column
// tells the row, the second tells the column and third the value.
// The last row of the file should hold the lower right corner element of the matrix
// even if it is zero.
CRSMatrix::CRSMatrix(std::string path) {
  // Variables to read the line contents to
  int row, col;
  double val;

  int nTokens = _numTokens(lastLine.str());
  
  if (nTokens == 3) {
    lastLine >> row >> col >> val;
    
    _nrows = row + 1;
    _ncols = col + 1;

    rowPtrs = std::vector<int>(_nrows + 1, 0);

    // Start reading the lines from the beginning of the file
    std::ifstream file(path);

    while (file >> row >> col >> val) {
      this->place(row, col, val);
    }
    
    file.close();
  }

  else if (nTokens == 2) {
    lastLine >> row  >> val;
    
    _nrows = row + 1;
    _ncols = 1;

    rowPtrs = std::vector<int>(_nrows + 1, 0);

    // Start reading the lines from the beginning of the file
    std::ifstream file(path);

    while (file >> row >> val) {
      this->place(row, 0, val);
    }

    file.close();
  }
  else {
    throw std::invalid_argument(_formErrorMsg("Improper data file!", __FILE__, __func__, __LINE__));
  }
}

// Constructor that is very similar to the one above, but allows telling the offset for
// indexing. That is if the file was written so that indexing starts at 1 passing an
// offset of 1 negates this.
CRSMatrix::CRSMatrix(std::string path, int offset) {
  // Variables to read the line contents to
  int row, col;
  double val;
  
  // Read the last line of the file to get the dimensions of the matrix
  std::stringstream lastLine = _lastLine(path);

  int nTokens = _numTokens(lastLine.str());
  
  if (nTokens == 3) {
    lastLine >> row >> col >> val;
    
    _nrows = row + 1 - offset;
    _ncols = col + 1 - offset;

    rowPtrs = std::vector<int>(_nrows + 1, 0);

    // Start reading the lines from the beginning of the file
    std::ifstream file(path);

    while (file >> row >> col >> val) {
      this->place(row - offset, col - offset, val);
    }
    
    file.close();
  }

  else if (nTokens == 2) {
    lastLine >> row  >> val;
    
    _nrows = row + 1 - offset;
    _ncols = 1;

    rowPtrs = std::vector<int>(_nrows + 1, 0);

    // Start reading the lines from the beginning of the file
    std::ifstream file(path);

    while (file >> row >> val) {
      this->place(row - offset, 0, val);
    }

    file.close();
  }
  else {
    throw std::invalid_argument(_formErrorMsg("Improper data file!", __FILE__, __func__, __LINE__));
  }
}


// ---------------------OVERLOADED BASIC MATH OPERATORS-----------------------

CRSMatrix& CRSMatrix::operator+= (const CRSMatrix& that) {
  if (_ncols != that._ncols || _nrows != that._nrows) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must match!", __FILE__, __func__, __LINE__));
  }

  for (int row = 0; row < _nrows; row++) {
    int rowPtr = that.rowPtrs[row];
    int nrowElems = that.rowPtrs[row + 1] - rowPtr;
    for (int col_i = 0; col_i < nrowElems; col_i++) {
      int col = that.colInds[rowPtr + col_i];
      double val = this->operator() (row, col) + that(row, col);
      this->place(row, col, val);
    }
  }
  
  return *this;
}

const CRSMatrix CRSMatrix::operator+ (const CRSMatrix& that) const {
  return CRSMatrix(*this) += that;
}

CRSMatrix& CRSMatrix::operator-= (const CRSMatrix& that) {
  if (_ncols != that._ncols || _nrows != that._nrows) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must match!", __FILE__, __func__, __LINE__));
  }

  for (int row = 0; row < _nrows; row++) {
    int rowPtr = that.rowPtrs[row];
    int nrowElems = that.rowPtrs[row + 1] - rowPtr;
    for (int col_i = 0; col_i < nrowElems; col_i++) {
      int col = that.colInds[rowPtr + col_i];
      double val = this->operator() (row, col) - that(row, col);
      this->place(row, col, val);
    }
  }
  
  return *this;
}

const CRSMatrix CRSMatrix::operator- (const CRSMatrix& that) const {
  return CRSMatrix(*this) -= that;
}

CRSMatrix& CRSMatrix::operator*= (const CRSMatrix& that) {
  if (_ncols != that._ncols || _nrows != that._nrows) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must match!", __FILE__, __func__, __LINE__));
  }

  for (int row = 0; row < _nrows; row++) {
    for (int col = 0; col < _ncols; col++) {
      double val = this->operator() (row, col) * that(row, col);
      this->place(row, col, val);
    }
  }
  
  return *this;
}

const CRSMatrix CRSMatrix::operator* (const CRSMatrix& that) const {
  return CRSMatrix(*this) *= that;
}

const CRSMatrix CRSMatrix::operator* (const double that) const {
  if (_ncols < 1 || _nrows < 1) {
    return *this;
  }

  CRSMatrix ret = CRSMatrix(*this);

  for (int i = 0; i < (int)vals.size(); i++) {
    ret.vals[i] *= that;
  }

  return ret;
}

const CRSMatrix lalib::operator* (double scalar, const CRSMatrix& matrix) {
  return matrix * scalar;
}

CRSMatrix& CRSMatrix::operator/= (const CRSMatrix& that) {
  if (_ncols != that._ncols || _nrows != that._nrows) {
    throw std::invalid_argument(_formErrorMsg("Matrix dimensions must match!", __FILE__, __func__, __LINE__));
  }

  for (int row = 0; row < _nrows; row++) {
    for (int col = 0; col < _ncols; col++) {
      double val = this->operator() (row, col) / that(row, col);
      this->place(row, col, val);
    }
  }
  
  return *this;
}

const CRSMatrix CRSMatrix::operator/ (const CRSMatrix& that) const {
  return CRSMatrix(*this) /= that;
}


// ----------------------OVERLOADED INDEXING OPERATORS--------------------------

void CRSMatrix::place(int row, int col, double val) {
  if (row < 0 || col < 0 || row >= _nrows || col >= _ncols) {
    throw std::invalid_argument(_formErrorMsg("Given dimensions out of bounds!", __FILE__, __func__, __LINE__));
  }

  // If value is not zero it needs to be placed into the matrix
  if (val != 0.0) {

    // If there are no elements yet in the matrix just add the value
    if (vals.empty()) {
      vals.push_back(val);
      colInds.push_back(col);
    }

    // Otherwise, we need to find the correct location for the value
    else {
	
      int rowPtr = rowPtrs[row];
      int nextRow = rowPtrs[row + 1];

      // If the row is empty just add to the row pointers position
      if (rowPtr == nextRow) {
	vals.insert(vals.begin() + rowPtr, val);
	colInds.insert(colInds.begin() + rowPtr, col);
      }

      // Otherwise, iterate over the columns to find the correct gap
      else {

	int col_i = rowPtr;
	for (; col_i < nextRow; col_i++) {
	  int col0 = colInds[col_i];

	  // If there already is a value at given location replace it
	  if (col0 == col) {
	    vals[col_i] = val;
	    return;
	  }

	  // If the found row is larger than given column then insert value before it
	  else if (col0 > col) {
	    vals.insert(vals.begin() + col_i, val);
	    colInds.insert(colInds.begin() + col_i, col);
	    break;
	  }
	}

	// New column is the largest that has a value in the given row
	if (col_i == nextRow) {
	  vals.insert(vals.begin() + nextRow, val);
	  colInds.insert(colInds.begin() + nextRow, col);
	}
      }
    }

    // Increment the row pointers accordingly
    for (int row_i = row + 1; row_i <= _nrows; row_i++) {
      rowPtrs[row_i] += 1;
    }

  }

  // If input is zero we need to check if it replaces some non-zero value
  else {

    int rowPtr = rowPtrs[row];
    int nextRow = rowPtrs[row + 1];

    // If the row is empty zero cannot replace a non-zero value
    if (rowPtr == nextRow) {
      return;
    }

    // Otherwise, iterate over the columns to check if there is a non-zero value
    else {

      int col_i = rowPtr;
      for (; col_i < nextRow; col_i++) {
	int col0 = colInds[col_i];

	// If there already is a value at given location replace it
	if (col0 == col) {
	  vals.erase(vals.begin() + col_i);
	  colInds.erase(colInds.begin() + col_i);
	  break;
	}

	// If the found column is larger than given column then the zero didn't replace a non-zero
	else if (col0 > col) {
	  return;
	}
      }

      // If the new column is the largest then it cannot replace a non-zero element
      if (col_i == nextRow) {
	return;
      }
    }
      
    // Decrement the row pointers accordingly
    for (int row_i = row + 1; row_i <= _nrows; row_i++) {
      rowPtrs[row_i] -= 1;
    }
  }
}

double CRSMatrix::operator() (int row, int col) const {
  if (row < 0 || col < 0 || row >= _nrows || col >= _ncols) {
    throw std::invalid_argument(_formErrorMsg("Given dimensions out of bounds!", __FILE__, __func__, __LINE__));
  }

  int rowPtr = rowPtrs[row];
  int nextRow = rowPtrs[row + 1];

  if (rowPtr == nextRow) {
    return 0.0;
  }

  else {
    int col_i = rowPtr;
    for (; col_i < nextRow; col_i++) {
      int col0 = colInds[col_i];
      if (col0 == col) {
	return vals[col_i];
      }
      else if (col0 > col) {
	return 0.0;
      }
    }
  }
  
  return 0.0;
}

double CRSMatrix::operator[] (int num) const {
  
  int row = num / _nrows;
  int col = num % _nrows;

  return this->operator() (row, col);
}

double CRSMatrix::get(int row, int col) const {
  return this->operator() (row, col);
}


// -------------------- OTHER OVERLOADED OPERATORS -----------------------


CRSMatrix& CRSMatrix::operator= (const CRSMatrix& that) {
  // Check for self-assignment ie. case where a = a is called by comparing the pointers of the objects
  if (this == &that) return *this; 

  _ncols = that._ncols;
  _nrows = that._nrows;

  vals = that.vals;
  colInds = that.colInds;
  rowPtrs = that.rowPtrs;

  return *this;
}


bool CRSMatrix::operator== (const CRSMatrix& that) {
  if (_nrows != that._nrows || _ncols != that._ncols) {
    return false;
  }

  int this_num_non_zeros = vals.size();
  int that_num_non_zeros = that.vals.size();

  if (this_num_non_zeros != that_num_non_zeros) {
    return false;
  }

  for (int i = 0; i < this_num_non_zeros; i++) {
    if (vals[i] != that.vals[i] || colInds[i] != that.colInds[i]) {
      return false;
    }
  }

  return true;
}


bool CRSMatrix::operator!= (const CRSMatrix& that) {
  return !(*this == that);
}



// ----------------------- MISC ----------------------------


bool CRSMatrix::isclose(const CRSMatrix& that, double tol) {
  if (_nrows != that._nrows || _ncols != that._ncols) {
    return false;
  }

  int this_num_non_zeros = vals.size();
  int that_num_non_zeros = that.vals.size();

  if (this_num_non_zeros != that_num_non_zeros) {
    return false;
  }

  for (int i = 0; i < this_num_non_zeros; i++) {
    if (fabs(vals[i] - that.vals[i]) > tol || colInds[i] != that.colInds[i]) {
      return false;
    }
  }

  return true;
}


bool CRSMatrix::save(std::string path) {
  if (_ncols <= 0 || _nrows <= 0) {
    throw std::invalid_argument(_formErrorMsg("Cannot save an unitialized matrix!", __FILE__, __func__, __LINE__));
  }

  std::ofstream file(path);
  bool success = true;

  for (int row = 0; row < _nrows; row++) {
    for (int row_p = rowPtrs[row]; row_p < rowPtrs[row + 1]; row_p++) {
      int col = colInds[row_p];
      double val = vals[row_p];

      if (!(file << row << " " << col << " " << val << "\n")) {
	success = false;
      }
    }
  }
  
  file.close();
  
  return success;
}


// TODO: Optimize transpose operation


const CRSMatrix CRSMatrix::transpose() const {
  
  if (_ncols <= 0 || _nrows <= 0) {
    return *this;
  }

  // Initialize the transpose matrix
  CRSMatrix ret = CRSMatrix(_ncols, _nrows);

  for (int col = 0; col < _ncols; col++) {
    for (int row = 0; row < _nrows; row++) {
      double val = this->operator() (row, col);

      if (val != 0.0) {
	ret.vals.push_back(val);
	ret.colInds.push_back(row);

	for (int col_i = col + 1; col_i <= _ncols; col_i++) {
	  ret.rowPtrs[col_i] += 1;
	}
      }
    }
  }
  
  return ret;
}

const CRSMatrix CRSMatrix::naiveTranspose() const {
  if (_ncols <= 0 || _nrows <= 0) {
    return *this;
  }

  // Initialize the transpose matrix
  CRSMatrix ret = CRSMatrix(_ncols, _nrows);

  for (int row = 0; row < _nrows; row++) {
    for (int col = 0; col < _ncols; col++) {
      double val = this->operator() (row, col);

      if (val != 0.0) {
	ret.place(col, row, val);
      }
    }
  }
  
  return ret;
}

const CRSMatrix CRSMatrix::T() const {
  return this->transpose();
}


double CRSMatrix::asDouble() const {
  if (_ncols != 1 || _nrows != 1) {
    throw std::invalid_argument(_formErrorMsg("Matrix must be a 1 x 1 matrix!", __FILE__, __func__, __LINE__));
  }

  if (vals.size() > 0) return vals[0];

  return 0.0;
}


double CRSMatrix::norm() const {
  if (_ncols <= 0 || _nrows <= 0) {
    throw std::invalid_argument(_formErrorMsg("Matrix must be initialized!", __FILE__, __func__, __LINE__));
  }

  double ret = 0.0;

  for (double val: vals) ret += pow(val, 2.0);

  return pow(ret, 1.0 / 2.0);
}


void CRSMatrix::_printArrays() {
  std::cout << "vals: [";
  for (double val: vals) std::cout << val << " ";
  std::cout << "]\n";

  std::cout << "col_i: [";
  for (int col: colInds) std::cout << col << " ";
  std::cout << "]\n";
  
  std::cout << "row_p: [";
  for (int row_p: rowPtrs) std::cout << row_p << " ";
  std::cout << "]\n";
}

std::ostream& lalib::operator<<(std::ostream& os, CRSMatrix& A) {
  if (A.ncols() == 0 || A.nrows() == 0) {
    os << "[]" << std::endl;  // Signifies uninitialized matrix
        
    return os;
  }
    
  os << "[";
  for (int row = 0; row < A.nrows(); row++) {
    if (row > 0) os << ' ';

    os << "[";
    for (int col = 0; col < A.ncols() - 1; col++) {
      os << A(row, col) << ' ';
    }
    os << A(row, A.ncols() - 1) << "]";

    if (row < A.nrows() - 1) os << std::endl; 
  }
  os << "]" << std::endl;

  return os;
}

