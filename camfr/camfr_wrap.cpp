
/////////////////////////////////////////////////////////////////////////////
//
// File:          camfr_wrap.cpp
// Author:        Peter.Bienstman@rug.ac.be
// Date:          20020403
// Version:       2.0
//
// Copyright (C) 2002 Peter Bienstman - Ghent University
//
/////////////////////////////////////////////////////////////////////////////

// TODO
//
// Try implicit conversions (note: const arg for constructor required)
// Enums and constants
// Default arguments

#define BOOST_PYTHON_V2
#define BOOST_PYTHON_SOURCE

#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/type_from_python.hpp>
#include <boost/python/object/value_holder.hpp>
#include <boost/python/object/pointer_holder.hpp>
#include <boost/python/object/class.hpp>
#include <boost/python/reference_existing_object.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/errors.hpp>
#include <libs/python/src/converter/builtin_converters.cpp>

#include "Numeric/arrayobject.h"

#include "defs.h"
#include "coord.h"
#include "mode.h"
#include "field.h"
#include "material.h"
#include "waveguide.h"
#include "scatterer.h" 
#include "expression.h"
#include "stack.h"
#include "cavity.h"
#include "bloch.h"
#include "infstack.h"
#include "math/calculus/function.h"
#include "primitives/planar/planar.h"
#include "primitives/circ/circ.h"
#include "primitives/slab/generalslab.h"
#include "primitives/slab/isoslab/slab.h"
#include "primitives/slab/isoslab/slabwall.h"
#include "primitives/slab/isoslab/slabdisp.h"
#include "primitives/section/section.h"
#include "primitives/section/sectiondisp.h"

/////////////////////////////////////////////////////////////////////////////
//
// Python wrappers for the CAMFR classes.
// The wrappers are created with the Boost library.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// Additional functions used in the Python interface.
//
/////////////////////////////////////////////////////////////////////////////

inline void set_lambda(Real l)
  {global.lambda = l;}

inline Real get_lambda()
  {return global.lambda;}

inline void set_N(int n)
  {global.N = n;}

inline int get_N()
  {return global.N;}

inline void set_polarisation(long pol)
  {global.polarisation = Polarisation(pol);}

inline void set_gain_material(Material* m)
  {global.gain_mat = m;} 

inline void set_solver(long s)
  {global.solver = Solver(s);} 

inline void set_stability(long s)
  {global.stability = Stability(s);}

inline void set_precision(unsigned int i)
  {global.precision = i;} 

inline void set_precision_enhancement(unsigned int i)
  {global.precision_enhancement = i;}

inline void set_dx_enhanced(Real d)
  {global.dx_enhanced = d;} 

inline void set_precision_rad(unsigned int i)
  {global.precision_rad = i;} 

inline void set_C_upperright(const Complex& c)
  {global.C_upperright = c;} 

inline void set_sweep_from_previous(bool b)
  {global.sweep_from_previous = b;} 

inline void set_sweep_steps(unsigned int i)
  {global.sweep_steps = i;} 

inline void set_eps_trace_coarse(Real d)
  {global.eps_trace_coarse = d;} 

inline void set_chunk_tracing(bool b)
  {global.chunk_tracing = b;} 

inline void set_unstable_exp_threshold(Real d)
  {global.unstable_exp_threshold = d;} 

inline void set_field_calc(long f)
  {global.field_calc = Field_calc(f);} 

inline void set_bloch_calc(long s)
  {global.bloch_calc = Bloch_calc(s);}

inline void set_eigen_calc(long s)
  {global.eigen_calc = Eigen_calc(s);}

inline void set_orthogonal(bool b)
  {global.orthogonal = b;}

inline void set_degenerate(bool b)
  {global.degenerate = b;}

inline void set_circ_order(int n)
  {global_circ.order = n;}

inline void set_circ_fieldtype(long f)
  {global_circ.fieldtype = Fieldtype(f);}

inline void set_left_wall(SlabWall* w)
  {global_slab.leftwall = w;}

