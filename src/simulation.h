#ifndef SIMULATION_H
#define SIMULATION_H

// functions for the time iteration and the output of the tidal accelerations,
// shared by the simulations of the different planetary systems

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#define NDEBUG
#include <cassert>
#include <algorithm>
#include <utility>
#include <tuple>

#include <Eigen/Dense>

#include "csvdata.h"
#include "body.h"
#include "moon.h"
#include "sun.h"
#include "relevantobjects.h"


// start value for time in julian date
double Body::s_time(2451545.);

// convert degrees to radians
double deg_to_rad(const double& degree)
{
    return (M_PI / 180.) * degree;
}

// converts string to double
// if string is empty, it returns 0.
double str_to_d_zero(const std::string& input_str)
{
    if (input_str != "")
        return std::stod(input_str);
    else
        return 0.;
}

// converts angle precession period to temporal rate of change of angle
// resulting angle/time_unit is in radians/time_unit
double period_to_d(const std::string& input_str, const Tu& given_day_year_century, const Tu& desired_per_day_year_century)
{
    if (input_str == "")
        return 0;
    else if (given_day_year_century == Tu::PER_DAY)
    {
        if (desired_per_day_year_century == Tu::PER_DAY)
            return 2. * M_PI / std::stod(input_str);
        else if (desired_per_day_year_century == Tu::PER_YEAR)
            return 2. * M_PI * 365.25 / std::stod(input_str);
        else if (desired_per_day_year_century == Tu::PER_CENTURY)
            return 2. * M_PI * 36525. / std::stod(input_str);
    }
    else if (given_day_year_century == Tu::PER_YEAR)
    {
        if (desired_per_day_year_century == Tu::PER_DAY)
            return 2. * M_PI / (std::stod(input_str) * 365.25);
        else if (desired_per_day_year_century == Tu::PER_YEAR)
            return 2. * M_PI / std::stod(input_str);
        else if (desired_per_day_year_century == Tu::PER_CENTURY)
            return 2. * M_PI * 100. / std::stod(input_str);
    }
    else if (given_day_year_century == Tu::PER_CENTURY)
    {
        if (desired_per_day_year_century == Tu::PER_DAY)
            return 2. * M_PI / (std::stod(input_str) * 36525.);
        else if (desired_per_day_year_century == Tu::PER_YEAR)
            return 2. * M_PI / (std::stod(input_str) * 100.);
        else if (desired_per_day_year_century == Tu::PER_CENTURY)
            return 2. * M_PI / std::stod(input_str);
    }
}

// prepare text files for calculation of tidal acceleration
// each 'Body' in 'objects' gets one .csv file with the julian date, all other 'objects'
// and total acceleration listed in first row
// you have to pass 'objects' as vector of vectors, which include the different types of objects
// e.g. you have a vector with asteroids and one with the planets
// you concatenate them to a vector of vectors
void prep_tidal_acc(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, double placebo2)
{
    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            // generate .csv only for objects you want to calculate the tidal acceleration
            if (object->get_calc_tidal())
            {
                std::string filename(directory + std::string("/") + filename_prefix + std::string("_tidal_acc_") +
                                     object->get_primary_designation() + std::string("_") + object->get_name() + std::string(".csv"));

                std::ofstream output(filename);
                if (!output.is_open())
                    throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

                output << "julian_date";

                // iterate over all 'other' objects
                for (const vec_smrtptr<Body>& other_list:objects)
                {
                    for (smrtptr<Body> other_object:other_list)
                    {
                        if ((*object) != (*other_object))
                            output << ',' << other_object->get_primary_designation()
                                   << '_' << other_object->get_name() << ",min_mass,max_mass";
                    }
                }

                output << ",total,min_mass,max_mass,total_change,min_mass,max_mass\n";

                output.close();
            }
        }
    }

    return ;
}

