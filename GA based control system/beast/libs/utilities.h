/**
 * \file utilities.h
 * \author David Gordon
 * \addtogroup utilities
 * This file defines a number of useful functions and functors. 
 */

#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <typeinfo>
#include <functional>

namespace BEAST {

/**
 * \addtogroup utilities Utilities and Helper Functions
 * @{
 */

/**
 * Class wrapper for a member variable which allows member data to be exposed
 * with invisible get/set semantics.
 * \note A slight speed penalty is incurred by the time taken to dereference
 * the accessor/mutator functions, if faster access is required the actual
 * accessor functions should be used.
 */
template <class _Owner, typename _Type, typename _In = _Type, typename _Out = _In>
class property
{
public:
	property()
	{
	}

	property(_Owner* t, _Out (_Owner::*get)(void)const, void (_Owner::*set)(_In)):
	that(t), _Get(get), _Set(set)
	{
	}

	void init(_Owner* t, _Out (_Owner::*get)(void)const, void (_Owner::*set)(_In))
	{
		that = t;
		_Get = get;
		_Set = set;
	}

	property& operator=(_In v)
	{
		(that->*_Set)(v);
		return *this;
	}

	operator _Out()
	{
		return (that->*_Get)();
	}

	_Type* operator->()const
	{
		return &((that->*_Get)());
	}

	template<typename _CastType>
	_CastType as()const
	{
		return static_cast<_CastType>((that->*_Get)());
	}

private:
	_Owner* that;
	_Out (_Owner::*_Get)(void)const;
	void (_Owner::*_Set)(_In);
};

template <class _Owner, typename _Type, typename _In = _Type, typename _Out = _In>
class pointer_property : public property<_Owner, _Type, _In, _Out>
{
	pointer_property()
	{
	}

	pointer_property(_Owner* t, _Out (_Owner::*get)(void)const, void (_Owner::*set)(_In)):
		property<_Owner, _Type, _In, _Out>(t, get, set)
	{
	}

	_Type operator->()const
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
		return ((this->that->*this->_Get)());
	}

	template<typename _CastType>
	_CastType as()const
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
		return dynamic_cast<_CastType>((this->that->*this->_Get)());
	}
};

template <typename _Type, typename _In = _Type, typename _Out = _In>
class auto_property
{
public:
	auto_property(_Type& v): value(v)
	{
	}

	auto_property& operator=(_In v)
	{
		value = v;
		return *this;
	}

	operator _Out()const
	{
		return value;
	}

	_Type* operator->()const
	{
		return &value;
	}

	template<typename _CastType>
	_CastType as()const
	{
		return static_cast<_CastType>(value);
	}

protected:
	_Type& value;
};

template <typename _Type, typename _In = _Type, typename _Out = _In>
class auto_pointer_property : public auto_property<_Type, _In, _Out>
{
public:
	auto_pointer_property(_Type& v): auto_property<_Type, _In, _Out>(v)
	{}

	_Type operator->()const
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
		return this->value;
	}

	template<typename _CastType>
	_CastType as()const
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
		return dynamic_cast<_CastType>(this->value);
	}
};

template <typename _Type, typename _In = _Type, typename _Out = _In, typename _Key = typename _Type::key_type,
		  typename _Data = typename _Type::mapped_type, typename _Output = auto_property<_Data> >
class auto_indexed_property : public auto_property<_Type, _In, _Out>
{
public:
	auto_indexed_property(_Type& v): auto_property<_Type, _In, _Out>(v)
	{
	}

	_Output operator[](_Key k)
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
	        return _Output((this->value)[k]);
	}
};

template <typename _Type, typename _In = _Type, typename _Out = _In, typename _Key = typename _Type::key_type,
		  typename _Data = typename _Type::mapped_type, typename _Output = auto_pointer_property<_Data> >
class auto_indexed_pointer_property : public auto_property<_Type, _In, _Out>
{
public:
	auto_indexed_pointer_property(_Type& v): auto_property<_Type, _In, _Out>(v)
	{
	}

	_Output operator[](_Key k)
	{
	  // MdK: 02-1-07: added this pointer to make protected template member visible
		return _Output(this->value[k]);
	}
};

/**
 * Version of the STL accumulate algorithm which computes a sum of all the
 * results of a unary function _Func applied to the values between _First
 * and _Last.
 */
template<class _InIt, class _Ty, class _Fn1> inline
_Ty accumulate_fun(_InIt _First, _InIt _Last, _Ty _Val, _Fn1 _Func)
{	
	for (; _First != _Last; ++_First)
		_Val += _Func(*_First);
	return (_Val);
}

/**
 * Allows us to bind functors so that they work on particular members of
 * classes, useful for using for_each on maps.
 */
template <class T, typename M, class OP>
class call_on_mem_t : std::unary_function<T, typename OP::result_type>
{
public:
	typedef M argument_type;
	typedef typename OP::result_type result_type;

	explicit call_on_mem_t(M T:: *m, OP op):
		m_(m), op_(op) {}

