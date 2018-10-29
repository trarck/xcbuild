/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PbxWriter.h>
#include <pbxproj/Proj.h>
#include <plist/Objects.h>

#include <cassert>
#include <cinttypes>

using plist::String;
using plist::Integer;
using plist::Real;
using plist::Boolean;
using plist::Data;
using plist::Date;
using plist::Array;
using plist::UID;
using plist::Dictionary;
using plist::CastTo;

namespace pbxproj {


	const std::string AsciiHeader = "// !$*UTF8*$!\n";

	bool DictComp(const std::string &a, const std::string &b)
    {
		if (a == "isa") {
			return true;
		}
		else if (b == "isa")
		{
			return false;
		}
		else
		{
			return a < b;
		}
	}

	PbxWriter::
		PbxWriter(PbxProj const *root, bool strings) :
		_root(root),
		_strings(strings),
		_indent(0),
		_lastKey(false)
	{
	}

	PbxWriter::
		~PbxWriter()
	{
	}

	static bool
		NeedsQuoting(std::string const &string)
	{
		if (string.empty()) {
			return true;
		}

		for (char c : string) {
			if (!isalnum(c) && ( c != '_' && c!='.' && c!='/')) {
				return true;
			}
		}

		return false;
	}

	bool PbxWriter::
		write()
	{
		writeString(AsciiHeader, false);

		handlePbxProj(_root);


		if (!_strings) {
			if (!writeString("\n", false)) {
				return false;
			}
		}

		return true;
	}

