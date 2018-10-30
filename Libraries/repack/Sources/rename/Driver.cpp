/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <repack/rename/Driver.h>
#include <repack/rename/Options.h>
#include <pbxproj/pbxproj.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/XC/Config.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Format/ASCII.h>
#include <xcscheme/xcscheme.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <process/DefaultContext.h>
#include <process/Context.h>
#include <process/DefaultUser.h>
#include <process/User.h>

#include <cstring>
#include <cerrno>
#include <set>

#define AppliationType "com.apple.product-type.application"

using repack::rename::Driver;
using repack::rename::Options;
using libutil::Filesystem;
using libutil::Permissions;
using namespace pbxproj;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

std::string Driver::
name()
{
    return "repack-rename";
}

//static bool
//Save(plist::Object *root, plist::Format::Any const &format, Filesystem *filesystem, std::string const &path)
//{
//    auto serialize = plist::Format::Any::Serialize(root, format);
//    if (serialize.first == nullptr) {
//        fprintf(stderr, "Error: %s\n", serialize.second.c_str());
//        return false;
//    }
//
//    if (!filesystem->write(*serialize.first, path)) {
//        fprintf(stderr, "Could not write to output\n");
//        return false;
//    }
//
//    return true;
//}

int renameProductName(Filesystem const *filesystem, PbxProj::shared_ptr const &project,const std::string& productName)
{
	//get first application build target
	PBX::Target::shared_ptr buildTarget = nullptr;
	
	for (auto I : project->targets()) {
		printf("\t%s\n", I->name().c_str());
		printf("\t\tProduct Name = %s\n", I->productName().c_str());
		if (I->type() == PBX::Target::Type::Native) {
			auto NT = static_cast <PBX::NativeTarget const *> (I.get());
			
			printf("\t\tProduct Type = %s\n", NT->productType().c_str());
			if (NT->productType() == AppliationType) {
				buildTarget = I;
				break;
			}
		}
	}

	if (buildTarget!=nullptr) {
		buildTarget->setName(productName);
		buildTarget->setProductName(productName);
	}

	return 0;
}

int Driver::
run(process::Context const *processContext, libutil::Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n", result.second.c_str());
        return 1;
    }

	if (!options.projectPath()) {
		fprintf(stderr, "error: no project path provided\n");
		return 1;
	}

	if (!options.name()) {
		fprintf(stderr, "error: no name provided\n");
		return 1;
	}

	if (!options.package()) {
		fprintf(stderr, "error: no package provided\n");
		return 1;
	}

	auto project = PbxProj::Open(filesystem, *options.projectPath());
	if (!project) {
		fprintf(stderr, "error opening project (%s)\n",
			strerror(errno));
		return -1;
	}
	fprintf(stdout, "old package (%s)\n",
		strerror(errno));


	printf("open project ok");

	//renameProductName(filesystem, project, *options.target());

	project->save(filesystem,"");

    return 0;
}
