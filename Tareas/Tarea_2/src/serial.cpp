#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

// using namespace std;

typedef unsigned char ubyte;

ubyte *m_data;
ubyte *m_resultData;

size_t m_worldWidth;
size_t m_worldHeight;
size_t m_dataLength;

void randomizeWorld() {
  for (int i = 0; i < m_worldWidth * m_worldHeight; ++i)
    m_data[i] = rand() % 2;
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
  // std::cout << "------------------" << std::endl;
}

inline ubyte countAliveCells(size_t x0, size_t x1, size_t x2, size_t y0,
                             size_t y1, size_t y2) {
  return m_data[x0 + y0] + m_data[x1 + y0] + m_data[x2 + y0] + m_data[x0 + y1] +
         m_data[x2 + y1] + m_data[x0 + y2] + m_data[x1 + y2] + m_data[x2 + y2];
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

int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  m_worldHeight = 39;
  m_worldWidth = 165;
  m_dataLength = m_worldHeight * m_worldWidth;

  m_data = new ubyte[m_dataLength];
  m_resultData = new ubyte[m_dataLength];

  randomizeWorld();

  while (1) {
    computeIterationSerial();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
