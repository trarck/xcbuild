/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/Context.h>
#include <pbxproj/PBX/Project.h>
#include <pbxproj/Proj.h>

using pbxproj::Context;
namespace PBX = pbxproj::PBX;

void Context::
cacheObject(PBX::Object::shared_ptr const &O, std::string const &id)
{
	//TODO [dhh] remove start
    if (project == nullptr && O->isa <PBX::Project> ()) {
        project = std::static_pointer_cast <PBX::Project> (O);
    }
	//TODO [dhh] remove end
    O->setUuid(id);

	if (pbxproj != nullptr) {
		pbxproj->addObject(O);
	}
	//TODO [dhh] remove start
    if (project != nullptr && project != O) {
        project->cacheObject(O);
    }
	//TODO [dhh] remove end
}
