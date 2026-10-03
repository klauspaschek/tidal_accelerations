#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "simulation.h"


// usage: ./planetary_system <data file> [--time-step <days>] [--duration <days>] [--simplified] [--output <directory>]
//
// simulation of the tidal accelerations in an extrasolar planetary system with one star, given in a data file
// (see 'data/README.md'), e.g., ./planetary_system data/trappist-1.csv
//   --time-step   time step in days (default: 1/20 of the shortest orbital period)
//   --duration    simulated time span in days (default: 10 times the longest orbital period)
//   --simplified  only use semi-major axis a and mean motion n of the orbits (e, i, omega, Omega, M_0 set to zero)
//   --output      directory for the results (default: 'results/<name of data file>'), has to exist
// the simulation starts at epoch J2000.0, to which the mean anomalies M_0 refer

const double mass_earth(5.9722e24);         // (kg)
const double mass_sun(1.9884e30);           // (kg)
const double radius_earth(6378.137);        // (km), equatorial radius
const double radius_sun(695700.);           // (km)
const double gravitational_constant(6.67408e-11);
const double astronomical_unit(149597870700.);

// returns value of column, 'default_value' if the entry is empty
double value_or(const CSVData& data, unsigned int row, const std::string& column, double default_value)
{
    std::string entry(data(row, column));
    return (entry == "") ? default_value : std::stod(entry);
}

// rounds down to three significant digits, so that it can be given exactly in the names of the output files
double round_down_significant(double value)
{
    double magnitude(std::pow(10., std::floor(std::log10(value)) - 2.));
    return std::floor(value / magnitude) * magnitude;
}

