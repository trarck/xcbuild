/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <repack/rename/Options.h>

using repack::rename::Options;

Options::
Options()
{
}

Options::
~Options()
{
}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (arg == "-name") {
		return libutil::Options::Next<std::string>(&_name, args, it);
	}else if (arg == "-project-path") {
		return libutil::Options::Next<std::string>(&_projectPath, args, it);
    } else if (arg == "-package") {
		return libutil::Options::Next<std::string>(&_package, args, it);
    } else if (arg == "-target") {
		return libutil::Options::Next<std::string>(&_target, args, it);
    } else if (arg == "-code-sign-identity") {
		return libutil::Options::Next<std::string>(&_codeSignIdentity, args, it);
    } else if (arg == "-provisioning-profile") {
		return libutil::Options::Next<std::string>(&_provisioningProfile, args, it);
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

