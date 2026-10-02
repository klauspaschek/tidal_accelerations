#ifndef BODY_H
#define BODY_H

#include <string>
#include <map>
#include <exception>
#include <array>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <memory>

#include <Eigen/Dense>

#include "exceptions.h"


template<typename T>
using vec_smrtptr = std::vector<std::shared_ptr<T> >;

template<typename T>
using vec_vec_smrtptr = std::vector<std::vector<std::shared_ptr<T> > >;

template<typename T>
using smrtptr = std::shared_ptr<T>;

template<typename T>
using weak_smrtptr = std::weak_ptr<T>;

// defines alteration rates of orbital elements
enum class Tu    // time unit
{
    PER_DAY,
    PER_YEAR,
    PER_CENTURY
};

std::string Tu_to_string(const Tu& unit)
{
    if (unit == Tu::PER_DAY)
        return std::string("PER_DAY");
    else if (unit == Tu::PER_YEAR)
        return std::string("PER_YEAR");
    else
        return std::string("PER_CENTURY");
}

// make it easy to address orbital elements
enum class Oe
{
    LONG_ASC_NODE,    // longitude of the ascending node (radians)  -  O(mega)
    ARG_PERIHEL,      // argument of perihelion (radians)           -  w  -  w + O(mega) = w_bar (longitude of perihelion)
    MEAN_ANOMALY,     // mean anomaly (radians)                     -  M
    SEM_MAJ_AXIS,     // semi major axis (au)                       -  a
    ECCENTRICITY,     // eccentricity                               -  e
    INCLINATION       // inclination of orbit (radians)             -  i
};

// make it easy to address different types of orbital elements
enum class Oe_type
{
    ZERO,    // orbital element at time julian date zero (JD_O)
    DOT,     // temporal change of orbital element per time_unit
    NOW      // current value in time
};

// make it easy to address components of Eigen::Vector3d
enum class Cc    // components of cartesian coordinates
{
    X,
    Y,
    Z
};

enum class M_type    // mass type in Body class
{
    TRUE_MEAN,    // given mass or mean estimate
    MIN,          // minimal estimate
    MAX           // maximal estimate
};

// wrapping class for fixed sized array with three doubles
class Array3d
{
  private:
    std::array<double, 3> m_entries;

  public:
    Array3d()
    : m_entries{ {0.0, 0.0, 0.0} }
    {}

    Array3d(const double& first, const double& second, const double& third)
    {
        m_entries[0] = first;
        m_entries[1] = second;
        m_entries[2] = third;
    }

    // access elements
    double& operator[](const Oe_type& type)
    {
        return m_entries[static_cast<unsigned int>(type)];
    }

    double operator[](const Oe_type& type) const
    {
        return m_entries[static_cast<unsigned int>(type)];
    }
};

// wrapping class of map which stores orbital elements
// each orbital element has three different types
class Map_Array3d
{
  private:
    std::map<Oe, Array3d> m_entries;

  public:
    Map_Array3d()
    {}

    // access to array with all three types of the orbital element
    Array3d& operator[](const Oe& orb_elem)
    {
        return m_entries[orb_elem];
    }

    // access to special type of one orbital element
    double& operator()(const Oe& orb_elem, const Oe_type& type)
    {
        return m_entries[orb_elem][type];
    }

    double operator()(const Oe& orb_elem, const Oe_type& type) const
    {
        return m_entries.at(orb_elem)[type];
    }

    // access to orbital elements with int to make iteration over orbital elements easier
    double& operator()(const unsigned int& index, const Oe_type& type)
    {
        return m_entries[static_cast<Oe>(index)][type];
    }

    double operator()(const unsigned int& index, const Oe_type& type) const
    {
        return m_entries.at(static_cast<Oe>(index))[type];
    }

    unsigned int size() const
    {
        return m_entries.size();
    }
};


class RelevantObjects;

// properties of celestial body and time evolution
class Body
{
  private:
    std::string m_name;
    std::string m_primary_designation;    // number of object (IAU)  -  for planets radial order of orbits
    bool m_calc_tidal;                    // do you want to calculate tidal effects on this object?
    Tu m_per_day_year;                    // time unit
    double m_mass;                        // (kg)
    double m_min_mass;                    // (kg)
    double m_max_mass;                    // (kg)
    double m_radius;                      // (km)

    Map_Array3d m_orbital_elements;

    // parameters in time evolution
  public:
    static double s_time;             // julian date
  private:
    double m_epoch;                   // reference point in time in julian date
    Eigen::Vector3d m_cart_coords;    // ecliptic heliocentric cartesian coordinates x,y,z  -
                                      // x-axis points at vernal equinox  -
                                      // (au)