inline void set_right_wall(SlabWall* w)
  {global_slab.rightwall = w;}

inline void set_beta(const Complex& beta)
  {global_slab.beta = beta;}

inline int mode_pol(const Mode& m) {return m.pol;}

inline Complex field_E1(const Field& f) {return f.E1;}
inline Complex field_E2(const Field& f) {return f.E2;}
inline Complex field_Ez(const Field& f) {return f.Ez;}
inline Complex field_H1(const Field& f) {return f.H1;}
inline Complex field_H2(const Field& f) {return f.H2;}
inline Complex field_Hz(const Field& f) {return f.Hz;}

inline void check_index(int i)
{
  if ( (i<0) || (i>=int(global.N)) )
  {
    PyErr_SetString(PyExc_IndexError, "index out of bounds.");
    throw std::out_of_range("index out of bounds.");
  }
}

inline void check_bloch_index(int i)
{
  if ( (i<0) || (i>=int(2*global.N)) )
  {
    PyErr_SetString(PyExc_IndexError, "index out of bounds.");
    throw std::out_of_range("index out of bounds.");
  }
}

inline Mode* waveguide_get_mode(const Waveguide& w, int i)
  {check_index(i); return w.get_mode(i+1);}
inline Mode* waveguide_get_fw_mode(const Waveguide& w, int i)
  {check_index(i); return w.get_fw_mode(i+1);}
inline Mode* waveguide_get_bw_mode(const Waveguide& w, int i)
  {check_index(i); return w.get_bw_mode(i+1);}

inline BlochMode* blochstack_get_mode(BlochStack& b, int i)
  {check_bloch_index(i); return dynamic_cast<BlochMode*>(b.get_mode(i+1));}

inline cMatrix stack_get_R12(const Stack& s)
  {if (s.as_multi()) return s.as_multi()->get_R12();}
inline cMatrix stack_get_R21(const Stack& s)
  {if (s.as_multi()) return s.as_multi()->get_R21();}
inline cMatrix stack_get_T12(const Stack& s)
  {if (s.as_multi()) return s.as_multi()->get_T12();}
inline cMatrix stack_get_T21(const Stack& s)
  {if (s.as_multi()) return s.as_multi()->get_T21();}

inline Complex stack_R12(const Stack& s, int i, int j)
  {check_index(i); check_index(j); return s.R12(i+1,j+1);}
inline Complex stack_R21(const Stack& s, int i, int j)
  {check_index(i); check_index(j); return s.R21(i+1,j+1);}
inline Complex stack_T12(const Stack& s, int i, int j)
  {check_index(i); check_index(j); return s.T12(i+1,j+1);}
inline Complex stack_T21(const Stack& s, int i, int j)
  {check_index(i); check_index(j); return s.T21(i+1,j+1);}

inline Real stack_inc_S_flux(Stack& s, Real c1_start, Real c1_stop, Real eps)
  {return dynamic_cast<MultiWaveguide*>(s.get_inc())
     ->S_flux(s.inc_field_expansion(),c1_start,c1_stop,eps);}

inline Real stack_ext_S_flux(Stack& s, Real c1_start, Real c1_stop, Real eps)
  {return dynamic_cast<MultiWaveguide*>(s.get_ext())
     ->S_flux(s.ext_field_expansion(),c1_start,c1_stop,eps);}



/////////////////////////////////////////////////////////////////////////////
//
// Functions convertion C++ objects to and from Python objects.
//
// Note on memory management: as the Blitz ref-counting system might free
// data that is still in use in the Python environment, we have to make
// a copy of the array data.
//
/////////////////////////////////////////////////////////////////////////////

using boost::python::to_python_converter;

struct cVector_to_python : to_python_converter<cVector, cVector_to_python>
{
  static PyObject* convert(const cVector& c)
  {
    char* c_copy = (char*) malloc(c.rows()*sizeof(Complex));
    memcpy(c_copy, c.data(), c.rows()*sizeof(Complex));
    
    int dim[1]; dim[0] = c.rows();

    PyArrayObject* result = (PyArrayObject*)
      PyArray_FromDimsAndData(1, dim, PyArray_CDOUBLE, c_copy);

    return PyArray_Return(result);
  }
};


