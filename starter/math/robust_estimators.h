/*
 Copyright 2005, 2006 Computer Vision Lab,
 Ecole Polytechnique Federale de Lausanne (EPFL), Switzerland.
 Modified by Damian Stewart <damian@frey.co.nz> 2009-2010;
 modifications Copyright 2009, 2010 Damian Stewart <damian@frey.co.nz>.

 Distributed under the terms of the GNU General Public License v3.
 
 This file is part of The Artvertiser.
 
 The Artvertiser is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The Artvertiser is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with The Artvertiser.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ROBUST_ESTIMATORS_H
#define ROBUST_ESTIMATORS_H

#include <math.h>

//! \ingroup starter 
//@{

double rho_tukey_without_sqrt(const double x2, const double c);
double rho_tukey_without_sqrt_derivative(const double x2, const double c);

double rho_huber_without_sqrt(const double x2, const double c);
double rho_huber_without_sqrt_derivative(const double x2, const double c);


inline double rho_tukey_without_sqrt(const double x2, const double c)
{
  double c2 = c * c;

  if (x2 > c2)
    return c2 / 6;
  else
  {
    double t = 1. - x2 / c2;

    return c2 / 6 * (1. - t * t * t);
  }
}

inline double rho_tukey_without_sqrt_derivative(const double x2, const double c)
{
  double c2 = c * c;

  if (x2 > c2)
    return 0;
  else
  {
    double t = 1. - x2 / c2;

    return t * t / 2.;
  }
}

inline double rho_huber_without_sqrt(const double x2, const double c)
{
  double c2 = c * c;

  if (x2 < c2)
    return x2;
  else
    return 2 * c * sqrt(x2) - c2;
}

inline double rho_huber_without_sqrt_derivative(const double x2, const double c)
{
  double c2 = c * c;

  if (x2 < c2)
    return 1;
  else
    return c / sqrt(x2);
}

//@}
#endif // ROBUST_ESTIMATORS_H