    // give every object a unique ID
    static unsigned int s_id_generator;
    unsigned int m_id;

    weak_smrtptr<RelevantObjects> m_relevant_objects;

  public:
    Body(std::string name, std::string primary_designation,
         const bool& calc_tidal, const Tu& per_day_year_century,
         const double& mass, const double& min_mass, const double& max_mass,
         const double& radius,
         const double& O, const double& O_dot,
         const double& w, const double& w_dot,
         const double& M, const double& M_dot,
         const double& a, const double& a_dot,
         const double& e, const double& e_dot,
         const double& i, const double& i_dot,
         const double& epoch = 2451545.)    // epoch J2000.0
    : m_calc_tidal(calc_tidal), m_per_day_year(per_day_year_century),
      m_mass(mass), m_min_mass(min_mass), m_max_mass(max_mass),
      m_radius(radius),
      m_epoch(epoch)
    {
        std::replace(name.begin(), name.end(), ' ', '_');
        std::replace(name.begin(), name.end(), '/', '_');
        std::replace(name.begin(), name.end(), '"', '_');
        m_name = name;
        std::replace(primary_designation.begin(), primary_designation.end(), ' ', '_');
        std::replace(primary_designation.begin(), primary_designation.end(), '/', '_');
        std::replace(primary_designation.begin(), primary_designation.end(), '"', '_');
        m_primary_designation = primary_designation;

        m_orbital_elements[Oe::LONG_ASC_NODE] = Array3d(O, O_dot, O);
        m_orbital_elements[Oe::ARG_PERIHEL]   = Array3d(w, w_dot, w);
        m_orbital_elements[Oe::MEAN_ANOMALY]  = Array3d(M, M_dot, M);
        m_orbital_elements[Oe::SEM_MAJ_AXIS]  = Array3d(a, a_dot, a);
        m_orbital_elements[Oe::ECCENTRICITY]  = Array3d(e, e_dot, e);
        m_orbital_elements[Oe::INCLINATION]   = Array3d(i, i_dot, i);

        m_cart_coords << 0.0, 0.0, 0.0;

        m_id = s_id_generator++;
    }

    // do time evolution of orbital elements and calculate cartesian coordinates
    // in heliocentric ecliptic system for current time 's_time'
    virtual void time_evol()
    {
        time_evol_orb_elements();
        calc_cart_orbital();
        rot_orbital_ecliptic();

        return ;
    }

    // get access to cartesian coordinates
    Eigen::Vector3d get_cart_coords() const
    {
        return m_cart_coords;
    }

    Eigen::Vector3d& get_cart_coords()
    {
        return m_cart_coords;
    }

    double operator()(const Cc& component) const
    {
        return m_cart_coords[static_cast<unsigned int>(component)];
    }

    // gives you unit vector with euclidian norm = 1 pointing from object to 'other'
    Eigen::Vector3d unit_vector_to(const Body& other) const
    {
        Eigen::Vector3d unit_vec(other.m_cart_coords - this->m_cart_coords);

        return unit_vec / unit_vec.norm();
    }

    // calculates unit vector in meters and gives tidal acceleration acting on point of body closest to 'other' Body
    // unit: m/s^2
    Eigen::Vector3d tidal_acc(const Body& other, const M_type& mass_type) const
    {
        Eigen::Vector3d vec_to_other(other.m_cart_coords - m_cart_coords);    // (au)
        double vto_norm(vec_to_other.norm());    // (au)
        double r(vto_norm * 149597870700.);    // (m)

        double other_mass(0.);
        if (mass_type == M_type::TRUE_MEAN)
            other_mass = other.m_mass;
        else if (mass_type == M_type::MIN)
            other_mass = other.m_min_mass;
        else
            other_mass = other.m_max_mass;

        Eigen::Vector3d result(((6.67408e-11 * other_mass) / std::pow(r, 2)) *
                               ((1. / std::pow(1. - (m_radius * 1.e3 / r), 2)) - 1.) *
                                (vec_to_other / vto_norm));

        bool contains_nan(false);
        if (std::isnan(result[0]) || std::isnan(result[1]) || std::isnan(result[2]))
            contains_nan = true;

        if (contains_nan)
            return Eigen::Vector3d::Zero();
        else
            return result;
    }

