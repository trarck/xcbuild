/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_String_h
#define __plist_String_h

#include <plist/Base.h>
#include <plist/Object.h>

namespace plist {

class String : public Object {
private:
    std::string _value;
	std::string _annotation;

public:
    String(std::string const &value = std::string(),std::string const &annotation=std::string()) :
        _value(value),
		_annotation(annotation)
    {
    }

    String(std::string &&value) :
        _value(std::move(value))
    {
    }

	String(std::string &&value,std::string&& annotation) :
		_value(std::move(value)),
		_annotation(std::move(annotation))
	{
	}

public:
    inline std::string const &value() const
    {
        return _value;
    }

    inline void setValue(std::string const &value)
    {
        _value = value;
    }

    inline void setValue(std::string &&value)
    {
        _value = std::move(value);
    }

	inline std::string const &annotation() const
	{
		return _annotation;
	}

	inline void setAnnotation(std::string const &annotation)
	{
		_annotation = annotation;
	}

	inline void setAnnotation(std::string &&annotation)
	{
		_annotation = std::move(annotation);
	}

public:
    static std::unique_ptr<String> New(std::string const &value = std::string(), std::string const &annotation = std::string());
    static std::unique_ptr<String> New(std::string &&value);
	static std::unique_ptr<String> New(std::string &&value, std::string &&annotation);

public:
    static std::unique_ptr<String> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return String::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::String;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<String> copy() const
    { return plist::static_unique_pointer_cast<String>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        String const *objt = CastTo <String> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(String const *obj) const
    {
        return (obj != nullptr && (obj == this || value() == obj->value()));
    }
};

}

#endif  // !__plist_String_h
