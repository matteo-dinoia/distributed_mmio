/*
 *   Matrix Market I/O library for ANSI C
 *   See http://math.nist.gov/MatrixMarket for details.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <string>

#include "../include/mmio.h"

#define MMIO_EXPLICIT_TEMPLATE_INST(IT, VT) \
  template int mm_read_mtx_crd_data(FILE *f, int nnz, Entry<IT, VT> *entries, MM_typecode matcode, bool is_bmtx, uint8_t idx_bytes, uint8_t val_bytes); \
  template CSR_local<IT, VT>* Distr_MMIO_CSR_local_create(IT nrows, IT ncols, IT nnz, bool alloc_val); \
  template void Distr_MMIO_CSR_local_destroy(CSR_local<IT, VT> **csr); \
  template COO_local<IT, VT>* Distr_MMIO_COO_local_create(IT nrows, IT ncols, IT nnz, bool alloc_val); \
  template void Distr_MMIO_COO_local_destroy(COO_local<IT, VT> **coo); \
  template void entries_to_local_csr(Entry<IT, VT> *entries, CSR_local<IT, VT> *csr); \
  template void entries_to_local_coo(Entry<IT, VT> *entries, COO_local<IT, VT> *coo); \
  template CSR_local<IT, VT>* Distr_MMIO_CSR_local_read(const char *filename, bool expl_val_for_bin_mtx); \
  template CSR_local<IT, VT>* Distr_MMIO_CSR_local_read_f(FILE *f, bool is_bmtx, bool expl_val_for_bin_mtx); \
  template COO_local<IT, VT>* Distr_MMIO_COO_local_read(const char *filename, bool expl_val_for_bin_mtx); \
  template COO_local<IT, VT>* Distr_MMIO_COO_local_read_f(FILE *f, bool is_bmtx, bool expl_val_for_bin_mtx);
  // template Entry<IT, VT>* mm_parse_file(FILE *f); \
  // template int compare_entries_csr(const void *a, const void *b);

/**
 * Matrix Market parsing utilities
 */ 

int mm_read_banner(FILE *f, MM_typecode *matcode, bool is_bmtx) {
  char line[MM_MAX_LINE_LENGTH];
  char banner[MM_MAX_TOKEN_LENGTH];
  char mtx[MM_MAX_TOKEN_LENGTH];
  char crd[MM_MAX_TOKEN_LENGTH];
  char data_type[MM_MAX_TOKEN_LENGTH];
  char storage_scheme[MM_MAX_TOKEN_LENGTH];
  uint8_t idx_bytes, val_bytes;
  char *p;

  mm_clear_typecode(matcode);

  if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
    return MM_PREMATURE_EOF;

  if (is_bmtx) {
    if (sscanf(line, "%s %s %s %s %s %hhu %hhu", banner, mtx, crd, data_type, storage_scheme, &idx_bytes, &val_bytes) != 7)
      return MM_PREMATURE_EOF;
    mm_set_idx_bytes(matcode, idx_bytes);
    mm_set_val_bytes(matcode, val_bytes);    
  } else {
    if (sscanf(line, "%s %s %s %s %s", banner, mtx, crd, data_type, storage_scheme) != 5)
      return MM_PREMATURE_EOF;
  }

  for (p = mtx; *p != '\0'; *p = tolower(*p), p++)
    ; /* convert to lower case */
  for (p = crd; *p != '\0'; *p = tolower(*p), p++)
    ;
  for (p = data_type; *p != '\0'; *p = tolower(*p), p++)
    ;
  for (p = storage_scheme; *p != '\0'; *p = tolower(*p), p++)
    ;

  /* check for banner */
  if (strncmp(banner, MatrixMarketBanner, strlen(MatrixMarketBanner)) != 0)
    return MM_NO_HEADER;

  /* first field should be "mtx" */
  if (strcmp(mtx, MM_MTX_STR) != 0)
    return MM_UNSUPPORTED_TYPE;
  mm_set_matrix(matcode);

  /* second field describes whether this is a sparse matrix (in coordinate
          storgae) or a dense array */

  if (strcmp(crd, MM_SPARSE_STR) == 0)
    mm_set_sparse(matcode);
  else if (strcmp(crd, MM_DENSE_STR) == 0)
    mm_set_dense(matcode);
  else
    return MM_UNSUPPORTED_TYPE;

  /* third field */

  if (strcmp(data_type, MM_REAL_STR) == 0)
    mm_set_real(matcode);
  else if (strcmp(data_type, MM_COMPLEX_STR) == 0)
    mm_set_complex(matcode);
  else if (strcmp(data_type, MM_PATTERN_STR) == 0)
    mm_set_pattern(matcode);
  else if (strcmp(data_type, MM_INT_STR) == 0)
    mm_set_integer(matcode);
  else
    return MM_UNSUPPORTED_TYPE;

  /* fourth field */

  if (strcmp(storage_scheme, MM_GENERAL_STR) == 0)
    mm_set_general(matcode);
  else if (strcmp(storage_scheme, MM_SYMM_STR) == 0)
    mm_set_symmetric(matcode);
  else if (strcmp(storage_scheme, MM_HERM_STR) == 0)
    mm_set_hermitian(matcode);
  else if (strcmp(storage_scheme, MM_SKEW_STR) == 0)
    mm_set_skew(matcode);
  else
    return MM_UNSUPPORTED_TYPE;

  return 0;
}

