#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
using Complex = std::complex<double>;
constexpr double tau = 2.0 * std::numbers::pi;

struct Term { int frequency; Complex coefficient; };
struct Options {
  std::filesystem::path output{"out/005-fourier-epicycles.svg"};
  std::filesystem::path input{};
  int samples{256};
  int terms{32};
  int width{2160};
  int height{2160};
  double margin{180.0};
  double time{0.72};
  double trace_length{1.0};
  std::string resampling{"arc-length"};
  std::string term_order{"amplitude"};
  bool show_circles{true};
  bool show_source{true};
  bool self_test{false};
};

template <typename T> T number(std::string_view text, std::string_view name) {
  std::string value{text}; std::size_t used = 0;
  try {
    if constexpr (std::is_integral_v<T>) {
      const auto parsed = std::stoll(value, &used);
      if (used != value.size() || parsed < std::numeric_limits<T>::min() || parsed > std::numeric_limits<T>::max())
        throw std::invalid_argument("range");
      return static_cast<T>(parsed);
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
    auto next = [&]() -> std::string_view {
      if (++i >= argc) throw std::runtime_error("missing value after " + arg);
      return argv[i];
    };
    if (arg == "--output") o.output = std::string{next()};
    else if (arg == "--input") o.input = std::string{next()};
    else if (arg == "--samples") o.samples = number<int>(next(), "samples");
    else if (arg == "--terms") o.terms = number<int>(next(), "terms");
    else if (arg == "--width") o.width = number<int>(next(), "width");
    else if (arg == "--height") o.height = number<int>(next(), "height");
    else if (arg == "--margin") o.margin = number<double>(next(), "margin");
    else if (arg == "--time") o.time = number<double>(next(), "time");
    else if (arg == "--trace-length") o.trace_length = number<double>(next(), "trace-length");
    else if (arg == "--resampling") o.resampling = std::string{next()};
    else if (arg == "--term-order") o.term_order = std::string{next()};
    else if (arg == "--hide-circles") o.show_circles = false;
    else if (arg == "--hide-source") o.show_source = false;
    else if (arg == "--self-test") o.self_test = true;
    else if (arg == "--help") {
      std::cout << "fourier_epicycles [--output FILE] [--input CSV] [--samples N] [--terms N]\n"
                   "  [--time 0..1] [--trace-length 0..1] [--resampling arc-length|input]\n"
                   "  [--term-order amplitude|frequency] [--hide-circles] [--hide-source]\n";
      std::exit(0);
    } else throw std::runtime_error("unknown option: " + arg);
  }
  if (o.samples < 4 || o.samples > 4096) throw std::runtime_error("samples must be from 4 to 4096");
  if (o.terms < 1) throw std::runtime_error("terms must be positive");
  if (o.width < 64 || o.height < 64 || o.margin < 0) throw std::runtime_error("invalid canvas geometry");
  if (o.time < 0 || o.time >= 1) throw std::runtime_error("time must be in [0,1)");
  if (o.trace_length < 0 || o.trace_length > 1) throw std::runtime_error("trace-length must be in [0,1]");
  if (o.resampling != "arc-length" && o.resampling != "input") throw std::runtime_error("invalid resampling");
  if (o.term_order != "amplitude" && o.term_order != "frequency") throw std::runtime_error("invalid term-order");
  return o;
}

std::vector<Complex> heart(int count) {
  std::vector<Complex> points; points.reserve(static_cast<std::size_t>(count));
  for (int j = 0; j < count; ++j) {
    const double t = tau * j / count;
    const double x = 16.0 * std::pow(std::sin(t), 3);
    const double y = 13.0 * std::cos(t) - 5.0 * std::cos(2*t) - 2.0 * std::cos(3*t) - std::cos(4*t);
    points.emplace_back(x, -y);
  }
  return points;
}

std::vector<Complex> read_csv(const std::filesystem::path& path) {
  std::ifstream in{path};
  if (!in) throw std::runtime_error("cannot open input: " + path.string());
  std::vector<Complex> points; std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream row{line}; double x = 0, y = 0;
    if (!(row >> x >> y)) throw std::runtime_error("invalid CSV point: " + line);
    points.emplace_back(x, y);
  }
  if (points.size() < 3) throw std::runtime_error("input needs at least three points");
  if (std::abs(points.front() - points.back()) < 1e-12) points.pop_back();
  return points;
}

