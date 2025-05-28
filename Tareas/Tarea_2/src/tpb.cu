#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

typedef unsigned char ubyte;
typedef unsigned short ushort;

void runSimpleLifeKernel(ubyte *&d_lifeData, ubyte *&d_lifeDataBuffer,
                         size_t worldWidth, size_t worldHeight,
                         size_t iterationsCount, ushort threadsCount);

void runSimpleLifeKernelIfs(ubyte *&d_lifeData, ubyte *&d_lifeDataBuffer,
                            size_t worldWidth, size_t worldHeight,
                            size_t iterationsCount, ushort threadsCount);

void runSimpleLifeKernel2D(ubyte **&d_lifeData, ubyte **&d_lifeDataBuffer,
                           size_t worldWidth, size_t worldHeight,
                           size_t iterationsCount, ushort threadsCount);

__global__ void fillRandomLifeData(ubyte *lifeData, size_t size,
                                   unsigned int seed);

void runExperiment1D(int iterations, ushort threads, size_t height,
                     size_t width, std::ofstream &outfile, std::string title) {
  size_t totalCells = height * width;
  ubyte *d_lifeData = nullptr, *d_lifeDataBuffer = nullptr;

  cudaMalloc(&d_lifeData, totalCells * sizeof(ubyte));
  cudaMalloc(&d_lifeDataBuffer, totalCells * sizeof(ubyte));

  unsigned int seed = static_cast<unsigned int>(time(nullptr));
  int blocks = std::min((totalCells + threads - 1) / threads, 32768UL);
  std::vector<double> timings;

  for (ushort i = 0; i < 15; ++i) {
    cudaMemcpy(d_lifeDataBuffer, d_lifeData, totalCells * sizeof(ubyte),
               cudaMemcpyDeviceToDevice);
    cudaDeviceSynchronize();

    fillRandomLifeData<<<blocks, threads>>>(d_lifeData, totalCells, seed++);
    cudaDeviceSynchronize();

    double duration;
    if (title == "CUDA") { // xd
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(d_lifeData, d_lifeDataBuffer, width, height,
                          iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();

      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
    } else {
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernelIfs(d_lifeData, d_lifeDataBuffer, width, height,
                             iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();

      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
    }

    timings.push_back(duration);
  }

  std::sort(timings.begin(), timings.end());
  double medianTime = timings[timings.size() / 2];
  double cellsPerSecond = (double)(totalCells * iterations) / medianTime * 1e6;

  outfile << title << ',' << width << ',' << height << ',' << totalCells << ','
          << threads << ',' << iterations << ',' << medianTime << ','
          << cellsPerSecond << '\n';

  cudaFree(d_lifeData);
  cudaFree(d_lifeDataBuffer);
}

void runExperiment2D(int iterations, ushort threads, size_t height,
                     size_t width, std::ofstream &outfile, std::string title) {
  size_t totalCells = height * width;

  ubyte **d_lifeData = nullptr;
  ubyte **d_lifeDataBuffer = nullptr;
  ubyte *d_lifeDataRows = nullptr;
  ubyte *d_lifeDataBufferRows = nullptr;

  cudaMalloc(&d_lifeDataRows, totalCells * sizeof(ubyte));
  cudaMalloc(&d_lifeDataBufferRows, totalCells * sizeof(ubyte));
  cudaMalloc(&d_lifeData, height * sizeof(ubyte *));
  cudaMalloc(&d_lifeDataBuffer, height * sizeof(ubyte *));

  std::vector<ubyte *> h_lifeData(height);
  std::vector<ubyte *> h_lifeDataBuffer(height);
  for (size_t i = 0; i < height; ++i) {
    h_lifeData[i] = d_lifeDataRows + i * width;
    h_lifeDataBuffer[i] = d_lifeDataBufferRows + i * width;
  }

  cudaMemcpy(d_lifeData, h_lifeData.data(), height * sizeof(ubyte *),
             cudaMemcpyHostToDevice);
  cudaMemcpy(d_lifeDataBuffer, h_lifeDataBuffer.data(),
             height * sizeof(ubyte *), cudaMemcpyHostToDevice);

  unsigned int seed = static_cast<unsigned int>(time(nullptr));
  int blocks = std::min((totalCells + threads - 1) / threads, 32768UL);
  std::vector<double> timings;

  for (int i = 0; i < 31; ++i) {
    cudaMemcpy(d_lifeDataBufferRows, d_lifeDataRows, totalCells * sizeof(ubyte),
               cudaMemcpyDeviceToDevice);
    cudaDeviceSynchronize();

    fillRandomLifeData<<<blocks, threads>>>(d_lifeDataRows, totalCells, seed++);
    cudaDeviceSynchronize();

    auto start = std::chrono::high_resolution_clock::now();
    runSimpleLifeKernel2D(d_lifeData, d_lifeDataBuffer, width, height,
                          iterations, threads);
    auto end = std::chrono::high_resolution_clock::now();

    double duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    timings.push_back(duration);
  }

  std::sort(timings.begin(), timings.end());
  double medianTime = timings[timings.size() / 2];
  double cellsPerSecond = (double)(totalCells * iterations) / medianTime * 1e6;

  outfile << title << ',' << width << ',' << height << ',' << totalCells << ','
          << threads << ',' << iterations << ',' << medianTime << ','
          << cellsPerSecond << '\n';

  cudaFree(d_lifeData);
  cudaFree(d_lifeDataBuffer);
  cudaFree(d_lifeDataRows);
  cudaFree(d_lifeDataBufferRows);
}

void experiment(int iterations, ushort threads, size_t height, size_t width,
                std::ofstream &outfile) {
  // Optimal case
  runExperiment1D(iterations, threads, height, width, outfile, "CUDA");

  // Ifs case
  runExperiment1D(iterations, threads, height, width, outfile, "CUDA Ifs");

  // 2D case
  runExperiment2D(iterations, threads, height, width, outfile, "CUDA 2D");
}

int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  const int iterations = 16;
  const ushort threadOptions[5] = {64, 128, 256, 512, 1024};
  std::ofstream outfile("cuda_benchmark.csv");
  outfile << "Mode,Width,Height,Length,Threads,Iterations,Time[μs],"
             "Cells/s\n";

  size_t worldWidth = 1ull << 16;

  for (ushort exp = 4; exp <= 16; ++exp) {
    size_t worldHeight = 1ull << exp;

    for (ushort threads : threadOptions)
      experiment(iterations, threads, worldHeight, worldWidth, outfile);
  }

  outfile.close();
  return 0;
}
