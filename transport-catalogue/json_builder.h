#pragma once
#include "json.h"
#include <deque>
#include <stdexcept>
#include <algorithm>

namespace json {

	class Builder;
	class KeyContext;
	class DictContext;
	class StartArrayContext;
	class ValueDictContext;

	class BaseContext {
	
	public:
		BaseContext(Builder& builder): builder_(builder){
		}
		DictContext StartDict();
		KeyContext Key(std::string key);
		Builder& EndDict();
		StartArrayContext StartArray();
		Builder& EndArray();
		Node& Build();	
	protected:
		Builder& builder_;
	};

	class DictContext : public BaseContext {
	public:
		DictContext(Builder& builder) : BaseContext(builder){}
		DictContext(DictContext& other) : BaseContext(other.builder_) {}

		DictContext StartDict() = delete;
		StartArrayContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node& Build() = delete;

	};

	class KeyContext : public BaseContext {
	public:
		KeyContext(Builder& builder) : BaseContext(builder) {}
		KeyContext(KeyContext& other) : BaseContext(other.builder_) {}

		KeyContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node& Build() = delete;
		ValueDictContext Value(Node::Value value);

	};

	class StartArrayContext : public BaseContext {
	public:
		StartArrayContext(Builder& builder) : BaseContext(builder) {}
		StartArrayContext(StartArrayContext& other) : BaseContext(other.builder_) {}

		Builder& Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Node& Build() = delete;
		StartArrayContext Value(Node::Value value);

	};

	class ValueDictContext : public BaseContext {
	public:
		ValueDictContext(Builder& builder) : BaseContext(builder) {}
		ValueDictContext(ValueDictContext& other) : BaseContext(other.builder_) {}

		DictContext StartDict() = delete;
		StartArrayContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node& Build() = delete;
	};

	class Builder {
	public:

		DictContext StartDict();
		KeyContext Key(std::string key);

		ValueDictContext ValueDict(Node::Value value);
		Builder& Value(Node::Value value);

		Builder& EndDict();
		StartArrayContext StartArray();
		Builder& EndArray();
		Node& Build();

	private:
		Node root_ = nullptr;
		std::deque<Node*> nodes_stack_;
		enum kind_function {
			start_dict_,
			key_,
			value_,
			end_dict_,
			start_array_,
			end_array_,
			build_
		};
		std::vector<kind_function>allowed_function_;
		std::deque<std::string> keys_stack_;
	};
}
