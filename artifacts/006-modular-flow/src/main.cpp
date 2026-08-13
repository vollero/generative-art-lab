#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
struct Options {
  std::filesystem::path output{"out/006-modular-flow.svg"};
  std::uint64_t modulus{63};
  std::uint64_t exponent{2};
  int width{2160};
  int height{2160};
  double margin{150};
  double edge_alpha{0.20};
  double vertex_size{7};
  std::string encoding{"depth"};
  std::string depth_scale{"sqrt"};
  std::int64_t highlight_start{-1};
  bool show_labels{false};
  bool self_test{false};
};

struct Node { std::uint64_t next{}; int component{-1}; int depth{-1}; int cycle_length{}; bool cycle{}; };
struct Graph { std::vector<Node> nodes; std::vector<int> basin_sizes; int cycles{}; int longest_cycle{}; int maximum_depth{}; };

template <typename T> T number(std::string_view text, std::string_view name) {
  std::string value{text}; std::size_t used = 0;
  try {
    if constexpr (std::is_integral_v<T>) {
      if constexpr (std::is_signed_v<T>) {
        const auto parsed = std::stoll(value, &used);
        if (used != value.size() || parsed < std::numeric_limits<T>::min() || parsed > std::numeric_limits<T>::max()) throw std::invalid_argument("range");
        return static_cast<T>(parsed);
      } else {
        const auto parsed = std::stoull(value, &used);
        if (used != value.size() || parsed > std::numeric_limits<T>::max()) throw std::invalid_argument("range");
        return static_cast<T>(parsed);
      }
    } else {
      const auto parsed = std::stod(value, &used);
      if (used != value.size()) throw std::invalid_argument("trailing");
      return parsed;
    }
  } catch (...) { throw std::runtime_error("invalid value for --" + std::string{name}); }
}

Options parse(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&]() -> std::string_view { if (++i >= argc) throw std::runtime_error("missing value after " + arg); return argv[i]; };
    if (arg == "--output") o.output = std::string{next()};
    else if (arg == "--modulus") o.modulus = number<std::uint64_t>(next(), "modulus");
    else if (arg == "--exponent") o.exponent = number<std::uint64_t>(next(), "exponent");
    else if (arg == "--width") o.width = number<int>(next(), "width");
    else if (arg == "--height") o.height = number<int>(next(), "height");
    else if (arg == "--margin") o.margin = number<double>(next(), "margin");
    else if (arg == "--edge-alpha") o.edge_alpha = number<double>(next(), "edge-alpha");
    else if (arg == "--vertex-size") o.vertex_size = number<double>(next(), "vertex-size");
    else if (arg == "--encoding") o.encoding = std::string{next()};
    else if (arg == "--depth-scale") o.depth_scale = std::string{next()};
    else if (arg == "--highlight-start") o.highlight_start = number<std::int64_t>(next(), "highlight-start");
    else if (arg == "--show-labels") o.show_labels = true;
    else if (arg == "--self-test") o.self_test = true;
    else if (arg == "--help") {
      std::cout << "modular_flow [--output FILE] [--modulus N] [--exponent E]\n"
                   "  [--encoding role|depth|basin] [--depth-scale linear|sqrt|log]\n"
                   "  [--highlight-start X] [--show-labels] [--edge-alpha 0..1]\n";
      std::exit(0);
    } else throw std::runtime_error("unknown option: " + arg);
  }
  if (o.modulus < 2 || o.modulus > 10000) throw std::runtime_error("modulus must be from 2 to 10000");
  if (o.exponent < 2 || o.exponent > 64) throw std::runtime_error("exponent must be from 2 to 64");
  if (o.width < 64 || o.height < 64 || o.margin < 0) throw std::runtime_error("invalid canvas geometry");
  if (o.edge_alpha < 0 || o.edge_alpha > 1 || o.vertex_size <= 0) throw std::runtime_error("invalid styling value");
  if (o.encoding != "role" && o.encoding != "depth" && o.encoding != "basin") throw std::runtime_error("invalid encoding");
  if (o.depth_scale != "linear" && o.depth_scale != "sqrt" && o.depth_scale != "log") throw std::runtime_error("invalid depth-scale");
  if (o.highlight_start >= static_cast<std::int64_t>(o.modulus)) throw std::runtime_error("highlight-start must be below modulus");
  if (o.show_labels && o.modulus > 127) throw std::runtime_error("show-labels requires modulus <= 127");
  return o;
}

