#pragma once

#include "json.h"

#include <vector>
#include <string>

namespace json {

	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class Builder;

class DictItemContext {
public:
	virtual KeyItemContext& Key(std::string str) = 0;
	virtual Builder& EndDict() = 0;
	virtual ~DictItemContext() = default;
};

class ArrayItemContext {
public:
	virtual ArrayItemContext& Value(Node::Value val) = 0;
	virtual DictItemContext& StartDict() = 0;
	virtual ArrayItemContext& StartArray() = 0;
	virtual Builder& EndArray() = 0;
	virtual ~ArrayItemContext() = default;
};

class KeyItemContext {
public:
	virtual DictItemContext& Value(Node::Value val) = 0;
	virtual ArrayItemContext& StartArray() = 0;
	virtual DictItemContext& StartDict() = 0;
	virtual ~KeyItemContext() = default;
};

class Builder : public DictItemContext, ArrayItemContext, KeyItemContext {
public:

	KeyItemContext& Key(std::string str) override;
	Builder& Value(Node::Value val) override;
	DictItemContext& StartDict() override;
	ArrayItemContext& StartArray() override;
	Builder& EndDict() override;
	Builder& EndArray() override;
	Node Build();
	~Builder() override = default;


private:
	Node root_;
	std::vector<Node*> nodes_stack_;
	std::string nodes_key_;
	bool is_ready_ = false;
	bool is_key_ = false;

};

} // namespace json