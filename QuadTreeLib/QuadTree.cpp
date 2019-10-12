#include "QuadTree.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace detail
{
  constexpr uint32_t x_integer_space_ = 0xFFFFFFFF;
  constexpr uint32_t y_integer_space_ = 0xFFFFFFFF;

  uint8_t _stdcall msb32(uint32_t x)
  {
    uint32_t depth = (~0u);
    uint32_t shift = 0u;

    depth = (x > 0xffff) << 4;
    x >>= depth;
    shift = (x > 0xff) << 3;
    x >>= shift;
    depth |= shift;
    shift = (x > 0xf) << 2;
    x >>= shift;
    depth |= shift;
    shift = (x > 0x3) << 1;
    x >>= shift;
    depth |= shift;
    shift = (x > 0x1);
    depth |= shift;

    return static_cast<uint8_t>(depth);
  }

  uint64_t _stdcall spread_by_1_bit(int64_t x)
  {
    x &= 0x00000000ffffffffull;
    x = (x | (x << 16)) & 0x0000ffff0000ffffull;
    x = (x | (x << 8)) & 0x00ff00ff00ff00ffull;
    x = (x | (x << 4)) & 0x0f0f0f0f0f0f0f0full;
    x = (x | (x << 2)) & 0x3333333333333333ull;
    x = (x | (x << 1)) & 0x5555555555555555ull;

    return x;
  }

  int64_t _stdcall compact_by_1_bit(int64_t x)
  {
    x &= 0x5555555555555555ull;
    x = (x | (x >> 1)) & 0x3333333333333333ull;
    x = (x | (x >> 2)) & 0x0f0f0f0f0f0f0f0full;
    x = (x | (x >> 4)) & 0x00ff00ff00ff00ffull;
    x = (x | (x >> 8)) & 0x0000ffff0000ffffull;
    x = (x | (x >> 16)) & 0x00000000ffffffffull;

    return x;
  }

  uint8_t _stdcall max_depth()
  {
    return 31u;
  }

  uint32_t _stdcall max_rows(uint8_t depth)
  {
    if (depth > max_depth()) {
      return 0;
    }
    if (depth == 0) {
      return 1;
    }
    return 1 << depth;
  }

  uint32_t _stdcall max_cols(uint8_t depth)
  {
    if (depth > max_depth()) {
      return 0;
    }
    return 1 << (depth + 1);
  }

  uint64_t _stdcall compute_quad_key(
    const Point& p,
    uint8_t depth,
    const Rect& bounds)
  {
    float domain = bounds.hx - bounds.lx;
    float range = bounds.hy - bounds.ly;

    float percent_x = (p.x - bounds.lx) / domain;
    float percent_y = (p.y - bounds.ly) / range;

    uint32_t max_32_bit_uint = std::numeric_limits<uint32_t>::max();

    uint32_t percent_x_i = (std::min)(
      static_cast<uint64_t>(percent_x * x_integer_space_),
      static_cast<uint64_t>(max_32_bit_uint));
    uint32_t percent_y_i = (std::min)(
      static_cast<uint64_t>(percent_y * y_integer_space_),
      static_cast<uint64_t>(max_32_bit_uint));

    uint64_t xbits = spread_by_1_bit(percent_x_i);
    uint64_t ybits = spread_by_1_bit(percent_y_i);
    uint64_t ybits_shifted = (ybits << 1);

    uint64_t morton = xbits | ybits_shifted;
    bool chop_bit_to_prevent_barrel_roll = morton & 0x8000000000000000;
    if (chop_bit_to_prevent_barrel_roll) {
      morton &= (~0x8000000000000000); // To prevent barrel rolling
    }

    uint64_t shift = (64ull - static_cast<uint64_t>(depth * 2ull));
    if (shift == 64) {
      shift = 63; // On x64 the next line will fail to shift mith msvc.
    }
    uint64_t morton_shifted_by_depth = (morton >> shift);
    uint64_t depth_bit = (0x1ull << (2 * depth));

    uint64_t morton_shifted_with_depth_bit = morton_shifted_by_depth |
      depth_bit;

    if (chop_bit_to_prevent_barrel_roll && depth != 0) {
      uint64_t y_bit_back_in = (0x1ull << (depth * 2ull - 1ull));
      morton_shifted_with_depth_bit |= y_bit_back_in;
    }

    return morton_shifted_with_depth_bit;
  }

  uint64_t _stdcall min_id(uint8_t depth)
  {
    uint64_t depth_bit = (0x1ull << (2 * depth));
    uint64_t ret = 0ull | depth_bit;
    return ret;
  }

  uint64_t _stdcall max_id(uint8_t depth)
  {
    uint64_t depth_bit = (0x1ull << (2 * depth));
    uint64_t ret = 0ull | depth_bit;
    ret |= (0x7FFFFFFFFFFFFFFF >> 63 - 2 * depth);
    return ret;
  }

  bool _stdcall is_valid(uint64_t quad_key)
  {
    bool ret = true;
    if (quad_key == 0 || quad_key > 0x8000000000000000) {
      ret = false;
    }
    return ret;
  }

  void _stdcall compute_children(uint64_t parent, Children_t& children)
  {
    if (!is_valid(parent)) {
      throw std::runtime_error("Invalid child of " + std::to_string(parent));
    }
    else if (parent >= max_id(max_depth() - 1)) {
      throw std::runtime_error("You have reached the maximum depth.");
    }
    for (std::size_t i : {0, 1, 2, 3}) {
      children[i] = (parent << 2) + i;
    }
  }

  uint64_t _stdcall compute_parent(uint64_t child)
  {
    if (!is_valid(child)) {
      throw std::runtime_error("Invalid child of " + std::to_string(child));
    }
    else if (child == min_id(0)) {
      throw std::runtime_error("Root key does not have a parent.");
    }
    child = child & 0x7FFFFFFFFFFFFFFF;
    uint64_t parent = child >> 2;
    return parent;
  }
}

