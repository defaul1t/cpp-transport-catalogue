#include "json.h"

using namespace std;

namespace json {
     
    namespace {
        int step = 0;
        Node LoadNode(istream& input);

        Node LoadNone()
        {
            return Node();
        }

        Node LoadBool(bool state)
        {
            return Node(state);
        }

        Node LoadNull()
        {
            return Node();
        }

        Node LoadArray(istream& input) { // {

            Array result;
            char c = ' ';
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') { throw ParsingError("Array not closed"); }
            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                double buf = std::stod(parsed_num);
                return Node(buf);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;
           // setlocale(LC_ALL, "Rus");
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }


        Node LoadDict(istream& input) {
           
            Dict result;
            char c = ' ';
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                while (c == '\n' || c == '\t' || c == '\r' || c == ' ')
                {
                    input >> c;
                }
                if (c == '\"') {
                    string key = LoadString(input).AsString();
                    input >> c;
                    while (c == '\n' || c == '\t' || c == '\r' || c == ' ')
                    {
                        input >> c;
                    }



                    if (c == ':') {
                      
                        result.insert({ move(key), LoadNode(input) });
                    }
                    else { throw ParsingError("Dict break"); }
                }
                else { throw ParsingError("Dict break"); }
            }

            if (c != '}') { throw ParsingError("Dict not closed"); }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;

            input >> c;

