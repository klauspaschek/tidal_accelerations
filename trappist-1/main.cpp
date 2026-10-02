#include <string>
#include <iostream>

#include "simulation.h"


// usage: ./trappist-1
// run from the directory 'trappist-1', results are written to 'results'
//
// setup of the extrasolar planetary system TRAPPIST-1 as used in Paschek et al. (2021), Appl. Sci. 11, 8624:
// star from Van Grootel et al. (2018), planets from Grimm et al. (2018), Delrez et al. (2018), and Gillon et al. (2017),
// see 'data/README.md' - the longitude of the ascending node, the argument of periapsis, and the mean anomaly
// at epoch are set to zero, as no data is available for them
int main()
{
    std::string directory(".");
    // sub directories have to start with "/"
    std::string sub_directory_data("/data");
    std::string sub_directory_tidal_acc("/results");

    try
    {
        // only objects whose maximal tidal acceleration of a single perturber lies in
        // [min_tidal_value, max_tidal_value) (in m/s^2) are written out, [0, 1) selects all
        double min_tidal_value(0.);
        double max_tidal_value(1.);

        std::cout << "Start creating objects...";

        // create star (0.089 solar masses, 0.121 solar radii)
        vec_smrtptr<Body> suns;
        suns.push_back(smrtptr<Body>(new
            Sun("Trappist1", 0.089*1.9884e30, 0.121*696342.)));

        // create planets
        // angles are given in radians and the mean motion n in radians/day
        CSVData planets_csv(directory + sub_directory_data + std::string("/trappist-1_planets.csv"), 0);
        vec_smrtptr<Body> planets;
        for (unsigned int i(0); i < planets_csv.get_size_data(); ++i)
            planets.push_back(smrtptr<Body>(new
                Body(planets_csv(i, "Planet"), planets_csv(i, "pdes"),
                     true, Tu::PER_DAY,
                     std::stod(planets_csv(i, "mean_mass")), std::stod(planets_csv(i, "min_mass")),
                     std::stod(planets_csv(i, "max_mass")),
                     std::stod(planets_csv(i, "radius")),
                     std::stod(planets_csv(i, "O")),         0.                              ,
                     std::stod(planets_csv(i, "w")),         0.                              ,
                     std::stod(planets_csv(i, "M")),         (std::stod(planets_csv(i, "n"))),
                     std::stod(planets_csv(i, "a")),         0.                              ,
                     std::stod(planets_csv(i, "e")),         0.                              ,
                     std::stod(planets_csv(i, "i")),         0.
                               )));

        std::cout << "finished\n";

        // collect all objects to deliver them to time_iteration()
        vec_vec_smrtptr<Body> objects;
        objects.push_back(suns);
        objects.push_back(planets);

        // start at epoch J2000.0 (default value of Body::s_time)

        vec_smrtptr<RelevantObjects> relevant_objects;

        double time_step(0.05);
        unsigned int iterations(1800);

        std::cout << "Start search for relevant objects... (1/2)\nProgress:\n";
        prep_relevant_objects(objects, relevant_objects, time_step, iterations, 8, 0);
        std::cout << "...finished\n";

        std::cout << "Start time iteration for calculation of tidal accelerations... (2/2)\nProgress:\n";
        time_iteration(objects, directory + sub_directory_tidal_acc, "delta_t_0_05d", time_step, iterations,
                       prep_tidal_acc_relevant, calc_tidal_acc_relevant, min_tidal_value, max_tidal_value, true);
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