	bool PbxWriter::
		handlePbxProj(PbxProj const* pbxProj)
	{
		if (!writeString("{\n", !_lastKey)) {
			return false;
		}

		_indent++;

		writePairInteger("archiveVersion", pbxProj->archiveVersion());
		
		auto classes = plist::Dictionary::New();

		writePair("classes", classes.get());

		writePairInteger("objectVersion", pbxProj->objectVersion());

		//write objects
		if (!writeString("objects", true)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		if (!writeString("{\n", false)) {
			return false;
		}
		
		auto groupObjects = pbxProj->getObjectsGroupByISA();
		std::vector<std::string> keys;
		for (auto it : groupObjects) {
			keys.push_back(it.first);
		}

		std::sort(keys.begin(), keys.end());

		for (auto key : keys) {
			_indent++;

			if (!writeString("\n", false)) {
				return false;
			}
			std::string sectionBegin = "/* Begin " + key + " section */\n";
			std::string sectionEnd = "/* End " + key + " section */\n";
			if (!writeString(sectionBegin, false)) {
				return false;
			}

			auto items = groupObjects[key];

			std::sort(items.begin(), items.end(), [](PBX::Object::shared_ptr &a, PBX::Object::shared_ptr &b)->bool {
				return a->uuid() < b->uuid();
			});

			for (auto objIter : items) {

				auto dict = objIter->toPlist();
				//std::sort(dict->begin(), dict->end(), DictComp);

				writePair(objIter->uuid(), dict.get(), objIter->isa() == "PBXBuildFile" || objIter->isa() == "PBXFileReference", objIter->annotation());
			}

			if (!writeString(sectionEnd, false)) {
				return false;
			}
			_indent--;
		}

		if (!writeString("};\n", true)) {
			return false;
		}

		writePair("rootObject", pbxProj->rootObject()->uuid(), pbxProj->rootObject()->annotation());

		if (!_strings || !pbxProj) {
			_indent--;
			if (!writeString("}", true)) {
				return false;
			}
		}

		return true;
	}

	/*
		* Low level functions.
		*/

	bool PbxWriter::
		primitiveWriteString(std::string const &string)
	{
		_contents.insert(_contents.end(), string.begin(), string.end());
		return true;
	}

	bool PbxWriter::
		primitiveWriteEscapedString(std::string const &string)
	{
		_contents.reserve(_contents.size() + string.size());

		if (!primitiveWriteString("\"")) {
			return false;
		}

		for (std::string::const_iterator it = string.begin(); it != string.end(); ++it) {
			uint8_t c = *it;

			if (c > 0x80) {
				uint32_t codepoint;

				if ((c & 0xE0) == 0xC0) {
					uint8_t next = *(++it);
					codepoint = ((c & 0x1F) << 6) | next;
				}
				else if ((c & 0xF0) == 0xE0) {
					uint8_t next1 = *(++it);
					uint8_t next2 = *(++it);
					codepoint = ((c & 0x0F) << 12) | (next1 << 6) | next2;
				}
				else if ((c & 0xF8) == 0xF0) {
					uint8_t next1 = *(++it);
					uint8_t next2 = *(++it);
					uint8_t next3 = *(++it);
					codepoint = (((c & 0x0F) << 18) | (next1 << 12) | (next2 << 6) | next3) + 65536;
				}
				else {
					/* Invalid UTF-8. */
					return false;
				}

				if (codepoint < 0xFF) {
					char buf[64];
					int rc = snprintf(buf, sizeof(buf), "\\%03o", codepoint);
					assert(rc < (int)sizeof(buf));
					(void)rc;

					if (!primitiveWriteString(buf)) {
						return false;
					}
				}
				else {
					char buf[64];
					int rc = snprintf(buf, sizeof(buf), "\\%04u", codepoint);
					assert(rc < (int)sizeof(buf));
					(void)rc;

					if (!primitiveWriteString(buf)) {
						return false;
					}
				}
			}
			else {
				switch (c) {
				case '\a': if (!primitiveWriteString("\\a")) { return false; } break;
				case '\b': if (!primitiveWriteString("\\b")) { return false; } break;
				case '\v': if (!primitiveWriteString("\\v")) { return false; } break;
				case '\f': if (!primitiveWriteString("\\f")) { return false; } break;
				case '"':  if (!primitiveWriteString("\\\"")) { return false; } break;
				case '\\': if (!primitiveWriteString("\\\\")) { return false; } break;
				case '\n': if (!primitiveWriteString("\\n")) { return false; } break;
				default: _contents.push_back(c); break;
				}
			}
		}

		if (!primitiveWriteString("\"")) {
			return false;
		}

		return true;
	}

	bool PbxWriter::
		writeString(std::string const &string, bool final)
	{
		if (final) {
			for (int n = 0; n < _indent; n++) {
				if (!primitiveWriteString("\t")) {
					return false;
				}
			}
		}

		return primitiveWriteString(string);
	}

	bool PbxWriter::
		writeEscapedString(std::string const &string, bool final)
	{
		if (!NeedsQuoting(string)) {
			return writeString(string, final);
		}

		if (final) {
			for (int n = 0; n < _indent; n++) {
				if (!primitiveWriteString("\t")) {
					return false;
				}
			}
		}

		return primitiveWriteEscapedString(string);
	}

	bool  PbxWriter::writeAnnotation(std::string const& annotation)
	{
		if (!annotation.empty()) {
			if (!writeString(" /* ", false)) {
				return false;
			}

			if (!writeString(annotation, false)) {
				return false;
			}

			if (!writeString(" */", false)) {
				return false;
			}
		}
		return true;
	}

	bool PbxWriter::
		writeBoolean(bool boolean)
	{
		if (!writeString(boolean ? "YES" : "NO", !_lastKey)) {
			return false;
		}

		_lastKey = false;
		return true;
	}

	bool PbxWriter::
		writeReal(double real)
	{
		char buf[64];
		int rc = snprintf(buf, sizeof(buf), "%g", real);
		assert(rc < (int)sizeof(buf));
		(void)rc;

		if (!writeString(buf, !_lastKey)) {
			return false;
		}

		_lastKey = false;

		return true;
	}

	bool PbxWriter::
		writeInteger(int64_t integer)
	{
		int               rc;
		char              buf[32];

		rc = snprintf(buf, sizeof(buf), "%" PRId64, integer);
		assert(rc < (int)sizeof(buf));
		(void)rc;

		if (!writeString(buf, !_lastKey)) {
			return false;
		}

		_lastKey = false;

		return true;
	}

	bool PbxWriter::
		writeData(std::vector <uint8_t> const &data)
	{
		if (!writeString("<", !_lastKey)) {
			return false;
		}

		_lastKey = false;

		for (auto it : data) {
			char buf[3];
			int  rc = snprintf(buf, sizeof(buf), "%02x", it);
			assert(rc < (int)sizeof(buf));
			(void)rc;

			if (!writeString(buf, false)) {
				return false;
			}
		}

		return writeString(">", false);
	}


	bool PbxWriter::
		writePairInteger(const std::string& key, int64_t value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeInteger(value)) {
			return false;
		}

		if (!annotation.empty()) {
			if (!writeEscapedString(annotation, false)) {
				return false;
			}
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}

	bool PbxWriter::
		writePair(const std::string& key, int64_t value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeInteger(value)) {
			return false;
		}

		if (!annotation.empty()) {
			if (!writeEscapedString(annotation, false)) {
				return false;
			}
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}

	bool PbxWriter::writePair(const std::string& key, double value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeReal(value)) {
			return false;
		}

		if (!annotation.empty()) {
			if (!writeEscapedString(annotation, false)) {
				return false;
			}
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}

	bool PbxWriter::writePair(const std::string& key, bool value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeBoolean(value)) {
			return false;
		}

		if (!writeAnnotation(annotation)) {
			return false;
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}
	bool PbxWriter::writePair(const std::string& key, const std::string& value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeString(value, false)) {
			return false;
		}

		if (!writeAnnotation(annotation)) {
			return false;
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}
	bool PbxWriter::writePair(const std::string& key, const std::vector <uint8_t>& value, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!writeData(value)) {
			return false;
		}

		if (!writeAnnotation(annotation)) {
			return false;
		}

		if (!writeString(";\n", false)) {
			return false;
		}
		return true;
	}

