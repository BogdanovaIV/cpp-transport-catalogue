#include "json_builder.h"

namespace json {

	DictContext BaseContext::StartDict() {
		return builder_.StartDict();
	}
	
	KeyContext BaseContext::Key(std::string key) {
		return builder_.Key(key);
	}

	Builder& BaseContext::EndDict() {
		builder_.EndDict();
		return builder_;
	}

	StartArrayContext BaseContext::StartArray() {	
		return builder_.StartArray();;
	}

	Builder& BaseContext::EndArray() {
		builder_.EndArray();
		return builder_;
	}

	Node& BaseContext::Build() {
		return builder_.Build();
	}

	ValueDictContext KeyContext::Value(Node::Value value) {
		return builder_.ValueDict(value);
	}

	StartArrayContext StartArrayContext::Value(Node::Value value) {
		builder_.Value(value);
		return StartArrayContext{ builder_ };
	}

	DictContext Builder::StartDict() {
		kind_function Kfunction = kind_function::start_dict_;
		if (!allowed_function_.empty() && !std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error StartDict (wrong step).");
		}
		Node result(Dict{});
		if (root_.IsNull()) {
			root_ = std::move(result);
			nodes_stack_.push_back(&root_);
		}
		else {
			if (nodes_stack_.empty()) {
				throw std::logic_error("Logic error StartDict (didn't find right parent).");
			}
			else {
				Node* last_action = nodes_stack_.back();

				if (last_action->IsArray()) {
					Array& array = const_cast<Array&>(last_action->AsArray());
					array.emplace_back(std::move(result));
					Node& array_back = array.back();
					nodes_stack_.emplace_back(&array_back);
				}
				else if (last_action->IsDict()) {
					Dict& dict = const_cast<Dict&>(last_action->AsDict());
					std::string last_key = keys_stack_.back();
					keys_stack_.pop_back();
					
					dict[last_key] = std::move(result);
					Node& dict_back = dict[last_key];
					nodes_stack_.emplace_back(&dict_back);
				}
				else {
					throw std::logic_error("Logic error StartDict (didn't find right parent).");
				}
			}
		}

		allowed_function_.clear();
		allowed_function_.push_back(kind_function::key_);
		allowed_function_.push_back(kind_function::end_dict_);
		//auto res = DictContext{ *this };
		return DictContext{ *this };
	}

	KeyContext Builder::Key(std::string key) {
		kind_function Kfunction = kind_function::key_;
		if (! std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error Key (wrong step).");
		}

		if (nodes_stack_.empty()) {
			throw std::logic_error("Logic error Key (didn't find right parent).");
		}
		else {

			Node* last_action = nodes_stack_.back();

			if (last_action->IsDict()) {
				Dict& dict = const_cast<Dict&>(last_action->AsDict());
				dict.emplace(key, nullptr);
				keys_stack_.push_back(std::move(key));
			}
			else {
				throw std::logic_error("Logic error Key (didn't find right parent).");
			}
		}
		allowed_function_.clear();
		allowed_function_.push_back(kind_function::value_);
		allowed_function_.push_back(kind_function::start_array_);
		allowed_function_.push_back(kind_function::start_dict_);
		KeyContext res{ *this };
		return res;
	}

	Builder& Builder::Value(Node::Value value) {
		kind_function Kfunction = kind_function::value_;
		if (!allowed_function_.empty() && !std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error Value (wrong step).");
		}
		if (root_.IsNull()) {
			root_ = Node(value);

			allowed_function_.clear();
			allowed_function_.push_back(kind_function::build_);
		}
		else {
			if (nodes_stack_.empty()) {
				throw std::logic_error("Logic error StartDict (didn't find right parent).");
			}
			else {
				Node* last_action = nodes_stack_.back();

				if (last_action->IsArray()) {
					Array& array = const_cast<Array&>(last_action->AsArray());
					array.emplace_back(Node(value));

					allowed_function_.clear();
					allowed_function_.push_back(kind_function::value_);
					allowed_function_.push_back(kind_function::start_dict_);
					allowed_function_.push_back(kind_function::end_array_);

				}
				else if (last_action->IsDict()) {
					Dict& dict = const_cast<Dict&>(last_action->AsDict());
					std::string last_key = keys_stack_.back();
					keys_stack_.pop_back();

					dict[last_key] = Node(value);

					allowed_function_.clear();
					allowed_function_.push_back(kind_function::key_);
					allowed_function_.push_back(kind_function::end_dict_);
				}
				else {
					throw std::logic_error("Logic error StartDict (didn't find right parent).");
				}
			}
		}

		return *this;

	}