// calculate tidal accelerations for one point in time and write them in prepared .csv files
std::vector<std::vector<Eigen::Vector3d> > calc_tidal_acc(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const std::vector<std::vector<Eigen::Vector3d> >& last_iteration, double placebo2)
{
    std::vector<std::vector<Eigen::Vector3d> > new_iteration;
    new_iteration.reserve(last_iteration.size());
    unsigned int i(0);

    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            if (object->get_calc_tidal())
            {
                std::string filename(directory + std::string("/") + filename_prefix + std::string("_tidal_acc_") +
                                     object->get_primary_designation() + std::string("_") + object->get_name() +
                                     std::string(".csv"));

                std::ofstream output(filename, std::fstream::app);
                if (!output.is_open())
                    throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

                output.precision(20.);

                output << Body::s_time;

                Eigen::Vector3d total(Eigen::Vector3d::Zero()),
                                total_min(Eigen::Vector3d::Zero()),
                                total_max(Eigen::Vector3d::Zero());

                for (const vec_smrtptr<Body>& other_list:objects)
                {
                    for (smrtptr<Body> other_object:other_list)
                    {
                        if ((*object) != (*other_object))
                        {
                            Eigen::Vector3d tidal_acc_vec(object->tidal_acc(*other_object, M_type::TRUE_MEAN)),
                                            tidal_acc_vec_min(object->tidal_acc(*other_object, M_type::MIN)),
                                            tidal_acc_vec_max(object->tidal_acc(*other_object, M_type::MAX));
                            total     += tidal_acc_vec;
                            total_min += tidal_acc_vec_min;
                            total_max += tidal_acc_vec_max;

                            output << ',' << tidal_acc_vec.norm()
                                   << ',' << tidal_acc_vec_min.norm()
                                   << ',' << tidal_acc_vec_max.norm();
                        }
                    }
                }

                assert(last_iteration.at(i).size() == 3);

                Eigen::Vector3d total_change(    total     - last_iteration.at(i).at(0)),
                                total_change_min(total_min - last_iteration.at(i).at(1)),
                                total_change_max(total_max - last_iteration.at(i).at(2));

                std::vector<Eigen::Vector3d> new_iteration_sub;
                new_iteration_sub.push_back(total);
                new_iteration_sub.push_back(total_min);
                new_iteration_sub.push_back(total_max);
                new_iteration.push_back(new_iteration_sub);

                output << ',' << total.norm() << ',' << total_min.norm() << ',' << total_max.norm()
                       << ',' << total_change.norm() << ',' << total_change_min.norm() << ',' << total_change_max.norm()
                       << std::endl;

                output.close();

                ++i;
            }
        }
    }

    return new_iteration;
}

// writes file containing positions of Body objects in time evolution
void prep_positions(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, double placebo2, double placebo3)
{
    std::ofstream output(directory + std::string("/") + filename_prefix + std::string("_positions.csv"));
    output << "julian_date";

    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            output << ',' << object->get_name();
        }
    }

    output << std::endl;

    return ;
}

std::vector<std::vector<Eigen::Vector3d> > write_positions(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const std::vector<std::vector<Eigen::Vector3d> >& placebo, double placebo2, double placebo3)
{
    std::ofstream output(directory + std::string("/") + filename_prefix + std::string("_positions.csv"), std::fstream::app);
    output.precision(20.);
    output << Body::s_time;

    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            output << ',' << (*object)(Cc::X) << '/' << (*object)(Cc::Y) << '/' << (*object)(Cc::Z);
        }
    }

    output << std::endl;

    return placebo;
}

// do full time iteration and set relevant objects for each object
void find_relevant_objects(const vec_vec_smrtptr<Body>& objects, const vec_smrtptr<RelevantObjects>& relevant_objects, double time_step, unsigned int iterations)
{
    for (unsigned int i(0); i < iterations; ++i)
    {
        Body::s_time += time_step;

        for (const vec_smrtptr<Body>& list:objects)
        {
            for (smrtptr<Body> object:list)
                object->time_evol();
        }

        for (smrtptr<RelevantObjects> rel_object:relevant_objects)
            rel_object->update(objects);

        int prev_count(static_cast<int>(static_cast<double>(i) / iterations * 100.)),
            current_count(static_cast<int>(static_cast<double>(i + 1) / iterations * 100.));
        if (prev_count < current_count)
            std::cout << current_count << "%\r" << std::flush;
    }
}

// create an instance of RelevantObjects for each Body object
// you want to calculate the tidal effects
void prep_relevant_objects(const vec_vec_smrtptr<Body>& objects, vec_smrtptr<RelevantObjects>& relevant_objects, double time_step, unsigned int iterations, unsigned int n_max_tidal, unsigned int n_var_tidal)
{
    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            if (object->get_calc_tidal())
            {
                relevant_objects.push_back(smrtptr<RelevantObjects>(new
                    RelevantObjects(object, n_max_tidal, n_var_tidal/*, iterations*/)));
                object->set_relevant_objects(relevant_objects.back());
            }
        }
    }

    double time_start(Body::s_time);
    find_relevant_objects(objects, relevant_objects, time_step, iterations);
    Body::s_time = time_start;

    return ;
}