std::uint64_t add_mod(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
  return a >= modulus - b ? a - (modulus - b) : a + b;
}
std::uint64_t multiply_mod(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
  std::uint64_t result = 0; a %= modulus;
  while (b) { if (b & 1U) result = add_mod(result, a, modulus); b >>= 1U; if (b) a = add_mod(a, a, modulus); }
  return result;
}
std::uint64_t power_mod(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus) {
  std::uint64_t result = 1 % modulus;
  while (exponent) { if (exponent & 1U) result = multiply_mod(result, base, modulus); exponent >>= 1U; if (exponent) base = multiply_mod(base, base, modulus); }
  return result;
}

Graph analyze(std::uint64_t modulus, std::uint64_t exponent) {
  Graph graph; graph.nodes.resize(static_cast<std::size_t>(modulus));
  for (std::uint64_t x = 0; x < modulus; ++x) graph.nodes[x].next = power_mod(x, exponent, modulus);
  std::vector<int> position(static_cast<std::size_t>(modulus), -1);
  for (std::uint64_t start = 0; start < modulus; ++start) {
    if (graph.nodes[start].component >= 0) continue;
    std::vector<std::uint64_t> path; std::uint64_t current = start;
    while (graph.nodes[current].component < 0 && position[current] < 0) {
      position[current] = static_cast<int>(path.size()); path.push_back(current); current = graph.nodes[current].next;
    }
    int prefix_end = static_cast<int>(path.size());
    if (graph.nodes[current].component < 0) {
      const int cycle_begin = position[current];
      const int cycle_length = static_cast<int>(path.size()) - cycle_begin;
      const int component = graph.cycles++;
      graph.longest_cycle = std::max(graph.longest_cycle, cycle_length);
      for (int j = cycle_begin; j < static_cast<int>(path.size()); ++j) {
        auto& node = graph.nodes[path[j]]; node.component = component; node.depth = 0; node.cycle_length = cycle_length; node.cycle = true;
      }
      prefix_end = cycle_begin;
    }
    for (int j = prefix_end - 1; j >= 0; --j) {
      auto& node = graph.nodes[path[j]]; const auto& next = graph.nodes[node.next];
      node.component = next.component; node.depth = next.depth + 1; node.cycle_length = next.cycle_length;
      graph.maximum_depth = std::max(graph.maximum_depth, node.depth);
    }
    for (const auto vertex : path) position[vertex] = -1;
  }
  graph.basin_sizes.assign(static_cast<std::size_t>(graph.cycles), 0);
  for (const auto& node : graph.nodes) ++graph.basin_sizes[static_cast<std::size_t>(node.component)];
  return graph;
}

void verify(const Graph& graph) {
  std::vector<int> cycle_vertices(graph.basin_sizes.size(), 0);
  for (std::size_t x = 0; x < graph.nodes.size(); ++x) {
    const auto& node = graph.nodes[x];
    if (node.next >= graph.nodes.size() || node.component < 0 || node.component >= graph.cycles) throw std::runtime_error("classification range invariant failed");
    const auto& next = graph.nodes[node.next];
    if (next.component != node.component) throw std::runtime_error("component invariant failed");
    if (node.cycle) { if (node.depth != 0 || !next.cycle) throw std::runtime_error("cycle invariant failed"); ++cycle_vertices[node.component]; }
    else if (node.depth != next.depth + 1) throw std::runtime_error("depth invariant failed");
  }
  for (std::size_t c = 0; c < cycle_vertices.size(); ++c)
    if (cycle_vertices[c] <= 0 || cycle_vertices[c] > graph.basin_sizes[c]) throw std::runtime_error("basin invariant failed");
}

double mapped_depth(int depth, int maximum, const std::string& scale) {
  if (maximum == 0) return 1;
  const double ratio = static_cast<double>(depth) / maximum;
  if (scale == "sqrt") return std::sqrt(ratio);
  if (scale == "log") return std::log1p(depth) / std::log1p(maximum);
  return ratio;
}

std::string basin_color(int component) {
  static const std::vector<std::string> colors{"#f6d35f", "#70e1f5", "#e98f72", "#9ed68f", "#bd9fe8", "#f2a7c4"};
  return colors[static_cast<std::size_t>(component) % colors.size()];
}

void self_test() {
  const auto graph = analyze(5, 2); verify(graph);
  const std::vector<std::uint64_t> expected{0,1,4,4,1};
  for (std::size_t x = 0; x < expected.size(); ++x) if (graph.nodes[x].next != expected[x]) throw std::runtime_error("n=5 successor test failed");
  if (graph.cycles != 2 || graph.longest_cycle != 1 || graph.maximum_depth != 2) throw std::runtime_error("n=5 summary test failed");
  if (!graph.nodes[0].cycle || !graph.nodes[1].cycle || graph.nodes[2].depth != 2 || graph.nodes[3].depth != 2 || graph.nodes[4].depth != 1)
    throw std::runtime_error("n=5 classification test failed");
  std::cout << "Self-test passed: n=5 successors, cycles, depths, and graph invariants\n";
}
}

