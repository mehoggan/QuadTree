#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace detail
{
  #pragma pack(push, 1)
  struct __declspec(dllexport) Point
  {
    int8_t id;
    int32_t rank;
    float x;
    float y;
  };

  struct __declspec(dllexport) Rect
  {
    float lx;
    float ly;
    float hx;
    float hy;
  };
  #pragma pack(pop)

  __declspec(dllexport) uint8_t _stdcall msb32(uint32_t x);

  __declspec(dllexport) uint64_t _stdcall spread_by_1_bit(int64_t x);

  __declspec(dllexport) int64_t _stdcall compact_by_1_bit(int64_t x);

  __declspec(dllexport) uint8_t _stdcall max_depth();

  __declspec(dllexport) uint32_t _stdcall max_rows(uint8_t depth);

  __declspec(dllexport) uint32_t _stdcall max_cols(uint8_t depth);

  __declspec(dllexport) uint64_t _stdcall compute_quad_key(
    const Point& p,
    uint8_t depth,
    const Rect &bounds);

  __declspec(dllexport) uint64_t _stdcall min_id(uint8_t depth);

  __declspec(dllexport) uint64_t _stdcall max_id(uint8_t depth);

  __declspec(dllexport) bool _stdcall is_valid(uint64_t quad_key);

  typedef uint64_t Children_t[4];
  __declspec(dllexport) void _stdcall compute_children(uint64_t parent,
    Children_t& children);

  __declspec(dllexport) uint64_t _stdcall compute_parent(uint64_t child);
}

class __declspec(dllexport) QuadTree
{
public:
  

private:
  struct __declspec(dllexport) Node
  {
    enum class ChildId {
      LowerLeft = 0,
      LowerRight = 1,
      UpperLeft = 2,
      UpperRight = 3
    };

    explicit Node(uint64_t quad_key);

    ~Node();

    void set_data(
      std::vector<detail::Point *>::iterator begin,
      std::vector<detail::Point *>::iterator end);

    void set_child(const ChildId id, Node* child);

    uint64_t quad_key_;
    std::vector<detail::Point> points_;
    Node* children_[4];
  };

public:
  constexpr static std::size_t MAX_BLOCK_SIZE = 1000ull;

  QuadTree(
    std::vector<detail::Point *>::iterator begin,
    std::vector<detail::Point *>::iterator end);

  ~QuadTree();

  const detail::Rect& global_bounds() const;

  uint8_t max_depth() const;

  static void compute_bounds(
    std::vector<detail::Point *>::iterator begin,
    std::vector<detail::Point *>::iterator end,
    detail::Rect& out_rect);

private:
  inline std::size_t compute_points_size(const detail::Point* start_point,
    const detail::Point* end_point)
  {
    return std::distance(start_point, end_point);
  }

  void build_tree(Node* node,
    std::vector<detail::Point *>::iterator begin,
    std::vector<detail::Point *>::iterator end,
    uint8_t depth);

  int8_t max_depth_recursive(const Node* node) const;

private:
  Node* root_;
  detail::Rect global_bounds_;
};

#endif