// prepare text files for calculation of tidal acceleration
// each 'Body' in 'objects' gets one .csv file with the julian date, all relevant 'objects',
// other and total acceleration listed in first row
// you have to pass 'objects' as vector of vectors, which include the different types of objects
// e.g. you have a vector with asteroids and one with the planets
// you concatenate them to a vector of vectors
void prep_tidal_acc_relevant(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, double min_tidal_value, double max_tidal_value)
{
    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            // generate .csv only for objects you want to calculate the tidal acceleration
            if (object->get_calc_tidal())
            {
                if (object->get_relevant_objects()->get_max_value() >= min_tidal_value &&
                    object->get_relevant_objects()->get_max_value() <  max_tidal_value)
                {
                    std::string filename(directory + std::string("/") + filename_prefix + std::string("_tidal_acc_") +
                                         object->get_primary_designation() + std::string("_") + object->get_name() + std::string(".csv"));

                    std::ofstream output(filename);
                    if (!output.is_open())
                        throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

                    output << "julian_date";

                    // iterate over all 'other' relevant objects
                    vec_smrtptr<Body> relevant_objects(object->get_relevant_objects()->get_calc_relevant_objects());
                    for (smrtptr<Body> rel_object:relevant_objects)
                    {
                        output << ',' << rel_object->get_primary_designation()
                               << '_' << rel_object->get_name() << ",min_mass,max_mass";
                    }

                    output << ",other,min_mass,max_mass,total,min_mass,max_mass,total_change,min_mass,max_mass\n";

                    output.close();
                }
            }
        }
    }

    return ;
}

// calculate tidal accelerations for one point in time and write them in prepared .csv files
std::vector<std::vector<Eigen::Vector3d> > calc_tidal_acc_relevant(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const std::vector<std::vector<Eigen::Vector3d> >& last_iteration, double min_tidal_value, double max_tidal_value)
{
    std::vector<std::vector<Eigen::Vector3d> > new_iteration;
    new_iteration.reserve(last_iteration.size());
    unsigned int i(0);

    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            if (object->get_calc_tidal())
            {
                if (object->get_relevant_objects()->get_max_value() >= min_tidal_value &&
                    object->get_relevant_objects()->get_max_value() <  max_tidal_value)
                {
                    std::string filename(directory + std::string("/") + filename_prefix + std::string("_tidal_acc_") +
                                         object->get_primary_designation() + std::string("_") + object->get_name() +
                                         std::string(".csv"));

                    std::ofstream output(filename, std::fstream::app);
                    if (!output.is_open())
                        throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

                    output.precision(20.);

                    output << Body::s_time;

                    Eigen::Vector3d total(Eigen::Vector3d::Zero()),
                                    total_min(Eigen::Vector3d::Zero()),
                                    total_max(Eigen::Vector3d::Zero());

                    vec_smrtptr<Body> relevant_objects(object->get_relevant_objects()->get_calc_relevant_objects());
                    for (smrtptr<Body> rel_object:relevant_objects)
                    {
                        Eigen::Vector3d tidal_acc_vec(object->tidal_acc(*rel_object, M_type::TRUE_MEAN)),
                                        tidal_acc_vec_min(object->tidal_acc(*rel_object, M_type::MIN)),
                                        tidal_acc_vec_max(object->tidal_acc(*rel_object, M_type::MAX));
                        total     += tidal_acc_vec;
                        total_min += tidal_acc_vec_min;
                        total_max += tidal_acc_vec_max;

                        output << ',' << tidal_acc_vec.norm()
                               << ',' << tidal_acc_vec_min.norm()
                               << ',' << tidal_acc_vec_max.norm();
                    }

                    Eigen::Vector3d other(Eigen::Vector3d::Zero()),
                                    other_min(Eigen::Vector3d::Zero()),
                                    other_max(Eigen::Vector3d::Zero());

                    for (const vec_smrtptr<Body>& other_list:objects)
                    {
                        for (smrtptr<Body> other_object:other_list)
                        {
                            if (((*object) != (*other_object)) &&
                                std::none_of(relevant_objects.begin(), relevant_objects.end(), [other_object](smrtptr<Body> element)
                                                                                               {
                                                                                                   return element == other_object;
                                                                                               }))
                            {
                                Eigen::Vector3d tidal_acc_vec(object->tidal_acc(*other_object, M_type::TRUE_MEAN)),
                                                tidal_acc_vec_min(object->tidal_acc(*other_object, M_type::MIN)),
                                                tidal_acc_vec_max(object->tidal_acc(*other_object, M_type::MAX));
                                total     += tidal_acc_vec;
                                total_min += tidal_acc_vec_min;
                                total_max += tidal_acc_vec_max;
                                other     += tidal_acc_vec;
                                other_min += tidal_acc_vec_min;
                                other_max += tidal_acc_vec_max;
                            }
                        }
                    }

                    assert(last_iteration.at(i).size() == 3);

                    Eigen::Vector3d total_change(    total     - last_iteration.at(i).at(0)),
                                    total_change_min(total_min - last_iteration.at(i).at(1)),
                                    total_change_max(total_max - last_iteration.at(i).at(2));

                    std::vector<Eigen::Vector3d> new_iteration_sub;
                    new_iteration_sub.push_back(total);
                    new_iteration_sub.push_back(total_min);
                    new_iteration_sub.push_back(total_max);
                    new_iteration.push_back(new_iteration_sub);

                    output << ',' << other.norm() << ',' << other_min.norm() << ',' << other_max.norm()
                           << ',' << total.norm() << ',' << total_min.norm() << ',' << total_max.norm()
                           << ',' << total_change.norm() << ',' << total_change_min.norm() << ',' << total_change_max.norm()
                           << std::endl;

                    output.close();

                    ++i;
                }
            }
        }
    }

    return new_iteration;
}

