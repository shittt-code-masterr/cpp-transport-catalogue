


#include "json.h"

namespace json {

	
		using namespace std::literals;



		Node::Node(const Value& value) {
			if (std::holds_alternative<int>(value)) {
				this->emplace<int>(std::get<int>(value));
			}
			else if (std::holds_alternative<double>(value)) {
				this->emplace<double>(std::get<double>(value));
			}
			else if (std::holds_alternative<Array>(value)) {
				this->emplace<Array>(std::get<Array>(value));
			}
			else if (std::holds_alternative<std::string>(value)) {
				this->emplace<std::string>(std::get<std::string>(value));
			}
			else if (std::holds_alternative<Dict>(value)) {
				this->emplace<Dict>(std::get<Dict>(value));
			}
			else if (std::holds_alternative<bool>(value)) {
				this->emplace<bool>(std::get<bool>(value));
			}
			else {
				this->emplace<std::nullptr_t>(nullptr);
			}
		}

		bool Node::IsInt() const {
			return std::holds_alternative<int>(*this);
		}

		int Node::AsInt() const {
			using namespace std::literals;
			if (!IsInt()) {
				throw std::logic_error("Not an int"s);
			}
			return std::get<int>(*this);
		}

		bool Node::IsPureDouble() const {
			return std::holds_alternative<double>(*this);
		}

		bool Node::IsDouble() const {
			return IsInt() || IsPureDouble();
		}

		double Node::AsDouble() const {
			using namespace std::literals;
			if (!IsDouble()) {
				throw std::logic_error("Not a double"s);
			}
			return IsPureDouble() ? std::get<double>(*this) : AsInt();
		}

		bool Node::IsBool() const {
			return std::holds_alternative<bool>(*this);
		}
		bool Node::AsBool() const {
			using namespace std::literals;
			if (!IsBool()) {
				throw std::logic_error("Not a bool"s);
			}

			return std::get<bool>(*this);
		}

		bool Node::IsNull() const {
			return std::holds_alternative<std::nullptr_t>(*this);
		}

		bool Node::IsArray() const {
			return std::holds_alternative<Array>(*this);
		}
		const Array& Node::AsArray() const {
			using namespace std::literals;
			if (!IsArray()) {
				throw std::logic_error("Not an array"s);
			}

			return std::get<Array>(*this);
		}

		bool Node::IsString() const {
			return std::holds_alternative<std::string>(*this);
		}
		const std::string& Node::AsString() const {
			using namespace std::literals;
			if (!IsString()) {
				throw std::logic_error("Not a string"s);
			}

			return std::get<std::string>(*this);
		}

		bool Node::IsMap() const {
			return std::holds_alternative<Dict>(*this);
		}
		const Dict& Node::AsMap() const {
			using namespace std::literals;
			if (!IsMap()) {
				throw std::logic_error("Not a dict"s);
			}

			return std::get<Dict>(*this);
		}

		bool Node::operator==(const Node& rhs) const {
			return GetValue() == rhs.GetValue();
		}

		inline bool operator!=(const Node& lhs, const Node& rhs) {
			return !(rhs == lhs);
		}
		const Node::Value& Node::GetValue() const {
			return *this;
		}


		Document::Document(Node root)
			: root_(std::move(root)) {
		}

		const Node& Document::GetRoot() const {
			return root_;
		}

		inline bool operator==(const Document& lhs, const Document& rhs) {
			return lhs.GetRoot() == rhs.GetRoot();
		}

		inline bool operator!=(const Document& lhs, const Document& rhs) {
			return !(lhs == rhs);
		}
		Node LoadNode(std::istream& input);
		Node LoadString(std::istream& input);

		std::string LoadLiteral(std::istream& input) {
			std::string result_literal;
			while (std::isalpha(input.peek())) {
				result_literal.push_back(static_cast<char>(input.get()));
			}
			return result_literal;
		}

		Node LoadArray(std::istream& input) {
			std::vector<Node> result;

			for (char character; input >> character && character != ']';) {
				if (character != ',') {
					input.putback(character);
				}
				result.push_back(LoadNode(input));
			}
			if (!input) {
				throw ParsingError("Array parsing error"s);
			}
			return Node(std::move(result));
		}

		Node LoadDict(std::istream& input) {
			Dict dict;

			for (char character; input >> character && character != '}';) {
				if (character == '"') {
					std::string key = LoadString(input).AsString();
					if (input >> character && character == ':') {
						if (dict.find(key) != dict.end()) {
							throw ParsingError("Duplicate key '"s + key + "' have been found"s);
						}
						dict.emplace(std::move(key), LoadNode(input));
					}
					else {
						throw ParsingError(": is expected but '"s + character + "' has been found"s);
					}
				}
				else if (character != ',') {
					throw ParsingError(R"(',' is expected but ')"s + character + "' has been found"s);
				}
			}
			if (!input) {
				throw ParsingError("Dictionary parsing error"s);
			}
			return Node(std::move(dict));
		}

		Node LoadString(std::istream& input) {
			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			std::string input_string;
			while (true) {
				if (it == end) {
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					++it;
					break;
				}
				else if (ch == '\\') {
					++it;
					if (it == end) {
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					switch (escaped_char) {
					case 'n':
						input_string.push_back('\n');
						break;
					case 't':
						input_string.push_back('\t');
						break;
					case 'r':
						input_string.push_back('\r');
						break;
					case '"':
						input_string.push_back('"');
						break;
					case '\\':
						input_string.push_back('\\');
						break;
					default:
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					input_string.push_back(ch);
				}
				++it;
			}

			return Node(std::move(input_string));
		}

		Node LoadBool(std::istream& input) {
			const std::string literal = LoadLiteral(input);
			if (literal == "true"sv) {
				return Node{ true };
			}
			else if (literal == "false"sv) {
				return Node{ false };
			}
			else {
				throw ParsingError("Failed to parse '"s + literal + "' as bool"s);
			}
		}

		Node LoadNull(std::istream& input) {
			if (std::string literal = LoadLiteral(input); literal == "null"sv) {
				return Node{ nullptr };
			}
			else {
				throw ParsingError("Failed to parse '"s + literal + "' as null"s);
			}
		}

		Node LoadNumber(std::istream& input) {
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
			if (int character = input.peek(); character == 'e' || character == 'E') {
				read_char();
				if (character = input.peek(); character == '+' || character == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return std::stoi(parsed_num);
					}
					catch (...) {
						// В случае неудачи, например, при переполнении
						// код ниже попробует преобразовать строку в double
					}
				}
				return std::stod(parsed_num);
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadNode(std::istream& input) {
			char character;
			if (!(input >> character)) {
				throw ParsingError("Unexpected EOF"s);
			}
			switch (character) {
			case '[':
				return LoadArray(input);
			case '{':
				return LoadDict(input);
			case '"':
				return LoadString(input);
			case 't':
				// Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
				// подсказкой компилятору и человеку, что здесь программист явно задумывал
				// разрешить переход к инструкции следующей ветки case, а не случайно забыл
				// написать break, return или throw.
				// В данном случае, встретив t или f, переходим к попытке парсинга
				// литералов true либо false
				[[fallthrough]];
			case 'f':
				input.putback(character);
				return LoadBool(input);
			case 'n':
				input.putback(character);
				return LoadNull(input);
			default:
				input.putback(character);
				return LoadNumber(input);
			}
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
			for (const char character : value) {
				switch (character) {
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
					out.put(character);
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
					ctx.out;
				},
				node.GetValue());
		}

	  // namespace

	Document Load(std::istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		PrintNode(doc.GetRoot(), PrintContext{ output });
	}

}  // namespace json