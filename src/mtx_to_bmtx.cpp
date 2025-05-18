#include <stdio.h>
#include <string>
#include <algorithm>

#include "../include/mmio.h"
#include "../include/mmio_utils.h" // FIXME

template<typename IT, typename VT>
void write_binary_matrix_market(const char *filename, std::string mm_header, const char *mm_header_body, COO_local<IT, VT> *coo, bool double_val) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    perror("Error opening output file");
    exit(EXIT_FAILURE);
  }

  bool is_symmetric = (mm_header.find("symmetric") != std::string::npos);
  bool is_pattern = (mm_header.find("pattern") != std::string::npos);
  int index_bytes = required_bytes_index(std::max(coo->nrows, coo->ncols));

  IT nentries = coo->nnz;
  if (is_symmetric) { // TODO optimize
    nentries = 0;
    for (int64_t i = 0; i < coo->nnz; ++i) {
      if (coo->row[i] >= coo->col[i]) {
        ++nentries;
      }
    }
  }

  // Write the modified MM header
  fprintf(f, "%s %d %d\n%s", mm_header.c_str(), index_bytes, double_val ? 8 : 4, mm_header_body);
  // Write size line
  fprintf(f, "%ld %ld %ld\n", coo->nrows, coo->ncols, nentries);

  // Write binary data
  for (int64_t i = 0; i < coo->nnz; ++i) {
    if (is_symmetric && coo->row[i] < coo->col[i]) continue; // For patter matrices

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

    // Write value
    if (!is_pattern && coo->val != NULL) {
      if (double_val) { double v = (double)coo->val[i]; fwrite(&v, sizeof(double), 1, f); }
      else            { float  v =  (float)coo->val[i]; fwrite(&v, sizeof(float),  1, f); }
    }
  }

  fclose(f);
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    // printf("Usage: %s <filename> [-r|--reverse] [-d|--double-val]\n", argv[0]);
    printf("Usage: %s <filename> [-d|--double-val]\n", argv[0]);
    return EXIT_FAILURE;
  }

  std::string filename = argv[1];
  // bool reverse = false;
  bool double_val = false;

  uint32_t arg_i = 2;
  while (arg_i < argc) {    
    std::string flag = argv[arg_i];
    // if (flag == "-r" || flag == "--reverse") {
    //   reverse = true;
    // } else
    if (flag == "-d" || flag == "--double-val") {
      double_val = true;
    } else {
      printf("Unknown option: %s\n", argv[arg_i]);
    }
  }

  COO_local<uint64_t, double> *coo = Distr_MMIO_COO_local_read<uint64_t, double>(filename.c_str());
  if (coo == NULL) {
    fprintf(stderr, "Something went wrong\n");
    exit(EXIT_FAILURE);
  }
  // print_coo(coo);

  if (is_file_extension_bmtx(filename)) {
    // TODO implement
    printf("Converting BMTX file to MTX... (WIP, not implemented)\n");
    exit(0);
  } else {

    printf("Converting MTX file to BMTX...\n");

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

    write_binary_matrix_market(out_filename.c_str(), mm_header, mm_header_body.c_str(), coo, double_val);

    // Compare file sizes of input (.mtx) and output (.bmtx)
    FILE *f_mtx = fopen(filename.c_str(), "rb");
    FILE *f_bmtx = fopen(out_filename.c_str(), "rb");
    if (!f_mtx || !f_bmtx) {
      fprintf(stderr, "Error opening files for size comparison\n");
      if (f_mtx) fclose(f_mtx);
      if (f_bmtx) fclose(f_bmtx);
      exit(EXIT_FAILURE);
    }
    fseek(f_mtx, 0, SEEK_END);
    fseek(f_bmtx, 0, SEEK_END);
    long size_mtx = ftell(f_mtx);
    long size_bmtx = ftell(f_bmtx);
    fclose(f_mtx);
    fclose(f_bmtx);

    printf("Size of original file (%s):  %.3f MB\n", filename.c_str(), size_mtx/1000000.0f);
    printf("Size of new      file (%s): %.3f MB\n", out_filename.c_str(), size_bmtx/1000000.0f);
    printf("Ratio: %.2f%%\n", (float)size_bmtx/size_mtx*100.0f);
  }

  Distr_MMIO_COO_local_destroy(&coo);

  return 0;
}