// iterate time and execute function supplied by pointer
void time_iteration(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const double& time_step, const unsigned int& iterations, void (*prep_fct_ptr)(const vec_vec_smrtptr<Body>&, const std::string&, const std::string&, double, double), std::vector<std::vector<Eigen::Vector3d> > (*fct_ptr)(const vec_vec_smrtptr<Body>&, const std::string&, const std::string&, const std::vector<std::vector<Eigen::Vector3d> >&, double, double), double min_tidal_value = 0., double max_tidal_value = 0., bool supply_temp_change = false)
{
    prep_fct_ptr(objects, directory, filename_prefix, min_tidal_value, max_tidal_value);

    std::vector<std::vector<Eigen::Vector3d> > last_iteration;
    if (supply_temp_change)
    {
        for (const vec_smrtptr<Body>& list:objects)
        {
            for (smrtptr<Body> object:list)
            {
                if (object->get_calc_tidal())
                {
                    if (object->get_relevant_objects()->get_max_value() >= min_tidal_value &&
                        object->get_relevant_objects()->get_max_value() <  max_tidal_value)
                    {
                        std::vector<Eigen::Vector3d> last_iteration_sub;
                        last_iteration_sub.push_back(Eigen::Vector3d::Zero());
                        last_iteration_sub.push_back(Eigen::Vector3d::Zero());
                        last_iteration_sub.push_back(Eigen::Vector3d::Zero());
                        last_iteration.push_back(last_iteration_sub);
                    }
                }
            }
        }
    }

    for (unsigned int i(0); i < iterations; ++i)
    {
        Body::s_time += time_step;

        for (const vec_smrtptr<Body>& list:objects)
        {
            for (smrtptr<Body> object:list)
                object->time_evol();
        }

        last_iteration = fct_ptr(objects, directory, filename_prefix, last_iteration, min_tidal_value, max_tidal_value);

        int prev_count(static_cast<int>(static_cast<double>(i) / iterations * 100.)),
            current_count(static_cast<int>(static_cast<double>(i + 1) / iterations * 100.));
        if (prev_count < current_count)
            std::cout << current_count << "%\r" << std::flush;
    }

    return ;
}

void write_orb_periods(const vec_vec_smrtptr<Body>& objects)
{
    using tuple_sdd = std::tuple<std::string, double, double>;
    std::vector<tuple_sdd> orb_periods;
    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            orb_periods.push_back(tuple_sdd(object->get_primary_designation() + std::string("_") + object->get_name(),
                                            object->calc_orb_period(), object->get_radius()));
        }
    }

    std::sort(orb_periods.begin(), orb_periods.end(), [](const tuple_sdd& a, const tuple_sdd& b)
                                                        {
                                                            return std::get<1>(a) < std::get<1>(b);
                                                        });

    std::ofstream output("../orbital_periods.csv");
    if (!output.is_open())
        throw Simple_Err("Opening file '../orbital_periods.csv' failed");

    output << "object,orbital_period[days],orbital_period[years],radius[km]\n";
    for (const tuple_sdd& element:orb_periods)
        output << std::get<0>(element) << ',' << std::get<1>(element) << ',' << std::get<1>(element) / 365.25 << ','
               << std::get<2>(element) << std::endl;

    output.close();

    return ;
}

void write_promising_objects(const vec_vec_smrtptr<Body>& objects, double min_tidal_value)
{
    std::ofstream output("../tidal_promising_objects.csv");
    if (!output.is_open())
        throw Simple_Err("Opening file '../orbital_periods.csv' failed");

    output << "objects,radius[km],mass[kg]\n";
    for (const vec_smrtptr<Body>& list:objects)
        for(smrtptr<Body> object:list)
            if (object->get_calc_tidal())
                if (object->get_relevant_objects()->get_max_value() > min_tidal_value)
                    output << object->get_primary_designation() + std::string("_") + object->get_name() << ","
                           << object->get_radius() << "," << object->get_mass(M_type::TRUE_MEAN) << std::endl;

    return ;
}


#endif // SIMULATION_H