void normalize(std::vector<Complex>& points) {
  Complex center{}; for (const auto p : points) center += p; center /= static_cast<double>(points.size());
  double radius = 0; for (auto& p : points) { p -= center; radius = std::max(radius, std::abs(p)); }
  if (radius <= 0) throw std::runtime_error("curve has zero extent");
  for (auto& p : points) p /= radius;
}

std::vector<Complex> resample(const std::vector<Complex>& input, int count, bool arc_length) {
  if (!arc_length) {
    std::vector<Complex> out; out.reserve(static_cast<std::size_t>(count));
    for (int j = 0; j < count; ++j) out.push_back(input[static_cast<std::size_t>(j) * input.size() / count]);
    return out;
  }
  std::vector<double> cumulative(input.size() + 1, 0.0);
  for (std::size_t j = 0; j < input.size(); ++j)
    cumulative[j + 1] = cumulative[j] + std::abs(input[(j + 1) % input.size()] - input[j]);
  if (cumulative.back() <= 0) throw std::runtime_error("curve has zero length");
  std::vector<Complex> out; out.reserve(static_cast<std::size_t>(count));
  for (int j = 0; j < count; ++j) {
    const double target = cumulative.back() * j / count;
    const auto upper = std::upper_bound(cumulative.begin(), cumulative.end(), target);
    const std::size_t segment = static_cast<std::size_t>(upper - cumulative.begin() - 1);
    const double local = (target - cumulative[segment]) / (cumulative[segment + 1] - cumulative[segment]);
    out.push_back(input[segment] * (1.0 - local) + input[(segment + 1) % input.size()] * local);
  }
  return out;
}

std::vector<Term> dft(const std::vector<Complex>& points) {
  const int n = static_cast<int>(points.size()); std::vector<Term> terms; terms.reserve(points.size());
  for (int k = 0; k < n; ++k) {
    Complex sum{};
    for (int j = 0; j < n; ++j) sum += points[j] * std::polar(1.0, -tau * k * j / n);
    const int frequency = k <= n / 2 ? k : k - n;
    terms.push_back({frequency, sum / static_cast<double>(n)});
  }
  return terms;
}

Complex reconstruct(const std::vector<Term>& terms, double time) {
  Complex value{};
  for (const auto& term : terms) value += term.coefficient * std::polar(1.0, tau * term.frequency * time);
  return value;
}

void order(std::vector<Term>& terms, const std::string& mode) {
  if (mode == "amplitude")
    std::stable_sort(terms.begin(), terms.end(), [](const Term& a, const Term& b) {
      if (std::abs(a.coefficient) != std::abs(b.coefficient)) return std::abs(a.coefficient) > std::abs(b.coefficient);
      return a.frequency < b.frequency;
    });
  else std::sort(terms.begin(), terms.end(), [](const Term& a, const Term& b) {
    if (std::abs(a.frequency) != std::abs(b.frequency)) return std::abs(a.frequency) < std::abs(b.frequency);
    return a.frequency < b.frequency;
  });
}

double rms(const std::vector<Complex>& points, const std::vector<Term>& terms) {
  double squared = 0;
  for (std::size_t j = 0; j < points.size(); ++j) squared += std::norm(points[j] - reconstruct(terms, static_cast<double>(j) / points.size()));
  return std::sqrt(squared / points.size());
}

void self_test() {
  std::vector<Complex> constant(8, Complex{2.0, -3.0});
  const auto constant_terms = dft(constant);
  if (std::abs(constant_terms[0].coefficient - constant[0]) > 1e-12) throw std::runtime_error("DC coefficient test failed");
  for (std::size_t k = 1; k < constant_terms.size(); ++k)
    if (std::abs(constant_terms[k].coefficient) > 1e-12) throw std::runtime_error("constant spectrum test failed");
  const std::vector<Complex> signal{{1,2},{-2,1},{0,-1},{3,0},{-1,-2},{2,-1},{0,3},{-3,1}};
  const auto all = dft(signal);
  if (rms(signal, all) > 1e-12) throw std::runtime_error("exact reconstruction test failed");
  if (all.back().frequency != -1) throw std::runtime_error("negative frequency test failed");
  std::cout << "Self-test passed: DC, exact reconstruction, and negative frequency indexing\n";
}

