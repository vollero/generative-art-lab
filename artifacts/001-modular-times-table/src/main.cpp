#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace {

struct Options {
  std::filesystem::path output{"modular-times-table.svg"};
  int points{360};
  double multiplier{2.0};
  int width{2160};
  int height{2160};
  double radius{0.46};
  double line_width{0.65};
  double line_alpha{0.16};
  std::string background{"#080b14"};
  std::string foreground{"#70e1f5"};
  std::string accent{"#ffd166"};
  int visible_chords{-1};
  int highlight{-1};
  bool show_points{false};
  bool show_labels{false};
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
  std::cout << R"(Modular Times Table — Artifact 001

Draw n points around a circle and connect i to (multiplier * i) mod n.
Fractional multipliers are supported for smooth animation frame sequences.

Usage:
  modular_times_table [options]

Options:
  --output PATH          SVG output path (default: modular-times-table.svg)
  --points N             Number of chords, 3..200000 (default: 360)
  --multiplier M         Modular multiplier, >= 0 (default: 2)
  --width PX             Canvas width, 64..16384 (default: 2160)
  --height PX            Canvas height, 64..16384 (default: 2160)
  --radius R             Circle radius as fraction of shorter side, 0..0.5
  --line-width PX        Chord width, > 0 (default: 0.65)
  --line-alpha A         Chord opacity, 0..1 (default: 0.16)
  --background COLOR     SVG/CSS background color (default: #080b14)
  --foreground COLOR     SVG/CSS chord color (default: #70e1f5)
  --accent COLOR         Current-chord color (default: #ffd166)
  --chords N             Draw only the first N chords (default: all)
  --highlight INDEX      Accent one source chord (default: none)
  --show-points          Draw point markers around the circle
  --show-labels          Draw numeric point labels (at most 200 points)
  --help                 Show this message
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
    } else if (argument == "--points") {
      options.points = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--multiplier") {
      options.multiplier =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--width") {
      options.width = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--height") {
      options.height = parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--radius") {
      options.radius = parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--line-width") {
      options.line_width =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--line-alpha") {
      options.line_alpha =
          parse_number<double>(next_value(index, argc, argv), argument);
    } else if (argument == "--background") {
      options.background = next_value(index, argc, argv);
    } else if (argument == "--foreground") {
      options.foreground = next_value(index, argc, argv);
    } else if (argument == "--accent") {
      options.accent = next_value(index, argc, argv);
    } else if (argument == "--chords") {
      options.visible_chords =
          parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--highlight") {
      options.highlight =
          parse_number<int>(next_value(index, argc, argv), argument);
    } else if (argument == "--show-points") {
      options.show_points = true;
    } else if (argument == "--show-labels") {
      options.show_labels = true;
    } else {
      fail("Unknown option: " + std::string(argument));
    }
  }
  return options;
}

void validate(const Options& options) {
  if (options.points < 3 || options.points > 200'000) {
    fail("--points must be between 3 and 200000");
  }
  if (!std::isfinite(options.multiplier) || options.multiplier < 0.0) {
    fail("--multiplier must be a finite number greater than or equal to 0");
  }
  if (options.width < 64 || options.width > 16'384 || options.height < 64 ||
      options.height > 16'384) {
    fail("--width and --height must be between 64 and 16384");
  }
  if (!std::isfinite(options.radius) || options.radius <= 0.0 ||
      options.radius > 0.5) {
    fail("--radius must be greater than 0 and no greater than 0.5");
  }
  if (!std::isfinite(options.line_width) || options.line_width <= 0.0) {
    fail("--line-width must be greater than 0");
  }
  if (!std::isfinite(options.line_alpha) || options.line_alpha < 0.0 ||
      options.line_alpha > 1.0) {
    fail("--line-alpha must be between 0 and 1");
  }
  if (options.output.empty()) {
    fail("--output must not be empty");
  }
  if (options.visible_chords < -1 || options.visible_chords > options.points) {
    fail("--chords must be between 0 and --points");
  }
  if (options.highlight < -1 || options.highlight >= options.points) {
    fail("--highlight must identify a source point or be omitted");
  }
  if (options.show_labels && options.points > 200) {
    fail("--show-labels supports at most 200 points to keep labels legible");
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

struct Point {
  double x;
  double y;
};

Point point_on_circle(double index, int count, double center_x,
                      double center_y, double radius) {
  const double phase = -std::numbers::pi / 2.0;
  const double angle = std::numbers::pi * 2.0 * index /
                           static_cast<double>(count) +
                       phase;
  return {center_x + radius * std::cos(angle),
          center_y + radius * std::sin(angle)};
}

void render(const Options& options) {
  if (options.output.has_parent_path()) {
    std::filesystem::create_directories(options.output.parent_path());
  }

  std::ofstream output(options.output, std::ios::binary);
  if (!output) {
    fail("Could not open output file: " + options.output.string());
  }

  const double center_x = static_cast<double>(options.width) / 2.0;
  const double center_y = static_cast<double>(options.height) / 2.0;
  const double radius = static_cast<double>(std::min(options.width, options.height)) *
                        options.radius;

  const int chord_count = options.visible_chords < 0
                              ? options.points
                              : options.visible_chords;
  output << std::fixed << std::setprecision(3);
  output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
         << options.width << "\" height=\"" << options.height
         << "\" viewBox=\"0 0 " << options.width << ' ' << options.height
         << "\" role=\"img\">\n"
         << "  <title>Modular times table: " << options.points
         << " points, multiplier " << options.multiplier << "</title>\n"
         << "  <rect width=\"100%\" height=\"100%\" fill=\""
         << xml_escape(options.background) << "\"/>\n"
         << "  <g fill=\"none\" stroke=\"" << xml_escape(options.foreground)
         << "\" stroke-width=\"" << options.line_width
         << "\" stroke-opacity=\"" << options.line_alpha
         << "\" stroke-linecap=\"round\">\n";

  for (int source_index = 0; source_index < chord_count; ++source_index) {
    const double destination_index =
        std::fmod(options.multiplier * static_cast<double>(source_index),
                  static_cast<double>(options.points));
    const Point source = point_on_circle(source_index, options.points, center_x,
                                         center_y, radius);
    const Point destination = point_on_circle(destination_index, options.points,
                                              center_x, center_y, radius);
    if (source_index == options.highlight) {
      output << "    <line stroke=\"" << xml_escape(options.accent)
             << "\" stroke-opacity=\"1\" stroke-width=\""
             << options.line_width * 3.0 << "\" x1=\"" << source.x
             << "\" y1=\"" << source.y << "\" x2=\"" << destination.x
             << "\" y2=\"" << destination.y << "\"/>\n";
      continue;
    }
    output << "    <line x1=\"" << source.x << "\" y1=\"" << source.y
           << "\" x2=\"" << destination.x << "\" y2=\"" << destination.y
           << "\"/>\n";
  }

  output << "  </g>\n";

  if (options.show_points || options.show_labels) {
    const double marker_radius =
        std::max(2.5, static_cast<double>(std::min(options.width, options.height)) /
                          360.0);
    const double label_radius = radius + marker_radius * 5.0;
    const double label_size =
        std::max(12.0, static_cast<double>(std::min(options.width, options.height)) /
                           45.0);
    output << "  <g fill=\"" << xml_escape(options.foreground) << "\">\n";
    for (int index = 0; index < options.points; ++index) {
      const Point point = point_on_circle(index, options.points, center_x,
                                          center_y, radius);
      if (options.show_points) {
        output << "    <circle cx=\"" << point.x << "\" cy=\"" << point.y
               << "\" r=\"" << marker_radius << "\"/>\n";
      }
      if (options.show_labels) {
        const Point label = point_on_circle(index, options.points, center_x,
                                            center_y, label_radius);
        output << "    <text x=\"" << label.x << "\" y=\"" << label.y
               << "\" text-anchor=\"middle\" dominant-baseline=\"central\""
               << " font-family=\"ui-monospace,monospace\" font-size=\""
               << label_size << "\">" << index << "</text>\n";
      }
    }
    output << "  </g>\n";
  }

  output << "</svg>\n";
  if (!output) {
    fail("Failed while writing output file: " + options.output.string());
  }
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const Options options = parse_options(argc, argv);
    validate(options);
    render(options);
    const int chord_count = options.visible_chords < 0
                                ? options.points
                                : options.visible_chords;
    std::cout << "Rendered " << chord_count << " chords to "
              << options.output << '\n';
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
