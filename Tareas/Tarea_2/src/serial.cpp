#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

typedef unsigned char ubyte;

ubyte *m_data;
ubyte *m_resultData;

ubyte **m_data2D;
ubyte **m_resultData2D;

size_t m_worldWidth;
size_t m_worldHeight;
size_t m_dataLength;

void randomizeWorld() {
  for (int i = 0; i < m_dataLength; ++i)
    m_data[i] = rand() % 2;
}

void randomizeWorld2D() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    for (size_t x = 0; x < m_worldWidth; ++x)
      m_data2D[y][x] = rand() % 2;
  }
}

void clearTerminal() { std::cout << "\033[2J\033[1;1H"; }

void showGame() {
  clearTerminal();
  std::cout << "   ";
  for (int x = 0; x < m_worldWidth; ++x)
    std::cout << (x % 10);
  std::cout << '\n';

  for (size_t y = 0; y < m_worldHeight; ++y) {
    std::cout << (y < 10 ? " " : "") << y << " ";

    for (size_t x = 0; x < m_worldWidth; ++x) {
      int index = y * m_worldWidth + x;
      if (m_data[index] == 1)
        std::cout << "\033[42m \033[0m";
      else
        std::cout << "\033[40m \033[0m";
    }
    std::cout << '\n';
  }
}

void showGame2D() {
  clearTerminal();
  std::cout << "   ";
  for (int x = 0; x < m_worldWidth; ++x)
    std::cout << (x % 10);
  std::cout << '\n';

  for (size_t y = 0; y < m_worldHeight; ++y) {
    std::cout << (y < 10 ? " " : "") << y << " ";

    for (size_t x = 0; x < m_worldWidth; ++x) {
      if (m_data2D[y][x] == 1)
        std::cout << "\033[42m \033[0m";
      else
        std::cout << "\033[40m \033[0m";
    }
    std::cout << '\n';
  }
}

inline ubyte countAliveCells(size_t x0, size_t x1, size_t x2, size_t y0,
                             size_t y1, size_t y2) {
  return m_data[x0 + y0] + m_data[x1 + y0] + m_data[x2 + y0] + m_data[x0 + y1] +
         m_data[x2 + y1] + m_data[x0 + y2] + m_data[x1 + y2] + m_data[x2 + y2];
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
  showGame();
}

void computeIterationSerialIfs() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    size_t y0 = ((y + m_worldHeight - 1) % m_worldHeight) * m_worldWidth;
    size_t y1 = y * m_worldWidth;
    size_t y2 = ((y + 1) % m_worldHeight) * m_worldWidth;

    for (size_t x = 0; x < m_worldWidth; ++x) {
      size_t x0 = (x + m_worldWidth - 1) % m_worldWidth;
      size_t x2 = (x + 1) % m_worldWidth;

      ubyte aliveCells = countAliveCells(x0, x, x2, y0, y1, y2);
      if (aliveCells == 3 || (aliveCells == 2 && m_data[x + y1]))
        m_resultData[y1 + x] = 1;
      else
        m_resultData[y1 + x] = 0;
    }
  }
  std::swap(m_data, m_resultData);
  showGame();
}

void computeIterationSerial2D() {
  for (size_t y = 0; y < m_worldHeight; ++y) {
    size_t y0 = (y - 1) % m_worldHeight;
    size_t y2 = (y + 1) % m_worldHeight;

    for (size_t x = 0; x < m_worldWidth; ++x) {
      size_t x0 = (x - 1) % m_worldWidth;
      size_t x2 = (x + 1) % m_worldWidth;

      ubyte alive = countAliveCells2D(x0, x, x2, y0, y, y2);
      m_resultData2D[y][x] =
          (alive == 3 || (alive == 2 && m_data2D[y][x])) ? 1 : 0;
    }
  }
  std::swap(m_data2D, m_resultData2D);
  showGame2D();
}

int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  int iterations = 300;
  int tmpIterations = iterations;
  m_worldHeight = 39;
  m_worldWidth = 165;
  m_dataLength = m_worldHeight * m_worldWidth;

  m_data = new ubyte[m_dataLength];
  m_resultData = new ubyte[m_dataLength];

  // Serial case
  randomizeWorld();

  while (tmpIterations--) {
    computeIterationSerial();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Ifs case
  randomizeWorld();

  tmpIterations = iterations;
  while (tmpIterations--) {
    computeIterationSerialIfs();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // 2D case
  m_data2D = new ubyte *[m_worldHeight];
  m_resultData2D = new ubyte *[m_worldHeight];
  for (size_t y = 0; y < m_worldHeight; ++y) {
    m_data2D[y] = new ubyte[m_worldWidth];
    m_resultData2D[y] = new ubyte[m_worldWidth];
  }

  randomizeWorld2D();

  tmpIterations = iterations;
  while (tmpIterations--) {
    computeIterationSerial2D();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return 0;
}
