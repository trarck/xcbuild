/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */
#include <pbxproj/Proj.h>
#include <pbxproj/PBX/Project.h>
#include <pbxproj/PBX/AggregateTarget.h>
#include <pbxproj/PBX/LegacyTarget.h>
#include <pbxproj/PBX/NativeTarget.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Keys/Unpack.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

#define UTF8Head "// !$*UTF8*$!\n"

using pbxproj::PBX::Project;
using pbxproj::Context;
using libutil::Filesystem;
using libutil::FSUtil;

namespace pbxproj {

	PbxProj::PbxProj():
		_archiveVersion(0),
		_objectVersion(0)		
	{

	}
	pbxsetting::Level PbxProj::
		settings(void) const
	{
		std::vector<pbxsetting::Setting> settings = {
			pbxsetting::Setting::Create("PROJECT", _name),
			pbxsetting::Setting::Create("PROJECT_NAME", _name),
			pbxsetting::Setting::Create("PROJECT_DIR", sourceRoot()),
			pbxsetting::Setting::Create("PROJECT_FILE_PATH", _projectFile),
			pbxsetting::Setting::Create("SRCROOT", sourceRoot()),
			pbxsetting::Setting::Create("DEVELOPMENT_LANGUAGE", (!_rootObject->developmentRegion().empty() ? _rootObject->developmentRegion() : "English")),
		};

		return pbxsetting::Level(settings);
	}

	std::string PbxProj::
		sourceRoot() const
	{
		std::string root = _basePath;
		if (_rootObject != nullptr && !_rootObject->projectDirPath().empty()) {
			root += "/" + _rootObject->projectDirPath();
		}
		return FSUtil::NormalizePath(root);
	}

	bool PbxProj::
		parse(Context &context,plist::Dictionary const *dict, const std::string& projectFileName)
	{
		//
		// Fetch basic objects
		//
		std::unordered_set<std::string> seen;
		auto unpack = plist::Keys::Unpack("Root", dict, &seen);

		auto AV = unpack.coerce <plist::Integer>("archiveVersion");
		auto OV = unpack.coerce <plist::Integer>("objectVersion");
		auto Os = unpack.cast <plist::Dictionary>("objects");
		auto Cs = unpack.cast <plist::Dictionary>("classes");

		//
		// Handle basic objects
		//
		if (AV != nullptr) {
			if (AV->value() > 1) {
				fprintf(stderr, "warning: archive version %u may be unsupported\n",
					static_cast <unsigned> (AV->value()));
			}
			_archiveVersion = AV->value();
		}
		else {
			fprintf(stderr, "error: project file %s is not parseable (no archive version)\n", projectFileName.c_str());
			return false;
		}

		if (OV != nullptr) {
			if (OV->value() > 46) {
				fprintf(stderr, "warning: object version %u may be unsupported\n",
					static_cast <unsigned> (OV->value()));
			}
			_objectVersion = OV->value();
		}
		else {
			fprintf(stderr, "error: project file %s is not parseable (no object version)\n", projectFileName.c_str());
			return false;
		}

		if (Cs != nullptr && Cs->count() != 0) {
			fprintf(stderr, "warning: non-empty classes may be unsupported\n");
		}

		if (Os == nullptr) {
			return false;
		}

		//
		// Initialize context
		//
		context.objects = Os;
		//
		// Fetch the project dictionary (root object)
		//
		std::string PID;
		auto P = context.indirect <Project>(&unpack, "rootObject", &PID);
		if (P == nullptr) {
			fprintf(stderr, "error: unable to parse project\n");
			return false;
		}

		//
		// Verify that all keys are parsed.
		//
		if (!unpack.complete(true)) {
			fprintf(stderr, "%s", unpack.errorText().c_str());
		}

		//
		// Parse the project dictionary and create the project object.
		//
		_rootObject = context.parseObject(context.projects, PID, P);
		if (_rootObject == nullptr) {
			fprintf(stderr, "error: unable to create rootObject\n");
			return false;
		}
		

		//
		// Transfer all file references from cache.
		//
		for (auto const &I : context.fileReferences) {
			_fileReferences.push_back(I.second);
		}

		return true;
	}