namespace boost { namespace python { namespace converter { namespace {

struct cVector_from_python
{
  static unaryfunc* get_slot(PyObject* o)
  {
    return &py_object_identity;
  }
    
  static cVector extract(PyObject* o)
  {
    PyArrayObject* a 
      = (PyArrayObject*) PyArray_Cast((PyArrayObject*)o, PyArray_CDOUBLE);

    if ( (a->nd != 1) || (a->dimensions[0] != int(global.N)) )
    {
      PyErr_SetString(PyExc_ValueError, "array has wrong dimensions.");
      throw boost::python::argument_error();
    }
  
    return cVector((Complex*) a->data,global.N,
                   blitz::neverDeleteData,fortranArray);
  }
};

}}}} // namespace boost::python::converter


struct cMatrix_to_python : to_python_converter<cMatrix, cMatrix_to_python>
{
  static PyObject* convert(const cMatrix& c)
  {
    int dim[2]; dim[0] = global.N; dim[1] = global.N;

    cMatrix c_bak(global.N,global.N,fortranArray); c_bak = c.copy();
  
    PyArrayObject* result = (PyArrayObject*)
      PyArray_FromDimsAndData(2, dim, PyArray_CDOUBLE, (char*) c_bak.data());

    // Reflect Fortran storage orders.

    int tmp = result->strides[0];
    result->strides[0] = result->strides[1];
    result->strides[1] = tmp;

    return PyArray_Return(result);
  }
};



/////////////////////////////////////////////////////////////////////////////
//
// These functions compensate for the lack of implicit type conversion
// in Python, mainly when converting to Term in expressions like
//
//   const Expression  operator+(const Term& L,       const Term& R);
//   const Expression& operator+(const Expression& L, const Term& R);
//
/////////////////////////////////////////////////////////////////////////////

Term material_to_term(BaseMaterial& m, const Complex& d)
{
  if (real(d) < 0)
    std::cout << "Warning: negative real length of material." << std::endl;
  
  return Term(m(d));
} 

Term waveguide_to_term(Waveguide& w, const Complex& d)
{
  if (real(d) < 0)
    std::cout << "Warning: negative real length of waveguide." << std::endl;
  
  return Term(w(d));
}

#define TERM_PLUS_TERM(t1, t2) inline const Expression \
 operator+(t1& L, t2& R) {return Term(L) + Term(R);}

#define TERM_PLUS_TERM_F(t1, t2) (const Expression (*) (t1&, t2&)) &operator+

TERM_PLUS_TERM(Stack,      Stack)
TERM_PLUS_TERM(Stack,      const Expression)
TERM_PLUS_TERM(Stack,      const Term)
TERM_PLUS_TERM(Stack,      Scatterer)
TERM_PLUS_TERM(const Term, Stack)
TERM_PLUS_TERM(const Term, const Expression)
TERM_PLUS_TERM(const Term, Scatterer)
TERM_PLUS_TERM(Scatterer,  Stack)
TERM_PLUS_TERM(Scatterer,  const Expression)
TERM_PLUS_TERM(Scatterer,  const Term)
TERM_PLUS_TERM(Scatterer,  Scatterer)
  
#define EX_PLUS_TERM(t) const Expression \
 operator+(const Expression& L, t& R) {return L + Term(R);}

#define EX_PLUS_TERM_F(t) (const Expression (*) (const Expression&, t&)) \
      &operator+
  
EX_PLUS_TERM(Stack)
EX_PLUS_TERM(const Expression)
EX_PLUS_TERM(Scatterer)


  
/////////////////////////////////////////////////////////////////////////////
//
// These functions compensate for the current lack of automatic handling
// of default arguments in Boost.
//
/////////////////////////////////////////////////////////////////////////////

