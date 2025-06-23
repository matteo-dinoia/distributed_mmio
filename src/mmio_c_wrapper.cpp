#include "mmio.h"           // Original C++ library header
#include "mmio_c_wrapper.h" // Our new C API header

// The entire file provides C-linkage, so we wrap it in extern "C".
extern "C" {

/*
 * ============================================================================
 * Implementations for uint32_t / float
 * ============================================================================
 */
mmio_csr_u32_f32_t* mmio_read_csr_u32_f32(const char* filename, bool alloc_val) {
    // Call the original C++ templated function
    CSR_local<uint32_t, float>* cpp_csr = Distr_MMIO_CSR_local_read<uint32_t, float>(filename, alloc_val, NULL);
    // Cast the result to the C-style struct pointer. This is safe because layouts match.
    return reinterpret_cast<mmio_csr_u32_f32_t*>(cpp_csr);
}

mmio_coo_u32_f32_t* mmio_read_coo_u32_f32(const char* filename, bool alloc_val) {
    COO_local<uint32_t, float>* cpp_coo = Distr_MMIO_COO_local_read<uint32_t, float>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_coo_u32_f32_t*>(cpp_coo);
}

void mmio_destroy_csr_u32_f32(mmio_csr_u32_f32_t* matrix) {
    // Cast the C-style pointer back to the C++ type
    CSR_local<uint32_t, float>* cpp_csr = reinterpret_cast<CSR_local<uint32_t, float>*>(matrix);
    // Call the C++ destroy function, which expects a pointer-to-pointer
    Distr_MMIO_CSR_local_destroy(&cpp_csr);
}

void mmio_destroy_coo_u32_f32(mmio_coo_u32_f32_t* matrix) {
    COO_local<uint32_t, float>* cpp_coo = reinterpret_cast<COO_local<uint32_t, float>*>(matrix);
    Distr_MMIO_COO_local_destroy(&cpp_coo);
}


/*
 * ============================================================================
 * Implementations for uint32_t / double
 * ============================================================================
 */
mmio_csr_u32_f64_t* mmio_read_csr_u32_f64(const char* filename, bool alloc_val) {
    CSR_local<uint32_t, double>* cpp_csr = Distr_MMIO_CSR_local_read<uint32_t, double>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_csr_u32_f64_t*>(cpp_csr);
}

mmio_coo_u32_f64_t* mmio_read_coo_u32_f64(const char* filename, bool alloc_val) {
    COO_local<uint32_t, double>* cpp_coo = Distr_MMIO_COO_local_read<uint32_t, double>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_coo_u32_f64_t*>(cpp_coo);
}

void mmio_destroy_csr_u32_f64(mmio_csr_u32_f64_t* matrix) {
    CSR_local<uint32_t, double>* cpp_csr = reinterpret_cast<CSR_local<uint32_t, double>*>(matrix);
    Distr_MMIO_CSR_local_destroy(&cpp_csr);
}

void mmio_destroy_coo_u32_f64(mmio_coo_u32_f64_t* matrix) {
    COO_local<uint32_t, double>* cpp_coo = reinterpret_cast<COO_local<uint32_t, double>*>(matrix);
    Distr_MMIO_COO_local_destroy(&cpp_coo);
}


/*
 * ============================================================================
 * Implementations for uint64_t / float
 * ============================================================================
 */
mmio_csr_u64_f32_t* mmio_read_csr_u64_f32(const char* filename, bool alloc_val) {
    CSR_local<uint64_t, float>* cpp_csr = Distr_MMIO_CSR_local_read<uint64_t, float>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_csr_u64_f32_t*>(cpp_csr);
}

mmio_coo_u64_f32_t* mmio_read_coo_u64_f32(const char* filename, bool alloc_val) {
    COO_local<uint64_t, float>* cpp_coo = Distr_MMIO_COO_local_read<uint64_t, float>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_coo_u64_f32_t*>(cpp_coo);
}

void mmio_destroy_csr_u64_f32(mmio_csr_u64_f32_t* matrix) {
    CSR_local<uint64_t, float>* cpp_csr = reinterpret_cast<CSR_local<uint64_t, float>*>(matrix);
    Distr_MMIO_CSR_local_destroy(&cpp_csr);
}

void mmio_destroy_coo_u64_f32(mmio_coo_u64_f32_t* matrix) {
    COO_local<uint64_t, float>* cpp_coo = reinterpret_cast<COO_local<uint64_t, float>*>(matrix);
    Distr_MMIO_COO_local_destroy(&cpp_coo);
}


/*
 * ============================================================================
 * Implementations for uint64_t / double
 * ============================================================================
 */
mmio_csr_u64_f64_t* mmio_read_csr_u64_f64(const char* filename, bool alloc_val) {
    CSR_local<uint64_t, double>* cpp_csr = Distr_MMIO_CSR_local_read<uint64_t, double>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_csr_u64_f64_t*>(cpp_csr);
}

mmio_coo_u64_f64_t* mmio_read_coo_u64_f64(const char* filename, bool alloc_val) {
    COO_local<uint64_t, double>* cpp_coo = Distr_MMIO_COO_local_read<uint64_t, double>(filename, alloc_val, NULL);
    return reinterpret_cast<mmio_coo_u64_f64_t*>(cpp_coo);
}

void mmio_destroy_csr_u64_f64(mmio_csr_u64_f64_t* matrix) {
    CSR_local<uint64_t, double>* cpp_csr = reinterpret_cast<CSR_local<uint64_t, double>*>(matrix);
    Distr_MMIO_CSR_local_destroy(&cpp_csr);
}

void mmio_destroy_coo_u64_f64(mmio_coo_u64_f64_t* matrix) {
    COO_local<uint64_t, double>* cpp_coo = reinterpret_cast<COO_local<uint64_t, double>*>(matrix);
    Distr_MMIO_COO_local_destroy(&cpp_coo);
}

} // extern "C"