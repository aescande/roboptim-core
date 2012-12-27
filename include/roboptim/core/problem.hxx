// Copyright (C) 2009 by Thomas Moulard, AIST, CNRS, INRIA.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ROBOPTIM_CORE_PROBLEM_HXX
# define ROBOPTIM_CORE_PROBLEM_HXX
# include <algorithm>
# include <stdexcept>
# include <boost/numeric/ublas/io.hpp>
# include <boost/type_traits/is_pointer.hpp>
# include <boost/type_traits/remove_pointer.hpp>
# include <boost/variant.hpp>
# include <boost/variant/get.hpp>
# include <boost/variant/apply_visitor.hpp>

# include <roboptim/core/indent.hh>
# include <roboptim/core/util.hh>

namespace roboptim
{
  //
  // Template specialization for problem without constraint
  //
  template <typename F>
  Problem<F, boost::mpl::vector <> >::Problem (const function_t& f) throw ()
    : function_ (f),
      startingPoint_ (),
      argumentBounds_ (f.inputSize ()),
      argumentScales_ (f.inputSize ())
  {
    // Check that in the objective function m = 1 (R^n -> R).
    assert (f.outputSize () == 1);

    // Initialize bound.
    std::fill (argumentBounds_.begin (), argumentBounds_.end (),
	       Function::makeInfiniteInterval ());
    // Initialize scale.
    std::fill (argumentScales_.begin (), argumentScales_.end (), 1.);
  }

  // Copy constructor.
  template <typename F>
  Problem<F, boost::mpl::vector <> >::Problem
  (const Problem<F, boost::mpl::vector <> >& pb) throw () :
   function_ (pb.function_),
   startingPoint_ (pb.startingPoint_),
   argumentBounds_ (pb.argumentBounds_),
   argumentScales_ (pb.argumentScales_)
  {
  }

  // Copy constructor (convert from another class of problem).
  template <typename F>
  template <typename F_>
   Problem<F, boost::mpl::vector<> >::Problem
   (const Problem<F_, boost::mpl::vector<> >& pb) throw ()
    : function_ (pb.function_),
      startingPoint_ (pb.startingPoint_),
      argumentBounds_ (pb.argumentBounds_),
      argumentScales_ (pb.argumentScales_)
  {
    // Check that F is a subtype of F_.
    BOOST_STATIC_ASSERT((boost::is_base_of<F, F_>::value));

    //FIXME: check that CLIST is a MPL vector of Function's sub-classes.
  }

  template <typename F>
   Problem<F, boost::mpl::vector<> >::~Problem () throw ()
  {
  }

   template <typename F>
  const typename Problem<F, boost::mpl::vector<> >::function_t&
  Problem<F, boost::mpl::vector<> >::function () const throw ()
  {
    return function_;
  }

  template <typename F>
  typename Problem<F, boost::mpl::vector<> >::startingPoint_t&
  Problem<F, boost::mpl::vector<> >::startingPoint () throw ()
  {
    if (startingPoint_ && startingPoint_->size ()
	!= this->function ().inputSize ())
      assert (0 && "Invalid starting point (wrong size)");
    return startingPoint_;
  }

  template <typename F>
  const typename Problem<F, boost::mpl::vector<> >::startingPoint_t&
  Problem<F, boost::mpl::vector<> >::startingPoint () const throw ()
  {
    if (startingPoint_ && startingPoint_->size ()
	!= this->function ().inputSize ())
      assert (0 && "Invalid starting point (wrong size)");
    return startingPoint_;
  }

  template <typename F>
  std::ostream&
  operator<< (std::ostream& o,
	      const Problem<F, boost::mpl::vector<> > & pb)
  {
    return pb.print (o);
  }

  template <typename F>
  std::ostream&
  Problem<F, boost::mpl::vector<> >::
  print (std::ostream& o) const throw ()
  {
    using namespace boost;

    o << "Problem:" << incendl;
    // Function.
    o << this->function () << iendl;

    // Arguments' bounds.
    o << "Argument's bounds: " << this->argumentBounds () << iendl;
    // Arguments' scales.
    o << "Argument's scales: " << this->argumentScales () << iendl;

    // Starting point.
    if (startingPoint_)
      {
	o << iendl << "Starting point: " << *startingPoint_
	  << iendl << "Starting value: "
	  << this->function () (*startingPoint_);
      }
    else
      o << iendl << "No starting point.";

    // Infinity.
    o << iendl << "Infinity value (for all functions): "
      << Function::infinity ();
    return o << decindent;
  }

  template <typename F>
  typename Problem<F, boost::mpl::vector<> >::intervals_t&
  Problem<F, boost::mpl::vector<> >::argumentBounds () throw ()
  {
    return argumentBounds_;
  }

  template <typename F>
  const typename Problem<F, boost::mpl::vector<> >::intervals_t&
  Problem<F, boost::mpl::vector<> >::argumentBounds () const throw ()
  {
    return argumentBounds_;
  }

