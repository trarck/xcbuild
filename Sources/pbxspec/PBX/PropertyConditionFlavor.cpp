// Copyright 2013-present Facebook. All Rights Reserved.

#include <pbxspec/PBX/PropertyConditionFlavor.h>

using pbxspec::PBX::PropertyConditionFlavor;

PropertyConditionFlavor::PropertyConditionFlavor(bool isDefault) :
    Specification(ISA::PBXPropertyConditionFlavor, isDefault),
    _precedence  (0)
{
}

PropertyConditionFlavor::~PropertyConditionFlavor()
{
}

PropertyConditionFlavor::shared_ptr PropertyConditionFlavor::
Parse(plist::Dictionary const *dict)
{
    auto T = dict->value <plist::String> ("Type");
    if (T == nullptr || T->value() != Type())
        return false;

    PropertyConditionFlavor::shared_ptr result;
    auto C = dict->value <plist::String> ("Class");
    if (C == nullptr) {
        result.reset(new PropertyConditionFlavor(true));
    } else {
        fprintf(stderr, "warning: property condition flavor class '%s' not recognized\n",
                C->value().c_str());
        result.reset(new PropertyConditionFlavor(true));
    }
    if (!result->parse(dict))
        return nullptr;

    return result;
}

bool PropertyConditionFlavor::
parse(plist::Dictionary const *dict)
{
    plist::WarnUnhandledKeys(dict, "PropertyConditionFlavor",
        // Specification
        plist::MakeKey <plist::String> ("Class"),
        plist::MakeKey <plist::String> ("Type"),
        plist::MakeKey <plist::String> ("Identifier"),
        plist::MakeKey <plist::String> ("BasedOn"),
        plist::MakeKey <plist::String> ("Name"),
        plist::MakeKey <plist::String> ("Description"),
        plist::MakeKey <plist::String> ("Vendor"),
        plist::MakeKey <plist::String> ("Version"),
        // PropertyConditionFlavor
        plist::MakeKey <plist::Integer> ("Precedence"));

    if (!Specification::parse(dict))
        return false;

    auto P = dict->value <plist::Integer> ("Precedence");

    if (P != nullptr) {
        _precedence = P->value();
    }

    return true;
}

bool PropertyConditionFlavor::
inherit(Specification::shared_ptr const &base)
{
    if (!base->isa(PropertyConditionFlavor::Isa()))
        return false;

    return inherit(reinterpret_cast <PropertyConditionFlavor::shared_ptr const &> (base));
}

bool PropertyConditionFlavor::
inherit(PropertyConditionFlavor::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _precedence = base->precedence();

    return true;
}