int mm_read_mtx_crd_size(FILE *f, uint64_t *nrows, uint64_t *ncols, uint64_t *nnz) {
  char line[MM_MAX_LINE_LENGTH];
  int num_items_read;

  /* set return null parameter values, in case we exit with errors */
  *nrows = *ncols = *nnz = 0;

  /* now continue scanning until you reach the end-of-comments */
  do {
    if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
      return MM_PREMATURE_EOF;
  } while (line[0] == '%');

  /* line[] is either blank or has nrows,ncols, nnz */
  if (sscanf(line, "%lu %lu %lu", nrows, ncols, nnz) == 3)
    return 0;

  else
    do {
      num_items_read = fscanf(f, "%lu %lu %lu", nrows, ncols, nnz);
      if (num_items_read == EOF)
        return MM_PREMATURE_EOF;
    } while (num_items_read != 3);

  return 0;
}

template<typename IT, typename VT>
int mm_read_mtx_crd_data(FILE *f, int nentries, Entry<IT, VT> *entries, MM_typecode matcode, bool is_bmtx, uint8_t idx_bytes, uint8_t val_bytes) {
  bool is_pattern = mm_is_pattern(matcode);
  
  if (!is_bmtx) {
    // Original ASCII Matrix Market parsing
    const char *I_FMT = std::is_same<IT, uint64_t>::value ? "%lu" : "%u";
    const char *V_FMT = std::is_same<VT, double>::value   ? "%lg" : "%g";
    char fmt[32];
    int i;

    if (mm_is_real(matcode) || mm_is_integer(matcode)) {
      snprintf(fmt, sizeof(fmt), "%s %s %s", I_FMT, I_FMT, V_FMT);
      for (i = 0; i < nentries; i++) {
        if (fscanf(f, fmt, &entries[i].row, &entries[i].col, &entries[i].val) != 3)
          return MM_PREMATURE_EOF;
        --entries[i].row;
        --entries[i].col;
      }
    } else if (is_pattern) {
      snprintf(fmt, sizeof(fmt), "%s %s", I_FMT, I_FMT);
      for (i = 0; i < nentries; i++) {
        if (fscanf(f, fmt, &entries[i].row, &entries[i].col) != 2)
          return MM_PREMATURE_EOF;
        --entries[i].row;
        --entries[i].col;
        entries[i].val = static_cast<VT>(1.0);
      }
    } else return MM_UNSUPPORTED_TYPE;

    return 0;
  }

  // Binary BMTX parsing
  for (int i = 0; i < nentries; ++i) {
    uint64_t row = 0;
    if (fread(&row, idx_bytes, 1, f) != 1) return MM_PREMATURE_EOF;
    entries[i].row = static_cast<IT>(row);
    // --entries[i].row;

    uint64_t col = 0;
    if (fread(&col, idx_bytes, 1, f) != 1) return MM_PREMATURE_EOF;
    entries[i].col = static_cast<IT>(col);
    // --entries[i].col;

    if (!is_pattern) {
      if (val_bytes == 4) {
        float val_f;
        if (fread(&val_f, sizeof(float), 1, f) != 1) return MM_PREMATURE_EOF;
        entries[i].val = static_cast<VT>(val_f);
      } else if (val_bytes == 8) {
        double val_d;
        if (fread(&val_d, sizeof(double), 1, f) != 1) return MM_PREMATURE_EOF;
        entries[i].val = static_cast<VT>(val_d);
      } else {
        return MM_UNSUPPORTED_TYPE;
      }
    } else {
      entries[i].val = static_cast<VT>(1.0); // Pattern default
    }
    // printf("%u %u %.2f\n", entries[i].row, entries[i].col, entries[i].val);
  }

  return 0;
}


