typedef unsigned char ubyte;

kernel void fillRandomLifeData(global ubyte *lifeData, ulong size, uint seed) {
  size_t gid = get_global_id(0);
  size_t totalThreads = get_global_size(0);

  for (ulong i = gid; i < size; i += totalThreads) {
    uint x = i ^ seed;
    x = (x * 1664525u + 1013904223u);
    lifeData[i] = (x >> 24) & 1;
  }
}

kernel void simpleLifeKernel(global volatile const ubyte *lifeData,
                             ulong worldWidth, ulong worldHeight,
                             global ubyte *resultLifeData, ulong worldSize) {
  size_t index = get_global_id(0);

  if (index >= worldSize)
    return;

  uint x = index % worldWidth;
  uint yAbs = index / worldWidth;

  uint xLeft = (x + worldWidth - 1) % worldWidth;
  uint xRight = (x + 1) % worldWidth;

  uint yAbsUp = (yAbs + worldSize - worldWidth) % worldSize;
  uint yAbsDown = (yAbs + worldWidth) % worldSize;

  uint aliveCells = lifeData[xLeft + yAbsUp] + lifeData[x + yAbsUp] +
                    lifeData[xRight + yAbsUp] + lifeData[xLeft + yAbs] +
                    lifeData[xRight + yAbs] + lifeData[xLeft + yAbsDown] +
                    lifeData[x + yAbsDown] + lifeData[xRight + yAbsDown];

  resultLifeData[x + yAbs] =
      (aliveCells == 3 || (aliveCells == 2 && lifeData[x + yAbs])) ? 1 : 0;
}

kernel void simpleLifeKernelIfs(global volatile const ubyte *lifeData,
                                ulong worldWidth, ulong worldHeight,
                                global ubyte *resultLifeData, ulong worldSize) {
  size_t index = get_global_id(0);

  if (index >= worldSize)
    return;

  uint x = index % worldWidth;
  uint yAbs = index / worldWidth;

  uint xLeft = (x + worldWidth - 1) % worldWidth;
  uint xRight = (x + 1) % worldWidth;

  uint yAbsUp = (yAbs + worldSize - worldWidth) % worldSize;
  uint yAbsDown = (yAbs + worldWidth) % worldSize;

  uint aliveCells = 0;
  if (lifeData[xLeft + yAbsUp])
    aliveCells++;
  if (lifeData[x + yAbsUp])
    aliveCells++;
  if (lifeData[xRight + yAbsUp])
    aliveCells++;
  if (lifeData[xLeft + yAbs])
    aliveCells++;
  if (lifeData[xRight + yAbs])
    aliveCells++;
  if (lifeData[xLeft + yAbsDown])
    aliveCells++;
  if (lifeData[x + yAbsDown])
    aliveCells++;
  if (lifeData[xRight + yAbsDown])
    aliveCells++;

  if (aliveCells == 3 || (aliveCells == 2 && lifeData[x + yAbs]))
    resultLifeData[x + yAbs] = 1;
  else
    resultLifeData[x + yAbs] = 0;
}

kernel void simpleLifeKernel2D(global volatile const ubyte *lifeData,
                               ulong worldWidth, ulong worldHeight,
                               global ubyte *resultLifeData, ulong worldSize) {
  size_t x = get_global_id(0);
  size_t y = get_global_id(1);

  if (x >= worldWidth || y >= worldHeight)
    return;

  uint xLeft = (x + worldWidth - 1) % worldWidth;
  uint xRight = (x + 1) % worldWidth;
  uint yUp = (y + worldHeight - 1) % worldHeight;
  uint yDown = (y + 1) % worldHeight;

// 2D indexing
#define IDX(xx, yy) ((size_t)(yy) * (size_t)(worldWidth) + (size_t)(xx))

  uint aliveCells = lifeData[IDX(xLeft, yUp)] + lifeData[IDX(x, yUp)] +
                    lifeData[IDX(xRight, yUp)] + lifeData[IDX(xLeft, y)] +
                    lifeData[IDX(xRight, y)] + lifeData[IDX(xLeft, yDown)] +
                    lifeData[IDX(x, yDown)] + lifeData[IDX(xRight, yDown)];

  resultLifeData[IDX(x, y)] =
      (aliveCells == 3 || (aliveCells == 2 && lifeData[IDX(x, y)])) ? 1 : 0;
}
