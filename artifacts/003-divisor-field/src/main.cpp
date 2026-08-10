#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
struct Options {
  std::string output{"out/003-divisor-field.svg"};
  std::uint64_t start{1};
  std::uint64_t count{65536};
  std::uint64_t columns{256};
  int width{2160};
  int height{2160};
  double margin{80.0};
  double gap{0.08};
  std::string scale{"sqrt"};
  std::uint32_t ceiling{0};
  std::string background{"#080b14"};
  std::string foreground{"#f6d35f"};
  bool show_records{false};
  bool show_values{false};
};

template <typename T> T number(std::string_view text, std::string_view name) {
  std::string value{text};
  std::size_t used = 0;
  try {
    if constexpr (std::is_integral_v<T>) {
      const auto parsed = std::stoull(value, &used);
      if (used != value.size() || parsed > std::numeric_limits<T>::max()) throw std::invalid_argument("range");
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
    if (arg == "--output") o.output = next();
    else if (arg == "--start") o.start = number<std::uint64_t>(next(), "start");
    else if (arg == "--count") o.count = number<std::uint64_t>(next(), "count");
    else if (arg == "--columns") o.columns = number<std::uint64_t>(next(), "columns");
    else if (arg == "--width") o.width = number<int>(next(), "width");
    else if (arg == "--height") o.height = number<int>(next(), "height");
    else if (arg == "--margin") o.margin = number<double>(next(), "margin");
    else if (arg == "--gap") o.gap = number<double>(next(), "gap");
    else if (arg == "--scale") o.scale = next();
    else if (arg == "--ceiling") o.ceiling = number<std::uint32_t>(next(), "ceiling");
    else if (arg == "--background") o.background = next();
    else if (arg == "--foreground") o.foreground = next();
    else if (arg == "--show-records") o.show_records = true;
    else if (arg == "--show-values") o.show_values = true;
    else if (arg == "--help") {
      std::cout << "divisor_field [--output FILE] [--start N] [--count N] [--columns N]\n"
                   "  [--width PX] [--height PX] [--margin PX] [--gap 0..0.9]\n"
                   "  [--scale linear|sqrt|log] [--ceiling N] [--show-records] [--show-values]\n";
      std::exit(0);
    } else throw std::runtime_error("unknown option: " + arg);
  }
  if (o.start == 0 || o.count == 0 || o.columns == 0) throw std::runtime_error("start, count, and columns must be positive");
  if (o.start > std::numeric_limits<std::uint64_t>::max() - (o.count - 1)) throw std::runtime_error("integer range overflows");
  if (o.start + o.count - 1 > 10'000'000) throw std::runtime_error("largest integer must not exceed 10000000");
  if (o.width <= 0 || o.height <= 0 || o.margin < 0.0) throw std::runtime_error("invalid canvas geometry");
  if (o.gap < 0.0 || o.gap >= 0.9) throw std::runtime_error("gap must be in [0, 0.9)");
  if (o.scale != "linear" && o.scale != "sqrt" && o.scale != "log") throw std::runtime_error("scale must be linear, sqrt, or log");
  if (o.show_values && o.count > 400) throw std::runtime_error("show-values requires count <= 400");
  return o;
}

std::vector<std::uint32_t> divisor_counts(std::uint64_t limit) {
  std::vector<std::uint32_t> counts(static_cast<std::size_t>(limit + 1), 0);
  for (std::uint64_t divisor = 1; divisor <= limit; ++divisor)
    for (std::uint64_t multiple = divisor; multiple <= limit; multiple += divisor)
      ++counts[static_cast<std::size_t>(multiple)];
  return counts;
}

double mapped(std::uint32_t value, std::uint32_t ceiling, const std::string& scale) {
  const double ratio = std::min(1.0, static_cast<double>(value) / ceiling);
  if (scale == "sqrt") return std::sqrt(ratio);
  if (scale == "log") return std::log1p(value) / std::log1p(ceiling);
  return ratio;
}
}

