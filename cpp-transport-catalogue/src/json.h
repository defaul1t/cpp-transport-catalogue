#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>


namespace json {

  

    class Node;
    // —охраните объ€влени€ Dict и Array без изменени€
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        /* –еализуйте Node, использу€ std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        Node() = default;
        Node(nullptr_t);
        Node(Array array_);
        Node(Dict map);
        Node(int value);
        Node(std::string value);
        Node(bool value);
        Node(double value);


        const Array& AsArray() const;
        const Dict& AsMap() const;
        Array& AsArrayForMove();
        Dict& AsMapForMove();
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Value& GetValue() const { return value_; }
        bool AsBool() const;

        bool IsInt() const;

        bool IsDouble() const;

        bool IsPureDouble() const;

        bool IsBool() const;

        bool IsString() const;

        bool IsNull() const;

        bool IsArray() const;

        bool IsMap() const;

    private:
        Value value_;
    };

    class Document {
    public:
        Document() = default;
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);


    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    void Print(const Document& doc, std::ostream& output);




    // ѕерегрузка функции PrintValue дл€ вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out);
    // ƒругие перегрузки функции PrintValue пишутс€ аналогично
    void PrintValue(Array, std::ostream& out);

    void PrintValue(int, std::ostream& out);

    void PrintValue(double, std::ostream& out);

    void PrintValue(Dict, std::ostream& out);

    void PrintValue(std::string, std::ostream& out);

    void PrintValue(nullptr_t, std::ostream& out);

    void PrintNode(const Node& node, std::ostream& out);

    // Ўаблон, подход€щий дл€ вывода double и int


    bool operator==(const Node& n1, const Node& n2);

    bool operator!=(const Node& n1, const Node& n2);

    bool operator!=(const Document& lhs, const Document& rhs);

    bool operator==(const Document& lhs, const Document& rhs);

}  // namespace json