  template <typename F>
  typename Problem<F, boost::mpl::vector<> >::scales_t&
  Problem<F, boost::mpl::vector<> >::argumentScales () throw ()
  {
    return argumentScales_;
  }

  template <typename F>
  const typename Problem<F, boost::mpl::vector<> >::scales_t&
  Problem<F, boost::mpl::vector<> >::argumentScales () const throw ()
  {
    return argumentScales_;
  }

  //
  //
  // General template implementation
  //
  template <typename F, typename CLIST>
  Problem<F, CLIST>::Problem (const function_t& f) throw ()
    : function_ (f),
      startingPoint_ (),
      constraints_ (),
      bounds_ (),
      argumentBounds_ (f.inputSize ()),
      scales_ (),
      argumentScales_ (f.inputSize ())
  {
    // Check that in the objective function m = 1 (R^n -> R).
    assert (f.outputSize () == 1);

    // Initialize bound.
    std::fill (argumentBounds_.begin (), argumentBounds_.end (),
	       Function::makeInfiniteInterval ());
    // Initialize scale.
    std::fill (argumentScales_.begin (), argumentScales_.end (), 1.);
  }

  template <typename F, typename CLIST>
  Problem<F, CLIST>::~Problem () throw ()
  {
  }

  // Copy constructor.
  template <typename F, typename CLIST>
  Problem<F, CLIST>::Problem (const Problem<F, CLIST>& pb) throw ()
    : function_ (pb.function_),
      startingPoint_ (pb.startingPoint_),
      constraints_ (pb.constraints_),
      bounds_ (pb.bounds_),
      argumentBounds_ (pb.argumentBounds_),
      scales_ (pb.scales_),
      argumentScales_ (pb.argumentScales_)
  {
  }

