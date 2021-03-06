//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
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
//!
//!  @file
//!  Representation of a satellite_delivery_system_descriptor
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractDeliverySystemDescriptor.h"

namespace ts {
    //!
    //! Representation of a satellite_delivery_system_descriptor.
    //! @see ETSI 300 468, 6.2.13.2.
    //! @ingroup descriptor
    //!
    class TSDUCKDLL SatelliteDeliverySystemDescriptor : public AbstractDeliverySystemDescriptor
    {
    public:
        // SatelliteDeliverySystemDescriptor public members:
        uint64_t frequency;          //!< Frequency in Hz (warning: coded in 10 kHz units in descriptor).
        uint16_t orbital_position;   //!< Orbital position, unit is 0.1 degree.
        bool     east_not_west;      //!< True for East, false for West.
        uint8_t  polarization;       //!< Polarization, 0..3 (2 bits).
        uint8_t  roll_off;           //!< Roll-off factor, 0..3 (2 bits).
        bool     dvb_s2;             //!< True for DVB-S2, false for DVB-S.
        uint8_t  modulation_type;    //!< Modulation type, 0..3 (2 bits).
        uint64_t symbol_rate;        //!< Symbol rate (warning: coded in 100 symbol/s units in descriptor).
        uint8_t  FEC_inner;          //!< FEC inner, 4 bits.

        //!
        //! Default constructor.
        //!
        SatelliteDeliverySystemDescriptor();

        //!
        //! Constructor from a binary descriptor.
        //! @param [in,out] duck TSDuck execution context.
        //! @param [in] bin A binary descriptor to deserialize.
        //!
        SatelliteDeliverySystemDescriptor(DuckContext& duck, const Descriptor& bin);

        // Inherited methods
        virtual DeliverySystem deliverySystem() const override;
        virtual void serialize(DuckContext&, Descriptor&) const override;
        virtual void deserialize(DuckContext&, const Descriptor&) override;
        virtual void fromXML(DuckContext&, const xml::Element*) override;
        DeclareDisplayDescriptor();

    protected:
        // Inherited methods
        virtual void buildXML(DuckContext&, xml::Element*) const override;

    private:
        // Enumerations for XML.
        friend class S2XSatelliteDeliverySystemDescriptor;
        static const Enumeration DirectionNames;
        static const Enumeration PolarizationNames;
        static const Enumeration RollOffNames;
        static const Enumeration SystemNames;
        static const Enumeration ModulationNames;
        static const Enumeration CodeRateNames;
    };
}
