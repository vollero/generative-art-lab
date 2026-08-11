#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
struct Options {
  std::filesystem::path output{"out/004-coprime-lattice.svg"};
  int extent{200};
  int width{2160};
  int height{2160};
  double margin{80.0};
  double point_size{0.72};
  std::string encoding{"visibility"};
  std::string scale{"sqrt"};
  int ceiling{0};
  double blocked_alpha{0.08};
  std::string background{"#080b14"};
  std::string visible_color{"#f6d35f"};
  std::string blocked_color{"#27324a"};
  bool show_coordinates{false};
};

template <typename T> T number(std::string_view text, std::string_view name) {
  std::string value{text};
  std::size_t used = 0;
  try {
    if constexpr (std::is_integral_v<T>) {
      const auto parsed = std::stoll(value, &used);
      if (used != value.size() || parsed < std::numeric_limits<T>::min() || parsed > std::numeric_limits<T>::max())
        throw std::invalid_argument("range");
      return static_cast<T>(parsed);
    } else {
      const auto parsed = std::stod(value, &used);
      if (used != value.size()) throw std::invalid_argument("trailing");
      return static_cast<T>(parsed);
    }
  } catch (...) { throw std::runtime_error("invalid value for --" + std::string{name}); }
}

Options parse(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&]() -> std::string_view {
      if (++i >= argc) throw std::runtime_error("missing value after " + arg);
      return argv[i];
    };
    if (arg == "--output") o.output = std::string{next()};
    else if (arg == "--extent") o.extent = number<int>(next(), "extent");
    else if (arg == "--width") o.width = number<int>(next(), "width");
    else if (arg == "--height") o.height = number<int>(next(), "height");
    else if (arg == "--margin") o.margin = number<double>(next(), "margin");
    else if (arg == "--point-size") o.point_size = number<double>(next(), "point-size");
    else if (arg == "--encoding") o.encoding = next();
    else if (arg == "--scale") o.scale = next();
    else if (arg == "--ceiling") o.ceiling = number<int>(next(), "ceiling");
    else if (arg == "--blocked-alpha") o.blocked_alpha = number<double>(next(), "blocked-alpha");
    else if (arg == "--background") o.background = next();
    else if (arg == "--visible-color") o.visible_color = next();
    else if (arg == "--blocked-color") o.blocked_color = next();
    else if (arg == "--show-coordinates") o.show_coordinates = true;
    else if (arg == "--help") {
      std::cout << "coprime_lattice [--output FILE] [--extent N] [--encoding visibility|gcd]\n"
                   "  [--width PX] [--height PX] [--margin PX] [--point-size 0..1]\n"
                   "  [--blocked-alpha 0..1] [--scale linear|sqrt|log] [--ceiling N]\n"
                   "  [--show-coordinates]\n";
      std::exit(0);
    } else throw std::runtime_error("unknown option: " + arg);
  }
  if (o.extent < 1 || o.extent > 2000) throw std::runtime_error("extent must be from 1 to 2000");
  if (o.width < 64 || o.height < 64 || o.margin < 0) throw std::runtime_error("invalid canvas geometry");
  if (o.point_size <= 0 || o.point_size > 1) throw std::runtime_error("point-size must be in (0,1]");
  if (o.blocked_alpha < 0 || o.blocked_alpha > 1) throw std::runtime_error("blocked-alpha must be in [0,1]");
  if (o.encoding != "visibility" && o.encoding != "gcd") throw std::runtime_error("encoding must be visibility or gcd");
  if (o.scale != "linear" && o.scale != "sqrt" && o.scale != "log") throw std::runtime_error("scale must be linear, sqrt, or log");
  if (o.ceiling < 0) throw std::runtime_error("ceiling must be nonnegative");
  if (o.show_coordinates && o.extent > 10) throw std::runtime_error("show-coordinates requires extent <= 10");
  return o;
}

double mapped(int value, int ceiling, const std::string& scale) {
  const double ratio = std::min(1.0, static_cast<double>(value) / ceiling);
  if (scale == "sqrt") return std::sqrt(ratio);
  if (scale == "log") return std::log1p(value) / std::log1p(ceiling);
  return ratio;
}
}

