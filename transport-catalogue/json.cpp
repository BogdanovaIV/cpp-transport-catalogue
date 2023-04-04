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

    return Node(move(result));
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

    return Node(move(s));
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
    for (int i = 0; i < size && input >> c ; ++i) {
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
        result.insert({move(key), LoadNode(input)});
    }

    if (!symbol_exit) {
        throw ParsingError("Failed to read Array. Symbol } missing."s);
    }

    return Node(move(result));
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

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(int value)
    : value_(value) {
}

Node::Node(double value)
    : value_(value) {
}

Node::Node(string value)
    : value_(move(value)) {
}

Node::Node(bool value)
    : value_(value) {
}

bool Node::IsInt() const {
    return get_if<int>(&value_);
}

bool Node::IsDouble() const {
    bool res = get_if<int>(&value_);
    if (!res) {
        res = get_if<double>(&value_);
    }

    return res;
}

bool Node::IsPureDouble() const {
   return get_if<double>(&value_);
}

bool Node::IsBool() const {
    return get_if<bool>(&value_);
}

bool Node::IsString() const {
    return get_if<std::string>(&value_);
}

bool Node::IsNull() const {
    return get_if<std::nullptr_t>(&value_);
}

bool Node::IsArray() const {
    return get_if <Array>(&value_);
}

bool Node::IsMap() const {
    return get_if <Dict>(&value_);
}

int Node::AsInt() const {

    try {
        const int& value = std::get<int>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        throw std::logic_error("Node isn't type int."s);
    }
    
}

bool Node::AsBool() const {

    try {
        const bool& value = std::get<bool>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        throw std::logic_error("Node isn't type bool."s);
    }

}

double Node::AsDouble() const {

    try {
        const double& value = std::get<double>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        try {
            const int& value = std::get<int>(value_);
            return static_cast<const double&>(value);
        }
        catch (const bad_variant_access&) {
            throw std::logic_error("Node isn't type double."s);
        }
    }

}

const std::string& Node::AsString() const {

    try {
        const std::string& value = std::get<std::string>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        throw std::logic_error("Node isn't type string."s);
    }

}

const Array& Node::AsArray() const {

    try {
        const Array& value = std::get<Array>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        throw std::logic_error("Node isn't type Array."s);
    }

}

const Dict& Node::AsMap() const {

    try {
        const Dict& value = std::get<Dict>(value_);
        return value;
    }
    catch (const bad_variant_access&) {
        throw std::logic_error("Node isn't type Dict."s);
    }

}

const Node::Value& Node::GetValue() const {
    return value_; 
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
    else if (lhs.IsMap() && rhs.IsMap() && lhs.AsMap() == rhs.AsMap()) {
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
    else if (lhs.IsMap() && rhs.IsMap() && lhs.AsMap() == rhs.AsMap()) {
        return false;
    }

    return true;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(const int value, std::ostream& out) {
    out << value;
}

void PrintValue(const double value, std::ostream& out) {
    out << value;
}

void PrintValue(const std::string value, std::ostream& out) {
    out << '\"';
    for(char c : value) {
        if (c == '\"') {
            out << "\\\""sv;
        }
        else if (c == '\r') {
            out << "\\r"sv;
        }
        else if (c == '\n') {
            out << "\\n"sv;
        }
        else if (c == '\\') {
            out << "\\\\"sv;
        }
        else {
            out << c;
        }
    }
    out<<'\"';
}

void PrintValue(const bool value, std::ostream& out) {
    if (value) {
        out << "true"sv;
    }
    else {
        out << "false"sv;
    }
}

void PrintValue(const Array value, std::ostream& out) {
    out << '[';
    bool first = true;
    for (auto val : value) {
        if (!first) {
            out << ',';
        }
        PrintNode(val, out);
        first = false;
    }
    out << ']';
}

void PrintValue(const Dict value, std::ostream& out) {
    out << "{ ";
    bool first = true;
    for (auto& [key, val] : value) {
        if (!first) {
            out << ',';
        }
        out <<'\"' << key << "\": "sv;
        PrintNode(val, out);
        first = false;
    }
    out << " }";
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json