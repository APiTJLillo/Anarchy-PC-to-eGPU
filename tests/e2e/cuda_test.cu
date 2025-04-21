#include <stdio.h>
#include <cuda_runtime.h>

#define CHECK_CUDA(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            fprintf(stderr, "CUDA error in %s:%d: %s\n", __FILE__, __LINE__, \
                    cudaGetErrorString(err)); \
            return 1; \
        } \
    } while (0)

// Test parameters
#define DATA_SIZE (1024 * 1024 * 256)  // 256MB of data
#define BLOCK_SIZE 256
#define NUM_ITERATIONS 100

__global__ void vectorAdd(const float *A, const float *B, float *C, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        C[idx] = A[idx] + B[idx];
    }
}

int main() {
    float *h_A, *h_B, *h_C;    // Host arrays
    float *d_A, *d_B, *d_C;    // Device arrays
    int size = DATA_SIZE;
    size_t bytes = size * sizeof(float);
    cudaEvent_t start, stop;
    float milliseconds;

    printf("Anarchy eGPU CUDA Test\n");
    printf("=====================\n");

    // Allocate host memory
    h_A = (float*)malloc(bytes);
    h_B = (float*)malloc(bytes);
    h_C = (float*)malloc(bytes);

    // Initialize host arrays
    for (int i = 0; i < size; i++) {
        h_A[i] = rand() / (float)RAND_MAX;
        h_B[i] = rand() / (float)RAND_MAX;
    }

    // Create CUDA events for timing
    CHECK_CUDA(cudaEventCreate(&start));
    CHECK_CUDA(cudaEventCreate(&stop));

    // Allocate device memory
    CHECK_CUDA(cudaMalloc(&d_A, bytes));
    CHECK_CUDA(cudaMalloc(&d_B, bytes));
    CHECK_CUDA(cudaMalloc(&d_C, bytes));

    printf("\nMemory Transfer Test\n");
    printf("-------------------\n");

    // Test memory transfer (Host to Device)
    CHECK_CUDA(cudaEventRecord(start));
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        CHECK_CUDA(cudaMemcpy(d_A, h_A, bytes, cudaMemcpyHostToDevice));
        CHECK_CUDA(cudaMemcpy(d_B, h_B, bytes, cudaMemcpyHostToDevice));
    }
    CHECK_CUDA(cudaEventRecord(stop));
    CHECK_CUDA(cudaEventSynchronize(stop));
    CHECK_CUDA(cudaEventElapsedTime(&milliseconds, start, stop));
    
    float h2d_bandwidth = (2.0f * bytes * NUM_ITERATIONS) / (milliseconds * 1000000.0f);
    printf("Host to Device Bandwidth: %.2f GB/s\n", h2d_bandwidth);

    // Launch kernel
    int threadsPerBlock = BLOCK_SIZE;
    int blocksPerGrid = (size + threadsPerBlock - 1) / threadsPerBlock;

    printf("\nComputation Test\n");
    printf("----------------\n");

    CHECK_CUDA(cudaEventRecord(start));
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        vectorAdd<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, size);
    }
    CHECK_CUDA(cudaEventRecord(stop));
    CHECK_CUDA(cudaEventSynchronize(stop));
    CHECK_CUDA(cudaEventElapsedTime(&milliseconds, start, stop));

    float compute_throughput = (2.0f * size * NUM_ITERATIONS) / (milliseconds * 1000000.0f);
    printf("Computation Throughput: %.2f GFLOPS\n", compute_throughput);

    // Test memory transfer (Device to Host)
    CHECK_CUDA(cudaEventRecord(start));
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        CHECK_CUDA(cudaMemcpy(h_C, d_C, bytes, cudaMemcpyDeviceToHost));
    }
    CHECK_CUDA(cudaEventRecord(stop));
    CHECK_CUDA(cudaEventSynchronize(stop));
    CHECK_CUDA(cudaEventElapsedTime(&milliseconds, start, stop));

    float d2h_bandwidth = (bytes * NUM_ITERATIONS) / (milliseconds * 1000000.0f);
    printf("Device to Host Bandwidth: %.2f GB/s\n", d2h_bandwidth);

    // Verify results
    printf("\nVerification Test\n");
    printf("----------------\n");
    bool error = false;
    for (int i = 0; i < size; i++) {
        if (fabs(h_C[i] - (h_A[i] + h_B[i])) > 1e-5) {
            fprintf(stderr, "Verification failed at index %d\n", i);
            error = true;
            break;
        }
    }
    if (!error) {
        printf("All computations verified successfully!\n");
    }

    // Cleanup
    free(h_A);
    free(h_B);
    free(h_C);
    CHECK_CUDA(cudaFree(d_A));
    CHECK_CUDA(cudaFree(d_B));
    CHECK_CUDA(cudaFree(d_C));
    CHECK_CUDA(cudaEventDestroy(start));
    CHECK_CUDA(cudaEventDestroy(stop));

    return error ? 1 : 0;
} 