    std::string info()
    {
        std::string yesno;
        if (m_calc_tidal)
            yesno = "yes";
        else
            yesno = "no";

        return std::string("name: ") + m_primary_designation + std::string("_") + m_name +
               std::string("\ntidals are computed: ") + yesno +
               std::string("\nmass (kg): ") + std::to_string(m_mass) +
               std::string("\nradius (km): ") + std::to_string(m_radius) +
               std::string("\nlongitude of the ascending node (radians, radians/") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::LONG_ASC_NODE, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::LONG_ASC_NODE, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::LONG_ASC_NODE, Oe_type::NOW)) +
               std::string("\nargument of perihelion (radians, radians/") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::ARG_PERIHEL, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::ARG_PERIHEL, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::ARG_PERIHEL, Oe_type::NOW)) +
               std::string("\nmean anomaly (radians, radians/") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::NOW)) +
               std::string("\nsemi major axis (au, au/") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::SEM_MAJ_AXIS, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::SEM_MAJ_AXIS, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::SEM_MAJ_AXIS, Oe_type::NOW)) +
               std::string("\neccentricity (, /") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::ECCENTRICITY, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::ECCENTRICITY, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW)) +
               std::string("\ninclination of orbit (radians, radians/") + Tu_to_string(m_per_day_year) + std::string("): ") +
               std::to_string(m_orbital_elements(Oe::INCLINATION, Oe_type::ZERO)) + std::string(", ") +
               std::to_string(m_orbital_elements(Oe::INCLINATION, Oe_type::DOT)) + std::string(" - now: ") +
               std::to_string(m_orbital_elements(Oe::INCLINATION, Oe_type::NOW)) +
               std::string("\ntime: ") + std::to_string(s_time) +
               std::string("\ncartesian coordinates [x,y,z] (au): [") + std::to_string(m_cart_coords(0)) + std::string(",") +
                                                                   std::to_string(m_cart_coords(1)) + std::string(",") +
                                                                   std::to_string(m_cart_coords(2)) + std::string("]");
    }

    // calculate orbital period in days
    double calc_orb_period() const
    {
        if (m_per_day_year == Tu::PER_DAY)
            return 2. * M_PI / m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::DOT);
        else if (m_per_day_year == Tu::PER_YEAR)
            return 2. * M_PI * 365.25 / m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::DOT);
        else if (m_per_day_year == Tu::PER_CENTURY)
            return 2. * M_PI * 36525. / m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::DOT);
    }

    // some getters to members
    std::string get_name() const
    {
        return m_name;
    }

    std::string get_primary_designation() const
    {
        return m_primary_designation;
    }

    bool get_calc_tidal() const
    {
        return m_calc_tidal;
    }

    unsigned int get_id() const
    {
        return m_id;
    }

    double get_radius() const
    {
        return m_radius;
    }

    double get_mass(M_type mass_type) const
    {
        if (mass_type == M_type::TRUE_MEAN)
            return m_mass;
        else if (mass_type == M_type::MIN)
            return m_min_mass;
        else if (mass_type == M_type::MAX)
            return m_max_mass;
    }

    smrtptr<RelevantObjects> get_relevant_objects() const
    {
        if (m_relevant_objects.expired())
            throw Simple_Err("Couldn't lock smart-pointer to 'RelevantObjects' in 'Body'");
        return m_relevant_objects.lock();
    }

    // setters to members
    void set_calc_tidal(bool calc_tidal)
    {
        m_calc_tidal = calc_tidal;

        return ;
    }

    void set_relevant_objects(smrtptr<RelevantObjects> relevant_objects)
    {
        m_relevant_objects = relevant_objects;

        return ;
    }

    // simplified version of the orbit: keep only semi-major axis a and mean motion n (rate of the
    // mean anomaly), all other orbital elements and rates are set to zero and the epoch to J2000.0
    virtual void simplify()
    {
        for (unsigned int i(0); i < m_orbital_elements.size(); ++i)
        {
            if (static_cast<Oe>(i) != Oe::SEM_MAJ_AXIS)
                m_orbital_elements(i, Oe_type::ZERO) = 0.;
            if (static_cast<Oe>(i) != Oe::MEAN_ANOMALY)
                m_orbital_elements(i, Oe_type::DOT) = 0.;
            m_orbital_elements(i, Oe_type::NOW) = m_orbital_elements(i, Oe_type::ZERO);
        }
        m_epoch = 2451545.;

        return ;
    }

    // use ID of two Body object to check if they are the same or not
    bool operator==(const Body& other) const
    {
        return this->m_id == other.m_id;
    }

    bool operator!=(const Body& other) const
    {
        return this->m_id != other.m_id;
    }

  private:
    void time_evol_orb_elements()
    {
        double time_factor(0);
        if (m_per_day_year == Tu::PER_DAY)
            time_factor = s_time - m_epoch;
        else if (m_per_day_year == Tu::PER_YEAR)
            time_factor = (s_time - m_epoch) / 365.25;
        else if (m_per_day_year == Tu::PER_CENTURY)
            time_factor = (s_time - m_epoch) / 36525.;

        for (unsigned int i(0); i < m_orbital_elements.size(); ++i)
            m_orbital_elements(i, Oe_type::NOW) = m_orbital_elements(i, Oe_type::ZERO) +
                                                  m_orbital_elements(i, Oe_type::DOT) * time_factor;

        return ;
    }

    // type alias for member function pointer
    using ptr_mem_fcn = double (Body::*)(const double&) const;

    // find root of function f
    // you have to supply f and its derivative as member function pointers
    double newton_raphson(ptr_mem_fcn f, ptr_mem_fcn f_deriv, const double& x_start, const double& accuracy) const
    {
        double x_old(x_start), x_new(0.);
        for (int i(1); i <= 1.e6; ++i)
        {
            x_new = x_old - (this->*f)(x_old) / (this->*f_deriv)(x_old);

            // stop algorithm if accuracy is achieved
            if (std::abs((x_new - x_old) / x_new) < accuracy)
            {
                return x_new;
            }

            x_old = x_new;
        }

        std::cerr << "Finding root failed with requested accuracy!" << std::endl;

        return x_new;
    }

    // transcendental Kepler equation M = E - eccentricity * sin(E) rearranged to
    // 0 = M - E + eccentricity * sin(E)
    // Newton-Raphson method is used to get solution for eccentric anomaly E using
    // mean anomaly M as start value
    double kepler_equ(const double& ecc_anomaly) const
    {
        return m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::NOW) - ecc_anomaly +
               m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW) * std::sin(ecc_anomaly);
    }

    // derivative of rearranged Kepler equation needed in Newton_Raphson method
    double kepler_equ_deriv(const double& ecc_anomaly) const
    {
        return m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW) * std::cos(ecc_anomaly) - 1.;
    }

    // Newton-Raphson method is used to get solution for eccentric anomaly E using
    // mean anomaly M as start value in rearranged Kepler equation
    double calc_ecc_anomaly() const
    {
        return newton_raphson(&Body::kepler_equ, &Body::kepler_equ_deriv, m_orbital_elements(Oe::MEAN_ANOMALY, Oe_type::NOW), 1.e-9);
    }

    // calculate true anomaly T using eccentric anomaly E
    double calc_true_anom() const
    {
        return 2. * std::atan(std::sqrt((1. + m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW)) /
                                        (1. + m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW))) *
                              std::tan(calc_ecc_anomaly()));
    }

    // calculate distance r of object to central object in focal point of elliptical orbit
    double calc_r(const double& true_anom) const
    {
        double a(m_orbital_elements(Oe::SEM_MAJ_AXIS, Oe_type::NOW));
        double e(m_orbital_elements(Oe::ECCENTRICITY, Oe_type::NOW));
        double p(a * (1. - e * e));

        return p / (1. + e * std::cos(true_anom));
    }

    // use true anomaly T and distance r to focal point to calculate cartesian coordinates in orbital plane
    // with origin in focal point
    void calc_cart_orbital()
    {
        double true_anom(calc_true_anom()), r(calc_r(true_anom));

        m_cart_coords << r * std::cos(true_anom), r * std::sin(true_anom), 0.0;

        return ;
    }

  protected:    // rotation matrices are also needed in public derived moon class
    // cartesian rotation matrices to rotate orbital plane into ecliptic plane
    Eigen::Matrix3d rot_x(const double& angle) const
    {
        Eigen::Matrix3d rot_mat;
        rot_mat << 1.,               0.,              0.,
                   0.,  std::cos(angle), std::sin(angle),
                   0., -std::sin(angle), std::cos(angle);

        return rot_mat;
    }

    Eigen::Matrix3d rot_z(const double& angle) const
    {
        Eigen::Matrix3d rot_mat;
        rot_mat <<  std::cos(angle), std::sin(angle), 0.,
                   -std::sin(angle), std::cos(angle), 0.,
                                 0.,              0., 1.;

        return rot_mat;
    }

  private:
    // rotate orbital plane into ecliptic plane using the angles in the orbital elements
    void rot_orbital_ecliptic()
    {
        m_cart_coords = rot_z(-m_orbital_elements(Oe::LONG_ASC_NODE, Oe_type::NOW)) *
                        rot_x(-m_orbital_elements(Oe::INCLINATION  , Oe_type::NOW)) *
                        rot_z(-m_orbital_elements(Oe::ARG_PERIHEL  , Oe_type::NOW)) * m_cart_coords;

        return ;
    }
};

// start value for IDs of Body objects
unsigned int Body::s_id_generator(0);

#endif // BODY_H
