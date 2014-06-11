//
//  zoJSON.h
//
//  Created by Micah Pearlman on 5/30/11.
//  Copyright 2011 Zero Engineering. All rights reserved.
//
// Heavily based off jsonxx: 
// Author: Hong Jiang <hong@hjiang.net>
// Contributors:
//   Sean Middleditch <sean@middleditch.us>

#ifndef _jsonxx_h__
#define _jsonxx_h__

#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <boost/any.hpp>

namespace zo {

	// A value could be a number, an array, a string, an object, a
	// boolean, or null
	class Array;
	class Object;
	
	class Value {
	public:
		class Null {};
		
		Value();
		Value( const char* c ) {
			_value = std::string(c);
			_type = kTypeString;
		}
		Value( const std::string& s ) {
			_value = s;
			_type = kTypeString;
		}
		Value( double d ) {
			_value = d;
			_type = kTypeNumber;
		}
		Value( bool b ) {
			_value = b;
			_type = kTypeBool;
		}
		Value( const Object& o ) {
			_value = o;
			_type = kTypeObject;
		}
		Value( const Array& a ) {
			_value = a;
			_type = kTypeArray;
		}
		

		~Value() { reset(); }
		void reset();
		
		static bool parse(std::istream& input, Value& value);
		
		template<typename T> bool is() const;
		
		template<typename T> T get() const;
		template<typename T> T get();

		Value(const Value& o ) {
			_value = o._value;
			_type = o._type;
		}
		
		Value& operator=(const Value& o) {
			_value = o._value;
			_type = o._type;
			return *this;
		}
		
		Value operator[]( int i ); 
		const Value operator[]( int i ) const; 
		
		Value operator[]( const char* k );
		const Value operator[]( const char* k ) const;
		
		Value operator[]( const std::string& k );
		const Value operator[]( const std::string& k ) const;
		
		inline bool has( const std::string& key ) const;
		

		
		void describe() const;
		
	private:
		
		template<typename T> T& _get();
		
		
		enum {
			kTypeNumber = 0,
			kTypeString = 1,
			kTypeBool = 2,
			kTypeNull = 3,
			kTypeArray = 4,
			kTypeObject = 5,
			kTypeInvalid = 6
		} _type;
		
		boost::any		_value;
	};


	// A JSON Object
	class Object {
	public:
		Object();
		~Object();

		static bool parse(std::istream& input, Object& object);

		template <typename T> bool hasType(const std::string& key) const;
		inline bool has( const std::string& key ) const;

		// Always call has<>() first. If the key doesn't exist, consider
		// the behavior undefined.
		template <typename T> T& get(const std::string& key);
		template <typename T> const T& get(const std::string& key) const;
		
		void add( const std::string& key, const Value& v ) {
			_value_map[key] = Value( v );
		}

		const Value& value(const std::string& key) const {
			const Value& v = _value_map.find(key)->second;
			return v;
		}	
		Value& value(const std::string& key) { 
			Value& v = _value_map.find(key)->second;
			return v;
		}	

		const std::map<std::string, Value>& kv_map() const { return _value_map; }
		
		Object( const Object& o ) { _value_map = o._value_map; }
		Object& operator=( const Object& o ) { _value_map = o._value_map; return *this; }
		
		Value operator[]( const std::string& k ) { 
			std::map<std::string, Value>::iterator it = _value_map.find(k);
			if ( it != _value_map.end() ) {
				return it->second;
			}
			// else we create a NULL value
			add( k, Value() );
			it = _value_map.find(k);
			return it->second; 
		}
		const Value operator[]( const std::string& k ) const { return value(k); }

		
		void describe() const;

	private:

		std::map<std::string, Value> _value_map;
	};

	class Array {
	public:
		Array();
		Array( int sz );
		~Array();

		static bool parse(std::istream& input, Array& array);

		unsigned  long size() const { return _values.size(); }

		template <typename T> bool has(unsigned int i) const;

		template <typename T> T& get(unsigned int i);
		template <typename T> const T& get(unsigned int i) const;
		const Value operator[]( int i ) const { return value( i ); }
		Value operator[]( int i ) { return value( i ); }

		const Value& value( unsigned int i ) const { return _values[i]; }
		Value& value( unsigned int i ) {
			if( i >= size() )
				reserve( i + 1 );
			return _values[i];
		}

		const std::vector<Value>& values() const {
			return _values;
		}
		
		void reserve( int sz ) {
			for( long i = size(); i < sz; i++ ) {
				_values.push_back( Value() );
			}
		}

		Array(const Array& o) { _values = o._values; }
		Array& operator=(const Array& o) { _values = o._values; return *this; }
		
		void describe() const;

	private:
		std::vector<Value> _values;
	};


	template <typename T>
	bool Array::has(unsigned int i) const {
		if (i >= size()) {
			return false;
		} else {
			const Value& v = _values.at(i);
			return v.is<T>();
		}
	}

	template <typename T>
	T& Array::get(unsigned int i) {
		assert(i < size());
		Value& v = _values.at(i);
		return v.get<T>();
	}

	template <typename T>
	const T& Array::get(unsigned int i) const {
		assert(i < size());
		const Value& v = _values.at(i);
		return v.get<T>();
	}
	
	inline bool Object::has( const std::string& key ) const {
		std::map<std::string, Value>::const_iterator it(_value_map.find(key));
		return it != _value_map.end();
	}

	template <typename T>
	bool Object::hasType(const std::string& key) const {
		std::map<std::string, Value>::const_iterator it(_value_map.find(key));
		return it != _value_map.end() && it->second.is<T>();
	}

