#pragma once

#include<stack>
#include"json.h"

namespace json {

	class Builder
	{
	public:
		Builder();

		Builder& Value(Node value_);

		Builder& Key(std::string key_);

		Builder& StartDict();

		Builder& StartArray();

		Builder& EndArray();

		Builder& EndDict();

		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
	};


}//json