	PbxProj::shared_ptr PbxProj::
		Open(Filesystem const *filesystem, std::string const &path)
	{
		if (path.empty()) {
			fprintf(stderr, "error: project path is empty\n");
			return nullptr;
		}

		std::string projectFileName = path + "/project.pbxproj";
		if (!filesystem->isReadable(projectFileName)) {
			fprintf(stderr, "error: project file %s is not readable\n", projectFileName.c_str());
			return nullptr;
		}

		std::string realPath = filesystem->resolvePath(projectFileName);
		if (realPath.empty()) {
			fprintf(stderr, "error: project file %s is not resolvable\n", projectFileName.c_str());
			return nullptr;
		}

		std::vector<uint8_t> contents;
		if (!filesystem->read(&contents, realPath)) {
			fprintf(stderr, "error: project file %s is not readable\n", projectFileName.c_str());
			return nullptr;
		}

		//
		// Parse property list
		//
		auto result = plist::Format::Any::Deserialize(contents);
		if (result.first == nullptr) {
			fprintf(stderr, "error: project file %s is not parseable: %s\n", projectFileName.c_str(), result.second.c_str());
			return nullptr;
		}

		plist::Dictionary *plist = plist::CastTo<plist::Dictionary>(result.first.get());
		if (plist == nullptr) {
			fprintf(stderr, "error: project file %s is not a dictionary\n", projectFileName.c_str());
			return nullptr;
		}

		shared_ptr pbxProj = std::make_shared <PbxProj>();

		Context content;
		content.pbxproj = pbxProj;

		pbxProj->parse(content,plist,projectFileName);

		//
		// Save some useful info
		//
		pbxProj->_dataFile = realPath;
		pbxProj->_projectFile = FSUtil::GetDirectoryName(realPath);
		pbxProj->_basePath = FSUtil::GetDirectoryName(pbxProj->_projectFile);
		pbxProj->_name = FSUtil::GetBaseNameWithoutExtension(pbxProj->_projectFile);

		return pbxProj;
	}

	bool ObjectComp(PBX::Object::shared_ptr &a, PBX::Object::shared_ptr &b) {
		return a->uuid() > b->uuid();
	}

	std::unordered_map<std::string, std::vector<PBX::Object::shared_ptr>> PbxProj::getObjectsGroupByISA() const
	{
		std::unordered_map<std::string, std::vector<PBX::Object::shared_ptr>> groups;

		for (auto it : _objects) {
			groups[it.second->isa()].push_back(it.second);
		}

		//sort group
		for (auto it : groups) {
			std::sort(it.second.begin(), it.second.end(), ObjectComp);
		}
		
		return groups;
	}

	bool PbxProj::save() {
		std::vector<uint8_t> content;
		std::string head = UTF8Head;
		content.insert(content.end(), head.begin(), head.end());

		plist::Dictionary root;



		root.set("archiveVersion", plist::Integer::New(_archiveVersion));
		root.set("classes", plist::Dictionary::New());
		root.set("objectVersion", plist::Integer::New(_objectVersion));
		//group objects
		std::unordered_map<std::string, std::vector<PBX::Object::shared_ptr>> groups;

		for (auto it : _objects) {
			groups[it.second->isa()].push_back(it.second);			
		}

		//sort group
		for (auto it : groups) {
			std::sort(it.second.begin(), it.second.end(), ObjectComp);
		}

		root.set("objects", plist::Integer::New(_objectVersion)); 



		root.set("rootObject", plist::String::New(_rootObject->uuid()));

		plist::Format::ASCII ascii = plist::Format::ASCII::Create(false, plist::Format::Encoding::UTF8);
		auto result= plist::Format::ASCII::Serialize(&root, ascii);
		if (result.first != nullptr) {
			content.insert(content.end(), result.first->begin(), result.first->end());
		}

		std::string data(content.begin(),content.end());
		printf(data.c_str());
		return true;
	}

}
