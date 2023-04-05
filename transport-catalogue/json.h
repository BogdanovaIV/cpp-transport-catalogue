#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>


namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : private std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> {
public:

    using variant::variant;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>& GetValue() const;

};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);
bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

void PrintValue(std::nullptr_t, std::ostream& out);

void PrintValue(const int value, std::ostream& out);

void PrintValue(const double value, std::ostream& out);

void PrintValue(const std::string value, std::ostream& out);

void PrintValue(const bool value, std::ostream& out);

void PrintValue(const Array value, std::ostream& out);

void PrintValue(const Dict value, std::ostream& out);

void PrintNode(const Node& node, std::ostream& out);

void Print(const Document& doc, std::ostream& output);

}  // namespace json