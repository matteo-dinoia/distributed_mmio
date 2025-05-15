/*
 *   Matrix Market I/O library for ANSI C
 *   See http://math.nist.gov/MatrixMarket for details.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

int mm_read_banner(FILE *f, MM_typecode *matcode) {
  char line[MM_MAX_LINE_LENGTH];
  char banner[MM_MAX_TOKEN_LENGTH];
  char mtx[MM_MAX_TOKEN_LENGTH];
  char crd[MM_MAX_TOKEN_LENGTH];
  char data_type[MM_MAX_TOKEN_LENGTH];
  char storage_scheme[MM_MAX_TOKEN_LENGTH];
  char *p;

  mm_clear_typecode(matcode);

  if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
    return MM_PREMATURE_EOF;

  if (sscanf(line, "%s %s %s %s %s", banner, mtx, crd, data_type,
             storage_scheme) != 5)
    return MM_PREMATURE_EOF;

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

int mm_read_mtx_crd_size(FILE *f, uint32_t *nrows, uint32_t *ncols, uint32_t *nnz) {
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
  if (sscanf(line, "%u %u %u", nrows, ncols, nnz) == 3)
    return 0;

  else
    do {
      num_items_read = fscanf(f, "%u %u %u", nrows, ncols, nnz);
      if (num_items_read == EOF)
        return MM_PREMATURE_EOF;
    } while (num_items_read != 3);

  return 0;
}

/*-------------------------------------------------------------------------*/

/******************************************************************/
/* use when I[], J[], and val[]J, and val[] are already allocated */
/******************************************************************/

int mm_read_mtx_crd_data(FILE *f, int nnz, Entry entries[], MM_typecode matcode) {
  int i;
  if (mm_is_real(matcode) || mm_is_integer(matcode)) {
    for (i = 0; i < nnz; i++) {
      if (fscanf(f, "%u %u %lg\n", &entries[i].row, &entries[i].col, &entries[i].val) != 3)
        return MM_PREMATURE_EOF;
    }
  } else if (mm_is_pattern(matcode)) {
    for (i = 0; i < nnz; i++)
      if (fscanf(f, "%u %u", &entries[i].row, &entries[i].col) != 2)
        return MM_PREMATURE_EOF;
  } else
    return MM_UNSUPPORTED_TYPE;

  return 0;
}

/**
 *  Create a new copy of a string s.  mm_strdup() is a common routine, but
 *  not part of ANSI C, so it is included here.  Used by mm_typecode_to_str().
 *
 */
char *mm_strdup(const char *s) {
  int len = strlen(s);
  char *s2 = (char *)malloc((len + 1) * sizeof(char));
  return strcpy(s2, s);
}

char *mm_typecode_to_str(MM_typecode matcode) {
  char buffer[MM_MAX_LINE_LENGTH];
  char const *types[4];
  char *mm_strdup(const char *);

  /* check for MTX type */
  if (mm_is_matrix(matcode))
    types[0] = MM_MTX_STR;

  /* check for CRD or ARR matrix */
  if (mm_is_sparse(matcode))
    types[1] = MM_SPARSE_STR;
  else if (mm_is_dense(matcode))
    types[1] = MM_DENSE_STR;
  else
    return NULL;

  /* check for element data type */
  if (mm_is_real(matcode))
    types[2] = MM_REAL_STR;
  else if (mm_is_complex(matcode))
    types[2] = MM_COMPLEX_STR;
  else if (mm_is_pattern(matcode))
    types[2] = MM_PATTERN_STR;
  else if (mm_is_integer(matcode))
    types[2] = MM_INT_STR;
  else
    return NULL;

  /* check for symmetry type */
  if (mm_is_general(matcode))
    types[3] = MM_GENERAL_STR;
  else if (mm_is_symmetric(matcode))
    types[3] = MM_SYMM_STR;
  else if (mm_is_hermitian(matcode))
    types[3] = MM_HERM_STR;
  else if (mm_is_skew(matcode))
    types[3] = MM_SKEW_STR;
  else
    return NULL;

  sprintf(buffer, "%s %s %s %s", types[0], types[1], types[2], types[3]);
  return mm_strdup(buffer);
}


/*  high level routines */

int compare_entries_csr(const void *a, const void *b) {
  Entry *ea = (Entry *)a;
  Entry *eb = (Entry *)b;
  if (ea->row != eb->row)
    return ea->row - eb->row;
  return ea->col - eb->col;
}


void entries_to_local_csr(Entry *entries, CSR_local *csr, MM_typecode *matcode) {
  qsort(entries, csr->nnz, sizeof(Entry), compare_entries_csr);

  csr->row_ptr = (uint32_t *)malloc((csr->nrows + 1) * sizeof(uint32_t));
  csr->col_idx = (uint32_t *)malloc(csr->nnz * sizeof(uint32_t));
  csr->val = NULL;
  if (!mm_is_pattern(*matcode)) {
    printf("VALL\n");
    csr->val = (double *)malloc(csr->nnz * sizeof(double));
  }

  uint32_t i = 0;
  for (uint32_t v = 0; v < csr->nnz; v++) {
    csr->row_ptr[v] = i;
    while (i < csr->nnz && v == (entries[i].row - 1)) {
      // Matrix Market format is 1-indexed, convert to 0-indexed
      csr->col_idx[i] = entries[i].col - 1;
      i++;
    }
  }
  csr->row_ptr[csr->nrows] = csr->nnz;
}


CSR_local* Distr_MMIO_CSR_local_read(char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Could not open file [%s].\n", filename);
    return NULL;
  }
  return Distr_MMIO_CSR_local_read_f(f);
}

CSR_local* Distr_MMIO_CSR_local_read_f(FILE *f) {
  MM_typecode matcode;
  if (mm_read_banner(f, &matcode) != 0) {
    fprintf(stderr, "Could not process Matrix Market banner.\n");
    return NULL;
  }
  if (mm_is_complex(matcode)) {
    fprintf(stderr, "Cannot parse complex-valued matrices.\n");
    return NULL;
  }
  if (mm_is_array(matcode)) {
    fprintf(stderr, "Cannot parse array matrices.\n");
    return NULL;
  }

  uint32_t nrows, ncols, nnz;
  if (mm_read_mtx_crd_size(f, &nrows, &ncols, &nnz) != 0) {
    fprintf(stderr, "Could not parse matrix size.\n");
    return NULL;
  }

  CSR_local *csr = (CSR_local *)malloc(sizeof(CSR_local));
  csr->nrows = nrows;
  csr->ncols = ncols;

  if (mm_is_general(matcode)) {
    csr->nnz = nnz;
  } else {
    csr->nnz = nnz * 2; // For symmetric matrices
  }

  Entry *entries = (Entry *)malloc(csr->nnz * sizeof(Entry));
  if (mm_read_mtx_crd_data(f, nnz, entries, matcode) != 0) {
    printf("Could not parse matrix data.\n");
    free(entries);
    return NULL;
  }
  fclose(f);

  if (!mm_is_general(matcode))
    // Duplicate the entries for symmetric matrices
    for (uint32_t i = 0; i < nnz; i++) {
      entries[i + nnz].row = entries[i].col;
      entries[i + nnz].col = entries[i].row;
      entries[i + nnz].val = entries[i].val;
    }

  entries_to_local_csr(entries, csr, &matcode);

  free(entries);

  return csr;
}


void Distr_MMIO_CSR_local_destroy(CSR_local **csr) {
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

#ifdef __cplusplus
}
#endif