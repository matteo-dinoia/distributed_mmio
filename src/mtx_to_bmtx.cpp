#include <stdio.h>
#include <string>
#include <algorithm>

#include "../include/mmio.h"
#include "../include/mmio_utils.h" // FIXME

template<typename IT, typename VT>
void write_binary_matrix_market(const char *filename, const char *mm_header, const char *mm_header_body, COO_local<IT, VT> *coo, bool double_val) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    perror("Error opening output file");
    exit(EXIT_FAILURE);
  }
  int index_bytes = required_bytes_index(std::max(coo->nrows, coo->ncols) - 1);

  // Write the modified MM header
  fprintf(f, "%s %d %d\n%s", mm_header, index_bytes, double_val ? 64 : 32, mm_header_body);
  // Write size line
  fprintf(f, "%ld %ld %ld\n", coo->nrows, coo->ncols, coo->nnz);

  // Write binary data
  for (int64_t i = 0; i < coo->nnz; ++i) {
    // Write row
    switch (index_bytes) {
      case 1: { uint8_t  v = (uint8_t)coo->row[i];  fwrite(&v, sizeof(v), 1, f); break; }
      case 2: { uint16_t v = (uint16_t)coo->row[i]; fwrite(&v, sizeof(v), 1, f); break; }
      case 4: { uint32_t v = (uint32_t)coo->row[i]; fwrite(&v, sizeof(v), 1, f); break; }
      case 8: { uint64_t v = (uint64_t)coo->row[i]; fwrite(&v, sizeof(v), 1, f); break; }
    }

    // Write column
    switch (index_bytes) {
      case 1: { uint8_t  v = (uint8_t)coo->col[i];  fwrite(&v, sizeof(v), 1, f); break; }
      case 2: { uint16_t v = (uint16_t)coo->col[i]; fwrite(&v, sizeof(v), 1, f); break; }
      case 4: { uint32_t v = (uint32_t)coo->col[i]; fwrite(&v, sizeof(v), 1, f); break; }
      case 8: { uint64_t v = (uint64_t)coo->col[i]; fwrite(&v, sizeof(v), 1, f); break; }
    }

    // Write value if exists
    if (coo->val != NULL) {
      if (double_val) { double v = (double)coo->val[i]; fwrite(&v, sizeof(double), 1, f); }
      else            { float  v =  (float)coo->val[i]; fwrite(&v, sizeof(double), 1, f); }
    }
  }

  fclose(f);
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename> [-r|--reverse] [-d|--double-val]\n", argv[0]);
    return EXIT_FAILURE;
  }

  std::string filename = argv[1];
  bool reverse = false;
  bool double_val = false;

  uint32_t arg_i = 2;
  while (arg_i < argc) {    
    std::string flag = argv[arg_i];
    if (flag == "-r" || flag == "--reverse") {
      reverse = true;
    } else if (flag == "-d" || flag == "--double-val") {
      double_val = true;
    } else {
      printf("Unknown option: %s\n", argv[arg_i]);
    }
  }

  // CSR_local<uint64_t, double> *csr = Distr_MMIO_CSR_local_read<uint64_t, double>(filename.c_str());
  // print_csr(csr);

  COO_local<uint64_t, double> *coo = Distr_MMIO_COO_local_read<uint64_t, double>(filename.c_str());
  if (coo == NULL) {
    fprintf(stderr, "Something went wrong\n");
    exit(EXIT_FAILURE);
  }
  print_coo(coo);

  // Replace filename extension with ".bmtx"
  std::string out_filename = filename;
  size_t last_dot = out_filename.find_last_of('.');
  if (last_dot != std::string::npos) {
    out_filename = out_filename.substr(0, last_dot);
  }
  out_filename += ".bmtx";

  // Read the header from the input file
  FILE *f = fopen(filename.c_str(), "r");
  std::string mm_header;
  std::string mm_header_body;
  char line[1024];
  // Read and store all header lines (starting with '%')
  fgets(line, sizeof(line), f);
  mm_header = line;
  if (!mm_header.empty() && mm_header.back() == '\n') {
    mm_header.pop_back();
  }
  while (fgets(line, sizeof(line), f)) {
    if (line[0] != '%') break;
    mm_header_body += line;
  }
  fclose(f);

  write_binary_matrix_market(out_filename.c_str(), mm_header.c_str(), mm_header_body.c_str(), coo, double_val);

  Distr_MMIO_COO_local_destroy(&coo);

  return 0;
}
