/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_Object_h
#define __pbxproj_PBX_Object_h

#include <pbxproj/ISA.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace plist { class Dictionary;}
namespace pbxproj { class Context; }

namespace pbxproj { namespace PBX {

class Object {
public:
    typedef std::shared_ptr <Object> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string _isa;
    //std::string _blueprintIdentifier;
	std::string _uuid;

	std::weak_ptr<Object> _parent;
protected:
    Object(std::string const &isa);
public:
    inline std::string const &blueprintIdentifier() const
    { return _uuid; }
    inline void setBlueprintIdentifier(std::string const &identifier)
    { _uuid = identifier; }
	inline std::string const &uuid() const
	{
		return _uuid;
	}

	inline void setUuid(std::string const &uuid)
	{
		_uuid = uuid;
	}

	inline const Object::shared_ptr parent() const
	{
        return _parent.lock();
	}

	inline void setParent(Object::shared_ptr const & parent)
	{
		_parent = parent;
	}
public:
    inline std::string const &isa() const
    { return _isa; }



public:
    virtual inline bool isa(std::string const &isa) const
    { return (_isa == isa); }

private:
    friend class pbxproj::Context;
    bool parseObject(Context &context, plist::Dictionary const *dict);

protected:
    virtual bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);

public:
	virtual std::unique_ptr<plist::Dictionary> toPlist();
	virtual std::string displayName();
	virtual std::string annotation();

public:
    template <typename T>
    inline bool isa() const
    { return (T::Isa() == isa()); }
};

} }

#endif  // !__pbxproj_PBX_Object_h
