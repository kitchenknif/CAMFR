
/////////////////////////////////////////////////////////////////////////////
//
// File:     circ.h
// Author:   Peter.Bienstman@rug.ac.be
// Date:     19990513
// Version:  1.1
//
// Copyright (C) 1998,1999 Peter Bienstman - Ghent University
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CIRC_H
#define CIRC_H

#include "../../util/index.h"
#include "../../waveguide.h"
#include "../../material.h"
#include "../../coord.h"
#include "../../expression.h"
#include "circdisp.h"

/////////////////////////////////////////////////////////////////////////////
//
// STRUCT: CircGlobal
//
//   Groups global variables related to circular structures.
//
/////////////////////////////////////////////////////////////////////////////

typedef enum {cos_type, sin_type} Fieldtype;

struct CircGlobal
{
    int order; 
    Fieldtype fieldtype;
};

extern CircGlobal global_circ;



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Circ_M
//
//   Cross section consisting of arbitrary number M of rings, surrounded
//   by metal cylinder:
//
//      ring 0   :     0    -->  rad[0]  :  mat[0]
//      ring 1   :  rad[0]  -->  rad[1]  :  mat[1]
//      ..
//      ring M-1 : rad[M-2] --> rad[M-1] : mat[M-1]
//  
/////////////////////////////////////////////////////////////////////////////

// forward declaration - see circ.cpp and circmode.h

struct CircCache;
class Circ_M_Mode;
class Circ_2_Mode;
class Circ_1_Mode;

class Circ_M : public MultiWaveguide
{
  public:

    Circ_M() {}
      
    Circ_M(const vector<Complex> &r, const vector<Material*> &m)
      : M(m.size()), radius(r), material(m) {}

    Real S_flux(const FieldExpansion& f,
                Real c1_start, Real c1_stop,
                Real precision = 1e-10) const;

    bool operator==(const Waveguide& w) const;

    Complex lateral_S_corr(const Coord& c) const 
      {return (global_circ.order == 0) ? 2*pi*c.c1 : pi*c.c1;} 
    
    bool contains(const Material& m) const;

    bool no_gain_present() const;

    vector<Material*> get_materials() const
      {return material;}

    Complex eps_at(const Coord& coord) const
      {return material
         [index_lookup(coord.c1, coord.c1_limit, radius)]->eps();}
    
    Complex mu_at(const Coord& coord) const
      {return material
         [index_lookup(coord.c1, coord.c1_limit, radius)]->mu();}
    
    void find_modes();
    
    void calc_overlap_matrices
      (MultiWaveguide* w, cMatrix* O_I_II, cMatrix* O_II_I,
       cMatrix* O_I_I=NULL, cMatrix* O_II_II=NULL);
    
    // Create uniform stretching, starting halfway in outer region
    // and giving this particular complex R.
    
    CoordStretcher createStretcher();

  protected:

    static Hankel hankel;

    unsigned int M;
    
    vector<Complex>   radius;
    vector<Material*> material;

    friend class Circ_M_Mode;
    friend class Circ_2_Mode;
    friend class Circ_1_Mode;
    friend Complex overlap(const Circ_M_Mode*, const Circ_M_Mode*,
                           const CircCache* c=NULL,
                           const vector<Complex>* v=NULL,
                           int=0, int=0, int=0, int=0);
};



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Circ_2
//
//   Core material with radius r,
//   cladding material enclosed in metal cilinder of radius R > r.
//  
/////////////////////////////////////////////////////////////////////////////

class Circ_2 : public Circ_M
{
  public:

    Circ_2(const Complex& r, Material& core,
           const Complex& R, Material& cladding);
    
    void find_modes();

  protected:

    void find_modes_from_scratch_by_ADR();
    void find_modes_from_scratch_by_track();
    void find_modes_by_sweep();
    
    vector<Complex> kr2_backward;
    vector<Complex> guided_disp_params;
    vector<Complex> rad_disp_params;

    unsigned int no_of_guided_modes;
};



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Circ_1
//
//   Uniform material enclosed in metal cilinder of radius R.
//  
/////////////////////////////////////////////////////////////////////////////

class Circ_1 : public Circ_M
{ 
  public:

    Circ_1(const Complex& radius, Material& m);

    void find_modes();
};



/////////////////////////////////////////////////////////////////////////////
//
// CLASS: Circ
//
//   Encapsulates different kinds of Circ structures through a pointer.
//   Can be initialised by an expression.
//   
/////////////////////////////////////////////////////////////////////////////

class Circ : public MultiWaveguide
{
  public:

    Circ(const Expression& ex);
    Circ(const Term& t);    
    ~Circ() {delete c;}

    bool operator==(const Waveguide& w) const {return this == &w;}
    vector<Material*> get_materials()   const {return c->get_materials();}
    bool contains(const Material& m)    const {return c->contains(m);}
    bool no_gain_present()              const {return c->no_gain_present();}

    void find_modes() {return c->find_modes();}
    
    Mode* get_mode(int i)    const {return c->get_mode(i);}
    Mode* get_fw_mode(int i) const {return c->get_fw_mode(i);}
    Mode* get_bw_mode(int i) const {return c->get_bw_mode(i);}
    
    Complex eps_at(const Coord& coord) const {return c->eps_at(coord);}
    Complex  mu_at(const Coord& coord) const {return c-> mu_at(coord);}
    Complex   n_at(const Coord& coord) const {return c->  n_at(coord);}
    
    int N() const {return c->N();}

    const FieldExpansion field_from_source
      (const Coord& pos, const Coord& orientation)
        {return c->field_from_source(pos, orientation);}

    Real S_flux(const FieldExpansion& f,
                Real c1_start, Real c1_stop,
                Real precision = 1e-10) const
      {return c->S_flux(f, c1_start, c1_stop, precision);}

    void calc_overlap_matrices
      (MultiWaveguide* w2, cMatrix* O_I_II, cMatrix* O_II_I,
       cMatrix* O_I_I=NULL, cMatrix* O_II_II=NULL)
      {return c->calc_overlap_matrices(dynamic_cast<Circ*>(w2)->c,
                                       O_I_II,O_II_I,O_I_I,O_II_II);}

    string repr() const {return c->repr();}
    
  protected:

    Circ_M* c;
};

inline ostream& operator<<(ostream& s, const Circ& circ)
  {return s << circ.repr();}  



#endif
