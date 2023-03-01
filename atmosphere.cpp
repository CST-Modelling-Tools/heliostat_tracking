#include <math.h>
#include "atmosphere.h"

void hypl::Atmosphere::AssignTransmittanceFunction(TransmittanceModel transmittance_model)
{
    switch ( m_transmittance_model )
    {
        case TransmittanceModel::SW:
            m_ptrTransmittanceFunction = &Atmosphere::TransmittanceSW;
            break;
        case TransmittanceModel::VB:
             m_ptrTransmittanceFunction = &Atmosphere::TransmittanceVB;
            break;
        case TransmittanceModel::LH:
            m_ptrTransmittanceFunction = &Atmosphere::TransmittanceLH;
            break;
    }    
}

double hypl::Atmosphere::Transmittance(double slant_range) const
{
    return ((*this).*m_ptrTransmittanceFunction)(slant_range);
}

double hypl::Atmosphere::TransmittanceSW(double slant_range) const
{
    return exp(-(0.000010696 + 0.0009196 * m_beta) * slant_range);
}

double hypl::Atmosphere::TransmittanceVB(double slant_range) const
{
    return 0.99326 + slant_range * (-0.0001046 + (1.7000000000000003e-8 - 2.845e-12 * slant_range) * slant_range);
}

double hypl::Atmosphere::TransmittanceLH(double slant_range) const
{
    if(slant_range <= 1000.0) return 0.99321 + (-0.0001176 + 1.9699999999999998e-8 * slant_range) * slant_range;
    else return exp(-0.0001106 * slant_range);
}

double hypl::Atmosphere::DniFromSz(double sz) const
{
    if(sz>0.00001) return m_io * exp(-m_beta/sz);
    else return 0.0;
}
