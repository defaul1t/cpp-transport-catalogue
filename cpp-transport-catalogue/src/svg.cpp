#define _USE_MATH_DEFINES

#include "svg.h"


namespace svg {

   
    std::ostream& operator<<(std::ostream& os, const svg::Color& color)
    {
        std::visit(svg::ColorPrinter{ os }, color);
        return os;
    }


    std::ostream& operator<<(std::ostream& out, StrokeLineCap counter)
    {
        if (counter == StrokeLineCap::BUTT)
        {
            out << "butt";
        }
        else if (counter == StrokeLineCap::ROUND)
        {
            out << "round";
        }
        else if (counter == StrokeLineCap::SQUARE)
        {
            out << "square";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin counter)
    {
        if (counter == StrokeLineJoin::ARCS)
        {
            out << "arcs";
        }
        else if (counter == StrokeLineJoin::BEVEL)
        {
            out << "bevel";
        }
        else if (counter == StrokeLineJoin::MITER)
        {
            out << "miter";
        }
        else if (counter == StrokeLineJoin::MITER_CLIP)
        {
            out << "miter-clip";
        }
        else if (counter == StrokeLineJoin::ROUND)
        {
            out << "round";
        }
        return out;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        using namespace std::literals;
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"s;
    }


    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */

    void Polyline::RenderObject(const RenderContext& context) const
    {

        bool f = true;
        auto& out = context.out;
        out << "<polyline points=\""s;
        for (auto& point : points_)
        {
            if (f) {
                out << point.x << "," << point.y;
                f = false;
            }
            else
            {
                out << " " << point.x << "," << point.y;
            }
        }
        out << "\" "s;
        RenderAttrs(out);
        out << "/>"s;
    }

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size)
    {
        font_size = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data)
    {
        for (auto& sym : data)
        {
            if (sym == '\"')
            {
                data_.insert(data_.size(), "&quot;");
            }
            else if (sym == '\'')
            {
                data_.insert(data_.size(), "&apos;");
            }
            else if (sym == '<')
            {
                data_.insert(data_.size(), "&lt;");
            }
            else if (sym == '>')
            {
                data_.insert(data_.size(), "&gt;");
            }
            else if (sym == '&')
            {
                data_.insert(data_.size(), "&amp;");
            }
            else {
                data_.push_back(sym);
            }
        }

        return *this;
    }


    // Прочие данные и методы, необходимые для реализации элемента <text>

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text "sv;
        RenderAttrs(out);
        out << " x=\"" << pos_.x << "\""
            << " y=\"" << pos_.y << "\"";
        out << " dx=\"" << offset_.x << "\""
            << " dy=\"" << offset_.y << "\"";
        out << " font-size=\"" << font_size << "\"";
        if (!font_family_.empty())
        {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty())
        {
            out << " font-weight=\"" << font_weight_ << "\"";
        }


        out << ">" << data_ << "</text>";


    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        data_.emplace_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (auto& obj : data_)
        {
            out << "  ";
            obj->Render(out);
        }
        out << "</svg>";
    }

   

}  // namespace svg