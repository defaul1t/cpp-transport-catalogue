
#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {
using namespace std::literals;

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap counter);

std::ostream& operator<<(std::ostream& out, StrokeLineJoin counter);

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

struct Rgb {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
};

struct Rgba {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
inline const Color NoneColor{"none"};

struct ColorPrinter {
  std::ostream& out;

  void operator()(std::monostate) const { out << "none"; }
  void operator()(std::string color) const { out << color; }
  void operator()(svg::Rgb color) const {
    out << "rgb(" << static_cast<int>(color.red) << ","
        << static_cast<int>(color.green) << "," << static_cast<int>(color.blue)
        << ")";
  }
  void operator()(svg::Rgba color) const {
    out << "rgba(" << static_cast<int>(color.red) << ","
        << static_cast<int>(color.green) << "," << static_cast<int>(color.blue)
        << "," << color.opacity << ")";
  }
};

std::ostream& operator<<(std::ostream& os, const svg::Color& color);

/*
 * ��������������� ���������, �������� �������� ��� ������ SVG-��������� �
 * ���������. ������ ������ �� ����� ������, ������� �������� � ��� ������� ���
 * ������ ��������
 */
struct RenderContext {
  RenderContext(std::ostream& out) : out(out) {}

  RenderContext(std::ostream& out, int indent_step, int indent = 0)
      : out(out), indent_step(indent_step), indent(indent) {}

  RenderContext Indented() const {
    return {out, indent_step, indent + indent_step};
  }

  void RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  std::ostream& out;
  int indent_step = 0;
  int indent = 0;
};

class Object;

class ObjectContainer {
 public:
  template <typename T>
  void Add(T obj) {
    std::unique_ptr<T> ptr = std::make_unique<T>(obj);
    AddPtr(std::move(ptr));
  }

  virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
  virtual ~ObjectContainer() {}
};

class Drawable {
 public:
  virtual void Draw(ObjectContainer& obj) const = 0;
  virtual ~Drawable() {}
};

template <typename Owner>
class PathProps {
 public:
  Owner& SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }
  Owner& SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }
  Owner& SetStrokeWidth(double width) {
    stroke_width_ = std::move(width);
    return AsOwner();
  }
  Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_linecap_ = std::move(line_cap);
    return AsOwner();
  }
  Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_linejoin_ = std::move(line_join);
    return AsOwner();
  }

 protected:
  ~PathProps() = default;

  // ����� RenderAttrs ������� � ����� ����� ��� ���� ����� �������� fill �
  // stroke
  void RenderAttrs(std::ostream& out) const {
    using namespace std::literals;

    if (fill_color_) {
      out << "fill=\""sv << *fill_color_ << "\""sv;
    }

    if (stroke_color_) {
      out << " stroke=\""sv << *stroke_color_ << "\""sv;
    }

    if (stroke_width_) {
      out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }
    if (stroke_linecap_) {
      out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
    }
    if (stroke_linejoin_) {
      out << " stroke-linejoin=\""sv << *stroke_linecap_ << "\""sv;
    }
  }

 private:
  Owner& AsOwner() {
    // static_cast ��������� ����������� *this � Owner&,
    // ���� ����� Owner � ��������� PathProps
    return static_cast<Owner&>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> stroke_linecap_;
  std::optional<StrokeLineJoin> stroke_linejoin_;
};

/*
 * ����������� ������� ����� Object ������ ��� ���������������� ��������
 * ���������� ����� SVG-���������
 * ��������� ������� "��������� �����" ��� ������ ����������� ����
 */
class Object {
 public:
  void Render(const RenderContext& context) const;

  virtual ~Object() = default;

 private:
  virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * ����� Circle ���������� ������� <circle> ��� ����������� �����
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
 public:
  Circle& SetCenter(Point center);
  Circle& SetRadius(double radius);

 private:
  void RenderObject(const RenderContext& context) const override;

  Point center_;
  double radius_ = 1.0;
};

/*
 * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
 public:
  // ��������� ��������� ������� � ������� �����
  Polyline& AddPoint(Point point);

  /*
   * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
   */
 private:
  void RenderObject(const RenderContext& context) const override;

  std::vector<Point> points_;
};

/*
 * ����� Text ���������� ������� <text> ��� ����������� ������
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
 public:
  // ����� ���������� ������� ����� (�������� x � y)
  Text& SetPosition(Point pos);

  // ����� �������� ������������ ������� ����� (�������� dx, dy)
  Text& SetOffset(Point offset);

  // ����� ������� ������ (������� font-size)
  Text& SetFontSize(uint32_t size);

  // ����� �������� ������ (������� font-family)
  Text& SetFontFamily(std::string font_family);

  // ����� ������� ������ (������� font-weight)
  Text& SetFontWeight(std::string font_weight);

  // ����� ��������� ���������� ������� (������������ ������ ���� text)
  Text& SetData(std::string data);

  // ������ ������ � ������, ����������� ��� ���������� �������� <text>
 private:
  void RenderObject(const RenderContext& context) const override;

  Point pos_ = {0.0, 0.0};
  Point offset_ = {0.0, 0.0};
  uint32_t font_size = 1;
  std::string font_family_;
  std::string font_weight_;
  std::string data_;
};

class Document : public ObjectContainer {
 public:
  // ��������� � svg-�������� ������-��������� svg::Object
  void AddPtr(std::unique_ptr<Object>&& obj) override;

  // ������� � ostream svg-������������� ���������
  void Render(std::ostream& out) const;

  // ������ ������ � ������, ����������� ��� ���������� ������ Document
 private:
  std::vector<std::unique_ptr<Object>> data_;
};

}  // namespace svg
