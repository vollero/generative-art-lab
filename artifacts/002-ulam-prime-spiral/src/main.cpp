#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

struct Options {
  std::filesystem::path output{"ulam-prime-spiral.svg"};
  int side{1001};
  std::uint64_t start{1};
  int width{2160};
  int height{2160};
  double margin{0.045};
  double radius{0.34};
  std::string background{"#070912"};
  std::string prime_color{"#f4d35e"};
  std::string composite_color{"#27324a"};
  double composite_alpha{0.0};
  std::uint64_t visible_cells{0};
  bool show_values{false};
};

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message + "\nRun with --help for usage.");
}

template <typename T>
T parse_number(std::string_view text, std::string_view option) {
  T value{};
  const auto* begin = text.data();
  const auto* end = begin + text.size();
  const auto [position, error] = std::from_chars(begin, end, value);
  if (error != std::errc{} || position != end) {
    fail("Invalid number for " + std::string(option) + ": " + std::string(text));
  }
  return value;
}

std::string_view next_value(int& index, int argc, char** argv) {
  if (++index >= argc) {
    fail("Missing value for " + std::string(argv[index - 1]));
  }
  return argv[index];
}

void print_help() {
  std::cout << R"(Ulam Prime Spiral — Artifact 002

Arrange consecutive integers on a square spiral and mark the primes.

Usage:
  ulam_prime_spiral [options]

Options:
  --output PATH            SVG output path (default: ulam-prime-spiral.svg)
  --side N                 Odd grid side, 3..3001 (default: 1001)
  --start N                Center value, 0..10000000 (default: 1)
  --width PX               Canvas width, 64..16384 (default: 2160)
  --height PX              Canvas height, 64..16384 (default: 2160)
  --margin R               Fractional canvas margin, 0..0.25 (default: 0.045)
  --radius R               Dot radius as cell fraction, 0..0.5 (default: 0.34)
  --background COLOR       SVG/CSS background color (default: #070912)
  --prime-color COLOR      Prime point color (default: #f4d35e)
  --composite-color COLOR  Composite point color (default: #27324a)
  --composite-alpha A      Composite opacity, 0..1 (default: 0)
  --cells N                Draw first N cells, 0 means all (default: 0)
  --show-values            Label integers; supported up to side 51
  --help                   Show this message
)";
}

Options parse_options(int argc, char** argv) {
  Options options;
  for (int index = 1; index < argc; ++index) {
    const std::string_view argument{argv[index]};
    if (argument == "--help") {
      print_help();
      std::exit(EXIT_SUCCESS);
    } else if (argument == "--output") {
      options.output = next_value(index, argc, argv);
    } else if (argument == "--side") {
      options.side = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--start") {
      options.start =
          parse_number<std::uint64_t>(next_value(index, argc, argv), argument);
    } else if (argument == "--width") {
      options.width = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--height") {
      options.height = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--margin") {
      options.margin =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--radius") {
      options.radius =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--background") {
      options.background = next_value(index, argc, argv);
    } else if (argument == "--prime-color") {
      options.prime_color = next_value(index, argc, argv);
    } else if (argument == "--composite-color") {
      options.composite_color = next_value(index, argc, argv);
    } else if (argument == "--composite-alpha") {
      options.composite_alpha =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--cells") {
      options.visible_cells = parse_number<std::uint64_t>(
          next_value(index, argc, argv), argument);
    } else if (argument == "--show-values") {
      options.show_values = true;
    } else {
      fail("Unknown option: " + std::string(argument));
    }
  }
  return options;
}

void validate(const Options& options) {
  if (options.side < 3 || options.side > 3001 || options.side % 2 == 0) {
    fail("--side must be an odd integer between 3 and 3001");
  }
  if (options.start > 10'000'000) {
    fail("--start must be no greater than 10000000");
  }
  if (options.width < 64 || options.width > 16'384 || options.height < 64 ||
      options.height > 16'384) {
    fail("--width and --height must be between 64 and 16384");
  }
  if (!std::isfinite(options.margin) || options.margin < 0.0 ||
      options.margin > 0.25) {
    fail("--margin must be between 0 and 0.25");
  }
  if (!std::isfinite(options.radius) || options.radius <= 0.0 ||
      options.radius > 0.5) {
    fail("--radius must be greater than 0 and no greater than 0.5");
  }
  if (!std::isfinite(options.composite_alpha) || options.composite_alpha < 0.0 ||
      options.composite_alpha > 1.0) {
    fail("--composite-alpha must be between 0 and 1");
  }
  if (options.output.empty()) {
    fail("--output must not be empty");
  }
  const std::uint64_t cell_count = static_cast<std::uint64_t>(options.side) *
                                   static_cast<std::uint64_t>(options.side);
  if (options.visible_cells > cell_count) {
    fail("--cells must be 0 or no greater than side squared");
  }
  if (options.show_values && options.side > 51) {
    fail("--show-values supports side lengths no greater than 51");
  }
}

std::string xml_escape(std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const char character : value) {
    switch (character) {
      case '&': result += "&amp;"; break;
      case '"': result += "&quot;"; break;
      case '<': result += "&lt;"; break;
      case '>': result += "&gt;"; break;
      default: result += character;
    }
  }
  return result;
}

std::vector<bool> prime_sieve(std::size_t limit) {
  std::vector<bool> prime(limit + 1, true);
  prime[0] = false;
  if (limit >= 1) prime[1] = false;
  for (std::size_t factor = 2; factor <= limit / factor; ++factor) {
    if (!prime[factor]) continue;
    for (std::size_t multiple = factor * factor; multiple <= limit;
         multiple += factor) {
      prime[multiple] = false;
    }
  }
  return prime;
}

struct SpiralCursor {
  int x{0};
  int y{0};
  int dx{1};
  int dy{0};
  int segment_length{1};
  int segment_progress{0};
  int segments_at_length{0};

  void advance() {
    x += dx;
    y += dy;
    if (++segment_progress != segment_length) return;
    segment_progress = 0;
    const int old_dx = dx;
    dx = dy;
    dy = -old_dx;
    if (++segments_at_length == 2) {
      segments_at_length = 0;
      ++segment_length;
    }
  }
};

void render(const Options& options) {
  const std::size_t cell_count =
      static_cast<std::size_t>(options.side) * static_cast<std::size_t>(options.side);
  const std::uint64_t final_value = options.start + cell_count - 1;
  if (final_value > std::numeric_limits<std::size_t>::max()) {
    fail("Requested number range exceeds this platform's address space");
  }
  const auto primes = prime_sieve(static_cast<std::size_t>(final_value));

  if (options.output.has_parent_path()) {
    std::filesystem::create_directories(options.output.parent_path());
  }
  std::ofstream output(options.output, std::ios::binary);
  if (!output) fail("Could not open output file: " + options.output.string());

  const double shorter = static_cast<double>(std::min(options.width, options.height));
  const double extent = shorter * (1.0 - options.margin * 2.0);
  const double cell = extent / static_cast<double>(options.side);
  const double radius = cell * options.radius;
  const double label_size = cell * 0.38;
  const double center_x = static_cast<double>(options.width) / 2.0;
  const double center_y = static_cast<double>(options.height) / 2.0;

  output << std::fixed << std::setprecision(3)
         << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
         << options.width << "\" height=\"" << options.height
         << "\" viewBox=\"0 0 " << options.width << ' ' << options.height
         << "\" role=\"img\">\n"
         << "  <title>Ulam prime spiral from " << options.start << " to "
         << final_value << "</title>\n"
         << "  <rect width=\"100%\" height=\"100%\" fill=\""
         << xml_escape(options.background) << "\"/>\n";

  const std::size_t visible_count = options.visible_cells == 0
                                        ? cell_count
                                        : static_cast<std::size_t>(options.visible_cells);
  SpiralCursor cursor;
  std::size_t prime_count = 0;
  for (std::size_t index = 0; index < visible_count; ++index) {
    const std::size_t value = static_cast<std::size_t>(options.start + index);
    const bool is_prime = primes[value];
    if (is_prime) ++prime_count;
    if (is_prime || options.composite_alpha > 0.0) {
      const double x = center_x + static_cast<double>(cursor.x) * cell;
      const double y = center_y + static_cast<double>(cursor.y) * cell;
      output << "  <circle cx=\"" << x << "\" cy=\"" << y << "\" r=\""
             << radius << "\" fill=\""
             << xml_escape(is_prime ? options.prime_color : options.composite_color)
             << "\"";
      if (!is_prime) output << " fill-opacity=\"" << options.composite_alpha << "\"";
      output << "/>\n";
    }
    if (options.show_values) {
      const double x = center_x + static_cast<double>(cursor.x) * cell;
      const double y = center_y + static_cast<double>(cursor.y) * cell;
      output << "  <text x=\"" << x << "\" y=\"" << y
             << "\" fill=\""
             << xml_escape(is_prime ? options.prime_color : options.composite_color)
             << "\" text-anchor=\"middle\" dominant-baseline=\"central\""
             << " font-family=\"ui-monospace,monospace\" font-size=\""
             << label_size << "\">" << value << "</text>\n";
    }
    cursor.advance();
  }
  output << "</svg>\n";
  if (!output) fail("Failed while writing output file: " + options.output.string());
  const std::uint64_t visible_final = options.start + visible_count - 1;
  std::cout << "Rendered " << prime_count << " primes from " << options.start
            << " to " << visible_final << " into " << options.output << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const Options options = parse_options(argc, argv);
    validate(options);
    render(options);
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