int main(int argc, char** argv) {
  try {
    const Options o = parse(argc, argv);
    const int side = 2 * o.extent + 1;
    const std::uint64_t total = static_cast<std::uint64_t>(side) * side - 1;
    std::uint64_t visible = 0;
    int maximum_gcd = 1;
    for (int y = -o.extent; y <= o.extent; ++y)
      for (int x = -o.extent; x <= o.extent; ++x) {
        if (x == 0 && y == 0) continue;
        const int gcd = std::gcd(std::abs(x), std::abs(y));
        if (gcd == 1) ++visible;
        maximum_gcd = std::max(maximum_gcd, gcd);
      }
    const int ceiling = o.ceiling == 0 ? maximum_gcd : o.ceiling;
    const double usable_w = o.width - 2.0 * o.margin;
    const double usable_h = o.height - 2.0 * o.margin;
    if (usable_w <= 0 || usable_h <= 0) throw std::runtime_error("margin leaves no drawing area");
    const double cell = std::min(usable_w / side, usable_h / side);
    const double field = cell * side;
    const double x0 = (o.width - field) / 2.0;
    const double y0 = (o.height - field) / 2.0;
    const double mark = cell * o.point_size;
    const double inset = (cell - mark) / 2.0;
    std::vector<std::ostringstream> paths(static_cast<std::size_t>(maximum_gcd + 1));
    for (int y = -o.extent; y <= o.extent; ++y)
      for (int x = -o.extent; x <= o.extent; ++x) {
        if (x == 0 && y == 0) continue;
        const int gcd = std::gcd(std::abs(x), std::abs(y));
        const double px = x0 + (x + o.extent) * cell + inset;
        const double py = y0 + (o.extent - y) * cell + inset;
        paths[gcd] << std::fixed << std::setprecision(3) << 'M' << px << ' ' << py
                   << 'h' << mark << 'v' << mark << 'h' << -mark << 'z';
      }
    if (o.output.has_parent_path()) std::filesystem::create_directories(o.output.parent_path());
    std::ofstream svg{o.output};
    if (!svg) throw std::runtime_error("cannot open output: " + o.output.string());
    svg << std::fixed << std::setprecision(3)
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << o.width
        << "\" height=\"" << o.height << "\" viewBox=\"0 0 " << o.width << ' ' << o.height << "\">\n"
        << "<rect width=\"100%\" height=\"100%\" fill=\"" << o.background << "\"/>\n";
    for (int gcd = maximum_gcd; gcd >= 1; --gcd) {
      const std::string path = paths[gcd].str();
      if (path.empty()) continue;
      const bool primitive = gcd == 1;
      const std::string& color = primitive ? o.visible_color : o.blocked_color;
      double opacity = primitive ? 1.0 : o.blocked_alpha;
      if (o.encoding == "gcd" && !primitive)
        opacity = o.blocked_alpha + (1.0 - o.blocked_alpha) * mapped(gcd, ceiling, o.scale);
      // Keep individual XML attributes modest in size. Some otherwise capable
      // SVG rasterizers reject a single multi-megabyte path data attribute.
      constexpr std::size_t marks_per_path = 4096;
      std::size_t begin = 0;
      std::size_t marks = 0;
      for (std::size_t cursor = 0; cursor < path.size(); ++cursor) {
        if (path[cursor] != 'M') continue;
        if (marks++ == marks_per_path) {
          svg << "<path fill=\"" << color << "\" opacity=\"" << opacity
              << "\" d=\"" << path.substr(begin, cursor - begin) << "\"/>\n";
          begin = cursor;
          marks = 1;
        }
      }
      svg << "<path fill=\"" << color << "\" opacity=\"" << opacity
          << "\" d=\"" << path.substr(begin) << "\"/>\n";
    }
    const double cx = x0 + o.extent * cell + cell / 2.0;
    const double cy = y0 + o.extent * cell + cell / 2.0;
    svg << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\""
        << std::max(2.0, mark * 0.55) << "\" fill=\"#70e1f5\"/>\n";
    if (o.show_coordinates) {
      svg << "<g fill=\"#f3f5fa\" font-family=\"monospace\" text-anchor=\"middle\" font-size=\""
          << cell * 0.20 << "\">\n";
      for (int y = -o.extent; y <= o.extent; ++y)
        for (int x = -o.extent; x <= o.extent; ++x) {
          if (x == 0 && y == 0) continue;
          svg << "<text x=\"" << x0 + (x + o.extent) * cell + cell / 2.0
              << "\" y=\"" << y0 + (o.extent - y) * cell + cell * 0.56
              << "\">" << x << ',' << y << "</text>\n";
        }
      svg << "</g>\n";
    }
    svg << "</svg>\n";
    const double fraction = static_cast<double>(visible) / total;
    std::cout << "Rendered " << total << " lattice points; visible " << visible
              << " (" << std::setprecision(6) << fraction << ")\n";
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
