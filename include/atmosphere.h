#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

namespace hypl
{
    class Atmosphere
    {
    public:

        enum TransmittanceModel { SW, VB, LH };

        Atmosphere() :
            m_io {1110.0},
            m_beta {0.11},
            m_transmittance_model {TransmittanceModel::VB} { AssignTransmittanceFunction(m_transmittance_model); }

        Atmosphere(double io, double beta, Atmosphere::TransmittanceModel transmittance_model)
            : m_io {io}, 
              m_beta {beta}, 
              m_transmittance_model {transmittance_model} { AssignTransmittanceFunction(m_transmittance_model); }

        //Accessors
        double const& io() const { return m_io; }
        double const& beta() const {return m_beta; }
        TransmittanceModel transmittance_model() const { return m_transmittance_model; }

        //Mutators
        void set_io(double io) { m_io = io; }
        void set_beta( double beta) { m_beta = beta; }
        void set_transmittance_model(TransmittanceModel transmittance_model) {m_transmittance_model = transmittance_model; }

        //Specific public functions
        double Transmittance(double slant_range ) const;
        double DniFromSz(double sz) const;

    private:
        //Private functions
        double TransmittanceSW(double slant_range) const;
        double TransmittanceVB(double slant_range) const;
        double TransmittanceLH(double slant_range) const;
        void AssignTransmittanceFunction(TransmittanceModel transmittance_model);

        //Private variables
        double m_io;
        double m_beta;
        TransmittanceModel m_transmittance_model;
        double (Atmosphere::*m_ptrTransmittanceFunction)(double) const;
    };
}

#endif // ATMOSPHERE_H