int main(int argc, char** argv) {
  try {
    const Options o = parse(argc, argv); if (o.self_test) { self_test(); return 0; }
    const Graph graph = analyze(o.modulus, o.exponent); verify(graph);
    const double usable = std::min(o.width, o.height) - 2 * o.margin;
    if (usable <= 0) throw std::runtime_error("margin leaves no drawing area");
    const double cx = o.width / 2.0, cy = o.height / 2.0, radius = usable / 2.0;
    auto point = [&](std::uint64_t x, double r = 1.0) {
      const double angle = -std::numbers::pi / 2 + 2 * std::numbers::pi * x / o.modulus;
      return std::pair{cx + radius*r*std::cos(angle), cy + radius*r*std::sin(angle)};
    };
    std::vector<bool> highlighted(static_cast<std::size_t>(o.modulus), false);
    if (o.highlight_start >= 0) {
      std::uint64_t x = static_cast<std::uint64_t>(o.highlight_start);
      while (!highlighted[x]) { highlighted[x] = true; x = graph.nodes[x].next; }
    }
    if (o.output.has_parent_path()) std::filesystem::create_directories(o.output.parent_path());
    std::ofstream svg{o.output}; if (!svg) throw std::runtime_error("cannot open output");
    svg << std::fixed << std::setprecision(2)
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << o.width << "\" height=\"" << o.height << "\" viewBox=\"0 0 " << o.width << ' ' << o.height << "\">\n"
        << "<defs><marker id=\"arrow\" markerWidth=\"8\" markerHeight=\"8\" refX=\"7\" refY=\"4\" orient=\"auto\"><path d=\"M0 0L8 4L0 8z\" fill=\"context-stroke\"/></marker></defs>\n"
        << "<rect width=\"100%\" height=\"100%\" fill=\"#080b14\"/>\n";
    for (std::uint64_t x = 0; x < o.modulus; ++x) {
      const auto [x1,y1] = point(x); const auto [x2,y2] = point(graph.nodes[x].next);
      const bool hi = highlighted[x]; const bool cycle = graph.nodes[x].cycle;
      const std::string color = hi ? "#70e1f5" : (cycle ? "#f6d35f" : "#52617d");
      const double alpha = hi ? 0.95 : (cycle ? 0.75 : o.edge_alpha);
      if (x == graph.nodes[x].next)
        svg << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << o.vertex_size*2.4 << "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << (hi?5:3) << "\" opacity=\"" << alpha << "\"/>\n";
      else svg << "<path d=\"M" << x1 << ' ' << y1 << " Q" << cx << ' ' << cy << ' ' << x2 << ' ' << y2 << "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << (hi?5:(cycle?3:2)) << "\" opacity=\"" << alpha << "\" marker-end=\"url(#arrow)\"/>\n";
    }
    for (std::uint64_t x = 0; x < o.modulus; ++x) {
      const auto [px,py] = point(x); const auto& node = graph.nodes[x];
      std::string color = node.cycle ? "#f6d35f" : "#8290aa";
      double alpha = node.cycle ? 1.0 : 0.45 + 0.45*(1.0-mapped_depth(node.depth, graph.maximum_depth, o.depth_scale));
      if (o.encoding == "basin") { color = basin_color(node.component); alpha = node.cycle ? 1.0 : 0.72; }
      if (o.encoding == "role") alpha = node.cycle ? 1.0 : 0.55;
      if (highlighted[x]) { color = "#70e1f5"; alpha = 1.0; }
      svg << "<circle cx=\"" << px << "\" cy=\"" << py << "\" r=\"" << o.vertex_size*(node.cycle?1.45:1.0) << "\" fill=\"" << color << "\" opacity=\"" << alpha << "\"/>\n";
      if (o.show_labels) {
        const auto [tx,ty] = point(x, 1.08);
        svg << "<text x=\"" << tx << "\" y=\"" << ty+5 << "\" fill=\"#f3f5fa\" font-family=\"monospace\" font-size=\"22\" text-anchor=\"middle\">" << x << "</text>\n";
      }
    }
    svg << "</svg>\n";
    std::cout << "Rendered " << o.modulus << " residues; cycles " << graph.cycles
              << "; longest cycle " << graph.longest_cycle << "; maximum depth " << graph.maximum_depth << '\n';
  } catch (const std::exception& error) { std::cerr << "error: " << error.what() << '\n'; return 1; }
}
