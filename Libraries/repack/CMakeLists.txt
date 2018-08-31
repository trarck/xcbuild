#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(repack
            Sources/Driver.cpp
            Sources/rename/Options.cpp
            Sources/rename/Driver.cpp
            )

target_link_libraries(repack PUBLIC pbxproj xcscheme pbxsetting util plist)
target_include_directories(repack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
target_include_directories(repack PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PrivateHeaders")
install(TARGETS repack DESTINATION usr/lib)

add_executable(RenameProject Tools/RenameProject.cpp)
target_link_libraries(RenameProject repack)

