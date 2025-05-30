#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

typedef unsigned char ubyte;
typedef unsigned short ushort;

#define CHECK_CL_ERROR(err, msg)                                               \
  if (err != CL_SUCCESS) {                                                     \
    std::cerr << "OpenCL error (" << err << "): " << msg << '\n';              \
    std::exit(EXIT_FAILURE);                                                   \
  }

void runSimpleLifeKernel(cl_command_queue queue, cl_kernel kernel,
                         cl_mem &d_lifeData, cl_mem &d_lifeDataBuffer,
                         size_t worldWidth, size_t worldHeight,
                         size_t worldSize, size_t iterationsCount,
                         ushort threadsCount) {
  assert((worldWidth * worldHeight) % threadsCount == 0);
  size_t globalSize[2] = {worldWidth, worldHeight};
  size_t localSize[2] = {16, 16};

  clSetKernelArg(kernel, 1, sizeof(cl_ulong), &worldWidth);
  clSetKernelArg(kernel, 2, sizeof(cl_ulong), &worldHeight);
  clSetKernelArg(kernel, 4, sizeof(cl_ulong), &worldSize);
  for (size_t i = 0; i < iterationsCount; ++i) {
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_lifeData);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_lifeDataBuffer);

    cl_int err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalSize, localSize, 0,
                                        nullptr, nullptr);
    CHECK_CL_ERROR(err, "enqueueing kernel 1D");
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  cl_int err = clFinish(queue);
  CHECK_CL_ERROR(err, "finish queue 1D");
}

