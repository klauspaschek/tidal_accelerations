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
#include <array>
#include <sstream>

#include <Eigen/Dense>

#include "csvdata.h"
#include "body.h"
#include "moon.h"
#include "sun.h"
#include "relevantobjects.h"


// start value for time in julian date
double Body::s_time(2451545.);

// information about the current time step for the output
struct IterationInfo
{
    double time_step;    // (days)
    bool write;          // write tidal accelerations of this time step to the output files
    bool first_step;     // no change of the tidal accelerations defined in the first time step
};

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
// an empty entry or a period of zero means no precession
double period_to_d(const std::string& input_str, const Tu& given_day_year_century, const Tu& desired_per_day_year_century)
{
    if (input_str == "" || std::stod(input_str) == 0.)
        return 0.;

    // length of given and desired time unit in days
    std::map<Tu, double> days{ {Tu::PER_DAY, 1.}, {Tu::PER_YEAR, 365.25}, {Tu::PER_CENTURY, 36525.} };

    return 2. * M_PI * days[desired_per_day_year_century] / (std::stod(input_str) * days[given_day_year_century]);
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
std::vector<std::vector<Eigen::Vector3d> > calc_tidal_acc_relevant(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const std::vector<std::vector<Eigen::Vector3d> >& last_iteration, double min_tidal_value, double max_tidal_value, const IterationInfo& info)
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

                    std::ostringstream output;
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
                           << '\n';

                    if (info.write)
                    {
                        std::ofstream output_file(filename, std::fstream::app);
                        if (!output_file.is_open())
                            throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));
                        output_file << output.str();
                    }

                    // A_total * Delta A_total / Delta t for mean, minimal, and maximal mass
                    if (!info.first_step)
                        object->get_relevant_objects()->update_max_a_da_dt({ {total.norm() * total_change.norm() / info.time_step,
                                                                              total_min.norm() * total_change_min.norm() / info.time_step,
                                                                              total_max.norm() * total_change_max.norm() / info.time_step} },
                                                                            Body::s_time);

                    ++i;
                }
            }
        }
    }

    return new_iteration;
}

// write the maximal values of A_total * Delta A_total / Delta t over all time steps for each object
void write_maxima(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, double min_tidal_value, double max_tidal_value)
{
    std::string filename(directory + std::string("/") + filename_prefix + std::string("_maxima.csv"));
    std::ofstream output(filename);
    if (!output.is_open())
        throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

    output.precision(10);
    output << "object,radius[km],a_da_dt_max,min_mass,max_mass,julian_date_of_max\n";
    for (const vec_smrtptr<Body>& list:objects)
        for (smrtptr<Body> object:list)
            if (object->get_calc_tidal() &&
                object->get_relevant_objects()->get_max_value() >= min_tidal_value &&
                object->get_relevant_objects()->get_max_value() <  max_tidal_value)
            {
                std::array<double, 3> maxima(object->get_relevant_objects()->get_max_a_da_dt());
                output << object->get_primary_designation() << '_' << object->get_name() << ',' << object->get_radius() << ','
                       << maxima[0] << ',' << maxima[1] << ',' << maxima[2] << ','
                       << object->get_relevant_objects()->get_time_max_a_da_dt() << '\n';
            }

    return ;
}

// iterate time and calculate tidal accelerations of the relevant objects in each time step
// only every 'output_every'-th time step is written to the output files, the maxima of
// A_total * Delta A_total / Delta t are determined from all time steps
void time_iteration(const vec_vec_smrtptr<Body>& objects, const std::string& directory, const std::string& filename_prefix, const double& time_step, const unsigned int& iterations, double min_tidal_value = 0., double max_tidal_value = 1., unsigned int output_every = 1)
{
    prep_tidal_acc_relevant(objects, directory, filename_prefix, min_tidal_value, max_tidal_value);

    std::vector<std::vector<Eigen::Vector3d> > last_iteration;
    for (const vec_smrtptr<Body>& list:objects)
    {
        for (smrtptr<Body> object:list)
        {
            if (object->get_calc_tidal())
            {
                if (object->get_relevant_objects()->get_max_value() >= min_tidal_value &&
                    object->get_relevant_objects()->get_max_value() <  max_tidal_value)
                {
                    std::vector<Eigen::Vector3d> last_iteration_sub(3, Eigen::Vector3d::Zero());
                    last_iteration.push_back(last_iteration_sub);
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

        IterationInfo info{time_step, (i + 1) % output_every == 0, i == 0};
        last_iteration = calc_tidal_acc_relevant(objects, directory, filename_prefix, last_iteration, min_tidal_value, max_tidal_value, info);

        int prev_count(static_cast<int>(static_cast<double>(i) / iterations * 100.)),
            current_count(static_cast<int>(static_cast<double>(i + 1) / iterations * 100.));
        if (prev_count < current_count)
            std::cout << current_count << "%\r" << std::flush;
    }

    write_maxima(objects, directory, filename_prefix, min_tidal_value, max_tidal_value);

    return ;
}

#endif // SIMULATION_H
