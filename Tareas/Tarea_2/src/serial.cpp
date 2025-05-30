#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

typedef unsigned char ubyte;

ubyte *m_data;
ubyte *m_resultData;

ubyte **m_data2D;
ubyte **m_resultData2D;

size_t m_worldWidth;
size_t m_worldHeight;
size_t m_dataLength;

// void clearTerminal() { std::cout << "\033[2J\033[1;1H"; }
//
// void showGame() {
//   clearTerminal();
//   std::cout << "   ";
//   for (int x = 0; x < m_worldWidth; ++x)
//     std::cout << (x % 10);
//   std::cout << '\n';
//
//   for (size_t y = 0; y < m_worldHeight; ++y) {
//     std::cout << (y < 10 ? " " : "") << y << " ";
//
//     for (size_t x = 0; x < m_worldWidth; ++x) {
//       int index = y * m_worldWidth + x;
//       if (m_data[index] == 1)
//         std::cout << "\033[42m \033[0m";
//       else
//         std::cout << "\033[40m \033[0m";
//     }
//     std::cout << '\n';
//   }
// }

void randomizeWorld() {
  for (int i = 0; i < m_dataLength; ++i)
    m_data[i] = rand() % 2;
}

void randomizeWorld2D() {
  for (size_t y = 0; y < m_worldHeight; ++y)
    for (size_t x = 0; x < m_worldWidth; ++x)
      m_data2D[y][x] = rand() % 2;
}

inline ubyte countAliveCells(size_t x0, size_t x1, size_t x2, size_t y0,
                             size_t y1, size_t y2) {
  return m_data[x0 + y0] + m_data[x1 + y0] + m_data[x2 + y0] + m_data[x0 + y1] +
         m_data[x2 + y1] + m_data[x0 + y2] + m_data[x1 + y2] + m_data[x2 + y2];
}

inline ubyte countAliveCellsIfs(size_t x0, size_t x1, size_t x2, size_t y0,
                                size_t y1, size_t y2) {
  ubyte alive = 0;
  if (m_data[x0 + y0])
    alive += 1;
  if (m_data[x1 + y0])
    alive += 1;
  if (m_data[x2 + y0])
    alive += 1;
  if (m_data[x0 + y1])
    alive += 1;
  if (m_data[x2 + y1])
    alive += 1;
  if (m_data[x0 + y2])
    alive += 1;
  if (m_data[x1 + y2])
    alive += 1;
  if (m_data[x2 + y2])
    alive += 1;
  return alive;
}

inline ubyte countAliveCells2D(size_t x0, size_t x1, size_t x2, size_t y0,
                               size_t y1, size_t y2) {
  return m_data2D[y0][x0] + m_data2D[y0][x1] + m_data2D[y0][x2] +
         m_data2D[y1][x0] + m_data2D[y1][x2] + m_data2D[y2][x0] +
         m_data2D[y2][x1] + m_data2D[y2][x2];
}

void computeIterationSerial() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    size_t y0 = ((y + m_worldHeight - 1) % m_worldHeight) * m_worldWidth;
    size_t y1 = y * m_worldWidth;
    size_t y2 = ((y + 1) % m_worldHeight) * m_worldWidth;

    for (size_t x = 0; x < m_worldWidth; ++x) {
      size_t x0 = (x + m_worldWidth - 1) % m_worldWidth;
      size_t x2 = (x + 1) % m_worldWidth;

      ubyte aliveCells = countAliveCells(x0, x, x2, y0, y1, y2);
      m_resultData[y1 + x] =
          aliveCells == 3 || (aliveCells == 2 && m_data[x + y1]) ? 1 : 0;
    }
  }
  std::swap(m_data, m_resultData);
  // showGame();
}

