#include "pch.h"
#include "CppUnitTest.h"

#include <algorithm>
#include <ctime>
#include <cstdlib>

#include <QuadTree.h>

// For test macros
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace QuadTreeNS
{
  TEST_CLASS(TestQuadTree)
  {
  private:
    const detail::Rect rects[16] = {
      { -16.0f, -16.0f, -8.0f, -8.0f },
      { -8.0f, -16.0f, +0.0f, -8.0f },
      { +0.0f, -16.0f, +8.0f, -8.0f },
      { +8.0f, -16.0f, +16.0f, -8.0f },
      { -16.0f, -8.0f, -8.0f, +0.0f },
      { -8.0f, -8.0f, +0.0f, +0.0f },
      { +0.0f, -8.0f, +8.0f, +0.0f },
      { +8.0f, -8.0f, +16.0f, +0.0f },
      { -16.0f, +0.0f, -8.0f, +8.0f },
      { -8.0f, +0.0f, +0.0f, +8.0f },
      { +0.0f, +0.0f, +8.0f, +8.0f },
      { +8.0f, +0.0f, +16.0f, +8.0f },
      { -16.0f, +8.0f, -8.0f, +16.0f },
      { -8.0f, +8.0f, +0.0f, +16.0f },
      { +0.0f, +8.0f, +8.0f, +16.0f },
      { +8.0f, +8.0f, +16.0f, +16.0f },
    };

    float frand(float lower_bound, float upper_bound)
    {
      float random = ((float)rand()) / (float)RAND_MAX;
      float diff = upper_bound - lower_bound;
      float r = random * diff;
      return lower_bound + r;
    }

    std::vector<detail::Point *> acquire_random_point_distributed_equally()
    {
      std::size_t point_count = 16 * QuadTree::MAX_BLOCK_SIZE;

      std::size_t rect_count = sizeof(rects) / sizeof(rects[0]);
      std::size_t index = 0;

      std::vector<detail::Point *> points;

      points.push_back(new detail::Point {
        static_cast<int8_t>(std::rand()),
        std::rand(),
        -16.0f, -16.0f
      });
      ++index;
      for (std::size_t rect_i = 1; rect_i < rect_count - 1; ++rect_i) {
        const detail::Rect& rect = rects[rect_i];
        for (std::size_t j = 0; j < QuadTree::MAX_BLOCK_SIZE; ++j) {
          float x = frand(rect.lx, rect.hx);
          float y = frand(rect.ly, rect.hy);
          points.push_back(new detail::Point {
            static_cast<int8_t>(std::rand()),
            std::rand(),
            x, y
          });
          ++index;
        }
      }
      points.push_back(new detail::Point {
        static_cast<int8_t>(std::rand()),
        std::rand(),
        +16.0f, +16.0f
      });

      return points;
    }

    void release_resources(std::vector<detail::Point*>& points)
    {
      std::for_each(points.begin(), points.end(),
        [&](detail::Point* p)
        {
          delete p;
        });
    }

    detail::Point* build_big_data_set()
    {
    }

  public:
    TEST_METHOD(TestTestData)
    {
      auto points = acquire_random_point_distributed_equally();
      for (std::size_t i = 0; i < points.size(); ++i) {
        Assert::IsTrue(points[i]->x >= -16.0f && points[i]->x <= +16.0f);
        Assert::IsTrue(points[i]->y >= -16.0f && points[i]->y <= +16.0f);
      }
      release_resources(points);
    }

    TEST_METHOD(TestRectGenerateFromTestData)
    {
      std::size_t point_count = 16 * QuadTree::MAX_BLOCK_SIZE;
      detail::Rect bounds;
      auto points = acquire_random_point_distributed_equally();
      QuadTree::compute_bounds(points.begin(), points.end(), bounds);
      Assert::AreEqual(-16.0f, bounds.lx);
      Assert::AreEqual(-16.0f, bounds.ly);
      Assert::AreEqual(+16.0f, bounds.hx);
      Assert::AreEqual(+16.0f, bounds.hy);
      release_resources(points);
    }

    TEST_METHOD(TestInsertMaxBlockSizePointsDistributedEvenly)
    {
      std::size_t point_count = 16 * QuadTree::MAX_BLOCK_SIZE;
      auto points = acquire_random_point_distributed_equally();
      QuadTree quad_tree(points.begin(), points.end());
      std::size_t max_depth = quad_tree.max_depth();
      // Solve this bug...
      // Assert::AreEqual(2ull, max_depth);
      release_resources(points);
    }

    TEST_METHOD(TestMinIdAtDepth)
    {
      uint64_t actual = 0ull;

      actual = detail::min_id(0);
      Assert::AreEqual(1ull, actual);
      actual = detail::min_id(1);
      Assert::AreEqual(4ull, actual);
      actual = detail::min_id(2);
      Assert::AreEqual(16ull, actual);
      actual = detail::min_id(3);
      Assert::AreEqual(64ull, actual);
      actual = detail::min_id(detail::max_depth());
      Assert::AreEqual(0x4000000000000000ull, actual);
    }

    TEST_METHOD(TestMaxIdAtDepth)
    {
      // TODO: Here
      uint64_t actual = 0ull;

      actual = detail::max_id(0);
      Assert::AreEqual(1ull, actual);
      actual = detail::max_id(1);
      Assert::AreEqual(7ull, actual);
      actual = detail::max_id(2);
      Assert::AreEqual(31ull, actual);
      actual = detail::max_id(3);
      Assert::AreEqual(127ull, actual);
      actual = detail::max_id(detail::max_depth());
      Assert::AreEqual(0x7FFFFFFFFFFFFFFFull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth0)
    {
      srand(time(nullptr));
      detail::Rect bounds = { -16.0, -16.0, +16.0, +16.0 };
      detail::Point point_ll = { 0, rand(), -16.0, -16.0 };
      detail::Point point_lr = { 1, rand(), +16.0, -16.0 };
      detail::Point point_ur = { 2, rand(), +16.0, +16.0 };
      detail::Point point_ul = { 3, rand(), -16.0, +16.0 };
      uint64_t actual = 0ull;
      uint64_t expected = 1ull;
      actual = detail::compute_quad_key(point_ll, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = detail::compute_quad_key(point_lr, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = detail::compute_quad_key(point_ur, 0, bounds);
      Assert::AreEqual(1ull, actual);
      actual = detail::compute_quad_key(point_ul, 0, bounds);
      Assert::AreEqual(1ull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth1)
    {
      srand(time(nullptr));
      detail::Rect bb = { -16.0, -16.0, +16.0, +16.0 };
      //6-----677------7
      //|  6   |   7   |
      //|      |       |
      //6-----677------7
      //4     45       5
      //|  4   |   5   |
      //4-----455------5

      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 1;

      // Bottom Left to right.
      actual = detail::compute_quad_key({ id++, r, -16.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = detail::compute_quad_key({ id++, r, - 0.1f, -16.0f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.1f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = detail::compute_quad_key({ id++, r, +16.0f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);

      // Just bellow Middle Left to right.
      actual = detail::compute_quad_key({ id++, r, -16.0f, - 0.1f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = detail::compute_quad_key({ id++, r, - 0.1f, - 0.1f }, d, bb);
      Assert::AreEqual(0x4ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.0f, - 0.1f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.1f, -16.0f }, d, bb);
      Assert::AreEqual(0x5ull, actual);
      actual = detail::compute_quad_key({ id++, r, +16.0f, - 0.1f }, d, bb);
      Assert::AreEqual(0x5ull, actual);

      // Middle Left to right.
      actual = detail::compute_quad_key({ id++, r, -16.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x6ull, actual);
      actual = detail::compute_quad_key({ id++, r, - 0.1f, + 0.0f }, d, bb);
      Assert::AreEqual(0x6ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.1f, + 0.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
      actual = detail::compute_quad_key({ id++, r, +16.0f, + 0.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);

      // Top Left to right.
      actual = detail::compute_quad_key({ id++, r, -16.0f, +16.0f }, d, bb);
      Assert::AreEqual(0x6ull, actual);
      actual = detail::compute_quad_key({ id++, r, - 0.1f, +16.0f }, d, bb);
      Assert::AreEqual(0x6ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.0f, +16.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
      actual = detail::compute_quad_key({ id++, r, + 0.1f, +16.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
      actual = detail::compute_quad_key({ id++, r, +16.0f, +16.0f }, d, bb);
      Assert::AreEqual(0x7ull, actual);
    }

    TEST_METHOD(TestGenerateQuadKeyDepth2)
    {
      srand(time(nullptr));
      detail::Rect bb = { -16.0, -16.0, +16.0, +16.0 };
      //  o-----ooo-----ooo----ooo------o11 +16
      //  o 26  ooo 27  ooo 30 ooo 31   o10
      //  o     ooo     ooo    ooo      o 9
      //  o-----ooo-----ooo----ooo------o 8 + 8
      //  o 24  ooo 25  ooo 28 ooo 29   o 7
      //  o     ooo     ooo    ooo      o 6
      //  o-----ooo-----ooo----ooo------o 5 + 0
      //  o 18  ooo 19  ooo 22 ooo 23   o 4
      //  o     ooo     ooo    ooo      o 3
      //  o-----ooo-----ooo----ooo------o 2 - 8
      //  o 16  ooo 17  ooo 20 ooo 21   o 1
      //  |      |       |      |       |
      //  x-----xxx-----xxx----ooo------o 0 -16
      //-16     -8       0     +8       +16
      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 2;

      // Row 0 and 1 above left to right.
      for (auto row : { -16.0f, -8.1f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
      }

      // Row 2, 3, and 4 above left to right.
      for (auto row : { -8.0f, -7.9f, -0.1f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
      }
      
      // Row 5, 6, and 7 above left to right.
      for (auto row : { 0.0f, 0.1f, 7.9f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
      }

      // Row 8, 9, 10, and 11 above left to right.
      for (auto row : { 8.0f, 8.1f, 15.9f, 16.0f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
      }
    }

    TEST_METHOD(TestRandomPoints)
    {
      //  |-----------------------------| +16
      //  |                             |
      //  |                             |
      //  |                             | + 8
      //  |                             |
      //  |                             |
      //  |              1              | + 0
      //  |                             |
      //  |                             |
      //  |                             | - 8
      //  |                             |
      //  |                             |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +

      //  |-----------------------------| +16
      //  |              |              |
      //  |              |              |
      //  |      6       |      7       | + 8
      //  |              |              |
      //  |              |              |
      //  |--------------|--------------| + 0
      //  |              |              |
      //  |              |              |
      //  |      4       |      5       | - 8
      //  |              |              |
      //  |              |              |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +

      //  |-----------------------------| +16
      //  | 26   |  27   |  30  |  31   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 8
      //  | 24   |  25   |  28  |  29   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 0
      //  | 18   |  19   |  22  |  23   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| - 8
      //  | 16   |  17   |  20  |  21   |
      //  |      |       |      |       |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +16

      //  --------------------------------  +16
      //  |106|107|110|111|122|123|126|127|
      //  |---|---|---|---|---|---|---|---| +12
      //  |104|105|108|109|120|121|124|125|
      //  |---|---|---|---|---|---|---|---| + 8
      //  |98 |99 |102|103|114|115|118|119|
      //  |---|---|---|---|---|---|---|---| + 4
      //  |96 |97 |100|101|112|113|116|117|
      //  |---|---|---|---|---|---|---|---| + 0
      //  |74 |75 |78 |79 |90 |91 |94 |95 |
      //  |---|---|---|---|---|---|---|---| - 4
      //  |72 |73 |76 |77 |88 |89 |92 |93 |
      //  |---|---|---|---|---|---|---|---| - 8
      //  |66 |67 |70 |71 |82 |83 |86 |87 |
      //  |---|---|---|---|---|---|---|---| -12
      //  |64 |65 |68 |69 |80 |81 |84 |85 |
      //  --------------------------------  -16
      //-16 -12  -8  -4   0  +4  +8  +12  +16

      detail::Rect bounds = { -16.0, -16.0, +16.0, +16.0 };
      {
        detail::Point p = { 83, 12623, 5.88006210, -11.1282692 };
        uint64_t p_id_0 = detail::compute_quad_key(p, 0, bounds);
        Assert::AreEqual(1ull, p_id_0);
        uint64_t p_id_1 = detail::compute_quad_key(p, 1, bounds);
        Assert::AreEqual(5ull, p_id_1);
        uint64_t p_id_2 = detail::compute_quad_key(p, 2, bounds);
        Assert::AreEqual(20ull, p_id_2);
        uint64_t p_id_3 = detail::compute_quad_key(p, 3, bounds);
        Assert::AreEqual(83ull, p_id_3);
      }

      {
        detail::Point p = { 82, 24464, 2.80233169, -8.83230019 };
        uint64_t p_id_0 = detail::compute_quad_key(p, 0, bounds);
        Assert::AreEqual(1ull, p_id_0);
        uint64_t p_id_1 = detail::compute_quad_key(p, 1, bounds);
        Assert::AreEqual(5ull, p_id_1);
        uint64_t p_id_2 = detail::compute_quad_key(p, 2, bounds);
        Assert::AreEqual(20ull, p_id_2);
        uint64_t p_id_3 = detail::compute_quad_key(p, 3, bounds);
        Assert::AreEqual(82ull, p_id_3);
      }
    }

    TEST_METHOD(TestGenerateQuadKeyDepth2RectangleRegion)
    {
      srand(time(nullptr));
      detail::Rect bb = { -16.0, -8.0, +16.0, +8.0 };
      //  o-----ooo-----ooo----ooo------o11 +8
      //  o 26  ooo 27  ooo 30 ooo 31   o10
      //  o     ooo     ooo    ooo      o 9
      //  o-----ooo-----ooo----ooo------o 8 +4
      //  o 24  ooo 25  ooo 28 ooo 29   o 7
      //  o     ooo     ooo    ooo      o 6
      //  o-----ooo-----ooo----ooo------o 5 +0
      //  o 18  ooo 19  ooo 22 ooo 23   o 4
      //  o     ooo     ooo    ooo      o 3
      //  o-----ooo-----ooo----ooo------o 2 -4
      //  o 16  ooo 17  ooo 20 ooo 21   o 1
      //  |      |       |      |       |
      //  x-----xxx-----xxx----ooo------o 0 -8
      //-16     -8       0      8       16

      auto actual = 0ull;
      int32_t r = rand();
      int8_t id = 0;
      uint8_t d = 2;

      // Row 0 and 1 above left to right.
      for (auto row : { -8.0f, -4.1f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(16ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(17ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(20ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(21ull, actual);
      }

      // Row 2, 3, and 4 above left to right.
      for (auto row : { -4.0f, -3.9f, -0.1f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(18ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(19ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(22ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(23ull, actual);
      }
      
      // Row 5, 6, and 7 above left to right.
      for (auto row : { 0.0f, 0.1f, 3.9f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(24ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(25ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(28ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(29ull, actual);
      }

      // Row 8, 9, 10, and 11 above left to right.
      for (auto row : { 4.0f, 4.1f, 7.9f, 8.0f }) {
        actual = detail::compute_quad_key({ id++, r, -16.0f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.1f, row }, d, bb);
        Assert::AreEqual(26ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 8.0f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 7.9f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, - 0.1f, row }, d, bb);
        Assert::AreEqual(27ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.0f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 0.1f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 7.9f, row }, d, bb);
        Assert::AreEqual(30ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.0f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
        actual = detail::compute_quad_key({ id++, r, + 8.1f, row }, d, bb);
        Assert::AreEqual(31ull, actual);
      }
    }

    TEST_METHOD(ComputeChildrenAndParent)
    {
      //  |-----------------------------| +16
      //  |                             |
      //  |                             |
      //  |                             | + 8
      //  |                             |
      //  |                             |
      //  |              1              | + 0
      //  |                             |
      //  |                             |
      //  |                             | - 8
      //  |                             |
      //  |                             |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +
      {
        detail::Children_t depth0_children;
        detail::compute_children(1ull, depth0_children);
        Assert::AreEqual(4ull, depth0_children[0]);
        Assert::AreEqual(5ull, depth0_children[1]);
        Assert::AreEqual(6ull, depth0_children[2]);
        Assert::AreEqual(7ull, depth0_children[3]);
      }

      //  |-----------------------------| +16
      //  |              |              |
      //  |              |              |
      //  |      6       |      7       | + 8
      //  |              |              |
      //  |              |              |
      //  |--------------|--------------| + 0
      //  |              |              |
      //  |              |              |
      //  |      4       |      5       | - 8
      //  |              |              |
      //  |              |              |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +
      {
        {
          uint64_t parent = 0;
          parent = detail::compute_parent(4ull);
          Assert::AreEqual(1ull, parent);
          parent = detail::compute_parent(5ull);
          Assert::AreEqual(1ull, parent);
          parent = detail::compute_parent(6ull);
          Assert::AreEqual(1ull, parent);
          parent = detail::compute_parent(7ull);
          Assert::AreEqual(1ull, parent);
        }
        
        {
          detail::Children_t depth1_children4;
          detail::compute_children(4ull, depth1_children4);
          Assert::AreEqual(16ull, depth1_children4[0]);
          Assert::AreEqual(17ull, depth1_children4[1]);
          Assert::AreEqual(18ull, depth1_children4[2]);
          Assert::AreEqual(19ull, depth1_children4[3]);
        }

        {
          detail::Children_t depth1_children5;
          detail::compute_children(5ull, depth1_children5);
          Assert::AreEqual(20ull, depth1_children5[0]);
          Assert::AreEqual(21ull, depth1_children5[1]);
          Assert::AreEqual(22ull, depth1_children5[2]);
          Assert::AreEqual(23ull, depth1_children5[3]);
        }

        {
          detail::Children_t depth1_children6;
          detail::compute_children(6ull, depth1_children6);
          Assert::AreEqual(24ull, depth1_children6[0]);
          Assert::AreEqual(25ull, depth1_children6[1]);
          Assert::AreEqual(26ull, depth1_children6[2]);
          Assert::AreEqual(27ull, depth1_children6[3]);
        }

        {
          detail::Children_t depth1_children7;
          detail::compute_children(7ull, depth1_children7);
          Assert::AreEqual(28ull, depth1_children7[0]);
          Assert::AreEqual(29ull, depth1_children7[1]);
          Assert::AreEqual(30ull, depth1_children7[2]);
          Assert::AreEqual(31ull, depth1_children7[3]);
        }
      }

      //  |-----------------------------| +16
      //  | 26   |  27   |  30  |  31   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 8
      //  | 24   |  25   |  28  |  29   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| + 0
      //  | 18   |  19   |  22  |  23   |
      //  |      |       |      |       |
      //  |------|-------|------|-------| - 8
      //  | 16   |  17   |  20  |  21   |
      //  |      |       |      |       |
      //  |-----------------------------| -16
      //-16     -8       0     +8       +16
      {
        {
          {
            uint64_t parent = 0;
            parent = detail::compute_parent(16ull);
            Assert::AreEqual(4ull, parent);
            parent = detail::compute_parent(17ull);
            Assert::AreEqual(4ull, parent);
            parent = detail::compute_parent(18ull);
            Assert::AreEqual(4ull, parent);
            parent = detail::compute_parent(19ull);
            Assert::AreEqual(4ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = detail::compute_parent(20ull);
            Assert::AreEqual(5ull, parent);
            parent = detail::compute_parent(21ull);
            Assert::AreEqual(5ull, parent);
            parent = detail::compute_parent(22ull);
            Assert::AreEqual(5ull, parent);
            parent = detail::compute_parent(23ull);
            Assert::AreEqual(5ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = detail::compute_parent(24ull);
            Assert::AreEqual(6ull, parent);
            parent = detail::compute_parent(25ull);
            Assert::AreEqual(6ull, parent);
            parent = detail::compute_parent(26ull);
            Assert::AreEqual(6ull, parent);
            parent = detail::compute_parent(27ull);
            Assert::AreEqual(6ull, parent);
          }
          {
            uint64_t parent = 0;
            parent = detail::compute_parent(28ull);
            Assert::AreEqual(7ull, parent);
            parent = detail::compute_parent(29ull);
            Assert::AreEqual(7ull, parent);
            parent = detail::compute_parent(30ull);
            Assert::AreEqual(7ull, parent);
            parent = detail::compute_parent(31ull);
            Assert::AreEqual(7ull, parent);
          }
        }
        //  -------------------------------- +16
        //  |106|107|110|111|122|123|126|127|
        //  |---|---|---|---|---|---|---|---|
        //  |104|105|108|109|120|121|124|125|
        //  |---|---|---|---|---|---|---|---| + 8
        //  |98 |99 |102|103|114|115|118|119|
        //  |---|---|---|---|---|---|---|---|
        //  |96 |97 |100|101|112|113|116|117|
        //  |---|---|---|---|---|---|---|---| + 0
        //  |74 |75 |78 |79 |90 |91 |94 |95 |
        //  |---|---|---|---|---|---|---|---|
        //  |72 |73 |76 |77 |88 |89 |92 |93 |
        //  |---|---|---|---|---|---|---|---| - 8
        //  |66 |67 |70 |71 |82 |83 |86 |87 |
        //  |---|---|---|---|---|---|---|---|
        //  |64 |65 |68 |69 |80 |81 |84 |85 |
        //  -------------------------------- -16
        //-16     -8       0     +8       +16
        {
          std::vector<std::vector<uint64_t>> expected = {
            { 64ull, 65ull, 66ull, 67ull },
            { 68ull, 69ull, 70ull, 71ull },
            { 72ull, 73ull, 74ull, 75ull },
            { 76ull, 77ull, 78ull, 79ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 16ull, 17ull, 18ull, 19ull }) {
            detail::Children_t children;
            detail::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 80ull, 81ull, 82ull, 83ull },
            { 84ull, 85ull, 86ull, 87ull },
            { 88ull, 89ull, 90ull, 91ull },
            { 92ull, 93ull, 94ull, 95ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 20ull, 21ull, 22ull, 23ull }) {
            detail::Children_t children;
            detail::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 96ull, 97ull, 98ull, 99ull },
            { 100ull, 101ull, 102ull, 103ull },
            { 104ull, 105ull, 106ull, 107ull },
            { 108ull, 109ull, 110ull, 111ull },
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 24ull, 25ull, 26ull, 27ull }) {
            detail::Children_t children;
            detail::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }

        {
          std::vector<std::vector<uint64_t>> expected = {
            { 112ull, 113ull, 114ull, 115ull },
            { 116ull, 117ull, 118ull, 119ull },
            { 120ull, 121ull, 122ull, 123ull },
            { 124ull, 125ull, 126ull, 127ull }
          };
          std::size_t expected_index = 0;
          for (uint64_t parent : { 28ull, 29ull, 30ull, 31ull }) {
            detail::Children_t children;
            detail::compute_children(parent, children);
            Assert::AreEqual(expected[expected_index][0], children[0]);
            Assert::AreEqual(expected[expected_index][1], children[1]);
            Assert::AreEqual(expected[expected_index][2], children[2]);
            Assert::AreEqual(expected[expected_index][3], children[3]);
            ++expected_index;
          }
        }
      }
    }
  };
}
