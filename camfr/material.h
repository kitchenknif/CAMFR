
/////////////////////////////////////////////////////////////////////////////
//
// File:     material.h
// Author:   Peter.Bienstman@rug.ac.be
// Date:     19990503
// Version:  1.02
//
// Copyright (C) 1998,1999 Peter Bienstman - Ghent University
//
////////////////////////////////////////////////////////////////////////////

#ifndef MATERIAL_H
#define MATERIAL_H

#include <sstream>
#include <iostream>
#include "defs.h"

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Material_length
//
//   A piece of material with a certain length.
//
/////////////////////////////////////////////////////////////////////////////

class BaseMaterial; // Forward declaration.

struct Material_length
{
    Material_length(BaseMaterial& mat_, const Complex& d_=0.0)
      : mat(&mat_), d(d_) {}

    Material_length(BaseMaterial* mat_, const Complex& d_=0.0)
      : mat(mat_), d(d_) {}
    
    BaseMaterial* mat;
    const Complex d;
};



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: BaseMaterial
//
/////////////////////////////////////////////////////////////////////////////

class BaseMaterial
{
  public:

    const Material_length operator() (const Complex& d=0.0) const;

    virtual bool no_gain_present() const = 0;
    virtual string repr()          const = 0;
};

inline ostream& operator<<(ostream& s, const BaseMaterial& m)
  {return s << m.repr() << endl;}



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Material
//
//   The purpose of this class is to allow for easy incorporation of
//   future, more complicated material models.
//
//   For example, to add dispersive materials, you would want to subclass
//   this class and override its functions.
//
//   Note: access the current wavelength through the global variable
//   'global.lambda' (see defs.h)
//
//   Be careful not to confuse Material(3.5, 0.1)
//   and Material(Complex(3.5, 0.1)). The former is a magnetic material.
//   A check is provided to avoid this error.
//  
/////////////////////////////////////////////////////////////////////////////

class Material : public BaseMaterial
{
  public:

     Material(const Complex& n, const Complex& mur=1.0)
       : i_n(n), i_mur(mur)
       {if (i_mur != 1.0) cout << "Magnetic material" << endl;}
     
     const Complex    n() const {return i_n;}
     const Complex epsr() const {return i_n * i_n;}
     const Complex  mur() const {return i_mur;}
     const Complex  eps() const {return i_n * i_n * eps0;}
     const Complex   mu() const {return i_mur * mu0;}
     const Real    gain() const {return 4*imag(i_n)*pi/(global.lambda * 1e-4);}

     void set_n(Complex n)        {i_n   = n;}
     void set_mur(Complex mur)    {i_mur = mur;}   
     void set_n_imag(Real n_imag) {i_n   = Complex(real(i_n), n_imag);}

     bool no_gain_present() const {return (imag(i_n) < 1e-12);}

     bool operator==(const Material& m) const
       {return (abs(i_n - m.i_n) < 1e-12) && (abs(i_mur - m.i_mur) < 1e-12);}

     bool operator!=(const Material& m) const
       {return !(*this == m);}

     string repr() const
       {ostringstream s; s << "Isotropic n=" << i_n; return s.str();}
     
  protected:

     Complex i_n;
     Complex i_mur; 
};

inline ostream& operator<<(ostream& s, const Material& m)
  {return s << m.repr() << endl;}




/////////////////////////////////////////////////////////////////////////////
//
// signedsqrt(kz2, material)
//   Extra layer of indirection, for experimenting with sign of sqrt
//   in relation to material loss or gain.
//
//   Will be removed in the future.
//
/////////////////////////////////////////////////////////////////////////////

inline Complex signedsqrt(const Complex& kz2, const Material& material)
   {return sqrt(kz2);}

inline Complex signedsqrt(const Complex& kz2, const Material* material)
   {return signedsqrt(kz2, *material);}



#endif