Complex stack_lateral_S_flux(Stack& s, const Complex& c)
  {return s.lateral_S_flux(c);}

Complex stack_lateral_S_flux_2(Stack& s, const Complex& c, int k)
  {std::vector<Complex> S_k; s.lateral_S_flux(c, &S_k); return S_k[k];}

Real cavity_calc_sigma(Cavity& c)
  {return c.calc_sigma();}

void cavity_find_modes_in_region_3
  (Cavity& c, Real lambda_start, Real lambda_stop, Real delta_lambda)
    {return c.find_modes_in_region(lambda_start, lambda_stop, delta_lambda);} 

void cavity_find_modes_in_region_7
  (Cavity& c, Real lambda_start, Real lambda_stop,
   Real delta_lambda, unsigned int number,
   Real n_imag_start, Real n_imag_stop, unsigned int passes)
{
  return c.find_modes_in_region(lambda_start, lambda_stop, delta_lambda,
                                number, n_imag_start, n_imag_stop, passes);
}

void cavity_find_mode_2(Cavity& c, Real lambda_start, Real lambda_stop)
  {return c.find_mode(lambda_start, lambda_stop);}

void cavity_find_mode_5
  (Cavity& c, Real lambda_start, Real lambda_stop,
   Real n_imag_start, Real n_imag_stop, unsigned int passes)
{
  return
    c.find_mode(lambda_start, lambda_stop, n_imag_start, n_imag_stop, passes); 
}
void cavity_set_source(Cavity& c, Coord& pos, Coord& orientation)
  {c.set_source(pos,orientation);}

void stack_set_inc_field(Stack& s, const cVector& f)
  {s.set_inc_field(f);}

void stack_set_inc_field_2(Stack& s, const cVector& f, const cVector& b) 
  {s.set_inc_field(f, &const_cast<cVector&>(b));}



/////////////////////////////////////////////////////////////////////////////
//
// The CAMFR module itself
//
/////////////////////////////////////////////////////////////////////////////

#undef PyArray_Type
extern PyTypeObject PyArray_Type;