int required_bytes_index(uint64_t maxval) {
  if (maxval <= UINT8_MAX)  return 1;
  if (maxval <= UINT16_MAX) return 2;
  if (maxval <= UINT32_MAX) return 4;
  return 8;
}

/**
 * Structs conctructors and distructors
 */ 

// CSR

template<typename IT, typename VT>
CSR_local<IT, VT>* Distr_MMIO_CSR_local_create(IT nrows, IT ncols, IT nnz, bool alloc_val) {
  CSR_local<IT, VT> *csr = (CSR_local<IT, VT> *)malloc(sizeof(CSR_local<IT, VT>));
  csr->nrows = nrows;
  csr->ncols = ncols;
  csr->nnz = nnz;
  csr->row_ptr = (IT *)malloc((nrows + 1) * sizeof(IT));
  csr->col_idx = (IT *)malloc(nnz * sizeof(IT));
  csr->val = NULL;
  if (alloc_val) {
    csr->val = (VT *)malloc(nnz * sizeof(VT));
  }
  return csr;
}

template<typename IT, typename VT>
void Distr_MMIO_CSR_local_destroy(CSR_local<IT, VT> **csr) {
  if ((*csr)->row_ptr != NULL) {
    free((*csr)->row_ptr);
    (*csr)->row_ptr = NULL;
  }
  if ((*csr)->col_idx != NULL) {
    free((*csr)->col_idx);
    (*csr)->col_idx = NULL;
  }
  if ((*csr)->val != NULL) {
    free((*csr)->val);
    (*csr)->val = NULL;
  }
  free(*csr);
  *csr = NULL;
}

// COO

template<typename IT, typename VT>
COO_local<IT, VT>* Distr_MMIO_COO_local_create(IT nrows, IT ncols, IT nnz, bool alloc_val) {
  COO_local<IT, VT> *coo = (COO_local<IT, VT> *)malloc(sizeof(COO_local<IT, VT>));
  coo->nrows = nrows;
  coo->ncols = ncols;
  coo->nnz = nnz;
  coo->row = (IT *)malloc(nnz * sizeof(IT));
  coo->col = (IT *)malloc(nnz * sizeof(IT));
  coo->val = NULL;
  if (alloc_val) {
    coo->val = (VT *)malloc(nnz * sizeof(VT));
  }
  return coo;
}

template<typename IT, typename VT>
void Distr_MMIO_COO_local_destroy(COO_local<IT, VT> **coo) {
  if ((*coo)->row != NULL) {
    free((*coo)->row);
    (*coo)->row = NULL;
  }
  if ((*coo)->col != NULL) {
    free((*coo)->col);
    (*coo)->col = NULL;
  }
  if ((*coo)->val != NULL) {
    free((*coo)->val);
    (*coo)->val = NULL;
  }
  free(*coo);
  *coo = NULL;
}

/**
 * Specific parsing functions
 */ 

// CSR

template<typename IT, typename VT>
int compare_entries_csr(const void *a, const void *b) {
  Entry<IT, VT> *ea = (Entry<IT, VT> *)a;
  Entry<IT, VT> *eb = (Entry<IT, VT> *)b;
  if (ea->row != eb->row)
    return ea->row - eb->row;
  return ea->col - eb->col;
}