void computeIterationSerialIfs() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    size_t y0 = ((y + m_worldHeight - 1) % m_worldHeight) * m_worldWidth;
    size_t y1 = y * m_worldWidth;
    size_t y2 = ((y + 1) % m_worldHeight) * m_worldWidth;

    for (size_t x = 0; x < m_worldWidth; ++x) {
      size_t x0 = (x + m_worldWidth - 1) % m_worldWidth;
      size_t x2 = (x + 1) % m_worldWidth;

      ubyte aliveCells = countAliveCellsIfs(x0, x, x2, y0, y1, y2);
      m_resultData[y1 + x] =
          aliveCells == 3 || (aliveCells == 2 && m_data[x + y1]) ? 1 : 0;
    }
  }
  std::swap(m_data, m_resultData);
}

void computeIterationSerial2D() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    size_t y0 = (y + m_worldHeight - 1) % m_worldHeight;
    size_t y2 = (y + m_worldHeight + 1) % m_worldHeight;

    for (size_t x = 0; x < m_worldWidth; ++x) {
      size_t x0 = (x + m_worldWidth - 1) % m_worldWidth;
      size_t x2 = (x + m_worldWidth + 1) % m_worldWidth;

      ubyte alive = countAliveCells2D(x0, x, x2, y0, y, y2);
      m_resultData2D[y][x] =
          (alive == 3 || (alive == 2 && m_data2D[y][x])) ? 1 : 0;
    }
  }
  std::swap(m_data2D, m_resultData2D);
}

void cleanup() {
  delete[] m_data;
  delete[] m_resultData;

  for (size_t y = 0; y < m_worldHeight; ++y) {
    delete[] m_data2D[y];
    delete[] m_resultData2D[y];
  }

  delete[] m_data2D;
  delete[] m_resultData2D;
}

void runExperiment(ubyte iterations, void (*func)(void), std::ofstream &outfile,
                   std::string title) {
  randomizeWorld();
  std::vector<double> timings;

  for (int i = 0; i < 5; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < iterations; ++j) {
      // std::this_thread::sleep_for(std::chrono::milliseconds(50));
      func();
    }
    auto end = std::chrono::high_resolution_clock::now();

    double timePerIteration =
        std::chrono::duration<double>(end - start).count() / iterations;
    timings.push_back(timePerIteration);
  }

  std::sort(timings.begin(), timings.end());
  double medianTime = timings[timings.size() / 2];

  int cellsPerSecond = std::round(m_dataLength / medianTime);

  outfile << title << ',' << m_worldWidth << ',' << m_worldHeight << ','
          << m_dataLength << ',' << (uint)iterations << ',' << medianTime << ','
          << cellsPerSecond << '\n';
}

void experiment(ubyte iterations, int height, int width,
                std::ofstream &outfile) {
  m_worldHeight = height;
  m_worldWidth = width;
  m_dataLength = m_worldHeight * m_worldWidth;

  m_data = new ubyte[m_dataLength];
  m_resultData = new ubyte[m_dataLength];

  // Serial case
  runExperiment(iterations, computeIterationSerial, outfile, "Serial");

  // Ifs case
  runExperiment(iterations, computeIterationSerialIfs, outfile, "Serial Ifs");

  // 2D case
  m_data2D = new ubyte *[m_worldHeight];
  m_resultData2D = new ubyte *[m_worldHeight];
  for (size_t y = 0; y < m_worldHeight; ++y) {
    m_data2D[y] = new ubyte[m_worldWidth];
    m_resultData2D[y] = new ubyte[m_worldWidth];
  }
  runExperiment(iterations, computeIterationSerial2D, outfile, "Serial 2D");

  cleanup();
}

int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  std::ofstream outfile("serial_benchmark.csv");
  if (!outfile) {
    std::cerr << "Failed to open serial_results.csv for writing.\n";
    return 1;
  }
  outfile << "Mode,Width,Height,Length,Iterations,Time[s],Cells/s\n";

  size_t worldWidth = 1ull << 16;
  for (ushort exp = 4; exp <= 10; ++exp) {
    size_t worldHeight = 1ull << exp;
    std::cout << "Ejecutando 2^16" << "x2^" << exp << " ("
              << worldHeight * worldWidth << ")\n";
    experiment(16, worldWidth, worldHeight, outfile);
  }

  outfile.close();
  return 0;
}
