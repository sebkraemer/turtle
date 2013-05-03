// http://turtle.sourceforge.net
//
// Copyright Mathieu Champlon 2011
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MOCK_STREAM_HPP_INCLUDED
#define MOCK_STREAM_HPP_INCLUDED

#include "config.hpp"
#include <boost/utility/addressof.hpp>
#include <boost/noncopyable.hpp>
#include <ostream>

namespace mock
{
    struct stream
    {
        explicit stream( std::ostream& s )
            : s_( &s )
        {}
        std::ostream* s_;
    };

#ifdef MOCK_USE_CONVERSIONS

namespace detail
{
namespace conversion
{
    struct sink
    {
        template< typename T >
        sink( const T& )
        {}
    };

    inline std::ostream& operator<<( std::ostream& s, const sink& )
    {
        return s << '?';
    }

    struct holder : boost::noncopyable
    {
        virtual ~holder()
        {}
        virtual void serialize( std::ostream& s ) const = 0;
    };

    template< typename T >
    struct holder_imp : holder
    {
        explicit holder_imp( const T& t )
            : t_( boost::addressof( t ) )
        {}
        virtual void serialize( std::ostream& s ) const
        {
            // if an error about an ambiguous conversion is generated by the
            // line below the solution is to add a serialization operator to a
            // mock::stream for T
            s << *t_;
        }
        const T* t_;
    };

    struct any : boost::noncopyable
    {
        template< typename T >
        any( const T& t )
            : h_( new holder_imp< T >( t ) )
        {}
        ~any()
        {
            delete h_;
        }
        holder* h_;
    };
}
}

    inline stream& operator<<( stream& s, const detail::conversion::any& d )
    {
        d.h_->serialize( *s.s_ );
        return s;
    }

#else // MOCK_USE_CONVERSIONS

namespace detail
{
namespace conversion
{
    template< typename S, typename T >
    S& operator<<( S &s, const T& )
    {
        return s << '?';
    }
}
}

    template< typename T >
    stream& operator<<( stream& s, const T& t )
    {
        using namespace detail::conversion;
        *s.s_ << t;
        return s;
    }

#endif // MOCK_USE_CONVERSIONS

namespace detail
{
    template< typename T >
    void serialize( stream& s, const T& t )
    {
        // if an error about an ambiguous conversion is generated by the
        // line below the solution is to add a serialization operator to a
        // mock::stream for T
        s << t;
    }
    inline void serialize( stream& s, bool b )
    {
        s << (b ? "true" : "false");
    }
    template< typename C, typename T, typename A >
    void serialize( stream& s, const std::basic_string< C, T, A >& str )
    {
        s << '"' << str << '"';
    }
    inline void serialize( stream& s, const char* const str )
    {
        s << '"' << str << '"';
    }
    inline void serialize( stream& s, unsigned char c )
    {
        s << static_cast< int >( c );
    }
}
} // mock

#endif // MOCK_STREAM_HPP_INCLUDED
