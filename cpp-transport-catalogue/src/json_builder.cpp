

#include "json_builder.h"



namespace json
{

	Builder::KeyContext Builder::Key(std::string key)
	{
		if (!Builder::dict_active && Builder::key_active)
		{
			throw std::logic_error("Key called without a dictionary started");
		}
		key_ = std::move(key);
		key_active = true;
		return KeyContext(*this);
	}

	Builder::ValueContext Builder::Value(Node::Value val)
	{

		if (std::holds_alternative<nullptr_t>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<nullptr_t>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<nullptr_t>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<nullptr_t>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}
		if (std::holds_alternative<Array>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<Array>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<Array>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<Array>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}
		if (std::holds_alternative<Dict>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<Dict>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<Dict>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<Dict>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}

		}
		if (std::holds_alternative<bool>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<bool>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<bool>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<bool>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}
		if (std::holds_alternative<double>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<double>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<double>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<double>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}
		if (std::holds_alternative<int>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<int>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<int>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<int>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}

		if (std::holds_alternative<std::string>(val))
		{
			if (key_active && dict_active)
			{
				dict_.insert({ key_,  std::get<std::string>(val) });
				key_active = false;
			}
			else if (array_active && !key_active && !dict_active)
			{
				arr_.push_back(std::get<std::string>(val));
			}
			else if (first_elem)
			{
				first_ = std::get<std::string>(val);

				first_elem = false;
			}
			else
			{
				throw std::logic_error("Value sequence has broken");
			}
		}

		return ValueContext(*this);
	}
	Builder::DictValueContext Builder::StartDict()
	{
		if (dict_active)
		{
			if (!key_active)
			{
				throw std::logic_error("Wrong StartDict() Flag(without key in dict added)");
			}
			key_active = false;
			node_stack_.push_back(new Node(std::move(key_)));
			node_stack_.push_back(new Node(std::move(dict_)));
			dict_.clear();
		}
		else if (array_active)
		{
			node_stack_.push_back(new Node(std::move(arr_)));
			arr_.clear();
			array_active = false;
		}
		else if (first_elem)
		{
			first_elem = false;
		}
		else { throw std::logic_error("Wrong StartDict() Flag"); }
		dict_active = true;

		return DictValueContext(*this);
	}

	Builder::ValueContext Builder::EndDict()
	{
		if (first_elem || array_active || key_active || !dict_active)
		{
			throw std::logic_error("Incorrect calling EndDict()");
		}

		if (node_stack_.empty())
		{
			dict_active = false;
			first_ = Node(std::move(dict_));
		}
		else if (node_stack_.back()->IsArray())
		{
			dict_active = false;
			array_active = true;
			arr_ = std::move(node_stack_.back()->AsArrayForMove());
			delete node_stack_.back();
			node_stack_.pop_back();
			arr_.push_back(std::move(dict_));

		}
		else if (node_stack_.back()->IsMap())
		{
			Dict bf = std::move(dict_);
			dict_ = std::move(node_stack_.back()->AsMapForMove());
			delete node_stack_.back();
			node_stack_.pop_back();
			key_ = node_stack_.back()->AsString();
			delete node_stack_.back();
			node_stack_.pop_back();
			dict_.insert({ key_, std::move(bf) });

		}
		if (!dict_active)
		{
			dict_.clear();
		}
		return ValueContext(*this);
	}

	Builder::ArrayValueContext Builder::StartArray()
	{
		if (dict_active)
		{
			if (!key_active)
			{
				throw std::logic_error("Wrong StartArray() Flag(without key in dict added)");
			}
			key_active = false;
			node_stack_.push_back(new Node(std::move(key_)));
			node_stack_.push_back(new Node(std::move(dict_)));
			dict_.clear();
			dict_active = false;
		}
		else if (array_active)
		{
			node_stack_.push_back(new Node(std::move(arr_)));
			arr_.clear();

		}
		else if (first_elem)
		{
			first_elem = false;
		}
		else { throw std::logic_error("Wrong StartArray() Flag"); }
		array_active = true;

		return ArrayValueContext(*this);
	}

	Builder::ValueContext Builder::EndArray()
	{
		if (first_elem || dict_active || key_active || !array_active)
		{
			throw std::logic_error("Incorrect calling EndArray()");
		}

		if (node_stack_.empty())
		{
			array_active = false;
			first_ = Node(std::move(arr_));
		}
		else if (node_stack_.back()->IsArray())
		{

			Array bf = std::move(arr_);
			arr_ = std::move(node_stack_.back()->AsArrayForMove());
			delete node_stack_.back();
			node_stack_.pop_back();
			arr_.push_back(std::move(bf));

		}
		else if (node_stack_.back()->IsMap())
		{
			array_active = false;
			dict_active = true;
			dict_ = std::move(node_stack_.back()->AsMapForMove());
			delete node_stack_.back();
			node_stack_.pop_back();
			key_ = node_stack_.back()->AsString();
			delete node_stack_.back();
			node_stack_.pop_back();
			dict_.insert({ key_, std::move(arr_) });

		}

		if (!array_active) { arr_.clear(); }

		return ValueContext(*this);
	}

	Node Builder::Build()
	{
		if (key_active || dict_active || array_active || first_elem)
		{
			throw std::logic_error("Incorrect build");
		}
		return Node(std::move(first_));

	}

	Builder::~Builder()
	{
		for (auto& node : node_stack_)
		{
			delete node;
		}
	}

	Builder::KeyContext Builder::BaseContext::Key(std::string key)
	{
		return builder_.Key(std::move(key));
	}

	Builder::DictValueContext Builder::BaseContext::StartDict()
	{
		return builder_.StartDict();
	}
	Builder::ArrayValueContext Builder::BaseContext::StartArray()
	{
		return builder_.StartArray();
	}
	Builder::ValueContext Builder::BaseContext::EndDict()
	{
		return builder_.EndDict();
	}
	Builder::ValueContext Builder::BaseContext::EndArray()
	{
		return builder_.EndArray();
	}
	Node Builder::BaseContext::Build()
	{
		return builder_.Build();
	}

	Builder::DictValueContext Builder::KeyContext::Value(Node::Value val)
	{
		auto default_value = ValueContext(builder_);
		default_value.Value(std::move(val));
		return DictValueContext(default_value.builder_);
	}

	Builder::ValueContext Builder::ValueContext::Value(Node::Value val)
	{
		return builder_.Value(std::move(val));
	}

	Builder::ArrayValueContext Builder::ArrayValueContext::Value(Node::Value val)
	{
		ValueContext default_(builder_);
		default_.Value(std::move(val));
		return ArrayValueContext(default_.builder_);
	}

}