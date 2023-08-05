#include "json_builder.h"

namespace json {
    Builder::Builder() {
        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error("root has been added");
            }
            else {
                nodes_stack_.push_back(&root_);
            }
        }
    }
    Builder& Builder::Value(Node value_) {
         if (nodes_stack_.empty()) {
            nodes_stack_.push_back(&value_);

        }

        else if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value_);
        }
        else if(!nodes_stack_.empty()) {
            *nodes_stack_.back() = value_;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        }
        
        else {
            throw std::logic_error("root has been added");


        }
        return *this;
    }

    Builder& Builder::Key(std::string key_) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("empty json");
        }

        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_]);
        return *this;
    }

    Builder& Builder::StartDict() {

        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else if(!nodes_stack_.empty())
        {
            *nodes_stack_.back() = Dict();
        }
        else {
            throw std::logic_error("");

        }
        return *this;
    }

    Builder& Builder::StartArray() {
        if (!nodes_stack_.empty()&&nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else if(!nodes_stack_.empty())
        {
            *nodes_stack_.back() = Array();
        }
        else {
            throw std::logic_error("");
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {

            Node node = *nodes_stack_.back();

            nodes_stack_.erase(nodes_stack_.end() - 1);
            return *this;
        }
        else {
            throw std::logic_error("object ");

        }
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {

            Node node = *nodes_stack_.back();

            nodes_stack_.erase(nodes_stack_.end() - 1);
            return *this;
        }
        else {

            throw std::logic_error("object isn't dictionary");
        }
    }

    Node Builder::Build() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            return std::move(root_);
            
        }else{
            throw std::logic_error("invalid json");
        }

        
    }
}