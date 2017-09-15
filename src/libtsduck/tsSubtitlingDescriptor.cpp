//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2017, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//
//  Representation of a subtitling_descriptor
//
//----------------------------------------------------------------------------

#include "tsSubtitlingDescriptor.h"
#include "tsFormat.h"
#include "tsHexa.h"
#include "tsNames.h"
#include "tsTablesFactory.h"
TSDUCK_SOURCE;
TS_XML_DESCRIPTOR_FACTORY(ts::SubtitlingDescriptor, "subtitling_descriptor");
TS_ID_DESCRIPTOR_FACTORY(ts::SubtitlingDescriptor, ts::EDID(ts::DID_SUBTITLING));
TS_ID_DESCRIPTOR_DISPLAY(ts::SubtitlingDescriptor::DisplayDescriptor, ts::EDID(ts::DID_SUBTITLING));


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::SubtitlingDescriptor::Entry::Entry(const char* code, uint8_t subt, uint16_t comp, uint16_t ancil) :
    language_code(code != 0 ? code : ""),
    subtitling_type(subt),
    composition_page_id(comp),
    ancillary_page_id(ancil)
{
}

ts::SubtitlingDescriptor::SubtitlingDescriptor() :
    AbstractDescriptor(DID_SUBTITLING, "subtitling_descriptor"),
    entries()
{
    _is_valid = true;
}

ts::SubtitlingDescriptor::SubtitlingDescriptor(const Descriptor& desc) :
    AbstractDescriptor(DID_SUBTITLING, "subtitling_descriptor"),
    entries()
{
    deserialize(desc);
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::SubtitlingDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 8) {
        uint8_t type = data[3];
        uint16_t comp_page = GetUInt16(data + 4);
        uint16_t ancil_page = GetUInt16(data + 6);
        strm << margin << "Language: " << Printable(data, 3)
             << ", Type: " << int(type)
             << Format(" (0x%02X)", int(type)) << std::endl
             << margin << "Type: " << names::SubtitlingType(type) << std::endl
             << margin << "Composition page: " << comp_page
             << Format(" (0x%04X)", int(comp_page))
             << ", Ancillary page: " << ancil_page
             << Format(" (0x%04X)", int(ancil_page)) << std::endl;
        data += 8; size -= 8;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::SubtitlingDescriptor::serialize (Descriptor& desc) const
{
    if (entries.size() > MAX_ENTRIES) {
        desc.invalidate();
        return;
    }

    ByteBlockPtr bbp(new ByteBlock(2));
    CheckNonNull(bbp.pointer());

    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        if (it->language_code.length() != 3) {
            desc.invalidate();
            return;
        }
        bbp->append(it->language_code);
        bbp->appendUInt8(it->subtitling_type);
        bbp->appendUInt16(it->composition_page_id);
        bbp->appendUInt16(it->ancillary_page_id);
    }

    (*bbp)[0] = _tag;
    (*bbp)[1] = uint8_t(bbp->size() - 2);
    Descriptor d(bbp, SHARE);
    desc = d;
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::SubtitlingDescriptor::deserialize (const Descriptor& desc)
{
    entries.clear();

    if (!(_is_valid = desc.isValid() && desc.tag() == _tag)) {
        return;
    }

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    while (size >= 8) {
        Entry entry;
        entry.language_code = std::string(reinterpret_cast<const char*>(data), 3);
        entry.subtitling_type = data[3];
        entry.composition_page_id = GetUInt16(data + 4);
        entry.ancillary_page_id = GetUInt16(data + 6);
        entries.push_back(entry);
        data += 8; size -= 8;
    }

    _is_valid = size == 0;
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

ts::XML::Element* ts::SubtitlingDescriptor::toXML(XML& xml, XML::Element* parent) const
{
    XML::Element* root = _is_valid ? xml.addElement(parent, _xml_name) : 0;
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        XML::Element* e = xml.addElement(root, "subtitling");
        xml.setAttribute(e, "language_code", it->language_code);
        xml.setIntAttribute(e, "subtitling_type", it->subtitling_type, true);
        xml.setIntAttribute(e, "composition_page_id", it->composition_page_id, true);
        xml.setIntAttribute(e, "ancillary_page_id", it->ancillary_page_id, true);
    }
    return root;
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::SubtitlingDescriptor::fromXML(XML& xml, const XML::Element* element)
{
    entries.clear();
    XML::ElementVector children;
    _is_valid =
        checkXMLName(xml, element) &&
        xml.getChildren(children, element, "subtitling", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Entry entry;
        _is_valid =
            xml.getAttribute(entry.language_code, children[i], "language_code", true, "", 3, 3) &&
            xml.getIntAttribute<uint8_t>(entry.subtitling_type, children[i], "subtitling_type", true) &&
            xml.getIntAttribute<uint16_t>(entry.composition_page_id, children[i], "composition_page_id", true) &&
            xml.getIntAttribute<uint16_t>(entry.ancillary_page_id, children[i], "ancillary_page_id", true);
        if (_is_valid) {
            entries.push_back(entry);
        }
    }
}