	bool PbxWriter::writePair(const std::string& key, plist::Array* array, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeAnnotation(annotation)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!handleArray(array, false)) {
			return false;
		}


		if (!writeString(";\n", false)) {
			return false;
		}
        return true;
	}

	bool PbxWriter::writePair(const std::string& key, plist::Dictionary* dictionary, bool singleLine, const std::string& annotation)
	{
		_lastKey = false;

		if (!writeEscapedString(key, !_lastKey)) {
			return false;
		}

		if (!writeAnnotation(annotation)) {
			return false;
		}

		if (!writeString(" = ", false)) {
			return false;
		}

		_lastKey = true;

		if (!handleDictionary(dictionary, false, singleLine)) {
			return false;
		}


		if (!writeString(";\n", false)) {
			return false;
		}
        return true;
	}

	/*
		* Higher level functions.
		*/

	bool PbxWriter::
		handleObject(plist::Object *object, bool root,bool singleLine)
	{
		if (Dictionary *dictionary = CastTo<Dictionary>(object)) {
			if (!handleDictionary(dictionary, root, singleLine)) {
				return false;
			}
		}
		else if (Array *array = CastTo<Array>(object)) {
			if (!handleArray(array, root, singleLine)) {
				return false;
			}
		}
		else if (Boolean *boolean = CastTo<Boolean>(object)) {
			if (!handleBoolean(boolean, root)) {
				return false;
			}
		}
		else if (Integer *integer = CastTo<Integer>(object)) {
			if (!handleInteger(integer, root)) {
				return false;
			}
		}
		else if (Real *real = CastTo<Real>(object)) {
			if (!handleReal(real, root)) {
				return false;
			}
		}
		else if (String *string = CastTo<String>(object)) {
			if (!handleString(string, root)) {
				return false;
			}
		}
		else if (Data *data = CastTo<Data>(object)) {
			if (!handleData(data, root)) {
				return false;
			}
		}
		else if (Date *date = CastTo<Date>(object)) {
			if (!handleDate(date, root)) {
				return false;
			}
		}
		else if (UID *uid = CastTo<UID>(object)) {
			if (!handleUID(uid, root)) {
				return false;
			}
		}
		else {
			return false;
		}

		return true;
	}

	bool PbxWriter::
		handleDictionary(Dictionary *dictionary, bool root, bool singleLine)
	{
		dictionary->sort(DictComp);

		if (!_strings || !root) {
			/* Write '{'. */
			if (!writeString(singleLine ? "{" : "{\n", !_lastKey)) {
				return false;
			}

			_indent++;
		}

		_lastKey = false;

		for (size_t i = 0; i < dictionary->count(); ++i) {
			_lastKey = false;

			if (!writeEscapedString(dictionary->key(i),!singleLine && !_lastKey)) {
				return false;
			}

			if (!writeString(" = ", false)) {
				return false;
			}

			_lastKey = true;

			if (!handleObject(dictionary->value(i), false,singleLine)) {
				return false;
			}

			if (!writeString(singleLine ? "; " : ";\n", false)) {
				return false;
			}
		}

		if (!_strings || !root) {
			_indent--;
			if (!writeString("}", !singleLine)) {
				return false;
			}
		}

		return true;
	}

	bool PbxWriter::
		handleArray(Array *array, bool root, bool singleLine)
	{
		/* Write '('. */
		if (!writeString(singleLine?"(":"(\n", !_lastKey)) {
			return false;
		}

		_lastKey = singleLine;

		_indent++;

		for (size_t i = 0; i < array->count(); ++i) {
			/* Write ',' if not first entry. */
			//if (i != 0) {
			//	if (!writeString(",\n", false)) {
			//		return false;
			//	}
			//}

			if (!handleObject(array->value(i), false,singleLine)) {
				return false;
			}

			if (!writeString(singleLine?", ": ",\n", false)) {
				return false;
			}
		}

		/* Write ')'. */
		//if (!singleLine) {
		//	if (!writeString("\n", false)) {
		//		return false;
		//	}
		//}

		_indent--;
		return writeString(")", !singleLine);
	}

	bool PbxWriter::
		handleBoolean(Boolean *boolean, bool root)
	{
		if (!writeString(boolean->value() ? "YES" : "NO", !_lastKey)) {
			return false;
		}

		_lastKey = false;
		return true;
	}

	bool PbxWriter::
		handleString(String *string, bool root)
	{
		if (!writeEscapedString(string->value(), !_lastKey)) {
			return false;
		}

		if (!writeAnnotation(string->annotation())) {
			return false;
		}


		_lastKey = false;
		return true;
	}

	bool PbxWriter::
		handleData(Data *data, bool root)
	{
		if (!writeString("<", !_lastKey)) {
			return false;
		}

		_lastKey = false;

		std::vector<uint8_t> const &value = data->value();
		for (auto it : value) {
			char buf[3];
			int  rc = snprintf(buf, sizeof(buf), "%02x", it);
			assert(rc < (int)sizeof(buf));
			(void)rc;

			if (!writeString(buf, false)) {
				return false;
			}
		}

		return writeString(">", false);
	}

	bool PbxWriter::
		handleReal(Real *real, bool root)
	{
		char buf[64];
		int rc = snprintf(buf, sizeof(buf), "%g", real->value());
		assert(rc < (int)sizeof(buf));
		(void)rc;

		if (!writeString(buf, !_lastKey)) {
			return false;
		}

		_lastKey = false;

		return true;
	}

	bool PbxWriter::
		handleInteger(Integer *integer, bool root)
	{
		int               rc;
		char              buf[32];

		rc = snprintf(buf, sizeof(buf), "%" PRId64, integer->value());
		assert(rc < (int)sizeof(buf));
		(void)rc;

		if (!writeString(buf, !_lastKey)) {
			return false;
		}

		_lastKey = false;

		return true;
	}

	bool PbxWriter::
		handleDate(Date *date, bool root)
	{
		if (!writeEscapedString(date->stringValue(), !_lastKey)) {
			return false;
		}

		_lastKey = false;
		return true;
	}

	bool PbxWriter::
		handleUID(UID *uid, bool root)
	{
		/* Write a CF$UID dictionary. */
		std::unique_ptr<Dictionary> dictionary = Dictionary::New();
		dictionary->set("CF$UID", Integer::New(uid->value()));
		return handleDictionary(dictionary.get(), root);
	}
}
