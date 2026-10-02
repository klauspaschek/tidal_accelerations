#include <string>
#include <iostream>

#include "simulation.h"


// usage: ./solar_system [--simplified]
// run from the directory 'solar_system', results are written to 'results/full' or 'results/simplified'
// --simplified: only use semi-major axis a and mean motion n of the orbits (e, Omega, i, w, M_0 set to zero)
int main(int argc, char* argv[])
{
    bool simplified(argc > 1 && std::string(argv[1]) == "--simplified");

    std::string directory(".");
    // sub directories have to start with "/"
    std::string sub_directory_data("/data");
    std::string sub_directory_tidal_acc(simplified ? "/results/simplified" : "/results/full");

    try
    {
        // only objects whose maximal tidal acceleration of a single perturber lies in
        // [min_tidal_value, max_tidal_value) (in m/s^2) are written out, [0, 1) selects all
        double min_tidal_value(0.);
        double max_tidal_value(1.);

        std::cout << (simplified ? "Simplified orbital elements (only a and n)\n" : "Full orbital elements\n");

        std::cout << "Start creating objects...";

        // create sun
        vec_smrtptr<Body> suns;
        suns.push_back(smrtptr<Body>(new
            Sun("Sun", 1.9884e30, 696342.)));

        // create asteroids
        CSVData asteroids_csv(directory + sub_directory_data + std::string("/asteroids_r_greater_75km_or_GM_given_with_mass_with_guessed_mass.csv"), 0);
        vec_smrtptr<Body> asteroids;
        for (unsigned int i(0); i < asteroids_csv.get_size_data(); ++i)
            if (!(asteroids_csv(i, "pdes") == "15789"  ||
                  asteroids_csv(i, "pdes") == "20000"  ||
                  asteroids_csv(i, "pdes") == "42355"  ||
                  asteroids_csv(i, "pdes") == "127546"))
                asteroids.push_back(smrtptr<Body>(new
                    Body(asteroids_csv(i, "name"), std::string("A_") + asteroids_csv(i, "pdes"),
                         (std::stod(asteroids_csv(i, "diameter")) >= 150.) ? true : false, Tu::PER_DAY,
                         std::stod(asteroids_csv(i, "M", "M_mean_guess")), std::stod(asteroids_csv.fancy_access(i, "M_min_guess", "M")),
                         std::stod(asteroids_csv.fancy_access(i, "M_max_guess", "M")),
                         std::stod(asteroids_csv(i, "diameter")) / 2.,
                         deg_to_rad(std::stod(asteroids_csv(i, "om"))),                                         0.,
                         deg_to_rad(std::stod(asteroids_csv(i, "w" ))),                                         0.,
                         deg_to_rad(std::stod(asteroids_csv(i, "ma"))), deg_to_rad(std::stod(asteroids_csv(i, "n"))),
                                    std::stod(asteroids_csv(i, "a" )) ,                                         0.,
                                    std::stod(asteroids_csv(i, "e" )) ,                                         0.,
                         deg_to_rad(std::stod(asteroids_csv(i, "i" ))),                                         0.,
                         std::stod(asteroids_csv(i, "epoch")))));

        // create planets
        CSVData planets_csv(directory + sub_directory_data + std::string("/planets_orbital_elements_physical.csv"), 1);
        vec_smrtptr<Body> planets;
        for (unsigned int i(0); i < planets_csv.get_size_data(); ++i)
            planets.push_back(smrtptr<Body>(new
                Body(planets_csv(i, "Planet"), planets_csv(i, "pdes"),
                     (planets_csv(i, "Planet") == "Mercury" ||
                      planets_csv(i, "Planet") == "Venus"   ||
                      planets_csv(i, "Planet") == "Earth"   ||
                      planets_csv(i, "Planet") == "Mars"    ||
                      planets_csv(i, "Planet") == "Pluto") ? true : false, Tu::PER_CENTURY,
                     std::stod(planets_csv(i, "Mass")), std::stod(planets_csv(i, "Mass")) - std::stod(planets_csv(i, "Mass_sigma")),
                     std::stod(planets_csv(i, "Mass")) + std::stod(planets_csv(i, "Mass_sigma")),
                     std::stod(planets_csv(i, "Mean_Radius")),
                     deg_to_rad(std::stod(planets_csv(i, "long.node."))), deg_to_rad(std::stod(planets_csv(i, "long.node._dot"))),
                     deg_to_rad(std::stod(planets_csv(i, "long.peri.")) -
                                std::stod(planets_csv(i, "long.node."))), deg_to_rad(std::stod(planets_csv(i, "long.peri._dot"))),
                     deg_to_rad(std::stod(planets_csv(i, "L")) -
                                std::stod(planets_csv(i, "long.peri."))), deg_to_rad(std::stod(planets_csv(i, "L_dot")))         ,
                                std::stod(planets_csv(i, "a"))          ,            std::stod(planets_csv(i, "a_dot"))          ,
                                std::stod(planets_csv(i, "e"))          ,            std::stod(planets_csv(i, "e_dot"))          ,
                     deg_to_rad(std::stod(planets_csv(i, "I")))         , deg_to_rad(std::stod(planets_csv(i, "I_dot"))))));

        // create TNOs
        CSVData tnos_csv(directory + sub_directory_data + std::string("/TNOs_supplemented_with_guessed_mass.csv"), 0);
        vec_smrtptr<Body> tnos;
        for (unsigned int i(0); i < tnos_csv.get_size_data(); ++i)
            if (tnos_csv(i, "name") != "Pluto")
                tnos.push_back(smrtptr<Body>(new
                    Body(tnos_csv(i, "name"), std::string("TNO_") + tnos_csv(i, "pdes").substr(0, tnos_csv(i, "pdes").find(".")),
                         (std::stod(tnos_csv(i, "diameter")) >= 150.) ? true : false, Tu::PER_DAY,
                         std::stod(tnos_csv(i, "mass", "mass_mean_guess")), std::stod(tnos_csv.fancy_access(i, "mass_min_guess", "mass")),
                         std::stod(tnos_csv.fancy_access(i, "mass_max_guess", "mass")),
                         std::stod(tnos_csv(i, "diameter")) / 2.,
                         deg_to_rad(std::stod(tnos_csv(i, "om"))),                                      0.,
                         deg_to_rad(std::stod(tnos_csv(i, "w" ))),                                      0.,
                         deg_to_rad(std::stod(tnos_csv(i, "ma"))), deg_to_rad(std::stod(tnos_csv(i, "n"))),
                                    std::stod(tnos_csv(i, "a" )) ,                                      0.,
                                    std::stod(tnos_csv(i, "e" )) ,                                      0.,
                         deg_to_rad(std::stod(tnos_csv(i, "i" ))),                                      0.,
                         std::stod(tnos_csv(i, "epoch")))));

        // create moons
        // moons of planets
        CSVData moons_csv(directory + sub_directory_data + std::string("/moons_orbital_elements_physical.csv"), 1);
        vec_smrtptr<Body> moons;
        for (unsigned int i(0); i < moons_csv.get_size_data(); ++i)
            if (std::stod(moons_csv(i, "Mass")) != 0.)
                moons.push_back(smrtptr<Body>(new
                    Moon(moons_csv(i, "Moon"), moons_csv(i, "pdes"),
                         false, Tu::PER_DAY,
                         std::stod(moons_csv(i, "Mass")), std::stod(moons_csv(i, "Mass")) - std::stod(moons_csv(i, "Mass_sigma")),
                         std::stod(moons_csv(i, "Mass")) + std::stod(moons_csv(i, "Mass_sigma")),
                         std::stod(moons_csv(i, "Mean_radius")),
                         deg_to_rad(std::stod(moons_csv(i, "node"))), period_to_d(moons_csv(i, "Pnode"), Tu::PER_YEAR, Tu::PER_DAY),
                         deg_to_rad(std::stod(moons_csv(i, "w")))   , period_to_d(moons_csv(i, "Pw"), Tu::PER_YEAR, Tu::PER_DAY)   ,
                         deg_to_rad(std::stod(moons_csv(i, "M")))   , deg_to_rad(std::stod(moons_csv(i, "n")))                     ,
                         std::stod(moons_csv(i, "a")) / 149597870.7 , /* <- conversion of km to au */                            0.,
                                    std::stod(moons_csv(i, "e"))    ,                                                            0.,
                         deg_to_rad(std::stod(moons_csv(i, "i")))   ,                                                            0.,
                         moons_csv(i, "Corresponding_Planet"), planets,
                         std::stod(moons_csv(i, "epoch")),
                         str_to_rp(moons_csv(i, "referece_plane"), moons_csv(i, "Moon")),
                         deg_to_rad(str_to_d_zero(moons_csv(i, "R.A."))), deg_to_rad(str_to_d_zero(moons_csv(i, "Dec."))))));
        // moons of TNOs
        CSVData moons_tnos_csv(directory + sub_directory_data + std::string("/moons_TNOs_v2.csv"), 1);
        for (unsigned int i(0); i < moons_tnos_csv.get_size_data(); ++i)
            moons.push_back(smrtptr<Body>(new
                Moon(moons_tnos_csv(i, "Moon"), moons_tnos_csv(i, "pdes"),
                     true, Tu::PER_DAY,
                     std::stod(moons_tnos_csv(i, "Mass")),
                     std::stod(moons_tnos_csv(i, "Mass")) - std::stod(moons_tnos_csv(i, "Mass_sigma")),
                     std::stod(moons_tnos_csv(i, "Mass")) + std::stod(moons_tnos_csv(i, "Mass_sigma")),
                     std::stod(moons_tnos_csv(i, "Mean_radius")),
                     deg_to_rad(std::stod(moons_tnos_csv(i, "node"))), period_to_d(moons_tnos_csv(i, "Pnode"), Tu::PER_YEAR, Tu::PER_DAY),
                     deg_to_rad(std::stod(moons_tnos_csv(i, "w")))   , period_to_d(moons_tnos_csv(i, "Pw"), Tu::PER_YEAR, Tu::PER_DAY)   ,
                     deg_to_rad(std::stod(moons_tnos_csv(i, "M")))   , deg_to_rad(std::stod(moons_tnos_csv(i, "n")))                     ,
                     std::stod(moons_tnos_csv(i, "a")) / 149597870.7 ,                                                                 0.,
                                std::stod(moons_tnos_csv(i, "e"))    ,                                                                 0.,
                     deg_to_rad(std::stod(moons_tnos_csv(i, "i")))   ,                                                                 0.,
                     moons_tnos_csv(i, "Corresponding_Planet"), tnos,
                     std::stod(moons_tnos_csv(i, "epoch")),
                     str_to_rp(moons_tnos_csv(i, "referece_plane"), moons_tnos_csv(i, "Moon")),
                     deg_to_rad(str_to_d_zero(moons_tnos_csv(i, "R.A."))), deg_to_rad(str_to_d_zero(moons_tnos_csv(i, "Dec."))))));

        // create comets
        CSVData comets_csv(directory + sub_directory_data + std::string("/comets_with_mass_with_guessed_mass.csv"), 0);
        vec_smrtptr<Body> comets;
        for (unsigned int i(0); i < comets_csv.get_size_data(); ++i)
            if ((comets_csv(i, "M") != "") || (comets_csv(i, "M_mean_guess") != ""))
                comets.push_back(smrtptr<Body>(new
                    Body(comets_csv(i, "name"), std::string("C_") + comets_csv(i, "pdes"),
                         true, Tu::PER_DAY,
                         std::stod(comets_csv(i, "M", "M_mean_guess")), std::stod(comets_csv.fancy_access(i, "M_min_guess", "M")),
                         std::stod(comets_csv.fancy_access(i, "M_max_guess", "M")),
                         std::stod(comets_csv(i, "diameter")) / 2.,
                         deg_to_rad(std::stod(comets_csv(i, "om"))),                                         0.,
                         deg_to_rad(std::stod(comets_csv(i, "w" ))),                                         0.,
                         deg_to_rad(std::stod(comets_csv(i, "ma"))), deg_to_rad(std::stod(comets_csv(i, "n"))) ,
                                    std::stod(comets_csv(i, "a" )) ,                                         0.,
                                    std::stod(comets_csv(i, "e" )) ,                                         0.,
                         deg_to_rad(std::stod(comets_csv(i, "i" ))),                                         0.,
                         std::stod(comets_csv(i, "epoch")))));

        std::cout << "finished\n";

        // collect all objects to deliver them to time_iteration()
        vec_vec_smrtptr<Body> objects;
        objects.push_back(suns);
        objects.push_back(asteroids);
        objects.push_back(planets);
        objects.push_back(comets);
        objects.push_back(tnos);
        objects.push_back(moons);

        if (simplified)
            for (const vec_smrtptr<Body>& list:objects)
                for (smrtptr<Body> object:list)
                    object->simplify();

        Body::s_time = 2433283.;    // 1950 Jan. 01. 12:00 UT

        vec_smrtptr<RelevantObjects> relevant_objects;

        double time_step(90.);
        unsigned int iterations(2400);

       std::cout << "Start search for relevant objects... (1/4)\nProgress:\n";
       prep_relevant_objects(objects, relevant_objects, time_step, iterations, 10, 10);
       std::cout << "...finished\n";

       std::cout << "Start time iteration for calculation of tidal accelerations... (2/4)\nProgress:\n";
       time_iteration(objects, directory + sub_directory_tidal_acc, "delta_t_90d", time_step, iterations,
                      prep_tidal_acc_relevant, calc_tidal_acc_relevant, min_tidal_value, max_tidal_value, true);
       std::cout << "...finished\n";


        // set calculation for tidal acceleration only for moons to prepare new time iteration
        for (const smrtptr<Body>& asteroid:asteroids)
            asteroid->set_calc_tidal(false);
        for (const smrtptr<Body>& moon:moons)
            moon->set_calc_tidal((moon->get_radius() >= 75) ? true : false);
        for (const smrtptr<Body>& comet:comets)
            comet->set_calc_tidal(false);
        for (const smrtptr<Body>& tno:tnos)
            tno->set_calc_tidal((tno->get_name() == "Eris" || tno->get_name() == "Haumea") ? true : false);

        relevant_objects.clear();

        Body::s_time = 2451304.;    // 1999 May 05. 12:00 UT

        time_step = 0.25;

        std::cout << "Start search for relevant objects... (3/4)\nProgress:\n";
        prep_relevant_objects(objects, relevant_objects, time_step, iterations, 10, 10);
        std::cout << "...finished\n";

//        write_promising_objects(objects, min_tidal_value);

        std::cout << "Start time iteration for calculation of tidal accelerations... (4/4)\nProgress:\n";
        time_iteration(objects, directory + sub_directory_tidal_acc, "delta_t_0_25d", time_step, iterations,
                       prep_tidal_acc_relevant, calc_tidal_acc_relevant, min_tidal_value, max_tidal_value, true);
        std::cout << "...finished\n";

//        std::cout << "Start calculating and saving orbital periods of objects...";
//        write_orb_periods(objects);
//        std::cout << "finished\n";

//        std::cout << "Start time iteration for calculation of positions...\nProgress:\n";
//        time_iteration(objects, "/home/klaus/Documents/Studium/Sem7/Bachelor/positions", "moons_new",
//                       0.5, 1000, prep_positions, write_positions);

        std::cout << "\n*** Done! ***\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