            while (c == '\n' || c == '\t' || c == '\r' || c == ' ')
            {
                input >> c;
            }

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n')
            {
                std::string none;
                for (int i = 0; i < 4 && !input.eof(); ++i) {
                    none += c;
                    input >> c;

                }
                if (none == "none")
                {
                    if (input.eof()) {
                        return LoadNone();
                    }
                    input >> c;
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == '}' || c == ']') {
                        input.putback(c);
                        return LoadNone();
                    }
                    else { throw ParsingError("none has no detected"); }
                }
                if (none == "null")
                {
                    if (input.eof()) {
                        return LoadNull();
                    }
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == '}' || c == ']') {
                        input.putback(c);
                        return LoadNull();
                    }
                    else { throw ParsingError("none has no detected"); }
                }
                throw ParsingError("none has no detected");
            }
            else if (c == 't' || c == 'f')
            {
                if (c == 't')
                {
                    std::string tr_ue;
                    for (int i = 0; i < 4; ++i) {
                        tr_ue += c;
                        input >> c;

                    }
                    if (tr_ue == "true")
                    {
                        if (input.eof()) {
                            return LoadBool(true);
                        }
                        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ','|| c == '}' || c == ']') {
                            input.putback(c);
                            return LoadBool(true);
                        }
                        else { throw ParsingError(""); }
                    }
                }
                else if (c == 'f')
                {

                    std::string fal_se;
                    for (int i = 0; i < 5; ++i) {
                        fal_se += c;
                        input >> c;

                    }
                    if (fal_se == "false")
                    {
                        if (input.eof()) {
                            return LoadBool(false);
                        }
                        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == '}' || c == ']') { // miss
                            input.putback(c);
                            return LoadBool(false);
                        }
                        else { throw ParsingError(""); }
                    }


                }
                throw ParsingError("bool load break");
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(nullptr_t n)
        : value_(n) {
    }

    Node::Node(Array array_)
        : value_(move(array_)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(bool value)
        : value_(move(value)) {
    }

    Node::Node(double value)
        : value_(move(value)) {
    }


    const Array& Node::AsArray() const {
        if (!holds_alternative<Array>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!holds_alternative<Dict>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return get<Dict>(value_);
    }


     Array& Node::AsArrayForMove()  {
        if (!holds_alternative<Array>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return const_cast<Array&>(get<Array>(value_));
    }

     Dict& Node::AsMapForMove()  {
        if (!holds_alternative<Dict>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return const_cast<Dict&>(get<Dict>(value_));
    }


    int Node::AsInt() const {
        if (!holds_alternative<int>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return get<int>(value_);
    }

    const string& Node::AsString() const {
        if (!holds_alternative<string>(value_)) {
            // ?
            throw std::logic_error("");
        }

        return  get<string>(value_);
    }

    double Node::AsDouble() const {
        if ((holds_alternative<double>(value_))) {
            // ?
            return get<double>(value_);
        }


        if (!(holds_alternative<int>(value_))) {
            // ?
            throw std::logic_error("");
        }
        return get<int>(value_);
    }

    bool Node::AsBool() const
    {
        if (!holds_alternative<bool>(value_)) {
            // ?
            throw std::logic_error("");
        }
        return get<bool>(value_);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    bool operator==(const json::Document& lhs, const json::Document& rhs)
    {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const json::Document& lhs, const json::Document& rhs)
    {
        return lhs.GetRoot() != rhs.GetRoot();
    }

    bool Node::IsInt() const
    {
        return holds_alternative<int>(value_);
        
    }

    bool Node::IsDouble() const
    {

        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
     
    }

    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(value_) && !holds_alternative<int>(value_);
       
    }

    bool Node::IsBool() const
    {
      
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const
    {
      
        return holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const
    {

        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const
    {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(value_);
    }

    void Print(const Document& doc, std::ostream& output) {
        (void)&doc;
        (void)&output;

        PrintNode(doc.GetRoot(), output);

        
    }

    void PrintValue(Array arr, std::ostream& out)
    {
        bool first = true;
        
        out << "[";
        step += 4;
        
        for (auto val : arr)
        {
            
            if (first) {
                out << std::endl;
                out << std::string(step, ' ');
                PrintNode(val, out);
                first = false;
            }
            else
            {
                out << ",";
                out << std::endl;
                out << std::string(step, ' ');
                PrintNode(val, out);
                
            }
        }
        out << '\n';
        step -= 4;
        
        out << std::string(step, ' ');
        
        out << "]";
    }

    void PrintValue(Dict dict, std::ostream& out)
    {
        bool first = true;

   
        out << "{";
        step += 4;

        for (auto [key, val] : dict)
        {
            if (first) {
                out << std::endl;
                out << std::string(step, ' ');
                PrintValue(key, out);
                out << ": ";
                PrintNode(val, out);
                first = false;
            }
            else
            {
                out << ",";
                out << std::endl;
                out << std::string(step, ' ');
                PrintValue(key, out);
                out << ": ";
                PrintNode(val, out);
               
            }
        }
        out << '\n';
        step -= 4;
        out << std::string(step, ' ');
        out << "}";
    }
    void PrintValue(int dig, std::ostream& out)
    {
        out  << dig;
    }
    void PrintValue(double dig, std::ostream& out)
    {
        out  << dig;
    }

    void PrintValue(std::string str, std::ostream& out)
    {
       
        out << '\"';
        for (auto c : str) {
            if (c == '\"' || c == '\\')
            {
                out << "\\";
                out << c;
            }
            else if (c == '\t')
            {
                out << "\\";
                out << "t";
            }
            else if (c == '\r')
            {
                out << "\\";
                out << "r";
            }
            else if (c == '\n')
            {
                out << "\\";
                out << "n";
            }
            else {
                out << c;
            }
        }
        out << '\"';

    }
    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out  << "null"sv;
    }

    void PrintValue(bool v, std::ostream& out) {
        if (v)
        {
            out  << "true";
        }
        else { out << "false"; }
       
    }
    // Другие перегрузки функции PrintValue пишутся аналогично

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    bool operator==(const Node& n1, const Node& n2)
    {

        if (n1.IsArray() && n2.IsArray())
        {
            return n1.AsArray() == n2.AsArray();
        }
       
        if (n1.IsBool() && n2.IsBool())
        {
            return n1.AsBool() == n2.AsBool();
        }
        if (n1.IsInt() && n2.IsInt())
        {
            return n1.AsInt() == n2.AsInt();
        }
        if (n1.IsMap() && n2.IsMap())
        {
            return n1.AsMap() == n2.AsMap();
        }
        if (n1.IsNull() && n2.IsNull())
        {
            return true;
        }

        if (n1.IsPureDouble() && n2.IsPureDouble())
        {

            return n1.AsDouble() == n1.AsDouble();
        }

        if (n1.IsString() && n2.IsString())
        {
            return n1.AsString() == n1.AsString();
        }

        return false;

    }

    bool operator!=(const Node& n1, const Node& n2)
    {
        return !(n1 == n2);
    }

}  // namespace json