/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PbxWriter_h
#define __pbxproj_PbxWriter_h

#include <plist/Objects.h>

namespace pbxproj {
class PbxProj;

class PbxWriter {
private:
	PbxProj const         *_root;
    bool                  _strings;
    std::vector<uint8_t>  _contents;
    int                   _indent;
    bool                  _lastKey;

public:
	PbxWriter(PbxProj const *root, bool strings);
    ~PbxWriter();

public:
    std::vector<uint8_t> contents() const
    { return _contents; }

public:
    bool write();
private:
	bool handlePbxProj(PbxProj const* pbxProj);
private:
    bool primitiveWriteString(std::string const &string);
    bool primitiveWriteEscapedString(std::string const &string);

private:
    bool writeString(std::string const &string, bool final);
    bool writeEscapedString(std::string const &string, bool final);
	bool writeAnnotation(std::string const& annotation);

	bool writeBoolean(bool boolean);
	bool writeReal(double real);
	bool writeInteger(int64_t integer);
	bool writeData(std::vector <uint8_t> const &data);

//#define DECLARE_WRITE_PAIR(TYPE) bool writePair(const std::string& key, TYPE value, const std::string& annotation = "")
	bool writePairInteger(const std::string& key, int64_t value, const std::string& annotation = "");
	bool writePair(const std::string& key, int64_t value,const std::string& annotation="");
	bool writePair(const std::string& key, double value, const std::string& annotation = "");
	bool writePair(const std::string& key, bool value, const std::string& annotation = "");
	bool writePair(const std::string& key, const std::string& value, const std::string& annotation = "");
	bool writePair(const std::string& key, const std::vector <uint8_t>& value, const std::string& annotation = "");
	bool writePair(const std::string& key, plist::Array  *array, const std::string& annotation = "");
	bool writePair(const std::string& key, plist::Dictionary* dictionary, bool singleLine = false,const std::string& annotation = "");

private:
    bool handleObject(plist::Object *object, bool root, bool singleLine = false);
    bool handleArray(plist::Array *array, bool root, bool singleLine = false);
    bool handleDictionary(plist::Dictionary *dictionary, bool root,bool singleLine=false);
    bool handleBoolean(plist::Boolean *boolean, bool root);
    bool handleReal(plist::Real *real, bool root);
    bool handleInteger(plist::Integer *integer, bool root);
    bool handleString(plist::String *string, bool root);
    bool handleDate(plist::Date *date, bool root);
    bool handleData(plist::Data *data, bool root);
    bool handleUID(plist::UID *uid, bool root);
};

}

#endif  // !__pbxproj_PbxWriter_h
