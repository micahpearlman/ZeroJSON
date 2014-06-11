#include "zoJSON.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;
namespace zo {

// Try to consume characters from the input stream and match the
// pattern string.
bool match(const char* pattern, std::istream& input) {
    input >> std::ws;
    const char* cur(pattern);
    char ch(0);
    while(input && !input.eof() && *cur != 0) {
        input.get(ch);
        if (ch != *cur) {
            input.putback(ch);
            return false;
        } else {
            cur++;
        }
    }
    return *cur == 0;
}

bool parse_string(std::istream& input, std::string* value) {
    if (!match("\"", input))  {
        return false;
    }
    char ch = -1;
    while(!input.eof() && input.good()) {
        input.get(ch);
        if (ch == '"') {
            break;
        }
        if (ch == '\\') {
            input.get(ch);
            switch(ch) {
                case '"':
                case '\\':
                case '/':
                    value->push_back(ch);
                    break;
                case 'b':
                    value->push_back('\b');
                    break;
                case 'f':
                    value->push_back('\f');
                    break;
                case 'n':
                    value->push_back('\n');
                    break;
                case 'r':
                    value->push_back('\r');
                    break;
                case 't':
                    value->push_back('\t');
                    break;
                default:
                    value->push_back('\\');
                    value->push_back(ch);
                    break;
            }
        } else {
            value->push_back(ch);
        }
    }
    if (input && ch == '"') {
        return true;
    } else {
        return false;
    }
}

bool parse_bool(std::istream& input, bool* value) {
	
    if (match("true", input))  {
        *value = true;
        return true;
    }
    if (match("false", input)) {
        *value = false;
        return true;
    }
    return false;
}

bool parse_null(std::istream& input) {
    if (match("null", input))  {
        return true;
    }
    return false;
}

bool parse_number(std::istream& input, double* value) {
    input >> std::ws;
    input >> *value;
    if (input.fail()) {
        input.clear();
        return false;
    }
    return true;
}

Object::Object() : _value_map() {}

Object::~Object() {
//    std::map<std::string, Value>::iterator i;
//    for (i = _value_map.begin(); i != _value_map.end(); ++i) {
//        delete i->second;
//    }
//	cout << "######Destory ZeroJSON::Object:" << endl;
//	describe();
}

bool Object::parse(std::istream& input, Object& object) {
    object._value_map.clear();

    if (!match("{", input)) {
        return false;
    }

    do {
        std::string key;
        if (!parse_string(input, &key)) {
            return false;
        }
        if (!match(":", input)) {
            return false;
        }
        Value v;
        if (!Value::parse(input, v)) {
            break;
        }
        object._value_map[key] = v;
    } while (match(",", input));

    if (!match("}", input)) {
        return false;
    }
    return true;
}
	
	void Object::describe() const {
		std::map<std::string, Value>::const_iterator it;
		for ( it = _value_map.begin(); it != _value_map.end(); it++ ) {
			cout << "\tkey: " << it->first << endl;
			it->second.describe();
		}
	}


Value::Value() : _type(kTypeInvalid) {}

void Value::reset() {
	_value = boost::any();
}

bool Value::parse(std::istream& input, Value& value) {
    value.reset();

    std::string string_value;
    if (parse_string(input, &string_value)) {
        value._value = string_value;
        value._type = kTypeString;
        return true;
    }
	bool bool_value;
    if (parse_bool(input, &bool_value)) {
		value._value = bool_value;
        value._type = kTypeBool;
        return true;
    }
	double double_value;
    if (parse_number(input, &double_value)) {
		value._value = double_value;
        value._type = kTypeNumber;
        return true;
    }

    if (parse_null(input)) {
        value._type = kTypeNull;
        return true;
    }
    if (input.peek() == '[') {
		Array array_value;
        
        if (Array::parse(input, array_value)) {
			value._value = array_value;
            value._type = kTypeArray;
            return true;
        }
    }

	Object object_value;
    if (Object::parse(input, object_value)) {
		value._value = object_value;
        value._type = kTypeObject;
        return true;
    }
    return false;
}
	
	void Value::describe() const {
		switch (_type) {
			case kTypeNumber:
				std::cout << "number: " << get<double>() << std::endl; 
				break;
			case kTypeString:
				std::cout << "string: " << get<std::string>() << std::endl; 
				break;
			case kTypeBool:
				std::cout << "bool: " << get<bool>() << std::endl; 
				break;
			case kTypeNull:
				std::cout << "null: " << std::endl; 
				break;
			case kTypeArray:
				std::cout << "array: " << std::endl; 
				get<Array>().describe();
				break;
			case kTypeObject:
				std::cout << "object: " << std::endl; 
				get<Object>().describe();
				break;
				
				
			default:
				break;
		}
	}
	

Array::Array() : _values() {}
	Array::Array( int sz ) 
	:	_values()
	{
		reserve( sz );
	}

Array::~Array() {
//    for (unsigned int i = 0; i < _values.size(); ++i) {
//        delete _values[i];
//    }
}

bool Array::parse(std::istream& input, Array& array) {
    array._values.clear();

    if (!match("[", input)) {
        return false;
    }

    do {
        Value v;
        if (!Value::parse(input, v)) {
            break;
        }
        array._values.push_back(v);
    } while (match(",", input));

    if (!match("]", input)) {
        return false;
    }
    return true;
}
	
	void Array::describe() const {
		for (unsigned int i = 0; i < _values.size(); ++i) {
			_values[i].describe();
		}
	}

static std::ostream& stream_string(std::ostream& stream,
                                   const std::string& string) {
    stream << '"';
    for (std::string::const_iterator i = string.begin(),
                 e = string.end(); i != e; ++i) {
        switch (*i) {
            case '"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '/':
                stream << "\\/";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (*i < 32) {
                    stream << "\\u" << std::hex << std::setw(6) <<
                            std::setfill('0') << static_cast<int>(*i) << std::dec <<
                            std::setw(0);
                } else {
                    stream << *i;
                }
        }
    }
    stream << '"';
    return stream;
}

}  // namespace jsonxx

std::ostream& operator<<(std::ostream& stream, const zo::Value& v) {
    if (v.is<double>()) {
        return stream << v.get<double>();
    } else if (v.is<std::string>()) {
        return zo::stream_string(stream, v.get<std::string>());
    } else if (v.is<bool>()) {
        if (v.get<bool>()) {
            return stream << "true";
        } else {
            return stream << "false";
        }
    } else if (v.is<zo::Value::Null>()) {
        return stream << "null";
    } else if (v.is<zo::Object>()) {
        return stream << v.get<zo::Object>();
    } else if (v.is<zo::Array>()){
        return stream << v.get<zo::Array>();
    }
    // Shouldn't reach here.
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const zo::Array& v) {
    stream << "[";
    const std::vector<zo::Value>& values(v.values());
    for (unsigned int i = 0; i < values.size()-1; ++i) {
        stream << values[i] << ", ";
    }
    return stream << values[values.size()-1] << "]";
}

std::ostream& operator<<(std::ostream& stream, const zo::Object& v) {
    stream << "{";
    const std::map<std::string, zo::Value>& kv(v.kv_map());
    for (std::map<std::string, zo::Value>::const_iterator i = kv.begin();
         i != kv.end(); /**/) {
        zo::stream_string(stream, i->first);
        stream << ": " << i->second;
        ++i;
        if ( i != kv.end()) {
            stream << ", ";
        }
    }
    return stream << "}";
}
