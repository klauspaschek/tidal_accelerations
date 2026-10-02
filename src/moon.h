#ifndef MOON_H
#define MOON_H

#include <vector>
#include <memory>
#define _USE_MATH_DEFINES
#include <cmath>

#include "body.h"


enum class Rp    // reference plane of moon orbit
{
    ECLIPTIC,
    LAPLACE,
    EQUATORIAL
};

Rp str_to_rp(const std::string& input_str, const std::string& object_name)
{
    if (input_str == "ecliptic")
        return Rp::ECLIPTIC;
    else if (input_str == "Laplace_plane")
        return Rp::LAPLACE;
    else if (input_str == "equatorial")
        return Rp::EQUATORIAL;
    else
        throw Simple_Err(std::string("Type of reference plane of object '") + object_name +
                         std::string("' is not 'ecliptic', 'Laplace_plane' or 'equatorial'"));
}


class Moon : public Body
{
  private:
    std::string m_name_planet;
    smrtptr<Body> m_ptr_planet;    // pointer to have access to cartesian coordinates of corresponding planet
    unsigned int m_id_planet;      // allows to check if Body object of corresponding planet is still in
                                   // initially memory space and so 'm_ptr_planet' still points to right Body
    Rp m_ref_plane;
    double m_right_asc;      // right ascension and
    double m_declination;    // declination of the reference plane pole

  public:
    Moon(const std::string& name, const std::string& primary_designation,
         const bool& calc_tidal, const Tu& per_day_year_century,
         const double& mass, const double& min_mass, const double& max_mass,
         const double& radius,
         const double& O, const double& O_dot,
         const double& w, const double& w_dot,
         const double& M, const double& M_dot,
         const double& a, const double& a_dot,
         const double& e, const double& e_dot,
         const double& i, const double& i_dot,
         const std::string& name_planet, const vec_smrtptr<Body>& planets,
         const double& epoch = 2451545.,
         const Rp& ref_plane = Rp::ECLIPTIC,
         const double& right_asc = 0., const double& declination = 0.)
    : Body(name, primary_designation,
           calc_tidal, per_day_year_century,
           mass, min_mass, max_mass,
           radius,
           O, O_dot,
           w, w_dot,
           M, M_dot,
           a, a_dot,
           e, e_dot,
           i, i_dot,
           epoch),
      m_name_planet(name_planet),
      m_ref_plane(ref_plane), m_right_asc(right_asc), m_declination(declination)
    {
        for (smrtptr<Body> planet:planets)
        {
            if (planet->get_name() == m_name_planet)
            {
                m_ptr_planet = planet;
                m_id_planet  = planet->get_id();

                return ;
            }
        }

        throw Simple_Err(std::string("Couldn't find corresponding planet '") + m_name_planet +
                         std::string("' for moon '") + get_name() + std::string("'"));
    }

    virtual void time_evol() override
    {
        Body::time_evol();

        // rotate reference plane different from ecliptic into ICRF and then into ecliptic
        if (m_ref_plane != Rp::ECLIPTIC)
            get_cart_coords() = rot_x(M_PI / 180. * 23.43697) *
                                rot_z(-m_right_asc) *
                                rot_y(-(m_declination - (M_PI / 2))) * get_cart_coords();

        if (m_ptr_planet->get_id() != m_id_planet)
            throw Simple_Err(std::string("Planet '") + m_name_planet + std::string("' corresponding to moon '") + get_name() +
                             std::string("' was probably moved to other memory space and interchanged with other Body object"));
        // shift focal point of moon orbit to position of corresponding planet
        get_cart_coords() += m_ptr_planet->get_cart_coords();

        return ;
    }

    // simplified version of the orbit, which is then given in the ecliptic as reference plane
    virtual void simplify() override
    {
        Body::simplify();
        m_ref_plane = Rp::ECLIPTIC;

        return ;
    }

  private:
    // cartesian rotation matrix to rotate (together with using rotation matrices inherited from Body base class)
    // ICRF (international celestial reference frame) plane into ecliptic plane
    Eigen::Matrix3d rot_y(const double& angle) const
    {
        Eigen::Matrix3d rot_mat;
        rot_mat << std::cos(angle), 0., -std::sin(angle),
                                0., 1.,               0.,
                   std::sin(angle), 0.,  std::cos(angle);

        return rot_mat;
    }
};

#endif // MOON_H