template<typename IT, typename VT>
void entries_to_local_csr(Entry<IT, VT> *entries, CSR_local<IT, VT> *csr) {
  qsort(entries, csr->nnz, sizeof(Entry<IT, VT>), compare_entries_csr<IT, VT>);

  for (IT v = 0, i = 0; v < csr->nrows; v++) {
    csr->row_ptr[v] = i;
    while (i < csr->nnz && entries[i].row == v) {
      csr->col_idx[i] = entries[i].col;
      if (csr->val != NULL) {
        csr->val[i] = entries[i].val;
      }
      ++i;
    }
  }
  csr->row_ptr[csr->nrows] = csr->nnz;
}

// COO

template<typename IT, typename VT>
void entries_to_local_coo(Entry<IT, VT> *entries, COO_local<IT, VT> *coo) {
  for (IT i = 0; i < coo->nnz; ++i) {
    coo->row[i] = entries[i].row;
    coo->col[i] = entries[i].col;
    if (coo->val != NULL) coo->val[i] = entries[i].val;
  }
}

/**
 * Read functions
 */ 

FILE *open_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Could not open file [%s].\n", filename);
    return NULL;
  }
  return f;
}

bool is_file_extension_bmtx(std::string filename) {
  return filename.size() >= 5 && filename.compare(filename.size() - 5, 5, ".bmtx") == 0;
}

template<typename IT, typename VT>
Entry<IT, VT>* mm_parse_file(FILE *f, IT &nrows, IT &ncols, IT &nnz, MM_typecode *matcode, bool is_bmtx) {
  if (f == NULL) return NULL;

  int err = mm_read_banner(f, matcode, is_bmtx);
  if (err != 0) {
    fprintf(stderr, "Could not process Matrix Market banner. Error (%d)\n", err);
    return NULL;
  }
  if (mm_is_complex(*matcode)) {
    fprintf(stderr, "Cannot parse complex-valued matrices.\n");
    return NULL;
  }
  if (mm_is_array(*matcode)) {
    fprintf(stderr, "Cannot parse array matrices.\n");
    return NULL;
  }
  if (mm_is_skew(*matcode)) {
    fprintf(stderr, "Cannot parse skew-symmetric matrices.\n");
    return NULL;
  }
  if (mm_is_hermitian(*matcode)) {
    fprintf(stderr, "Cannot parse hermitian matrices.\n");
    return NULL;
  }

  uint64_t _nrows, _ncols, _nnz, mm_nnz;
  if (mm_read_mtx_crd_size(f, &_nrows, &_ncols, &mm_nnz) != 0) {
    fprintf(stderr, "Could not parse matrix size.\n");
    return NULL;
  }

  uint8_t idx_bytes = mm_get_idx_bytes(*matcode);
  uint8_t val_bytes = mm_get_val_bytes(*matcode);
  
  if (is_bmtx
      && !(idx_bytes == 1 || idx_bytes == 2 || idx_bytes == 4 || idx_bytes == 8)
      && !(val_bytes == 1 || val_bytes == 2 || val_bytes == 4 || val_bytes == 8)) {
    fprintf(stderr, "BMTX BUG: this should not happen. idx: %hhu bytes, val: %hhu bytes. Please report this.\n", idx_bytes, val_bytes);
    return NULL;
  }
  
  int IT_required_bytes = required_bytes_index(std::max(_nrows, _ncols) - 1);
  if (sizeof(IT) < IT_required_bytes) {
    fprintf(stderr, "Error: Index Type (IT) is too small to represent matrix indices (need at least %d bytes, got %zu bytes).\n", IT_required_bytes, sizeof(IT));
    return NULL;
  }
  if (idx_bytes < IT_required_bytes) {
    fprintf(stderr, "BMTX BUG: this should not happen. Need at least %d bytes, binary is written using %hhu bytes. Please report this.\n", IT_required_bytes, idx_bytes);
    return NULL;
  }

  _nnz = mm_is_symmetric(*matcode) ? mm_nnz * 2 : mm_nnz; // For symmetric matrices THIS IS AN UPPER BOUND
  nrows = static_cast<IT>(_nrows);
  ncols = static_cast<IT>(_ncols);

  Entry<IT, VT> *entries = (Entry<IT, VT> *)malloc(_nnz * sizeof(Entry<IT, VT>));
  if (mm_read_mtx_crd_data<IT, VT>(f, mm_nnz, entries, *matcode, is_bmtx, idx_bytes, val_bytes) != 0) {
    printf("Could not parse matrix data.\n");
    free(entries);
    fclose(f);
    return NULL;
  }
  fclose(f);

  if (mm_is_symmetric(*matcode)) {
    _nnz = mm_nnz;
    // Duplicate the entries for symmetric matrices
    for (uint64_t i = 0, j = 0; i < mm_nnz; ++i) {
      if (entries[i].row != entries[i].col) { // Do not duplicate diagonal
        entries[j + mm_nnz].row = entries[i].col;
        entries[j + mm_nnz].col = entries[i].row;
        entries[j + mm_nnz].val = entries[i].val;
        ++_nnz;
        ++j;
      }
    }
  }

  nnz = static_cast<IT>(_nnz);

  return entries;
}

