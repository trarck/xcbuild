/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_Proj_h
#define __pbxproj_Proj_h
#include <pbxproj/PBX/Project.h>
#include <pbxproj/PBX/FileReference.h>
#include <plist/Integer.h>

namespace pbxproj { 

class PbxProj {
public:
	typedef std::shared_ptr <PbxProj> shared_ptr;
	typedef std::vector <shared_ptr> vector;
private:
	std::string                        _projectFile;
	std::string                        _dataFile;
	std::string                        _basePath;
	std::string                        _name;
	std::unordered_map<std::string, PBX::Object::shared_ptr> _objects;
private:
	uint32_t				_archiveVersion;
	uint32_t				_objectVersion;
	std::shared_ptr<PBX::Project>	_rootObject;

	PBX::FileReference::vector              _fileReferences;
public:
	PbxProj();

public:
	static shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);
	bool save();
public:
	inline PBX::Target::vector const targets() const
	{
		if (_rootObject != nullptr) {
			return _rootObject->targets();
		}
		else
		{
			return PBX::Target::vector();
		}
	}
public:
	inline std::string const &name() const
	{
		return _name;
	}
	inline std::string const &dataFile() const
	{
		return _dataFile;
	}
	inline std::string const &projectFile() const
	{
		return _projectFile;
	}
	inline std::string const &basePath() const
	{
		return _basePath;
	}

	inline uint32_t const &archiveVersion() const
	{
		return _archiveVersion;
	}
	inline uint32_t const &objectVersion() const
	{
		return _objectVersion;
	}

	inline std::shared_ptr<PBX::Project> const &rootObject() const
	{
		return _rootObject;
	}
public:
	std::string sourceRoot() const;
protected:
	friend class pbxproj::Context;
	inline void addObject(PBX::Object::shared_ptr const &object)
	{
		_objects[object->uuid()] = object;
	}
public:
	inline PBX::FileReference::vector const &fileReferences() const
	{
		return _fileReferences;
	}

	std::unordered_map<std::string, std::vector<PBX::Object::shared_ptr>> getObjectsGroupByISA() const;                           
public:
	inline PBX::Object::shared_ptr resolveBuildableReference(std::string const &uuid) const
	{
		if (uuid.empty())
			return PBX::Object::shared_ptr();

		auto I = _objects.find(uuid);
		if (I == _objects.end())
			return PBX::Object::shared_ptr();
		else
			return I->second;
	}
public:
	pbxsetting::Level settings(void) const;
protected:
	bool parse(Context &context,plist::Dictionary const *dict,const std::string& projectFileName);
};

}           

#endif  // !__pbxproj_Proj_h
