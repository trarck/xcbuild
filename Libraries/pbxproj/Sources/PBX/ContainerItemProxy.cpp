/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/ContainerItemProxy.h>
#include <pbxproj/Context.h>
#include <pbxproj/PBX/Project.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::ContainerItemProxy;
using pbxproj::Context;

ContainerItemProxy::
ContainerItemProxy() :
    Object    (Isa()),
    _proxyType(0)
{
}

bool ContainerItemProxy::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    std::string CPID;

    auto unpack = plist::Keys::Unpack("ContainerItemProxy", dict, seen);
	
    auto PT   = unpack.coerce <plist::Integer> ("proxyType");
    auto RGIS = unpack.cast <plist::String> ("remoteGlobalIDString");
    auto RI   = unpack.cast <plist::String> ("remoteInfo");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

	auto CP = context.indirect <FileReference>(&unpack, "containerPortal", &CPID);
    if (CP != nullptr) {
        auto portal = context.parseObject<FileReference>(context.fileReferences, CPID, CP);
        if (!portal) {
            return false;
        }

        _containerPortal = portal;
    }
	else
	{
		CP = context.indirect<Project>(&unpack, "containerPortal", &CPID);
		if (CP != nullptr) {
			auto portal = context.parseObject<Project>(context.projects, CPID, CP);
			if (!portal) {
				return false;
			}
			_containerPortal = portal;
		}
	}

    if (PT != nullptr) {
        _proxyType = PT->value();
    }

    if (RGIS != nullptr) {
        _remoteGlobalIDString = RGIS->value();
    }

    if (RI != nullptr) {
        _remoteInfo = RI->value();
    }

    return true;
}

std::unique_ptr<plist::Dictionary> ContainerItemProxy::toPlist()
{
	auto dict = Object::toPlist();

	if (_containerPortal) {
		dict->set("containerPortal", plist::String::New(_containerPortal->uuid(), _containerPortal->annotation()));
	}
	dict->set("proxyType", plist::Integer::New(_proxyType));
	dict->set("remoteGlobalIDString", plist::String::New(_remoteGlobalIDString));
	dict->set("remoteInfo", plist::String::New(_remoteInfo));

	return dict;
}