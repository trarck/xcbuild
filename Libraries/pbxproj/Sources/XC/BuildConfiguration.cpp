/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/XC/BuildConfiguration.h>
#include <pbxproj/Context.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <pbxsetting/Value.h>

using pbxproj::XC::BuildConfiguration;
using pbxproj::Context;

std::vector<std::string> ARRAY_SETTINGS = { 
	"ALTERNATE_PERMISSIONS_FILES",
	"ARCHS",
	"BUILD_VARIANTS",
	"EXCLUDED_SOURCE_FILE_NAMES",
	"FRAMEWORK_SEARCH_PATHS",
	"GCC_PREPROCESSOR_DEFINITIONS",
	"GCC_PREPROCESSOR_DEFINITIONS_NOT_USED_IN_PRECOMPS",
	"HEADER_SEARCH_PATHS",
	"INFOPLIST_PREPROCESSOR_DEFINITIONS",
	"LIBRARY_SEARCH_PATHS",
	"OTHER_CFLAGS",
	"OTHER_CPLUSPLUSFLAGS",
	"OTHER_LDFLAGS",
	"REZ_SEARCH_PATHS",
	"SECTORDER_FLAGS",
	"WARNING_CFLAGS",
	"WARNING_LDFLAGS "
};

std::vector<std::string> ARRAY_SETTINGS_50 = {
	"ALTERNATE_PERMISSIONS_FILES",
	"ARCHS",
	"BUILD_VARIANTS",
	"EXCLUDED_SOURCE_FILE_NAMES",
	"FRAMEWORK_SEARCH_PATHS",
	"GCC_PREPROCESSOR_DEFINITIONS",
	"GCC_PREPROCESSOR_DEFINITIONS_NOT_USED_IN_PRECOMPS",
	"HEADER_SEARCH_PATHS",
	"INCLUDED_SOURCE_FILE_NAMES",
	"INFOPLIST_PREPROCESSOR_DEFINITIONS",
	"LD_RUNPATH_SEARCH_PATHS",
	"LIBRARY_SEARCH_PATHS",
	"LOCALIZED_STRING_MACRO_NAMES",
	"OTHER_CFLAGS",
	"OTHER_CPLUSPLUSFLAGS",
	"OTHER_LDFLAGS",
	"REZ_SEARCH_PATHS",
	"SECTORDER_FLAGS",
	"SYSTEM_FRAMEWORK_SEARCH_PATHS",
	"SYSTEM_HEADER_SEARCH_PATHS",
	"USER_HEADER_SEARCH_PATHS",
	"WARNING_CFLAGS",
	"WARNING_LDFLAGS"
};

BuildConfiguration::
BuildConfiguration() :
    Object        (Isa()),
    _buildSettings(pbxsetting::Level({ }))
{
}

BuildConfiguration::~BuildConfiguration()
{
}

bool BuildConfiguration::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BuildConfiguration", dict, seen);

    std::string BCRID;

    auto BCR = context.indirect <PBX::FileReference> (&unpack, "baseConfigurationReference", &BCRID);
    auto BS  = unpack.cast <plist::Dictionary> ("buildSettings");
    auto N   = unpack.cast <plist::String> ("name");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BCR != nullptr) {
        _baseConfigurationReference =
          context.parseObject(context.fileReferences, BCRID, BCR);
        if (!_baseConfigurationReference)
            return false;
    }

    if (BS != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < BS->count(); n++) {
            auto BSk = BS->key(n);
            auto BSv = BS->value(BSk);

			bool isArray = false;
			for (std::string iter : ARRAY_SETTINGS)
			{
				if (iter == BSk) {
					isArray = true;
					break;
				}
			}

            pbxsetting::Setting setting = pbxsetting::Setting::Create(BSk, pbxsetting::Value::FromObject(BSv,isArray));
            settings.push_back(setting);
        }
        _buildSettings = pbxsetting::Level(settings);
    }

    if (N != nullptr) {
        _name = N->value();
    }

    return true;
}

std::unique_ptr<plist::Dictionary>
BuildConfiguration::toPlist()
{
	auto dict = Object::toPlist();

	auto settingDict = plist::Dictionary::New();

	for (auto it : _buildSettings.settings()) {
		settingDict->set(it.name(), it.value().toPlist());
	}
	
	if (_baseConfigurationReference)
	{
		dict->set("baseConfigurationReference", plist::String::New(_baseConfigurationReference->uuid()));
	}

	dict->set("buildSettings", std::move(settingDict));
	dict->set("name", plist::String::New(_name));
	return dict;
}

std::string BuildConfiguration::displayName()
{
	if (_name.empty()) {
		return isa();
	}
	else
	{
		return _name;
	}
}
