#include <cassert>

typedef unsigned char ubyte;

__global__ void fillRandomLifeData(ubyte *lifeData, size_t size,
                                   unsigned int seed) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  for (size_t i = idx; i < size; i += blockDim.x * gridDim.x) {
    unsigned int x = i ^ seed;
    x = (x * 1664525u + 1013904223u);
    lifeData[i] = (x >> 24) & 1;
  }
}

__global__ void simpleLifeKernel(volatile const ubyte *lifeData,
                                 uint worldWidth, uint worldHeight,
                                 ubyte *resultLifeData) {
  uint worldSize = worldWidth * worldHeight;

  for (uint cellId = blockIdx.x * blockDim.x + threadIdx.x; cellId < worldSize;
       cellId += blockDim.x * gridDim.x) {

    uint x = cellId % worldWidth;
    uint yAbs = cellId - x;

    uint xLeft = (x + worldWidth - 1) % worldWidth;
    uint xRight = (x + 1) % worldWidth;

    uint yAbsUp = (yAbs + worldSize - worldWidth) % worldSize;
    uint yAbsDown = (yAbs + worldWidth) % worldSize;

    uint aliveCells = lifeData[xLeft + yAbsUp] + lifeData[x + yAbsUp] +
                      lifeData[xRight + yAbsUp] + lifeData[xLeft + yAbs] +
                      lifeData[xRight + yAbs] + lifeData[xLeft + yAbsDown] +
                      lifeData[x + yAbsDown] + lifeData[xRight + yAbsDown];

    resultLifeData[x + yAbs] =
        aliveCells == 3 || (aliveCells == 2 && lifeData[x + yAbs]) ? 1 : 0;
  }
}

__global__ void simpleLifeKernelIfs(volatile const ubyte *lifeData,
                                    uint worldWidth, uint worldHeight,
                                    ubyte *resultLifeData) {
  uint worldSize = worldWidth * worldHeight;

  for (uint cellId = blockIdx.x * blockDim.x + threadIdx.x; cellId < worldSize;
       cellId += blockDim.x * gridDim.x) {

    uint x = cellId % worldWidth;
    uint yAbs = cellId - x;

    uint xLeft = (x + worldWidth - 1) % worldWidth;
    uint xRight = (x + 1) % worldWidth;

    uint yAbsUp = (yAbs + worldSize - worldWidth) % worldSize;
    uint yAbsDown = (yAbs + worldWidth) % worldSize;

    uint aliveCells = 0;
    if (lifeData[xLeft + yAbsUp])
      aliveCells += 1;
    if (lifeData[x + yAbsUp])
      aliveCells += 1;
    if (lifeData[xRight + yAbsUp])
      aliveCells += 1;
    if (lifeData[xLeft + yAbs])
      aliveCells += 1;
    if (lifeData[xRight + yAbs])
      aliveCells += 1;
    if (lifeData[xLeft + yAbsDown])
      aliveCells += 1;
    if (lifeData[x + yAbsDown])
      aliveCells += 1;
    if (lifeData[xRight + yAbsDown])
      aliveCells += 1;

    if (aliveCells == 3 || (aliveCells == 2 && lifeData[x + yAbs]))
      resultLifeData[x + yAbs] = 1;
    else
      resultLifeData[x + yAbs] = 0;
  }
}

__global__ void simpleLifeKernel2D(volatile ubyte *const *lifeData,
                                   uint worldWidth, uint worldHeight,
                                   ubyte **resultLifeData) {
  uint x = blockIdx.x * blockDim.x + threadIdx.x;
  uint y = blockIdx.y * blockDim.y + threadIdx.y;

  if (x >= worldWidth || y >= worldHeight)
    return;

  uint xLeft = (x + worldWidth - 1) % worldWidth;
  uint xRight = (x + 1) % worldWidth;
  uint yUp = (y + worldHeight - 1) % worldHeight;
  uint yDown = (y + 1) % worldHeight;

  uint aliveCells = lifeData[yUp][xLeft] + lifeData[yUp][x] +
                    lifeData[yUp][xRight] + lifeData[y][xLeft] +
                    lifeData[y][xRight] + lifeData[yDown][xLeft] +
                    lifeData[yDown][x] + lifeData[yDown][xRight];

  resultLifeData[y][x] =
      (aliveCells == 3 || (aliveCells == 2 && lifeData[y][x])) ? 1 : 0;
}

void runSimpleLifeKernel(ubyte *&d_lifeData, ubyte *&d_lifeDataBuffer,
                         size_t worldWidth, size_t worldHeight,
                         size_t iterationsCount, ushort threadsCount) {
  size_t reqBlocksCount = (worldWidth * worldHeight) / threadsCount;
  ushort blocksCount = (ushort)std::min((size_t)32768, reqBlocksCount);

  for (size_t i = 0; i < iterationsCount; ++i) {
    simpleLifeKernel<<<blocksCount, threadsCount>>>(
        d_lifeData, worldWidth, worldHeight, d_lifeDataBuffer);
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  cudaDeviceSynchronize();
}

void runSimpleLifeKernelIfs(ubyte *&d_lifeData, ubyte *&d_lifeDataBuffer,
                            size_t worldWidth, size_t worldHeight,
                            size_t iterationsCount, ushort threadsCount) {
  size_t reqBlocksCount = (worldWidth * worldHeight) / threadsCount;
  ushort blocksCount = (ushort)std::min((size_t)32768, reqBlocksCount);

  for (size_t i = 0; i < iterationsCount; ++i) {
    simpleLifeKernelIfs<<<blocksCount, threadsCount>>>(
        d_lifeData, worldWidth, worldHeight, d_lifeDataBuffer);
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  cudaDeviceSynchronize();
}

void runSimpleLifeKernel2D(ubyte **&d_lifeData, ubyte **&d_lifeDataBuffer,
                           size_t worldWidth, size_t worldHeight,
                           size_t iterationsCount, ushort threadsCount) {
  dim3 threadsPerBlock(16, 16);
  dim3 numBlocks((worldWidth + threadsPerBlock.x - 1) / threadsPerBlock.x,
                 (worldHeight + threadsPerBlock.y - 1) / threadsPerBlock.y);

  for (size_t i = 0; i < iterationsCount; ++i) {
    simpleLifeKernel2D<<<numBlocks, threadsPerBlock>>>(
        d_lifeData, worldWidth, worldHeight, d_lifeDataBuffer);
    std::swap(d_lifeData, d_lifeDataBuffer);
  }
  cudaDeviceSynchronize();
}