QuadTree::Node::Node(uint64_t quad_key) :
  quad_key_(quad_key),
  children_()
{}

QuadTree::Node::~Node()
{}

void QuadTree::Node::set_data(
  std::vector<detail::Point*>::iterator begin,
  std::vector<detail::Point*>::iterator end)
{
  auto size = std::distance(begin, end);
  points_.resize(size);
  std::vector<detail::Point*>::iterator it = begin;

  std::size_t index = 0;
  while (it != end) {
    points_[index] = **it;
    ++it;
    ++index;
  }
}

void QuadTree::Node::set_child(const ChildId id, Node* child)
{
  children_[static_cast<std::uint8_t>(id)] = child;
}

QuadTree::QuadTree(
  std::vector<detail::Point*>::iterator begin,
  std::vector<detail::Point*>::iterator end) :
  root_(nullptr),
  global_bounds_({})
{
  if (begin == end) {
    return;
  }

  compute_bounds(begin, end, global_bounds_);
  root_ = new Node(detail::compute_quad_key(**begin, 0u, global_bounds_));
  build_tree(root_, begin, end, 0u);
}

QuadTree::~QuadTree()
{
  delete root_;
}

const detail::Rect& QuadTree::global_bounds() const
{
  return global_bounds_;
}

void QuadTree::compute_bounds(
    std::vector<detail::Point*>::iterator begin,
    std::vector<detail::Point*>::iterator end,
    detail::Rect& out_rect)
{
  float maxY = -(std::numeric_limits<float>::max)();
  float minY = +(std::numeric_limits<float>::max)();
  float maxX = -(std::numeric_limits<float>::max)();
  float minX = +(std::numeric_limits<float>::max)();

  std::for_each(begin, end,
    [&](const detail::Point* it)
    {
      if (it->x < minX) minX = it->x;
      if (it->x > maxX) maxX = it->x;
      if (it->y < minY) minY = it->y;
      if (it->y > maxY) maxY = it->y;
      int x = 0;
    });

  out_rect = { minX, minY, maxX, maxY };
}

uint8_t QuadTree::max_depth() const
{
  return max_depth_recursive(root_);
}

void QuadTree::build_tree(Node* node, 
  std::vector<detail::Point*>::iterator begin,
  std::vector<detail::Point*>::iterator end,
  uint8_t depth)
{
  auto count = std::distance(begin, end);

  if (node == nullptr || count == 0) {
    return;
  }

  if (count <= MAX_BLOCK_SIZE || depth == detail::max_depth()) {
    node->set_data(begin, end);
  } else {
    const detail::Point& ip = **begin;
    uint64_t p_pid = detail::compute_quad_key(ip, depth, global_bounds_);
    detail::Children_t children;
    detail::compute_children(p_pid, children);
    std::pair<uint64_t, std::vector<detail::Point*>> buckets[4];
    buckets[0] = std::make_pair(children[0], std::vector<detail::Point*>());
    buckets[1] = std::make_pair(children[1], std::vector<detail::Point*>());
    buckets[2] = std::make_pair(children[2], std::vector<detail::Point*>());
    buckets[3] = std::make_pair(children[3], std::vector<detail::Point*>());

    const uint64_t min_id = buckets[0].first;
    std::vector<detail::Point*>::iterator it = begin;
    while (it != end) {
      detail::Point& p = **it;
      uint64_t c_pid = detail::compute_quad_key(p, depth + 1, global_bounds_);

      uint64_t expected_parent = detail::compute_parent(c_pid);
      if (expected_parent != node->quad_key_) {
        throw std::runtime_error("A quadkey got bucketed wrong.");
      }

      std::size_t bucket_index = c_pid - min_id;
      buckets[bucket_index].second.push_back(&p);
      ++it;
    }

    for (std::size_t i = 0; i < 4; ++i) {
      if (!buckets[i].second.empty()) {
        node->children_[i] = new Node(buckets[i].first);
        build_tree(
          node->children_[i],
          buckets[i].second.begin(),
          buckets[i].second.end(),
          depth + 1);
      }
    }
  }
}

int8_t QuadTree::max_depth_recursive(const Node* node) const
{
  if (node == nullptr) {
    return -1;
  } else {
    auto depth0 = max_depth_recursive(node->children_[0]);
    auto depth1 = max_depth_recursive(node->children_[1]);
    auto depth2 = max_depth_recursive(node->children_[2]);
    auto depth3 = max_depth_recursive(node->children_[3]);
    std::vector<int8_t> depths = { depth0, depth1, depth2, depth3 };
    return 1 + *(std::max_element(depths.begin(), depths.end()));
  }
}