	ValueDictContext Builder::ValueDict(Node::Value value) {
		kind_function Kfunction = kind_function::value_;
		if (!allowed_function_.empty() && !std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error Value (wrong step).");
		}
		if (root_.IsNull()) {
			root_ = Node(value);

			allowed_function_.clear();
			allowed_function_.push_back(kind_function::build_);
		}
		else {
			if (nodes_stack_.empty()) {
				throw std::logic_error("Logic error StartDict (didn't find right parent).");
			}
			else {
				Node* last_action = nodes_stack_.back();

				if (last_action->IsDict()) {
					Dict& dict = const_cast<Dict&>(last_action->AsDict());
					std::string last_key = keys_stack_.back();
					keys_stack_.pop_back();

					dict[last_key] = Node(value);

					allowed_function_.clear();
					allowed_function_.push_back(kind_function::key_);
					allowed_function_.push_back(kind_function::end_dict_);
				}
				else {
					throw std::logic_error("Logic error StartDict (didn't find right parent).");
				}
			}
		}

		return ValueDictContext{ *this };

	}

	Builder& Builder::EndDict() {
		kind_function Kfunction = kind_function::end_dict_;
		if (!std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error EndDict (wrong step).");
		}
		if (nodes_stack_.empty()) {
			throw std::logic_error("Logic error StartDict (didn't find right parent).");
		}
		else {
			Node* last_action = nodes_stack_.back();

			if (last_action->IsDict()) {
				nodes_stack_.pop_back();
			}
			else {
				throw std::logic_error("Logic error EndDict (didn't find right parent).");
			}
		}
		
		allowed_function_.clear();
		if (!nodes_stack_.empty()) {
			allowed_function_.push_back(kind_function::value_);
			allowed_function_.push_back(kind_function::end_array_);
			allowed_function_.push_back(kind_function::key_);
			allowed_function_.push_back(kind_function::start_dict_);
		}
		else {
			allowed_function_.push_back(kind_function::build_);
		}

		return *this ;
	}

	StartArrayContext Builder::StartArray() {
		kind_function Kfunction = kind_function::start_array_;
		if (!allowed_function_.empty() && !std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error StartArray (wrong step).");
		}
		Node result(Array{});
		if (root_.IsNull()) {
			root_ = std::move(result);
			nodes_stack_.push_back(&root_);
		}
		else {
			if (nodes_stack_.empty()) {
				throw std::logic_error("Logic error StartArray (didn't find right parent).");
			}
			else {
				Node* last_action = nodes_stack_.back();

				if (last_action->IsArray()) {
					Array& array = const_cast<Array&>(last_action->AsArray());
					array.emplace_back(std::move(result));
					Node& array_back = array.back();
					nodes_stack_.emplace_back(&array_back);
				}
				else if (last_action->IsDict()) {
					Dict& dict = const_cast<Dict&>(last_action->AsDict());
					std::string last_key = keys_stack_.back();
					keys_stack_.pop_back();

					dict[last_key] = std::move(result);
					Node& dict_back = dict[last_key];
					nodes_stack_.emplace_back(&dict_back);
				}
				else {
					throw std::logic_error("Logic error StartArray (didn't find right parent).");
				}
			}
		}

		allowed_function_.clear();
		allowed_function_.push_back(kind_function::start_array_);
		allowed_function_.push_back(kind_function::start_dict_);
		allowed_function_.push_back(kind_function::value_);
		allowed_function_.push_back(kind_function::end_array_);

		return StartArrayContext{ *this };

	}

	Builder& Builder::EndArray() {
		kind_function Kfunction = kind_function::end_array_;
		if (!std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) {
			throw std::logic_error("Logic error EndArray (wrong step).");
		}
		if (nodes_stack_.empty()) {
			throw std::logic_error("Logic error EndArray (didn't find right parent).");
		}
		else {
			Node* last_action = nodes_stack_.back();

			if (last_action->IsArray()) {
				nodes_stack_.pop_back();
			}
			else {
				throw std::logic_error("Logic error EndArray (didn't find right parent).");
			}
		}

		allowed_function_.clear();
		if (!nodes_stack_.empty()) {
			allowed_function_.push_back(kind_function::value_);
			allowed_function_.push_back(kind_function::end_dict_);
			allowed_function_.push_back(kind_function::key_);
		}
		else {
			allowed_function_.push_back(kind_function::build_);
		}

		return *this;

	}

	Node& Builder::Build() {
		kind_function Kfunction = kind_function::build_;
		if ((!allowed_function_.empty() && !std::count(allowed_function_.begin(), allowed_function_.end(), Kfunction)) 
			|| !nodes_stack_.empty()
			|| root_.IsNull()) {
			throw std::logic_error("Logic error Build (wrong step).");
		}
		return root_;

	}
}