	result_type operator()(T &val)
	{
		return op_(val.*m_);
	}
private:
	M T::* m_;
	OP op_;
};

/**
 * Helper function for constructing call_on_mem_t function objects.
 */
template <class T, typename M, class OP>
call_on_mem_t<T, M, OP>
call_on_mem(M T::*m, OP op)
{
	return call_on_mem_t<T, M, OP>(m, op);
}

/**
 * A functor which creates a unary function from a unary member function,
 * binding an instance of the class to which the function belongs.
 * Equivalent to bind1st(mem_fun(class::fun)) which is not valid in
 * standard C++.
 */
template <class _Class, typename _Return, typename _Arg>
struct bound_mem_fun_t : std::unary_function<_Return, _Arg>
{
	bound_mem_fun_t(_Class& t, _Return (_Class::*m)(_Arg)):
	that(t), memfun(m) {}
	_Return operator()(_Arg a)
	{
		return (that.*memfun)(a);
	}

	_Return (_Class::*memfun)(_Arg);
	_Class& that;
};

/**
 * A helper function for constructing bound_mem_fun_t objects
 */
template <class _Class, typename _Return, typename _Arg>
inline bound_mem_fun_t<_Class, _Return, _Arg> bound_mem_fun(_Class& c, _Return (_Class::*memfun)(_Arg))
{
	return bound_mem_fun_t<_Class, _Return, _Arg>(c, memfun);
}

/**
 * A helper function for constructing bound_mem_fun_t objects
 */
template <class _Class, typename _Return, typename _Arg>
inline bound_mem_fun_t<_Class, _Return, _Arg> bound_mem_fun(_Class* c, _Return (_Class::*memfun)(_Arg))
{
	return bound_mem_fun_t<_Class, _Return, _Arg>(*c, memfun);
}


/**
 * A functor for use with the for_each algorithm which can perform creation
 * of objects when called on a container of pointers.
 */
template <typename T>
struct creator : public std::unary_function<void, T*> {
	T* operator()()
	{
		return new T;
	}
};

/**
 * A functor for use with the for_each algorithm which can perform deletion
 * of objects when called on a container of pointers.
 */
template <typename T>
struct deleter : public std::unary_function<T*, void> {
	void operator()(T* obj)
	{
		delete obj;
	}
};

/**
 * A wrapper for RTTI (RunTime Type Identification) typeid, which checks if
 * two pointers are of identical types.
 * \param in A pointer to the object being tested.
 * \param out A pointer of the type required, which will be set to point to
 * the same object as the first parameter if it turns out they are of
 * identical types.
 * \return True if a match is made.
 * \see IsKindOf
 */
template <typename _Type, typename _Base>
inline bool IsA(_Base* in, _Type*& out)
{
	if (in == NULL) return false;

	if (typeid(*in) == typeid(_Type)) {
		out = static_cast<_Type*>(in);
		return true;
	}
	return false;
}

/**
 * A wrapper for RTTI (RunTime Type Identification) using dynamic_cast which
 * checks if an object is of the same type or is in herited from an object of
 * the same type as an input pointer.
 * \param in A pointer to the object being tested.
 * \param out A pointer of the type being tested for, which will be set to
r is in herited from an object of
 * the same type as an input pointer.

 * point to the same object as the first paramter if it turns out it is of
 * the same or an inherited type.
 * \return True if a match is made.
 */
template <typename _Type, typename _Base>
inline bool IsKindOf(_Base* in, _Type*& out)
{
	if (in == NULL) return false;

	out = dynamic_cast<_Type*>(in);
	return out != NULL;
}

/**
 * Takes a type, a lower and an upper limit and bounds the input value to
 * those limits. Useful for angles.
 * \param T The type of value.
 * \param L The lower limit.
 * \param U The upper limit.
 * \param n The input value.
 * \return The output value.
 */
template <typename T>
inline T bound(T L, T U, T n)
{
	while (n >= U) n -= (U - L);
	while (n < L) n += (U - L);
	return n;
}

/**
 * A version of bound which takes a reference as its argument.
 * \param T The type of value.
 * \param L The lower limit.
 * \param U The upper limit.
 * \param n The input value.
 */
template <typename T>
inline void rbound(T L, T U, T& n)
{
	while (n >= U) n -= (U - L);
	while (n < L) n += (U - L);
}

/**
 * Limits the input value to the specified range, clipping at either extreme.
 * \param T The type of values to work on.
 * \param L The lower limit.
 * \param U The upper limit.
 * \param n The input value.
 * \return The output value.
 */
template <typename T>
inline T limit(T L, T U, T n)
{
	if (n > U) return U;
	else if (n < L) return L;
	else return n;
}

/**
 * A version of limit which takes a reference as its argument.
 * \param T The type of values to work on.
 * \param L The lower limit.
 * \param U The upper limit.
 * \param n The input value.
 * \return The output value.
 */
template <typename T>
inline void rlimit(T L, T U, T& n)
{
	if (n > U) n = U;
	else if (n < L) n = L;
}

/**
 * @}
 */

} // namespace BEAST

#endif
