#pragma once
#include "json.h"
#include <deque>
#include <stdexcept>
#include <algorithm>

namespace json {

	

	class Builder {

		class DictValueContext;
		class DictItemContext;
		class ArrayItemContext;

	public:

		DictItemContext StartDict();
		DictValueContext Key(std::string key);

		DictItemContext ValueDict(Node::Value value);
		Builder& Value(Node::Value value);

		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node& Build();

	private:
		Node root_ = nullptr;
		std::deque<Node*> nodes_stack_;
		enum kind_function_ {
			start_dict_,
			key_,
			value_,
			end_dict_,
			start_array_,
			end_array_,
			build_
		};
		std::vector<kind_function_>allowed_function_;
		std::deque<std::string> keys_stack_;


		class BaseContext {

		public:
			BaseContext(Builder& builder) : builder_(builder) {
			}
			DictItemContext StartDict();
			DictValueContext Key(std::string key);
			Builder& EndDict();
			ArrayItemContext StartArray();
			Builder& EndArray();
			Node Build();
		protected:
			Builder& builder_;
		};

		class DictItemContext : public BaseContext {
		public:
			DictItemContext(Builder& builder) : BaseContext(builder) {}
			DictItemContext(DictItemContext& other) : BaseContext(other.builder_) {}

			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			Builder& EndArray() = delete;
			Node Build() = delete;

		};

		class DictValueContext : public BaseContext {
		public:
			DictValueContext(Builder& builder) : BaseContext(builder) {}
			DictValueContext(DictValueContext& other) : BaseContext(other.builder_) {}

			DictValueContext Key(std::string key) = delete;
			Builder& EndDict() = delete;
			Builder& EndArray() = delete;
			Node Build() = delete;
			DictItemContext Value(Node::Value value);

		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(Builder& builder) : BaseContext(builder) {}
			ArrayItemContext(ArrayItemContext& other) : BaseContext(other.builder_) {}

			Builder& Key(std::string key) = delete;
			Builder& EndDict() = delete;
			Node Build() = delete;
			ArrayItemContext Value(Node::Value value);

		};

	};
}