int main(int argc, char** argv) {
  try {
    const Options o = parse(argc, argv);
    const std::uint64_t last = o.start + o.count - 1;
    const auto counts = divisor_counts(last);
    std::uint32_t maximum = 0;
    std::uint64_t maximum_at = o.start;
    for (std::uint64_t n = o.start; n <= last; ++n) {
      if (counts[n] > maximum) { maximum = counts[n]; maximum_at = n; }
    }
    const std::uint32_t ceiling = o.ceiling == 0 ? maximum : o.ceiling;
    const std::uint64_t rows = (o.count + o.columns - 1) / o.columns;
    const double usable_w = o.width - 2.0 * o.margin;
    const double usable_h = o.height - 2.0 * o.margin;
    if (usable_w <= 0.0 || usable_h <= 0.0) throw std::runtime_error("margin leaves no drawing area");
    const double cell = std::min(usable_w / o.columns, usable_h / rows);
    const double field_w = cell * o.columns;
    const double field_h = cell * rows;
    const double x0 = (o.width - field_w) / 2.0;
    const double y0 = (o.height - field_h) / 2.0;

    std::ofstream svg{o.output};
    if (!svg) throw std::runtime_error("cannot open output: " + o.output);
    svg << std::fixed << std::setprecision(3)
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << o.width
        << "\" height=\"" << o.height << "\" viewBox=\"0 0 " << o.width << ' ' << o.height << "\">\n"
        << "<rect width=\"100%\" height=\"100%\" fill=\"" << o.background << "\"/>\n";
    std::vector<std::ostringstream> paths(static_cast<std::size_t>(maximum + 1));
    struct RecordCell { double x; double y; double size; };
    std::vector<RecordCell> records;
    std::uint32_t record = 0;
    for (std::uint64_t index = 0; index < o.count; ++index) {
      const std::uint64_t n = o.start + index;
      const auto value = counts[n];
      const double x = x0 + (index % o.columns) * cell + cell * o.gap * 0.5;
      const double y = y0 + (index / o.columns) * cell + cell * o.gap * 0.5;
      const double size = cell * (1.0 - o.gap);
      paths[value] << std::fixed << std::setprecision(3) << 'M' << x << ' ' << y
                   << 'h' << size << 'v' << size << 'h' << -size << 'z';
      if (o.show_records && value > record) {
        record = value;
        records.push_back({x, y, size});
      }
    }
    for (std::uint32_t value = 1; value <= maximum; ++value) {
      const std::string path = paths[value].str();
      if (path.empty()) continue;
      const double alpha = 0.08 + 0.92 * mapped(value, ceiling, o.scale);
      svg << "<path fill=\"" << o.foreground << "\" opacity=\"" << alpha
          << "\" d=\"" << path << "\"/>\n";
    }
    if (!records.empty()) {
      svg << "<g fill=\"none\" stroke=\"#ffffff\" stroke-width=\""
          << std::max(0.5, cell * 0.08) << "\">\n";
      for (const auto& item : records)
        svg << "<rect x=\"" << item.x << "\" y=\"" << item.y << "\" width=\""
            << item.size << "\" height=\"" << item.size << "\"/>\n";
      svg << "</g>\n";
    }
    if (o.show_values) {
      svg << "<g fill=\"#f3f5fa\" text-anchor=\"middle\" font-family=\"monospace\">\n";
      for (std::uint64_t index = 0; index < o.count; ++index) {
        const std::uint64_t n = o.start + index;
        const double x = x0 + (index % o.columns) * cell + cell * 0.5;
        const double y = y0 + (index / o.columns) * cell + cell * 0.43;
        svg << "<text x=\"" << x << "\" y=\"" << y << "\" font-size=\""
            << cell * 0.24 << "\">" << n << "</text>\n"
            << "<text x=\"" << x << "\" y=\"" << y + cell * 0.25
            << "\" font-size=\"" << cell * 0.18 << "\">d=" << counts[n] << "</text>\n";
      }
      svg << "</g>\n";
    }
    svg << "</svg>\n";
    std::cout << "Rendered " << o.count << " integers; maximum divisor count "
              << maximum << " at " << maximum_at << "\n";
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
