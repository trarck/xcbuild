/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __repack_rename_Options_h
#define __repack_rename_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace repack {
namespace rename {

class Options {
public:
	ext::optional<std::string>        _projectPath;
    ext::optional<std::string>        _name;
	ext::optional<std::string>        _package;
	ext::optional<std::string>        _target;
	ext::optional<std::string>        _codeSignIdentity;
	ext::optional<std::string>        _provisioningProfile;


public:
    Options();
    ~Options();

public:
	ext::optional<std::string> const &projectPath() const
	{
		return _projectPath;
	}
	ext::optional<std::string> const &name() const
    { return _name; }
	ext::optional<std::string> const &package() const
    { return _package; }
		ext::optional<std::string> const &target() const
    { return _target; }
	ext::optional<std::string> const &codeSignIdentity() const
    { return _codeSignIdentity; }
		ext::optional<std::string> const &provisioningProfile() const
    { return _provisioningProfile; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__repack_rename_Options_h
