
/////////////////////////////////////////////////////////////////////////////
//
// File:     allroots.cpp
// Author:   Peter.Bienstman@rug.ac.be
// Date:     20010322
// Version:  1.0
//
// Copyright (C) 2001 Peter Bienstman - Ghent University
//
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "../../../util/cvector.h"
#include "../../linalg/linalg.h"
#include "../polyroot/polyroot.h"
#include "allroots.h"
#include "mueller.h"

/////////////////////////////////////////////////////////////////////////////
//
// Maximum number of roots in a contour before we invoke subdivision.
//
/////////////////////////////////////////////////////////////////////////////

const unsigned int N_max = 10;



/////////////////////////////////////////////////////////////////////////////
//
// roots_contour
//
//   Low level routine, used by allroots.
//
//   Locate at most N roots of the function f in a rectangular contour.
//   starting from the integrals z^n/f(z). The value of the integrals are
//   passed separately, rather than by calling contour.contour_integrals,
//   to allow reuse of calculations from mother contour.
//
//   Does not used adaptive subdivision, such that there is no guarantee
//   that all the roots are found.
//
/////////////////////////////////////////////////////////////////////////////

vector<Complex> roots_contour(const Contour& contour,
                              const vector<Complex>& integrals)
{
  const Complex bl = contour.get_bottom_left();
  const Complex tr = contour.get_top_right();
  
  // Calculate G coefficients and adjust N because round-off detection
  // might have changed the number of coefficients.

  vector<Complex> G = integrals / 2. / pi / I;

  unsigned int N = (unsigned int)(ceil(G.size() / 2.0));

  // Check if all G's are zero.

  bool all_zeros = true;
  for (unsigned int i=0; i<=2*N-1; i++)
    if (abs(G[i]) > 1e-10)
      all_zeros = false;

  if (all_zeros)
  {
    vector<Complex> empty;
    return empty;
  }

  // Construct linear system.

  cMatrix A(N,N,fortranArray), B(N,1,fortranArray), c(N,1,fortranArray);

  for (int i=1; i<=N; i++)
    for (int j=1; j<=N; j++)
      A(i,j) = G[i+j-2];

  for (int i=1; i<=N; i++)
    B(i,1) = -G[N+i-1];

  // Solve linear system and exploit the symmetry. Note: the fact that this
  // is a Hankel matrix is not used, since speedups would probable be minor
  // due to the small dimension.

  c = solve_sym(A,B);

  // Find roots of polynomial.

  vector<Complex> coeff;
  coeff.push_back(1.0);
  for (int i=N; i>=1; i--)
    coeff.push_back(c(i,1));

  vector<Complex> roots1 = polyroot(coeff);

  // Refine them using mueller solver with deflation.

  vector<Complex> roots2;
  for (unsigned int i=0; i<roots1.size(); i++)
  {
    // If the estimate is far outside the contour, don't bother.

    if ( abs(roots1[i] - (bl+tr)/2.) > 1000*abs(bl-tr) )
      continue;
    
    bool error = false;
    Complex root = mueller(*contour.get_f(), roots1[i], roots1[i]+.001,
                           1e-15, &roots2, 50, &error);

    if (error)
      break; // No more roots to be found.

    if (contour.encloses(root))
      roots2.push_back(root); 
  }
  
  return roots2;      
}



/////////////////////////////////////////////////////////////////////////////
//
// allroots_contour
//
//   Low level routine, used by allroots.
//
//   Recursively subdivides a contour to locate all roots of a function f
//   inside this contour.
//
/////////////////////////////////////////////////////////////////////////////

struct Sorter
{
    bool operator()(const Complex& a, const Complex& b)
       {return (real(a) < real(b));}
};

vector<Complex> allroots_contour(const Contour& c) 
{
  // Find roots in contour and subcontours.
  
  vector<Complex> roots = roots_contour(c, c.contour_integrals());

  vector<Complex> subroots;
  for (unsigned int i_=0; i_<4; i_++)
  {
    Subcontour i = Subcontour(i_);
    vector<Complex> subroots_i
      = roots_contour(c.subcontour(i), c.subcontour_integrals(i));
    subroots.insert(subroots.end(), subroots_i.begin(), subroots_i.end());
  }

  // If roots contains the same values as subroots, we are done.

  if (roots.size() == subroots.size())
  {
    Sorter sorter;

    sort(   roots.begin(),    roots.end(), sorter);
    sort(subroots.begin(), subroots.end(), sorter);

    bool equal = true;
    for (unsigned int i=0; i<roots.size(); i++)   
      if (abs(roots[i]-subroots[i]) > 1e-10)
        equal = false;
    
    if (equal)    
      return roots;
  }

  // Recatch case of single lost zero. Heuristic to avoid having to
  // increase the precision unduly.

  if ((roots.size() == 1) && (subroots.size() == 0))
    return roots;
  
  // Else, recursively subdivide contour further.
  
  cout << "Subdivide " << c.get_bottom_left() << c.get_top_right() << endl;
  
  roots.clear();
  for (unsigned int i_=0; i_<4; i_++)
  {
    Subcontour i = Subcontour(i_);
    vector<Complex> subroots_i = allroots_contour(c.subcontour(i));
    roots.insert(roots.end(), subroots_i.begin(), subroots_i.end());
  }

  return roots;
}



/////////////////////////////////////////////////////////////////////////////
//
// allroots
//
/////////////////////////////////////////////////////////////////////////////

vector<Complex> allroots
  (ComplexFunction& f, const Complex& bottom_left, const Complex& top_right,
   Real eps=1e-4, Real mu=1e-4, unsigned int max_k=4)
{
  max_k=4;
  
  Contour contour(bottom_left, top_right, f, 2*N_max-1, eps, mu, max_k);
  return allroots_contour(contour);
}



/////////////////////////////////////////////////////////////////////////////
//
// N_roots
//
/////////////////////////////////////////////////////////////////////////////

vector<Complex> N_roots(ComplexFunction& f, unsigned int N,
                        const Complex& bottom_left, const Complex& top_right,
                        ExpandDirection dir=ur,
                        Real eps=1e-4, Real mu=1e-4, unsigned int max_k=4)
{
  // Set up initial contour.

  Contour contour(bottom_left, top_right, f, 2*N-1, eps, mu, max_k);
  vector<Contour> contour_stack;
  contour_stack.push_back(contour);

  // Take additional contours into account until we have sufficient modes.

  vector<Complex> roots;
  
  while (!contour_stack.empty())
  {        
    Contour c = contour_stack.front();
    contour_stack.erase(contour_stack.begin());
    
    vector<Complex> roots_c
      = allroots(f, c.get_bottom_left(), c.get_top_right(), eps, mu, max_k);

    roots.insert(roots.end(), roots_c.begin(), roots_c.end());

    cout << "Roots so far : " << endl;

    for(unsigned int i=0; i<roots.size(); i++)
      cout << i << roots[i] << endl;

    if (    (roots.size() < N)
         && (!roots_c.empty() || (roots_c.empty() && contour_stack.empty())) )
    {
      if (dir == ur)
      {
        vector<Contour> new_c = c.adjacent_ur();

        for (unsigned int i=0; i<new_c.size(); i++)
          contour_stack.push_back(new_c[i]);
      }
      else
        contour_stack.push_back(c.adjacent_r());
    }
  }
  
  return roots;
}