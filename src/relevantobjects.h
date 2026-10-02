#ifndef RELEVANTOBJECTS_H
#define RELEVANTOBJECTS_H

#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <utility>
#include <algorithm>
#include <cmath>

#include <Eigen/Dense>

#include "body.h"


using pair_dVec3d       = std::pair<double, Eigen::Vector3d>;
using pair_smrtptrBodyd = std::pair<smrtptr<Body>, double>;

class RelevantObjects
{
  private:
    smrtptr<Body> m_object;

    std::map<smrtptr<Body>, double> m_max_objects;
    unsigned int m_n_max_tidal;
    std::map<smrtptr<Body>, pair_dVec3d> m_var_objects;
    unsigned int m_n_var_tidal;

    bool m_first_update;

    mutable vec_smrtptr<Body> m_relevant_objects;
    mutable bool m_relevant_objects_calculated;

  public:
    RelevantObjects(smrtptr<Body> object, unsigned int n_max_tidal, unsigned int n_var_tidal/*, unsigned int iterations*/)
    : m_object(object), m_n_max_tidal(n_max_tidal), m_n_var_tidal(n_var_tidal),
      m_first_update(true), m_relevant_objects_calculated(false)
    {}

    void update(const vec_vec_smrtptr<Body>& other_objects)
    {
        for (const vec_smrtptr<Body>& list:other_objects)
        {
            for (smrtptr<Body> other_object:list)
            {
                if ((*m_object) != (*other_object))
                {
                    Eigen::Vector3d tidal_acc_vec(m_object->tidal_acc(*other_object, M_type::TRUE_MEAN));
                    double norm(tidal_acc_vec.norm());

                    // update
                    if (m_max_objects.find(other_object) != m_max_objects.end())
                    {
                        if (norm > m_max_objects[other_object])
                            m_max_objects[other_object] = norm;
                    }
                    else if (m_max_objects.size() < m_n_max_tidal)
                    {
                        m_max_objects[other_object] = norm;
                    }
                    else
                    {
                        // Version 1.0 reproduces the selection of the original code used for the paper, which erased
                        // map elements while iterating over the map: if the erased element was the current one, the
                        // iterator was left dangling (undefined behavior, crashes with current compilers/glibc). With
                        // the allocator of 2018, the freed node was reused for the newly inserted perturber, so the
                        // iteration continued from there. This is emulated here with well-defined code. Note that the
                        // loop does not stop after replacing the weakest entry, so the map can shrink below
                        // n_max_tidal and is refilled by the following perturbers (see CHANGELOG.md).
                        std::map<smrtptr<Body>, double>::const_iterator it(m_max_objects.begin());
                        while (it != m_max_objects.end())
                        {
                            if (norm > it->second)
                            {
                                double min(it->second);
                                smrtptr<Body> min_object(it->first);
                                for (std::map<smrtptr<Body>, double>::const_iterator it_min(m_max_objects.begin()); it_min!=m_max_objects.end(); ++it_min)
                                    if (it_min->second < min)
                                    {
                                        min = it_min->second;
                                        min_object = it_min->first;
                                    }
                                smrtptr<Body> current_object(it->first);
                                m_max_objects.erase(min_object);
                                m_max_objects[other_object] = norm;
                                it = m_max_objects.find((current_object == min_object) ? other_object : current_object);
                            }
                            ++it;
                        }
                    }

                    if (m_first_update)
                    {
                        m_var_objects[other_object] = pair_dVec3d(0., tidal_acc_vec);
                    }
                    else
                    {
                        m_var_objects[other_object].first += (tidal_acc_vec - m_var_objects[other_object].second).norm() / tidal_acc_vec.norm();
                        m_var_objects[other_object].second = tidal_acc_vec;
                    }
                }
            }
        }
        m_first_update = false;
        m_relevant_objects_calculated = false;

        return ;
    }

    vec_smrtptr<Body> get_calc_relevant_objects() const
    {
        if (!m_relevant_objects_calculated)
        {
            m_relevant_objects.clear();

            std::vector<pair_smrtptrBodyd> max_relevant_objects;
            for (std::map<smrtptr<Body>, double>::const_iterator it(m_max_objects.begin()); it!=m_max_objects.end(); ++it)
                max_relevant_objects.push_back(*it);
            std::sort(max_relevant_objects.begin(), max_relevant_objects.end(), [](const pair_smrtptrBodyd& a, const pair_smrtptrBodyd& b)
                                                                                {
                                                                                    return a.second > b.second;
                                                                                });

            for (const pair_smrtptrBodyd& element:max_relevant_objects)
                m_relevant_objects.push_back(element.first);

            std::vector<pair_smrtptrBodyd> var_relevant_objects;
            for (std::map<smrtptr<Body>, pair_dVec3d>::const_iterator it(m_var_objects.begin()); it!=m_var_objects.end(); ++it)
                var_relevant_objects.push_back(pair_smrtptrBodyd(it->first, it->second.first/* / m_iterations*/));
            std::sort(var_relevant_objects.begin(), var_relevant_objects.end(), [](const pair_smrtptrBodyd& a, const pair_smrtptrBodyd& b)
                                                                                {
                                                                                    // undefined variations (NaN) last, sorting with NaN is undefined behavior
                                                                                    if (std::isnan(a.second) || std::isnan(b.second))
                                                                                        return !std::isnan(a.second) && std::isnan(b.second);
                                                                                    return a.second > b.second;
                                                                                });

            unsigned int i(0);
            for (unsigned int j(0); i < m_n_var_tidal; ++j)
            {
                smrtptr<Body> current_object(var_relevant_objects.at(j).first);
                if (std::none_of(m_relevant_objects.begin(), m_relevant_objects.end(), [current_object](smrtptr<Body> element)
                                                                                       {
                                                                                           return element == current_object;
                                                                                       }))
                {
                    m_relevant_objects.push_back(current_object);
                    ++i;
                }
            }
            m_relevant_objects_calculated = true;
        }

        return m_relevant_objects;
    }

    double get_max_value() const
    {
        double max_value(0.);
        for (std::map<smrtptr<Body>, double>::const_iterator it(m_max_objects.begin()); it!=m_max_objects.end(); ++it)
        {
            double current_value(it->second);
            if (current_value > max_value)
                max_value = current_value;
        }

        return max_value;
    }
};

#endif // RELEVANTOBJECTS_H
