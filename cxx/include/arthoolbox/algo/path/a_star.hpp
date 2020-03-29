#include <deque>      // deque -> output container
#include <functional> // functors
#include <limits>     // numeric_limits -> inf
#include <queue>      // priority_queue
#include <unordered_map>
#include <unordered_set>

namespace arthoolbox {
namespace algo {
namespace path {

/**
 * @brief Compute the shortest path using A* algorithm
 *
 * @tparam PositionCoordinatesType The type use as coordinates
 * @tparam PositionEqual           Use to compare 2 PositionCoordinatesType
 * @tparam Hash                    Use to hash a PositionCoordinatesType
 *
 * @param[in] from_position                The starting node
 * @param[in] to_position                  The targetted node
 * @param[in] computeHeuristic             A function called to compute the
 * heuristic from one node
 * @param[in] getNeighborWeightedPositions A function use to retreive valid
 * weighted neighbors around a node
 *
 * @return std::deque of position with .front() being the targeted position
 *
 * @details
 * This function try to find the shortest path, from one point to an other using
 * the A* algorithm.
 * It is completly unaware of the map and must be feed with functions (lambda,
 * std::functions..) that it will call to be able to compute the heuristics
 * score from 1 node and retreive all valid neighbors, with their associated
 * distance to 1 node.
 *
 * @warning
 * The output deque is 'reversed' (i.e. the path from start to finish must be
 * read from .back() to .front())
 *
 * @warning
 * The first node doesn't correspond to the from_position arg, but the first
 * node next to this one
 */
template <class PositionCoordinatesType,
          class Hash = std::hash<PositionCoordinatesType>,
          class PositionEqual = std::equal_to<PositionCoordinatesType>>
std::deque<PositionCoordinatesType> aStarShortestPath(
    const PositionCoordinatesType &from_position,
    const PositionCoordinatesType &to_position,
    std::function<double(const PositionCoordinatesType &)> computeHeuristic,
    std::function<std::vector<std::pair<PositionCoordinatesType, double>>(
        const PositionCoordinatesType &)>
        getNeighborWeightedPositions) {
  auto position_are_equals = PositionEqual();

  typedef std::pair<PositionCoordinatesType, double> node_t;

  auto compare_f_score = [](const node_t &lhs, const node_t &rhs) {
    return lhs.second > rhs.second;
  };

  typedef std::priority_queue<node_t, std::vector<node_t>,
                              decltype(compare_f_score)>
      node_heap_t;

  typedef std::unordered_map<PositionCoordinatesType, double, Hash,
                             PositionEqual>
      score_map_t;

  typedef std::unordered_map<PositionCoordinatesType, PositionCoordinatesType,
                             Hash, PositionEqual>
      path_map_t;

  typedef std::unordered_set<PositionCoordinatesType, Hash, PositionEqual>
      visited_pos_set_t;

  std::deque<PositionCoordinatesType> output_path;
  node_heap_t node_heap(compare_f_score);
  visited_pos_set_t visited_position(0);
  score_map_t g_score_map;
  path_map_t came_from;

  node_heap.emplace(from_position, computeHeuristic(from_position));
  g_score_map.emplace(from_position, 0);

  while (!node_heap.empty()) {
    PositionCoordinatesType current_position;
    double current_f_score;

    std::tie(current_position, current_f_score) = node_heap.top();
    node_heap.pop();

    visited_position.insert(current_position);

    if (position_are_equals(current_position, to_position)) {
      // Found -> reconstruct path
      output_path.push_back(current_position);
      auto pos_it = came_from.find(current_position);
      while (pos_it != came_from.end()) {
        output_path.push_back(pos_it->second);
        pos_it = came_from.find(pos_it->second);
      }

      break;

    } else {
      // explore
      for (const auto &neighbour_info :
           getNeighborWeightedPositions(current_position)) {
        PositionCoordinatesType neighbour_position;
        double neighbour_distance;

        std::tie(neighbour_position, neighbour_distance) = neighbour_info;

        // Compute the possible new score from this node to the neighbor
        double new_g_score = g_score_map[current_position] + neighbour_distance;

        auto neighbour_g_score_it = g_score_map.find(neighbour_position);

        // Get the neighbour g_score
        if (neighbour_g_score_it == g_score_map.end())
          std::tie(neighbour_g_score_it, std::ignore) = g_score_map.insert(
              {neighbour_position, std::numeric_limits<double>::infinity()});

        if (new_g_score < neighbour_g_score_it->second) {
          // Better g_score than neighbour
          came_from[neighbour_position] = current_position;
          g_score_map[neighbour_position] = new_g_score;

          if (visited_position.find(neighbour_position) ==
              visited_position.end()) {
            node_heap.emplace(neighbour_position,
                              new_g_score +
                                  computeHeuristic(neighbour_position));
          }
        }
      }
    }
  }

  return output_path;
}

// Specialised overload with Neighbors distance = 1
template <class PositionCoordinatesType,
          class Hash = std::hash<PositionCoordinatesType>,
          class PositionEqual = std::equal_to<PositionCoordinatesType>>
std::deque<PositionCoordinatesType> aStarShortestPath(
    const PositionCoordinatesType &from_position,
    const PositionCoordinatesType &to_position,
    std::function<double(const PositionCoordinatesType &)> computeHeuristic,
    std::function<
        std::vector<PositionCoordinatesType>(const PositionCoordinatesType &)>
        getNeighborPositions) {
  auto position_are_equals = PositionEqual();

  typedef std::pair<PositionCoordinatesType, double> node_t;

  auto compare_f_score = [](const node_t &lhs, const node_t &rhs) {
    return lhs.second > rhs.second;
  };

  typedef std::priority_queue<node_t, std::vector<node_t>,
                              decltype(compare_f_score)>
      node_heap_t;

  typedef std::unordered_map<PositionCoordinatesType, double, Hash,
                             PositionEqual>
      score_map_t;

  typedef std::unordered_map<PositionCoordinatesType, PositionCoordinatesType,
                             Hash, PositionEqual>
      path_map_t;

  typedef std::unordered_set<PositionCoordinatesType, Hash, PositionEqual>
      visited_pos_set_t;

  std::deque<PositionCoordinatesType> output_path;
  node_heap_t node_heap(compare_f_score);
  visited_pos_set_t visited_position(0);
  score_map_t g_score_map;
  path_map_t came_from;

  node_heap.emplace(from_position, computeHeuristic(from_position));
  g_score_map.emplace(from_position, 0);

  while (!node_heap.empty()) {
    PositionCoordinatesType current_position;
    double current_f_score;

    std::tie(current_position, current_f_score) = node_heap.top();
    node_heap.pop();

    visited_position.insert(current_position);

    if (position_are_equals(current_position, to_position)) {
      // Found -> reconstruct path
      output_path.push_back(current_position);
      auto pos_it = came_from.find(current_position);
      while (pos_it != came_from.end()) {
        output_path.push_back(pos_it->second);
        pos_it = came_from.find(pos_it->second);
      }

      break;

    } else {
      // explore
      for (const auto &neighbour_position :
           getNeighborPositions(current_position)) {

        // Compute the possible new score from this node to the neighbor
        double new_g_score = g_score_map[current_position] + 1;

        auto neighbour_g_score_it = g_score_map.find(neighbour_position);

        // Get the neighbour g_score
        if (neighbour_g_score_it == g_score_map.end())
          std::tie(neighbour_g_score_it, std::ignore) = g_score_map.insert(
              {neighbour_position, std::numeric_limits<double>::infinity()});

        if (new_g_score < neighbour_g_score_it->second) {
          // Better g_score than neighbour
          came_from[neighbour_position] = current_position;
          g_score_map[neighbour_position] = new_g_score;

          if (visited_position.find(neighbour_position) ==
              visited_position.end()) {
            node_heap.emplace(neighbour_position,
                              new_g_score +
                                  computeHeuristic(neighbour_position));
          }
        }
      }
    }
  }

  return output_path;
}

} // namespace path
} // namespace algo
} // namespace arthoolbox