void runExperiment(int iterations, ushort threads, size_t height,
                     size_t width, std::ofstream &outfile, std::string title,
                     cl_context context, cl_command_queue queue,
                     cl_kernel kernelDefault, cl_kernel kernelIfs,
                     cl_kernel kernel2D, cl_kernel fillRandomLifeDataKernel) {
  size_t totalCells = height * width;
  cl_int err;

  cl_mem d_lifeData = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     totalCells * sizeof(ubyte), nullptr, &err);
  cl_mem d_lifeDataBuffer = clCreateBuffer(
      context, CL_MEM_READ_WRITE, totalCells * sizeof(ubyte), nullptr, &err);

  unsigned int seed = static_cast<unsigned int>(time(nullptr));
  size_t globalSize = ((totalCells + threads - 1) / threads) * threads;
  std::vector<double> timings;

  clSetKernelArg(fillRandomLifeDataKernel, 0, sizeof(cl_mem), &d_lifeData);
  clSetKernelArg(fillRandomLifeDataKernel, 1, sizeof(cl_ulong), &totalCells);
  for (ushort i = 0; i < 15; ++i) {
    clSetKernelArg(fillRandomLifeDataKernel, 2, sizeof(cl_uint), &seed);
    cl_int err = clEnqueueNDRangeKernel(queue, fillRandomLifeDataKernel, 1, nullptr,
                                        &globalSize, nullptr, 0, nullptr, nullptr);
    CHECK_CL_ERROR(err, "enqueueing kernel random");

    clFlush(queue);
    clFinish(queue);
    seed++;

    double duration;
    if (title == "OpenCL") { // xd
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(queue, kernelDefault, d_lifeData, d_lifeDataBuffer,
                          width, height, totalCells, iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() / iterations;
    } else if (title == "OpenCL Ifs") {
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(queue, kernelIfs, d_lifeData, d_lifeDataBuffer, width,
                          height, totalCells, iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() / iterations;
    } else {
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(queue, kernel2D, d_lifeData, d_lifeDataBuffer, width,
                          height, totalCells, iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() / iterations;
    }

    timings.push_back(duration);
  }

  std::sort(timings.begin(), timings.end());
  double medianTime = timings[timings.size() / 2];
  double cellsPerSecond = (double)(totalCells * iterations) / medianTime * 1e6;

  outfile << title << ',' << width << ',' << height << ',' << totalCells << ','
          << threads << ',' << iterations << ',' << medianTime << ','
          << cellsPerSecond << '\n';

  clReleaseMemObject(d_lifeData);
  clReleaseMemObject(d_lifeDataBuffer);
}

void experiment(int iterations, ushort threads, size_t height, size_t width,
                std::ofstream &outfile, cl_context context,
                cl_command_queue queue, cl_kernel kernel1D, cl_kernel kernelIfs,
                cl_kernel kernel2D, cl_kernel fillRandomLifeDataKernel) {
  // Optimal case
  runExperiment(iterations, threads, height, width, outfile, "OpenCL",
                context, queue, kernel1D, kernelIfs, kernel2D,
                fillRandomLifeDataKernel);

  // Ifs case
  runExperiment(iterations, threads, height, width, outfile, "OpenCL Ifs",
                context, queue, kernel1D, kernelIfs, kernel2D,
                fillRandomLifeDataKernel);

  // 2D case
  runExperiment(iterations, threads, height, width, outfile, "OpenCL 2D",
                context, queue, kernel1D, kernelIfs, kernel2D,
                fillRandomLifeDataKernel);
}

int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  cl_int err;
  const int iterations = 16;
  const ushort threadOptions[5] = {64, 128, 256, 512, 1024};
  std::ofstream outfile("opencl_benchmark.csv");
  outfile << "Mode,Width,Height,Length,Threads,Iterations,Time[μs],Cells/s\n";

  // ====== Get device ======
  cl_uint numPlatforms = 0;
  CHECK_CL_ERROR(clGetPlatformIDs(0, nullptr, &numPlatforms),
                 "clGetPlatformIDs");

  std::vector<cl_platform_id> platforms(numPlatforms);
  CHECK_CL_ERROR(clGetPlatformIDs(numPlatforms, platforms.data(), nullptr),
                 "clGetPlatformIDs");

  cl_platform_id selectedPlatform = nullptr;
  cl_device_id selectedDevice = nullptr;

  for (const cl_platform_id platform : platforms) {
    char name[256];
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(name), name, nullptr);

    cl_uint numDevices = 0;
    cl_int err =
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    if (err != CL_SUCCESS || numDevices == 0)
      continue;

    std::vector<cl_device_id> devices(numDevices);
    CHECK_CL_ERROR(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices,
                                  devices.data(), nullptr),
                   "clGetDeviceIDs");

    selectedPlatform = platform;
    selectedDevice = devices[0];
    break;
  }

  // ====== Context and Queue ======
  cl_context context =
      clCreateContext(nullptr, 1, &selectedDevice, nullptr, nullptr, &err);
  CHECK_CL_ERROR(err, "creating context");

  cl_command_queue queue =
      clCreateCommandQueue(context, selectedDevice, 0, &err);
  CHECK_CL_ERROR(err, "creating queue");

  // ====== Build OpenCL Program ======
  std::ifstream kernelFile("kernel.cl");
  std::string src(std::istreambuf_iterator<char>(kernelFile), {});
  const char *srcStr = src.c_str();
  size_t length = src.length();

  cl_program program =
      clCreateProgramWithSource(context, 1, &srcStr, &length, &err);
  CHECK_CL_ERROR(err, "creating program");

  err = clBuildProgram(program, 1, &selectedDevice, nullptr, nullptr, nullptr);
  if (err != CL_SUCCESS) {
    size_t logSize;
    clGetProgramBuildInfo(program, selectedDevice, CL_PROGRAM_BUILD_LOG, 0,
                          nullptr, &logSize);
    std::vector<char> buildLog(logSize);
    clGetProgramBuildInfo(program, selectedDevice, CL_PROGRAM_BUILD_LOG,
                          logSize, buildLog.data(), nullptr);
    std::cerr << "Build error:\n" << buildLog.data() << '\n';
    return 1;
  }

  // ====== Create Kernels ======
  cl_kernel fillRandomLifeDataKernel =
      clCreateKernel(program, "fillRandomLifeData", &err);
  CHECK_CL_ERROR(err, "creating fillRandomLifeData");

  cl_kernel kernel = clCreateKernel(program, "simpleLifeKernel", &err);
  CHECK_CL_ERROR(err, "creating kernel1D");

  cl_kernel kernelIfs = clCreateKernel(program, "simpleLifeKernelIfs", &err);
  CHECK_CL_ERROR(err, "creating kernelIfs");

  cl_kernel kernel2D = clCreateKernel(program, "simpleLifeKernel2D", &err);
  CHECK_CL_ERROR(err, "creating kernel2D");

  // ====== Run Experiments ======
  std::cout << "- Experimentos: \n";
  size_t worldWidth = 1ull << 15;
  for (ushort exp = 4; exp <= 15; ++exp) {
    size_t worldHeight = 1ull << exp;
    std::cout << "2^15x2^" << exp << " (" << worldWidth * worldHeight << ")\n";

    for (ushort threads : threadOptions)
      experiment(iterations, threads, worldHeight, worldWidth, outfile, context,
                 queue, kernel, kernelIfs, kernel2D, fillRandomLifeDataKernel);
  }

  // ====== Cleanup ======
  clReleaseKernel(kernel);
  clReleaseKernel(kernelIfs);
  clReleaseKernel(kernel2D);
  clReleaseKernel(fillRandomLifeDataKernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  outfile.close();
  return 0;
}
