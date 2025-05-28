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

  clSetKernelArg(kernel, 1, sizeof(cl_uint), &worldWidth);
  clSetKernelArg(kernel, 2, sizeof(cl_uint), &worldHeight);
  clSetKernelArg(kernel, 4, sizeof(cl_uint), &worldSize);
  for (size_t i = 0; i < iterationsCount; ++i) {
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_lifeData);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_lifeDataBuffer);

    clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalSize, localSize, 0,
                           nullptr, nullptr);
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  clFinish(queue);
}

void runSimpleLifeKernel2D(cl_command_queue queue, cl_kernel kernel,
                           cl_mem &d_lifeData, cl_mem &d_lifeDataBuffer,
                           size_t worldWidth, size_t worldHeight,
                           size_t worldSize, size_t iterationsCount,
                           ushort threadsCount) {
  assert((worldWidth * worldHeight) % threadsCount == 0);
  size_t globalSize[2] = {worldWidth, worldHeight};
  size_t localSize[2] = {16, 16};

  clSetKernelArg(kernel, 1, sizeof(cl_uint), &worldWidth);
  clSetKernelArg(kernel, 2, sizeof(cl_uint), &worldHeight);
  clSetKernelArg(kernel, 4, sizeof(cl_uint), &worldSize);
  for (size_t i = 0; i < iterationsCount; ++i) {
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_lifeData);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_lifeDataBuffer);

    clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalSize, localSize, 0,
                           nullptr, nullptr);
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  clFinish(queue);
}

void runExperiment1D(int iterations, ushort threads, size_t height,
                     size_t width, std::ofstream &outfile, std::string title,
                     cl_context context, cl_command_queue queue,
                     cl_kernel kernelDefault, cl_kernel kernelIfs,
                     cl_kernel fillRandomLifeDataKernel) {
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
  clSetKernelArg(fillRandomLifeDataKernel, 1, sizeof(cl_uint), &totalCells);
  for (ushort i = 0; i < 15; ++i) {
    clSetKernelArg(fillRandomLifeDataKernel, 2, sizeof(cl_uint), &seed);
    clEnqueueNDRangeKernel(queue, fillRandomLifeDataKernel, 1, nullptr,
                           &globalSize, nullptr, 0, nullptr, nullptr);
    clFinish(queue);
    seed++;

    double duration;
    if (title == "OpenCL") {
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(queue, kernelDefault, d_lifeData, d_lifeDataBuffer,
                          width, height, totalCells, iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
    } else {
      auto start = std::chrono::high_resolution_clock::now();
      runSimpleLifeKernel(queue, kernelIfs, d_lifeData, d_lifeDataBuffer, width,
                          height, totalCells, iterations, threads);
      auto end = std::chrono::high_resolution_clock::now();
      duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
    }

    clFinish(queue);
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

void runExperiment2D(int iterations, ushort threads, size_t height,
                     size_t width, std::ofstream &outfile, std::string title,
                     cl_context context, cl_command_queue queue,
                     cl_kernel kernel2D, cl_kernel fillRandomLifeDataKernel) {
  size_t totalCells = height * width;
  cl_int err;

  cl_mem d_lifeDataRows = clCreateBuffer(
      context, CL_MEM_READ_WRITE, totalCells * sizeof(ubyte), nullptr, &err);
  cl_mem d_lifeDataBufferRows = clCreateBuffer(
      context, CL_MEM_READ_WRITE, totalCells * sizeof(ubyte), nullptr, &err);

  std::vector<cl_uchar *> h_lifeData(height);
  std::vector<cl_uchar *> h_lifeDataBuffer(height);
  for (size_t i = 0; i < height; ++i) {
    h_lifeData[i] = nullptr;
    h_lifeDataBuffer[i] = nullptr;
  }

  for (size_t i = 0; i < height; ++i) {
    h_lifeData[i] = reinterpret_cast<cl_uchar *>(
        reinterpret_cast<uintptr_t>(d_lifeDataRows) +
        i * width * sizeof(ubyte));
    h_lifeDataBuffer[i] = reinterpret_cast<cl_uchar *>(
        reinterpret_cast<uintptr_t>(d_lifeDataBufferRows) +
        i * width * sizeof(ubyte));
  }

  cl_mem d_lifeData = clCreateBuffer(
      context, CL_MEM_READ_ONLY, height * sizeof(cl_uchar *), nullptr, &err);
  cl_mem d_lifeDataBuffer = clCreateBuffer(
      context, CL_MEM_READ_ONLY, height * sizeof(cl_uchar *), nullptr, &err);
  clEnqueueWriteBuffer(queue, d_lifeData, CL_TRUE, 0,
                       height * sizeof(cl_uchar *), h_lifeData.data(), 0,
                       nullptr, nullptr);
  clEnqueueWriteBuffer(queue, d_lifeDataBuffer, CL_TRUE, 0,
                       height * sizeof(cl_uchar *), h_lifeDataBuffer.data(), 0,
                       nullptr, nullptr);

  std::vector<double> timings;
  unsigned int seed = static_cast<unsigned int>(time(nullptr));
  size_t globalSize1D = ((totalCells + threads - 1) / threads) * threads;
  size_t globalSize2D[2] = {width, height};

  clSetKernelArg(fillRandomLifeDataKernel, 1, sizeof(cl_ulong), &totalCells);
  for (int i = 0; i < 15; ++i) {
    clSetKernelArg(fillRandomLifeDataKernel, 0, sizeof(cl_mem),
                   &d_lifeDataRows);
    clSetKernelArg(fillRandomLifeDataKernel, 2, sizeof(cl_uint), &seed);
    clEnqueueNDRangeKernel(queue, fillRandomLifeDataKernel, 1, nullptr,
                           &globalSize1D, nullptr, 0, nullptr, nullptr);
    clFinish(queue);
    seed++;

    auto start = std::chrono::high_resolution_clock::now();
    runSimpleLifeKernel2D(queue, kernel2D, d_lifeData, d_lifeDataBuffer, width,
                          height, totalCells, iterations, threads);
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

  clReleaseMemObject(d_lifeData);
  clReleaseMemObject(d_lifeDataBuffer);
  clReleaseMemObject(d_lifeDataRows);
  clReleaseMemObject(d_lifeDataBufferRows);
}

void experiment(int iterations, ushort threads, size_t height, size_t width,
                std::ofstream &outfile, cl_context context,
                cl_command_queue queue, cl_kernel kernel1D, cl_kernel kernelIfs,
                cl_kernel kernel2D, cl_kernel fillRandomLifeDataKernel) {
  // Optimal case
  runExperiment1D(iterations, threads, height, width, outfile, "OpenCL",
                  context, queue, kernel1D, kernelIfs,
                  fillRandomLifeDataKernel);

  // Ifs case
  runExperiment1D(iterations, threads, height, width, outfile, "OpenCL Ifs",
                  context, queue, kernel1D, kernelIfs,
                  fillRandomLifeDataKernel);

  // 2D case
  runExperiment2D(iterations, threads, height, width, outfile, "OpenCL 2D",
                  context, queue, kernel2D, fillRandomLifeDataKernel);
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
  size_t worldWidth = 1ull << 16;
  for (ushort exp = 4; exp <= 6; ++exp) {
    size_t worldHeight = 1ull << exp;
    std::cout << "2^16x2^" << exp << " (" << worldWidth * worldHeight << ")\n";

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