std::string path(const std::vector<Complex>& points, double cx, double cy, double scale, bool close) {
  std::ostringstream out; out << std::fixed << std::setprecision(2);
  for (std::size_t j = 0; j < points.size(); ++j)
    out << (j ? 'L' : 'M') << cx + points[j].real() * scale << ' ' << cy + points[j].imag() * scale;
  if (close) out << 'Z';
  return out.str();
}
}

int main(int argc, char** argv) {
  try {
    const Options o = parse(argc, argv);
    if (o.self_test) { self_test(); return 0; }
    auto source = o.input.empty() ? heart(std::max(1024, o.samples * 4)) : read_csv(o.input);
    normalize(source);
    auto samples = resample(source, o.samples, o.resampling == "arc-length");
    auto terms = dft(samples);
    order(terms, "amplitude");
    const int retained = std::min(o.terms, static_cast<int>(terms.size()));
    terms.resize(static_cast<std::size_t>(retained));
    order(terms, o.term_order);
    const double error = rms(samples, terms);
    const double drawable = std::min(o.width, o.height) - 2.0 * o.margin;
    if (drawable <= 0) throw std::runtime_error("margin leaves no drawing area");
    const double scale = drawable * 0.42, cx = o.width / 2.0, cy = o.height / 2.0;
    std::vector<Complex> trace; const int trace_steps = 1200;
    for (int j = 0; j <= trace_steps; ++j) {
      const double t = o.time - o.trace_length + o.trace_length * j / trace_steps;
      trace.push_back(reconstruct(terms, t));
    }
    if (o.output.has_parent_path()) std::filesystem::create_directories(o.output.parent_path());
    std::ofstream svg{o.output}; if (!svg) throw std::runtime_error("cannot open output");
    svg << std::fixed << std::setprecision(3)
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << o.width << "\" height=\"" << o.height
        << "\" viewBox=\"0 0 " << o.width << ' ' << o.height << "\">\n"
        << "<rect width=\"100%\" height=\"100%\" fill=\"#080b14\"/>\n";
    if (o.show_source) svg << "<path d=\"" << path(samples, cx, cy, scale, true) << "\" fill=\"none\" stroke=\"#27324a\" stroke-width=\"5\"/>\n";
    svg << "<path d=\"" << path(trace, cx, cy, scale, false) << "\" fill=\"none\" stroke=\"#f6d35f\" stroke-width=\"8\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n";
    Complex cursor{};
    for (const auto& term : terms) {
      const Complex next = cursor + term.coefficient * std::polar(1.0, tau * term.frequency * o.time);
      if (o.show_circles && std::abs(term.coefficient) * scale > 1)
        svg << "<circle cx=\"" << cx + cursor.real()*scale << "\" cy=\"" << cy + cursor.imag()*scale
            << "\" r=\"" << std::abs(term.coefficient)*scale << "\" fill=\"none\" stroke=\"#52617d\" stroke-width=\"3\"/>\n";
      svg << "<line x1=\"" << cx + cursor.real()*scale << "\" y1=\"" << cy + cursor.imag()*scale
          << "\" x2=\"" << cx + next.real()*scale << "\" y2=\"" << cy + next.imag()*scale
          << "\" stroke=\"#70e1f5\" stroke-width=\"4\"/>\n";
      cursor = next;
    }
    svg << "<circle cx=\"" << cx + cursor.real()*scale << "\" cy=\"" << cy + cursor.imag()*scale
        << "\" r=\"10\" fill=\"#f3f5fa\"/>\n</svg>\n";
    std::cout << "Rendered " << retained << " of " << o.samples << " Fourier terms; RMS error "
              << std::setprecision(8) << error << '\n';
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n'; return 1;
  }
}