// e.g., 0.0755 -> "0_0755"
std::string number_for_filename(double value)
{
    std::ostringstream stream;
    stream << std::setprecision(10) << value;
    std::string result(stream.str());
    std::replace(result.begin(), result.end(), '.', '_');
    return result;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
            throw Simple_Err("Usage: ./planetary_system <data file> [--time-step <days>] [--duration <days>] [--simplified] [--output <directory>]");

        std::string data_file(argv[1]);
        double time_step(0.), duration(0.);
        bool simplified(false);
        std::string system_name(data_file.substr(data_file.find_last_of('/') + 1));
        system_name = system_name.substr(0, system_name.find_last_of('.'));
        std::string output_directory("results/" + system_name);
        for (int k(2); k < argc; ++k)
        {
            std::string option(argv[k]);
            if (option == "--time-step" && k + 1 < argc)
                time_step = std::stod(argv[++k]);
            else if (option == "--duration" && k + 1 < argc)
                duration = std::stod(argv[++k]);
            else if (option == "--simplified")
                simplified = true;
            else if (option == "--output" && k + 1 < argc)
                output_directory = argv[++k];
            else
                throw Simple_Err(std::string("Unknown option '") + option + std::string("'"));
        }

        std::cout << "Start creating objects...";

        // header line followed by one line giving the units
        CSVData system_csv(data_file, 1);

        // find star
        int star_row(-1);
        for (unsigned int i(0); i < system_csv.get_size_data(); ++i)
            if (system_csv(i, "type") == "star")
            {
                if (star_row >= 0)
                    throw Simple_Err("Only one star per system is supported");
                star_row = i;
            }
        if (star_row < 0)
            throw Simple_Err(std::string("No star given in '") + data_file + std::string("'"));

        double star_mass(std::stod(system_csv(star_row, "mass")) * mass_sun);
        vec_smrtptr<Body> suns;
        suns.push_back(smrtptr<Body>(new
            Sun(system_csv(star_row, "name"), star_mass, value_or(system_csv, star_row, "radius", 0.) * radius_sun)));

        // create planets
        vec_smrtptr<Body> planets;
        double period_min(0.), period_max(0.);
        for (unsigned int i(0); i < system_csv.get_size_data(); ++i)
        {
            if (system_csv(i, "type") == "star")
                continue;
            if (system_csv(i, "type") != "planet")
                throw Simple_Err(std::string("Unknown type '") + system_csv(i, "type") + std::string("' in '") + data_file + std::string("'"));

            std::string name(system_csv(i, "name"));
            if (system_csv(i, "mass") == "" || system_csv(i, "radius") == "")
                throw Simple_Err(std::string("Mass and radius are needed for planet '") + name + std::string("'"));
            double mass(std::stod(system_csv(i, "mass")) * mass_earth);
            double mass_min(value_or(system_csv, i, "mass_min", mass / mass_earth) * mass_earth);
            double mass_max(value_or(system_csv, i, "mass_max", mass / mass_earth) * mass_earth);

            // orbital period (days) and semi-major axis (au), one of them can be calculated from the other
            double period(value_or(system_csv, i, "period", 0.)), a(value_or(system_csv, i, "a", 0.));
            double gm(gravitational_constant * (star_mass + mass));
            if (period == 0. && a == 0.)
                throw Simple_Err(std::string("Orbital period or semi-major axis is needed for planet '") + name + std::string("'"));
            else if (period == 0.)
                period = 2. * M_PI * std::sqrt(std::pow(a * astronomical_unit, 3) / gm) / 86400.;
            else if (a == 0.)
                a = std::cbrt(gm * std::pow(period * 86400. / (2. * M_PI), 2)) / astronomical_unit;

            period_min = (period_min == 0.) ? period : std::min(period_min, period);
            period_max = std::max(period_max, period);

            planets.push_back(smrtptr<Body>(new
                Body(name, std::to_string(planets.size() + 1),
                     true, Tu::PER_DAY,
                     mass, mass_min, mass_max,
                     std::stod(system_csv(i, "radius")) * radius_earth,
                     deg_to_rad(value_or(system_csv, i, "Omega", 0.)), 0.,
                     deg_to_rad(value_or(system_csv, i, "omega", 0.)), 0.,
                     deg_to_rad(value_or(system_csv, i, "M0", 0.)),    2. * M_PI / period,
                     a,                                                0.,
                     value_or(system_csv, i, "e", 0.),                 0.,
                     deg_to_rad(value_or(system_csv, i, "i", 0.)),     0.)));
        }
        if (planets.empty())
            throw Simple_Err(std::string("No planets given in '") + data_file + std::string("'"));

        std::cout << "finished\n";

        // collect all objects to deliver them to time_iteration()
        vec_vec_smrtptr<Body> objects;
        objects.push_back(suns);
        objects.push_back(planets);

        if (simplified)
            for (const vec_smrtptr<Body>& list:objects)
                for (smrtptr<Body> object:list)
                    object->simplify();

        // time step resolving the shortest orbital period (18 degrees per time step) and time span covering
        // 10 orbits of the outermost planet, if not given
        if (time_step == 0.)
            time_step = round_down_significant(period_min / 20.);
        if (duration == 0.)
            duration = 10. * period_max;
        unsigned int iterations(static_cast<unsigned int>(std::ceil(duration / time_step - 1.e-9)));
        // write at most about 2400 time steps to the output files
        unsigned int output_every(std::max(1u, iterations / 2400));

        std::cout << "System '" << system_name << "': " << planets.size() << " planets, time step " << time_step
                  << " days, " << iterations << " iterations (" << iterations * time_step << " days), every "
                  << output_every << ". time step written" << (simplified ? ", simplified orbital elements" : "") << "\n";

        // start at epoch J2000.0 (default value of Body::s_time)

        vec_smrtptr<RelevantObjects> relevant_objects;

        // all other objects of the system are written out individually
        std::cout << "Start search for relevant objects... (1/2)\nProgress:\n";
        prep_relevant_objects(objects, relevant_objects, time_step, iterations, planets.size(), 0);
        std::cout << "...finished\n";

        std::cout << "Start time iteration for calculation of tidal accelerations... (2/2)\nProgress:\n";
        time_iteration(objects, output_directory, std::string("delta_t_") + number_for_filename(time_step) + std::string("d"),
                       time_step, iterations, 0., 1., output_every);
        std::cout << "...finished\n";

        std::cout << "\n*** Done! ***\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
