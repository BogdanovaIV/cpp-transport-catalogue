#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            bool symbol_exit = false;
            for (char c; input >> c;) {
                if (c == ']') {
                    symbol_exit = true;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!symbol_exit) {
                throw ParsingError("Failed to read Array. Symbol ] missing."s);
            }

            return Node(std::move(result));
        }

        Node LoadNumber(istream& input) {
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
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }

        }

        Node LoadString(istream& input) {
            using namespace std::literals;

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

            return Node(std::move(s));
        }

        Node LoadNull(istream& input) {
            string line;
            char c;
            for (int i = 0; input >> c && i < 4; ++i) {
                line += c;
            }
            if (line != "null"s) {
                throw ParsingError("Null parsing error");
            }
            return Node();
        }

        Node LoadBool(istream& input, bool loadtrue) {
            string line;
            char c;
            int size = loadtrue ? 4 : 5;
            for (int i = 0; i < size && input >> c; ++i) {
                line += c;
            }
            if ((loadtrue && line != "true"s) || (!loadtrue && line != "false"s)) {
                throw ParsingError("Bool parsing error");
            }
            return Node(loadtrue);
        }

        Node LoadDict(istream& input) {
            Dict result;
            bool symbol_exit = false;

            for (char c; input >> c;) {
                if (c == '}') {
                    symbol_exit = true;
                    break;
                }

                if (c == ',') {
                    input >> c;
                }

                if (c == '{' || c == '\n' || c == '\r' || c == ' ') {
                    continue;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }

            if (!symbol_exit) {
                throw ParsingError("Failed to read Array. Symbol } missing."s);
            }

            return Node(std::move(result));
        }

        Node LoadNode(istream& input) {

            for (char c; input >> c;) {
                if (c == ' ' || c == '\\' || c == '\n' || c == '\t' || c == '\r') {
                    continue;
                }
                else if (c == ']' || c == '}') {
                    throw ParsingError("Wrong format.");
                }
                else if (c == '[') {
                    return LoadArray(input);
                }
                else if (c == '{') {
                    return LoadDict(input);
                }
                else if (c == '\"') {
                    return LoadString(input);
                }
                else if (c == 'n') {
                    input.putback(c);
                    return LoadNull(input);
                }
                else if (c == 't') {
                    input.putback(c);
                    return LoadBool(input, true);
                }
                else if (c == 'f') {
                    input.putback(c);
                    return LoadBool(input, false);
                }
                else {
                    input.putback(c);
                    return LoadNumber(input);
                }
            }
            return Node();
        }

    }  // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        bool res = std::holds_alternative<double>(*this);
        if (!res) {
            res = std::holds_alternative<int>(*this);
        }

        return res;
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {

        try {
            const int& value = std::get<int>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type int."s);
        }

    }

    bool Node::AsBool() const {

        try {
            const bool& value = std::get<bool>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type bool."s);
        }

    }

    double Node::AsDouble() const {

        try {
            const double& value = std::get<double>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            try {
                const int& value = std::get<int>(*this);
                return static_cast<const double&>(value);
            }
            catch (const bad_variant_access&) {
                throw std::logic_error("Node isn't type double."s);
            }
        }

    }

    const std::string& Node::AsString() const {

        try {
            const std::string& value = std::get<std::string>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type string."s);
        }

    }

    const Array& Node::AsArray() const {

        try {
            const Array& value = std::get<Array>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type Array."s);
        }

    }

    const Dict& Node::AsDict() const {

        try {
            const Dict& value = std::get<Dict>(*this);
            return value;
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type Dict."s);
        }

    }

    const Node::Value& Node::GetValue() const {
        return *this;
    }

    bool operator==(const Node& lhs, const Node& rhs) {

        if (lhs.IsNull() && rhs.IsNull()) {
            return true;
        }
        else if (lhs.IsInt() && rhs.IsInt() && lhs.AsInt() == rhs.AsInt()) {
            return true;
        }
        else if (lhs.IsPureDouble() && rhs.IsPureDouble() && lhs.AsDouble() == rhs.AsDouble()) {
            return true;
        }
        else if (lhs.IsBool() && rhs.IsBool() && lhs.AsBool() == rhs.AsBool()) {
            return true;
        }
        else if (lhs.IsString() && rhs.IsString() && lhs.AsString() == rhs.AsString()) {
            return true;
        }
        else if (lhs.IsArray() && rhs.IsArray() && lhs.AsArray() == rhs.AsArray()) {
            return true;
        }
        else if (lhs.IsDict() && rhs.IsDict() && lhs.AsDict() == rhs.AsDict()) {
            return true;
        }

        return false;
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        if (lhs.IsNull() && rhs.IsNull()) {
            return false;
        }
        else if (lhs.IsInt() && rhs.IsInt() && lhs.AsInt() == rhs.AsInt()) {
            return false;
        }
        else if (lhs.IsPureDouble() && rhs.IsPureDouble() && lhs.AsDouble() == rhs.AsDouble()) {
            return false;
        }
        else if (lhs.IsBool() && rhs.IsBool() && lhs.AsBool() == rhs.AsBool()) {
            return false;
        }
        else if (lhs.IsString() && rhs.IsString() && lhs.AsString() == rhs.AsString()) {
            return false;
        }
        else if (lhs.IsArray() && rhs.IsArray() && lhs.AsArray() == rhs.AsArray()) {
            return false;
        }
        else if (lhs.IsDict() && rhs.IsDict() && lhs.AsDict() == rhs.AsDict()) {
            return false;
        }

        return true;
    }

    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() != rhs.GetRoot();
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& value, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    // В специализаци шаблона PrintValue для типа bool параметр value передаётся
    // по константной ссылке, как и в основном шаблоне.
    // В качестве альтернативы можно использовать перегрузку:
    // void PrintValue(bool value, const PrintContext& ctx);
    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {

        std::visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetValue());

    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json