BOOST_PYTHON_MODULE_INIT(_camfr)
{
  using namespace boost::python;
  using namespace boost::python::converter;
  using boost::shared_ptr;
  using boost::python::return_value_policy;
  using boost::python::reference_existing_object;

  import_array();

  cVector_to_python();
  cMatrix_to_python();

  slot_rvalue_from_python<cVector, cVector_from_python>();

  module camfr("_camfr");

#if 0

    // Wrap Limit enum.

  camfr
 
    .add(
      enum_<Limit>("Limit")
      .value("Plus", Plus)
      .value("Min",  Min)
      )

    // Wrap Solver enum.

    .add(
      enum_<Solver>("Solver")
      .value("ADR",   ADR)
      .value("track", track)
      )
 
    // Wrap Stability enum.

    .add(
      enum_<Stability>("Stability")
      .value("normal", ADR)
      .value("extra",  track)
      .value("SVD",    SVD)
      )

    // Wrap Field_calc enum.

    .add(
      enum_<Field_calc>("Field_calc")
      .value("T_T", T_T)
      .value("S_T", S_T)
      .value("S_S", S_S)
      )

    // Wrap Bloch_calc enum.

    .add(
      enum_<Bloch_calc>("Bloch_calc")
      .value("GEV", GEV)
      .value("T",   T)
      )

    // Wrap Eigen_calc enum.

    .add(
      enum_<Eigen_calc>("Eigen_calc")
      .value("lapack",  lapack)
      .value("arnoldi", arnoldi)
      )

    // Wrap Polarisation enum.

    .add(
      enum_<Polarisation>("Polarisation")
      .value("unknown", unknown)
      .value("TEM",     TEM)
      .value("TE",      TE)
      .value("TM",      TM)
      .value("HE",      HE)
      .value("EH",      EH)
      .value("TE_TM",   TE_TM)
      )

    // Wrap Fieldtype enum.

    .add(
      enum_<Fieldtype>("Fieldtype")
      .value("cos_type", cos_type)
      .value("sin_type", sin_type)
      );

#endif

  // Wrap getters and setters for global parameters.   

  camfr
    .def("set_lambda",                 set_lambda)
    .def("get_lambda",                 get_lambda)
    .def("set_N",                      set_N)
    .def("N",                          get_N)
    .def("set_polarisation",           set_polarisation)
    .def("set_gain_material",          set_gain_material)
    .def("set_solver",                 set_solver)
    .def("set_stability",              set_stability)
    .def("set_precision",              set_precision)
    .def("set_precision_enhancement",  set_precision_enhancement)
    .def("set_dx_enhanced",            set_dx_enhanced)
    .def("set_precision_rad",          set_precision_rad)
    .def("set_C_upperright",           set_C_upperright)
    .def("set_sweep_from_previous",    set_sweep_from_previous)
    .def("set_sweep_steps",            set_sweep_steps)
    .def("set_eps_trace_coarse",       set_eps_trace_coarse)
    .def("set_chunk_tracing",          set_chunk_tracing)
    .def("set_unstable_exp_threshold", set_unstable_exp_threshold)
    .def("set_field_calc",             set_field_calc)
    .def("set_bloch_calc",             set_bloch_calc)
    .def("set_eigen_calc",             set_eigen_calc)
    .def("set_orthogonal",             set_orthogonal)
    .def("set_degenerate",             set_degenerate)
    .def("set_circ_order",             set_circ_order)
    .def("set_circ_field_type",        set_circ_fieldtype)
    .def("set_left_wall",              set_left_wall)
    .def("set_right_wall",             set_right_wall)
    .def("set_beta",                   set_beta)
    .def("free_tmps",                  free_tmps);

  // Wrap Coord.

  camfr.add(
    class_<Coord>("Coord")
    .def_init(args<const Complex&, const Complex&, const Complex&>())
    //.def_init(args<const Complex&, const Complex&, const Complex&,
    //                          Limit,          Limit,          Limit>())
    .def("__repr__", &Coord::repr)
    );

  // Wrap Field.

  camfr.add(
    class_<Field>("Field")
    .def("E1",       field_E1)
    .def("E2",       field_E2)
    .def("Ez",       field_Ez)
    .def("H1",       field_H1)
    .def("H2",       field_H2)
    .def("Hz",       field_Hz)
    .def("abs_E",    &Field::abs_E)
    .def("abs_H",    &Field::abs_H)
    .def("__repr__", &Field::repr)
    );

  // Wrap FieldExpansion.

  camfr.add(
    class_<FieldExpansion>("FieldExpansion")
    .def("field",    &FieldExpansion::field)
    .def("__repr__", &FieldExpansion::repr)
    );

  // Wrap BaseMaterial.

  camfr.add(class_<BaseMaterial, boost::noncopyable>("BaseMaterial"));

  // Wrap Material.

  camfr.add(
    class_<Material, shared_ptr<Material>, bases<BaseMaterial> >("Material")
    .def_init(args<const Complex&>())
    .def_init(args<const Complex&, const Complex&>()) // TODO: def. arg
    .def("__call__", material_to_term)
    .def("n",        &Material::n)
    .def("epsr",     &Material::epsr)
    .def("mur",      &Material::mur)
    .def("eps",      &Material::eps)
    .def("mu",       &Material::mu)
    .def("gain",     &Material::gain)
    .def("__repr__", &Material::repr)
    );

  // Wrap Material_length.

  camfr.add(class_<Material_length>("Material_length"));

  // Wrap Mode.

  camfr.add(
    class_<Mode>("Mode")
    .def("field",    &Mode::field)
    .def("n_eff",    &Mode::n_eff)
    .def("kz",       &Mode::get_kz)
    .def("pol",      mode_pol)
    .def("__repr__", &Mode::repr)
    );

  // Wrap Waveguide.
  
  camfr.add(
    class_<Waveguide, boost::noncopyable>("Waveguide")
    .def("core",     &Waveguide::get_core,
         return_value_policy<reference_existing_object>())
    .def("eps",      &Waveguide::eps_at)
    .def("mu",       &Waveguide::mu_at)
    .def("n",        &Waveguide::n_at)
    .def("N",        &Waveguide::N)
    .def("mode",     waveguide_get_mode,
         return_value_policy<reference_existing_object>())
    .def("fw_mode",  waveguide_get_fw_mode,
         return_value_policy<reference_existing_object>())
    .def("bw_mode",  waveguide_get_bw_mode,
         return_value_policy<reference_existing_object>())
    .def("calc",     &Waveguide::find_modes)
    .def("__repr__", &Waveguide::repr)
    .def("__call__", waveguide_to_term)
    );

  // Wrap Waveguide_length.

  camfr.add(class_<Waveguide_length>("Waveguide_length"));

  // Wrap MultiWaveguide.

  camfr.add(
    class_<MultiWaveguide, bases<Waveguide>, boost::noncopyable>
    ("MultiWaveguide")
    .def("field_from_source", &MultiWaveguide::field_from_source)
    );

  // Wrap MonoWaveguide.

  camfr.add(
    class_<MonoWaveguide, bases<Waveguide>, boost::noncopyable>
    ("MonoWaveguide")
    );

  // Wrap Scatterer.

  camfr.add(
    class_<Scatterer, boost::noncopyable>("Scatterer")
    .def("calc", &Scatterer::calcRT)
    .def("free", &Scatterer::freeRT)
    .def("inc",  &Scatterer::get_inc,
         return_value_policy<reference_existing_object>())
    .def("ext",  &Scatterer::get_ext,
         return_value_policy<reference_existing_object>())
    .def("__add__", TERM_PLUS_TERM_F(Scatterer, const Term))
    .def("__add__", TERM_PLUS_TERM_F(Scatterer, Stack))
    .def("__add__", TERM_PLUS_TERM_F(Scatterer, const Expression))
    .def("__add__", TERM_PLUS_TERM_F(Scatterer, Scatterer))
    );

  // Wrap MultiScatterer.

  camfr.add(
    class_<MultiScatterer, bases<Scatterer>, boost::noncopyable>
    ("MultiScatterer")
    );

  // Wrap DenseScatterer.

  camfr.add(
    class_<DenseScatterer, bases<MultiScatterer>, boost::noncopyable>
    ("DenseScatterer")
    );

  // Wrap DiagScatterer.

  camfr.add(
    class_<DiagScatterer, bases<MultiScatterer>, boost::noncopyable>
    ("DiagScatterer")
    );

  // Wrap MonoScatterer.
  
  camfr.add(
    class_<MonoScatterer, bases<Scatterer>, boost::noncopyable>
    ("MonoScatterer")
    );

  // Wrap FlippedScatterer.

  camfr.add(
    class_<FlippedScatterer, bases<MultiScatterer> >("FlippedScatterer")
    .def_init(args<MultiScatterer&>())
    );

  // Wrap E_Wall.

  camfr.add(
    class_<E_Wall, bases<DiagScatterer> >("E_Wall")
    .def_init(args<Waveguide&>())
    );

  // Wrap H_Wall.

  camfr.add(
    class_<H_Wall, bases<DiagScatterer> >("H_Wall")
    .def_init(args<Waveguide&>())
    );

  // Wrap Expression.

  camfr.add(
    class_<Expression>("Expression")
    .def_init(args<>())
    .def_init(args<const Term&>())
    .def_init(args<const Expression&>())
    .def("flatten",  &Expression::flatten)
    .def("inc",  &Expression::get_inc,
         return_value_policy<reference_existing_object>())
    .def("ext",  &Expression::get_inc,
         return_value_policy<reference_existing_object>())
    .def("__repr__", &Expression::repr)
    .def("add",      &Expression::operator+=)
    .def("__iadd__", &Expression::operator+=)
    .def("__add__",  EX_PLUS_TERM_F(const Term))
    .def("__add__",  EX_PLUS_TERM_F(Stack))
    .def("__add__",  EX_PLUS_TERM_F(const Expression))
    .def("__add__",  EX_PLUS_TERM_F(Scatterer))
    .def("__rmul__",
         (const Term (*) (const Expression&, unsigned int)) &operator*)
    .def("__mul__",
         (const Term (*) (const Expression&, unsigned int)) &operator*)
    );

  // Wrap Term.

  camfr.add(
    class_<Term>("Term")
    .def_init(args<Scatterer&>())
    .def_init(args<Stack&>())
    .def_init(args<const Expression&>())
    .def("inc",  &Term::get_inc,
         return_value_policy<reference_existing_object>())
    .def("ext",  &Term::get_inc,
         return_value_policy<reference_existing_object>())
    .def("__repr__", &Term::repr)
    .def("__add__",  TERM_PLUS_TERM_F(const Term, const Term))
    .def("__add__",  TERM_PLUS_TERM_F(const Term, Stack))
    .def("__add__",  TERM_PLUS_TERM_F(const Term, const Expression))
    .def("__add__",  TERM_PLUS_TERM_F(const Term, Scatterer))
    .def("__rmul__", (const Term (*) (const Term&, unsigned int)) &operator*)
    .def("__mul__",  (const Term (*) (const Term&, unsigned int)) &operator*)
    );

  // Wrap Stack.

  camfr.add(
    class_<Stack>("Stack")
    .def_init(args<const Expression&>())
    .def_init(args<const Term&>())
    .def("calc",           &Stack::calcRT)
    .def("free",           &Stack::freeRT)
    .def("inc",            &Stack::get_inc,
         return_value_policy<reference_existing_object>())
    .def("ext",            &Stack::get_ext,
         return_value_policy<reference_existing_object>())
    .def("length",         &Stack::get_total_thickness)
    .def("set_inc_field",  stack_set_inc_field)
    .def("set_inc_field",  stack_set_inc_field_2)
    .def("inc_field",      &Stack::get_inc_field)
    .def("refl_field",     &Stack::get_refl_field)
    .def("trans_field",    &Stack::get_trans_field)
    .def("inc_S_flux",     stack_inc_S_flux)
    .def("ext_S_flux",     stack_ext_S_flux)
    .def("field",          &Stack::field)
    .def("lateral_S_flux", stack_lateral_S_flux)
    .def("lateral_S_flux", stack_lateral_S_flux_2)
    .def("eps",            &Stack::eps_at)
    .def("mu",             &Stack::mu_at)
    .def("n",              &Stack::n_at)
    .def("R12",            stack_get_R12)
    .def("R21",            stack_get_R21)
    .def("T12",            stack_get_T12)
    .def("T21",            stack_get_T21)
    .def("R12",            stack_R12)
    .def("R21",            stack_R21)
    .def("T12",            stack_T12)
    .def("T21",            stack_T21)
    .def("__add__", TERM_PLUS_TERM_F(Stack, const Term))
    .def("__add__", TERM_PLUS_TERM_F(Stack, Stack))
    .def("__add__", TERM_PLUS_TERM_F(Stack, const Expression))
    .def("__add__", TERM_PLUS_TERM_F(Stack, Scatterer))
    );

  // Wrap Cavity.

  camfr.add(
    class_<Cavity>("Cavity")
    .def_init(args<Stack&, Stack&>())
    .def("find_modes_in_region", cavity_find_modes_in_region_3)
    .def("find_modes_in_region", cavity_find_modes_in_region_7)
    .def("find_mode",            cavity_find_mode_2)
    .def("find_mode",            cavity_find_mode_5)
    .def("sigma",                cavity_calc_sigma)
    .def("set_source",           cavity_set_source)
    .def("field",                &Cavity::field)
    );

  // Wrap BlochStack.

  camfr.add(
    class_<BlochStack, bases<MultiWaveguide> >("BlochStack")
    .def_init(args<const Expression&>())
    .def("mode",        blochstack_get_mode,
         return_value_policy<reference_existing_object>())
    .def("length",      &BlochStack::get_total_thickness)
    .def("beta_vector", &BlochStack::get_beta_vector)
    .def("__repr__",    &BlochStack::repr)
    );

  // Wrap BlochMode.

  camfr.add(
    class_<BlochMode, bases<Mode> >("BlochMode")
    .def("fw_field", &BlochMode::fw_field)
    .def("bw_field", &BlochMode::bw_field)
    .def("S_flux",   &BlochMode::S_flux)
    );

  // Wrap InfStack.

  camfr.add(
    class_<InfStack, bases<DenseScatterer> >("InfStack")
    .def_init(args<const Expression&>())
    .def_init(args<const Expression&, const Complex&>()) // TMP
    .def("R12", &InfStack::get_R12,
         return_value_policy<reference_existing_object>())
    );

  // Wrap RealFunction.
  
  camfr.add(
    class_<RealFunction, boost::noncopyable>("RealFunction")
    .def("times_called", &RealFunction::times_called)
    .def("__call__",     &RealFunction::operator())
    );

  // Wrap ComplexFunction.

  camfr.add(
    class_<ComplexFunction, boost::noncopyable>("RealFunction")
    .def("times_called", &ComplexFunction::times_called)
    .def("__call__",     &ComplexFunction::operator())
    );

  // Wrap Planar.

  camfr.add(
    class_<Planar, bases<MonoWaveguide> >("Planar")
    .def_init(args<Material&>())
    .def("set_theta", &Planar::set_theta)
    );

  // Wrap Circ.

  camfr.add(
    class_<Circ, bases<MultiWaveguide> >("Circ")
    .def_init(args<Term&>())
    .def_init(args<Expression&>())
    );

  // Wrap SlabWall.

  camfr.add(
    class_<SlabWall, boost::noncopyable>("SlabWall")
    .def("R", &SlabWall::get_R12)
    );

  // Wrap SlabWallMixed.

  camfr.add(
    class_<SlabWallMixed, bases<SlabWall> >("SlabWallMixed")
    .def_init(args<const Complex&, const Complex&>())
    );

  //camfr.add(boost::python::make_ref(slab_E_wall),    "slab_E_wall");
  //camfr.add(boost::python::make_ref(slab_H_wall),    "slab_H_wall");
  //camfr.add(boost::python::make_ref(slab_open_wall), "slab_open_wall");

  // Wrap SlabWall_TBC.

  camfr.add(
    class_<SlabWall_TBC, bases<SlabWall> >("SlabWall_TBC")
    .def_init(args<const Complex&, const Material&>())
    );

  // Wrap SlabWall_PC.

  camfr.add(
    class_<SlabWall_PC, bases<SlabWall> >("SlabWall_PC")
    .def_init(args<const Expression&>())
    );

  // Wrap SlabDisp.

  camfr.add(
    class_<SlabDisp, bases<ComplexFunction> >("SlabDisp")
    .def_init(args<Expression&, Real>())
    .def_init(args<Expression&, Real, SlabWall*, SlabWall*>())
    );

  // Wrap Slab.

  camfr.add(
    class_<Slab, bases<MultiWaveguide> >("Slab")
    .def_init(args<const Term&>())
    .def_init(args<const Expression&>())
    .def("set_left_wall",  &Slab::set_left_wall)
    .def("set_right_wall", &Slab::set_right_wall)
    .def("width",          &Slab::get_width)
    );

  // Wrap SectionDisp.

  camfr.add(
    class_<SectionDisp, bases<ComplexFunction> >("SectionDisp")
    .def_init(args<Stack&, Stack&, Real, int>())
    );

  // Wrap Section.

  camfr.add(
    class_<Section, bases<MultiWaveguide> >("Section")
    .def_init(args<const Expression&>())
    .def_init(args<const Expression&, int>())
    .def_init(args<const Expression&, const Expression&>())
    .def_init(args<const Expression&, const Expression&, int>())
    .def("width", &Section::get_width)
    );
}
