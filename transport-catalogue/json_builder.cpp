#include "json_builder.h"

using namespace std::literals;

namespace json {

	KeyItemContext& Builder::Key(std::string str) {
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (is_key_) {
			throw std::logic_error("Key after key"s);
		}
		if (!(nodes_stack_.back()->IsDict())) {
			throw std::logic_error("Key at not a dict"s);
		}
		nodes_key_ = str;
		is_key_ = true;
		return *this;
	}

	Builder& Builder::Value(Node::Value val) {
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (nodes_stack_.empty()) {
			root_ = val;
			is_ready_ = true;
			return *this;
		}
		
		if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(val);
		}
		else if (nodes_stack_.back()->IsDict() && is_key_) {
			nodes_stack_.back()->AsDict().insert({ nodes_key_, val });
			nodes_key_.clear();
			is_key_ = false;
		}
		else {
			throw std::logic_error("Value constructor/key/elem of array"s);
		}
		return *this;

	}

	DictItemContext& Builder::StartDict(){
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (nodes_stack_.empty()) {
			root_ = Node{ Dict{} };
			nodes_stack_.push_back(&root_);
			return *this;
		}
		if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(Node{ Dict{} });
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if (nodes_stack_.back()->IsDict() && is_key_){
			nodes_stack_.back()->AsDict().insert({ nodes_key_, Node{Dict{}} });
			Node& it = nodes_stack_.back()->AsDict().find(nodes_key_)->second;
			nodes_stack_.push_back(&it);
			nodes_key_.clear();
			is_key_ = false;
		}
		else {
			throw std::logic_error("Value constructor/key/elem of array"s);
		}
		return *this;
	}

	ArrayItemContext& Builder::StartArray(){
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (nodes_stack_.empty()) {
			root_ = Node{ Array{} };
			nodes_stack_.push_back(&root_);
			return *this;
		}
		if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(Node{ Array{} });
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if (nodes_stack_.back()->IsDict() && is_key_) {
			nodes_stack_.back()->AsDict().insert({ nodes_key_, Node{Array{}} });
			Node& it = nodes_stack_.back()->AsDict().find(nodes_key_)->second;
			nodes_stack_.push_back(&it);
			nodes_key_.clear();
			is_key_ = false;
		}
		else {
			throw std::logic_error("Value constructor/key/elem of array"s);
		}
		
		return *this;
	}

	Builder& Builder::EndDict(){
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (!(nodes_stack_.back()->IsDict())) {
			throw std::logic_error("Element is not a Dict"s);
		}
		nodes_stack_.pop_back();
		if (nodes_stack_.size() == 0) {
			is_ready_ = true;
		}
		return *this;
	}

	Builder& Builder::EndArray(){
		if (is_ready_) {
			throw std::logic_error("Json already prepared"s);
		}
		if (!(nodes_stack_.back()->IsArray())) {
			throw std::logic_error("Element is not an Array"s);
		}
		nodes_stack_.pop_back();
		if (nodes_stack_.size() == 0) {
			is_ready_ = true;
		}
		return *this;
	}

	Node Builder::Build(){
		if (!is_ready_) {
			throw std::logic_error("Can't build. Not ready json"s);
		}
		return root_;
	}

} // namespace json