// CSR

template<typename IT, typename VT>
CSR_local<IT, VT>* Distr_MMIO_CSR_local_read(const char *filename, bool expl_val_for_bin_mtx) {
  return Distr_MMIO_CSR_local_read_f<IT, VT>(open_file(filename), is_file_extension_bmtx(std::string(filename)), expl_val_for_bin_mtx);
}
// template CSR_local<uint64_t, double>* Distr_MMIO_CSR_local_read(const char *filename, bool expl_val_for_bin_mtx);

template<typename IT, typename VT>
CSR_local<IT, VT>* Distr_MMIO_CSR_local_read_f(FILE *f, bool is_bmtx, bool expl_val_for_bin_mtx) {
  IT nrows, ncols, nnz;
  MM_typecode matcode;
  Entry<IT, VT> *entries = mm_parse_file<IT, VT>(f, nrows, ncols, nnz, &matcode, is_bmtx);
  if (entries == NULL) return NULL;

  CSR_local<IT, VT> *csr = Distr_MMIO_CSR_local_create<IT, VT>(nrows, ncols, nnz, expl_val_for_bin_mtx || !mm_is_pattern(matcode));
  entries_to_local_csr<IT, VT>(entries, csr);

  free(entries);

  return csr;
}
// template CSR_local<uint64_t, double>* Distr_MMIO_CSR_local_read_f(FILE *f, bool expl_val_for_bin_mtx);

// COO

template<typename IT, typename VT>
COO_local<IT, VT>* Distr_MMIO_COO_local_read(const char *filename, bool expl_val_for_bin_mtx) {
  return Distr_MMIO_COO_local_read_f<IT, VT>(open_file(filename), is_file_extension_bmtx(std::string(filename)), expl_val_for_bin_mtx);
}

template<typename IT, typename VT>
COO_local<IT, VT>* Distr_MMIO_COO_local_read_f(FILE *f, bool is_bmtx, bool expl_val_for_bin_mtx) {
  IT nrows, ncols, nnz;
  MM_typecode matcode;
  Entry<IT, VT> *entries = mm_parse_file<IT, VT>(f, nrows, ncols, nnz, &matcode, is_bmtx);
  if (entries == NULL) return NULL;
  
  COO_local<IT, VT> *coo = Distr_MMIO_COO_local_create<IT, VT>(nrows, ncols, nnz, expl_val_for_bin_mtx || !mm_is_pattern(matcode));
  entries_to_local_coo<IT, VT>(entries, coo);
  
  free(entries);

  return coo;
}


MMIO_EXPLICIT_TEMPLATE_INST(uint32_t, float)
MMIO_EXPLICIT_TEMPLATE_INST(uint32_t, double)
MMIO_EXPLICIT_TEMPLATE_INST(uint64_t, float)
MMIO_EXPLICIT_TEMPLATE_INST(uint64_t, double)