	template <typename T>
	T& Object::get(const std::string& key) {
	  assert(hasType<T>(key));
	  return _value_map.find(key)->second.get<T>();
	}

	template <typename T>
	const T& Object::get(const std::string& key) const {
	  assert(hasType<T>(key));
	  return _value_map.find(key)->second.get<T>();
	}

	template<>
	inline bool Value::is<Value::Null>() const {
	  return _type == kTypeNull;
	}

	template<>
	inline bool Value::is<bool>() const {
	  return _type == kTypeBool;
	}

	template<>
	inline bool Value::is<std::string>() const {
	  return _type == kTypeString;
	}

	template<>
	inline bool Value::is<double>() const {
	  return _type == kTypeNumber;
	}

	
	template<>
	inline bool Value::is<Array>() const {
	  return _type == kTypeArray;
	}

	template<>
	inline bool Value::is<Object>() const {
	  return _type == kTypeObject;
	}
	
	/////////////////

	template<>
	inline bool& Value::_get<bool>() {
		assert(is<bool>());
		bool* b = boost::any_cast<bool>(&_value);
		return *b;
	}

	template<>
	inline std::string& Value::_get<std::string>() {
		assert(is<std::string>());
		std::string* s = boost::any_cast<std::string>(&_value);
		return *s;
	}

	template<>
	inline double& Value::_get<double>() {
		assert(is<double>());
		double* d = boost::any_cast<double>(&_value);
		return *d;
	}
	

	template<>
	inline Array& Value::_get<Array>() {
		assert(is<Array>());
		Array* a = boost::any_cast<Array>(&_value);
		return *a;
	}

	template<>
	inline Object& Value::_get<Object>() {
		assert(is<Object>());
		Object* o = boost::any_cast<Object>(&_value);
		return *o;
	}
	
	
	//////////////////////

	template<>
	inline bool Value::get<bool>() const {
		assert(is<bool>());
		return boost::any_cast<bool>(_value); ;
	}

	template<>
	inline bool Value::get<bool>() {
		assert(is<bool>());
		return boost::any_cast<bool>(_value);
	}
	
	template<>
	inline std::string Value::get<std::string>() {
		assert(is<std::string>());
		return boost::any_cast<std::string>(_value);
	}
	
	template<>
	inline double Value::get<double>() {
		assert(is<double>());
		return boost::any_cast<double>(_value);
	}
	
	template<>
	inline int Value::get<int>() {
		assert(is<double>());
		return int(boost::any_cast<double>(_value));
	}
	
	template<>
	inline float Value::get<float>() {
		assert(is<double>());
		return float(boost::any_cast<double>(_value));
	}

	
	template<>
	inline Array Value::get<Array>() {
		assert(is<Array>());
		return boost::any_cast<Array>(_value);
	}
	
	template<>
	inline Object Value::get<Object>() {
		return boost::any_cast<Object>(_value);;
	}
	
	///////////////////

	template<>
	inline std::string Value::get<std::string>() const {
		assert(is<std::string>());
		return boost::any_cast<std::string>(_value);;
	}

	template<>
	inline double Value::get<double>() const {
		assert(is<double>());
		return boost::any_cast<double>(_value);;
	}

	template<>
	inline int Value::get<int>() const {
		assert(is<double>());
		return int(boost::any_cast<double>(_value));
	}

	template<>
	inline float Value::get<float>() const {
		assert(is<double>());
		return float(boost::any_cast<double>(_value));
	}

	template<>
	inline Array Value::get<Array>() const {
		assert(is<Array>());
		return boost::any_cast<Array>(_value);;
	}

	template<>
	inline Object Value::get<Object>() const {
		assert(is<Object>());
		return boost::any_cast<Object>(_value);;
	}

	
	inline Value Value::operator[]( int i ) {
		return get<Array>().value( i );
	}
	inline const Value Value::operator[]( int i ) const {
		return get<Array>().value( i );
	}
	
	inline Value Value::operator[]( const char* k ) {
		assert( has(k) );
		return get<Object>().value( std::string(k) );
	}
	inline const Value Value::operator[]( const char* k ) const {
		assert( has(k) );
		return get<Object>().value( std::string(k) );
	}
	
	inline Value Value::operator[]( const std::string& k ) {
		assert( has(k) );
		return get<Object>().value( k );
	}
	inline const Value Value::operator[]( const std::string& k ) const {
		assert( has(k) );
		return get<Object>().value( k );
	}

	inline bool Value::has( const std::string& key ) const {
		return get<Object>().has( key );
	}
	
	inline std::string& operator<<(std::string& s, const zo::Value& v ) {
		s = v.get<std::string>();
		return s;
	}
	
	inline double& operator<<(double& s, const zo::Value& v ) {
		s = v.get<double>();
		return s;
	}
	
	inline Array& operator<<(Array& s, const zo::Value& v ) {
		s = v.get<Array>();
		return s;
	}
	
	inline Object& operator<<(Object& s, const zo::Value& v ) {
		s = v.get<Object>();
		return s;
	}

	inline bool& operator<<(bool& s, const zo::Value& v ) {
		s = v.get<bool>();
		return s;
	}

	inline float& operator<<(float& s, const zo::Value& v ) {
		s = v.get<double>();
		return s;
	}


}  // namespace jsonxx

	std::ostream& operator<<(std::ostream& stream, const zo::Value& v);
	std::ostream& operator<<(std::ostream& stream, const zo::Object& v);
	std::ostream& operator<<(std::ostream& stream, const zo::Array& v);

#endif // _jsonxx_h__