  // Copy constructor (convert from another class of problem).
  template <typename F, typename CLIST>
  template <typename F_, typename CLIST_>
  Problem<F, CLIST>::Problem (const Problem<F_, CLIST_>& pb) throw ()
    : function_ (pb.function_),
      startingPoint_ (pb.startingPoint_),
      constraints_ (),
      bounds_ (pb.bounds_),
      argumentBounds_ (pb.argumentBounds_),
      scales_ (pb.scales_),
      argumentScales_ (pb.argumentScales_)
  {
    // Check that F is a subtype of F_.
    BOOST_STATIC_ASSERT((boost::is_base_of<F, F_>::value));

    //FIXME: check that CLIST is a MPL vector of Function's sub-classes.

    std::copy (pb.constraints_.begin (), pb.constraints_.end (),
               constraints_.begin ());
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::function_t&
  Problem<F, CLIST>::function () const throw ()
  {
    return function_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::constraints_t&
  Problem<F, CLIST>::constraints () const throw ()
  {
    return constraints_;
  }

  template <typename F, typename CLIST>
  template <typename C>
  void
  Problem<F, CLIST>::addConstraint (boost::shared_ptr<C> x,
				    interval_t b,
				    value_type s)
    throw (std::runtime_error)
  {
    //FIXME: check that C is in CLIST.

    if (x->inputSize () != this->function ().inputSize ())
      throw std::runtime_error ("Invalid constraint (wrong input size)");
    if (x->outputSize () != 1)
      throw std::runtime_error
	("Invalid constraint (output size is not equal to one)");

    // Check that the pointer is not null.
    assert (!!x.get ());
    assert (b.first <= b.second);
    constraints_.push_back (boost::static_pointer_cast<C> (x));
    bounds_.push_back (b);
    scales_.push_back (s);
  }

  template <typename F, typename CLIST>
  template <typename C>
  void
  Problem<F, CLIST>::addConstraint (boost::shared_ptr<C> x,
                  intervals_t b,
                  value_type s)
  throw (std::runtime_error)
  {
  //FIXME: check that C is in CLIST.

    if (x->inputSize () != this->function ().inputSize ())
      throw std::runtime_error ("Invalid constraint (wrong input size)");
    if (x->outputSize () != b.size())
      throw std::runtime_error
    ("Invalid constraint (output size is not equal to one)");

    // Check that the pointer is not null.
    assert (!!x.get ());
    constraints_.push_back (boost::static_pointer_cast<C> (x));

    for(size_t i=0; i< b.size(); ++i)
      assert (b[i].first <= b[i].second);

    for(size_t i=0; i< b.size(); ++i)
    {
      bounds_.push_back (b[i]);
      scales_.push_back (s);
    }
  }

  template <typename F, typename CLIST>
  typename Problem<F, CLIST>::startingPoint_t&
  Problem<F, CLIST>::startingPoint () throw ()
  {
    if (startingPoint_ && startingPoint_->size ()
	!= this->function ().inputSize ())
      throw std::runtime_error ("Invalid starting point (wrong size)");
    return startingPoint_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::startingPoint_t&
  Problem<F, CLIST>::startingPoint () const throw ()
  {
    if (startingPoint_ && startingPoint_->size ()
	!= this->function ().inputSize ())
      throw std::runtime_error ("Invalid starting point (wrong size)");
    return startingPoint_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::intervals_t&
  Problem<F, CLIST>::bounds () const throw ()
  {
    return bounds_;
  }

  template <typename F, typename CLIST>
  typename Problem<F, CLIST>::intervals_t&
  Problem<F, CLIST>::argumentBounds () throw ()
  {
    return argumentBounds_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::intervals_t&
  Problem<F, CLIST>::argumentBounds () const throw ()
  {
    return argumentBounds_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::scales_t&
  Problem<F, CLIST>::scales () const throw ()
  {
    return scales_;
  }

  template <typename F, typename CLIST>
  typename Problem<F, CLIST>::scales_t&
  Problem<F, CLIST>::argumentScales () throw ()
  {
    return argumentScales_;
  }

  template <typename F, typename CLIST>
  const typename Problem<F, CLIST>::scales_t&
  Problem<F, CLIST>::argumentScales () const throw ()
  {
    return argumentScales_;
  }


  namespace detail
  {
    template <typename T>
    std::ostream&
    impl_print (std::ostream& o, const T* t)
    {
      return o << *t;
    }

    template <typename T>
    std::ostream&
    impl_print (std::ostream& o, const T& t)
    {
      return o << t;
    }
  }

  namespace detail
  {
    template <typename P>
    struct printConstraint : public boost::static_visitor<void>
    {
      printConstraint (std::ostream& o,
           const P& problem,
           Function::size_type ci,
           Function::size_type bi) :
      problem_ (problem),
      o_ (o),
      ci_(ci),
      bi_(bi)
      {}

      template <typename U>
      void operator () (const U& constraint)
      {
        assert (problem_.constraints ().size () - bi_ > 0);
        using namespace boost;
        o_ << iendl << incindent
           << "Constraint " << ci_ << incindent << iendl
           << *constraint << iendl;
        o_ << "Bounds: ";
        for(unsigned j=0; j<(*constraint).outputSize(); ++j)
          o_ << problem_.bounds ()[bi_+j];
        o_ << iendl;
        o_ << "Scales: ";
        for(unsigned j=0; j<(*constraint).outputSize(); ++j)
          o_ << problem_.scales ()[bi_+j] << "  ";
        o_ << iendl;

        if (problem_.startingPoint ())
        {
          U g = get<U> (problem_.constraints ()[ci_]);
          Function::vector_t x = (*g) (*problem_.startingPoint ());
          o_ << "Initial value: "
             << x;
          for(unsigned j=0; j<x.size(); ++j)
          {
            if (x[j] < Function::getLowerBound (problem_.bounds ()[bi_])
               || x[j] > Function::getUpperBound (problem_.bounds ()[bi_]))
              o_ << " (constraint not satisfied)";
          }
          o_ << iendl;
        }
        o_ << decindent << decindent;
      }
    private:
      const P& problem_;
      std::ostream& o_;
      Function::size_type ci_; //constraint index
      Function::size_type bi_; //bound index
    };
  } // end of namespace detail.

  template <typename F, typename CLIST>
  std::ostream&
  Problem<F, CLIST>::print (std::ostream& o) const throw ()
  {
    using namespace boost;

    o << "Problem:" << incendl;
    // Function.
    o << this->function () << iendl;

    // Arguments' bounds.
    o << "Argument's bounds: " << this->argumentBounds () << iendl;
    // Arguments' scales.
    o << "Argument's scales: " << this->argumentScales () << iendl;

    // Constraints.
    if (this->constraints ().empty ())
      o << "No constraints.";
    else
    {
      int numConstraints=0;
      for (unsigned ci = 0; ci < this->constraints ().size (); ++ci)
      {
        const constraint_t & c = (this->constraints())[ci];
        boost::shared_ptr<F> g = get< boost::shared_ptr<F> > (c);
        numConstraints += g->outputSize();
      }
      o << "Number of constraints: " << numConstraints << iendl;
    }

    //ci is the index of the current constraint
    //bi is the index of the current bound
    //(the bi-th bound can correspond to the k-th value of the ci-th constraint)
    Function::size_type bi = 0;
    for (unsigned ci = 0; ci < this->constraints ().size (); ++ci)
    {
      detail::printConstraint<Problem<F, CLIST> > pc (o, *this, ci, bi);
      boost::apply_visitor (pc, this->constraints ()[ci]);
      const constraint_t & c = (this->constraints())[ci];
      boost::shared_ptr<F> g = get< boost::shared_ptr<F> > (c);
      bi += g->outputSize();
    }

    // Starting point.
    if (startingPoint_)
      {
	o << iendl << "Starting point: " << *startingPoint_
	  << iendl << "Starting value: "
	  << this->function () (*startingPoint_);
      }
    else
      o << iendl << "No starting point.";

    // Infinity.
    o << iendl << "Infinity value (for all functions): "
      << Function::infinity ();
    return o << decindent;
  }

  template <typename F, typename CLIST>
  std::ostream&
  operator<< (std::ostream& o, const Problem<F, CLIST>& pb)
  {
    return pb.print (o);
  }
} // end of namespace roboptim
#endif //! ROBOPTIM_CORE_PROBLEM_HH
