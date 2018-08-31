/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_Target_h
#define __pbxproj_PBX_Target_h

#include <pbxproj/XC/ConfigurationList.h>
#include <pbxproj/PBX/BuildRule.h>
#include <pbxproj/PBX/BuildPhase.h>
#include <pbxproj/PBX/TargetDependency.h>

namespace pbxproj {

	class PbxProj;

	namespace PBX {



class Target : public Object {
public:
    typedef std::shared_ptr <Target> shared_ptr;
    typedef std::vector <shared_ptr> vector;

public:
    enum class Type {
        Native,
        Legacy,
        Aggregate
    };

private:
    Type                              _type;

private:
    std::weak_ptr<PbxProj>            _project;

private:
    std::string                       _name;
    std::string                       _productName;
    XC::ConfigurationList::shared_ptr _buildConfigurationList;
    PBX::BuildPhase::vector           _buildPhases;
    PBX::TargetDependency::vector     _dependencies;

protected:
    Target(std::string const &isa, Type type);

public:
    inline Type type() const
    { return _type; }

public:
    inline std::shared_ptr<PbxProj> project() const
    { return _project.lock(); }

public:
    inline std::string const &name() const
    { return _name; }
	inline void setName(std::string const &name)
	{
		_name=name;
	}

    inline std::string const &productName() const
    { return _productName; }
	inline void setProductName(std::string const &productName)
	{
		_productName = productName;
	}
public:
    inline XC::ConfigurationList::shared_ptr const &buildConfigurationList() const
    { return _buildConfigurationList; }

public:
    inline BuildPhase::vector const &buildPhases() const
    { return _buildPhases; }

public:
    inline TargetDependency::vector const &dependencies() const
    { return _dependencies; }

public:
    pbxsetting::Level settings(void) const;

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;
public:
	std::unique_ptr<plist::Dictionary> toPlist() override;
};

} }

#endif  // !__pbxproj_PBX_Target_h
