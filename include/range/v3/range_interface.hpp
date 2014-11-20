// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//
#ifndef RANGES_V3_RANGE_INTERFACE_HPP
#define RANGES_V3_RANGE_INTERFACE_HPP

#include <array>
#include <range/v3/range_fwd.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/utility/concepts.hpp>
#include <range/v3/utility/iterator.hpp>
#include <range/v3/utility/common_iterator.hpp>

namespace ranges
{
    inline namespace v3
    {
        namespace detail
        {
            template<typename Rng, typename Cont, typename I = range_common_iterator_t<Rng>>
            using ConvertibleToContainer = fast_logical_and<
                Iterable<Cont>,
                logical_not<Range<Cont>>,
                Movable<Cont>,
                Convertible<range_value_t<Rng>, range_value_t<Cont>>,
                Constructible<Cont, I, I>>;

            template<typename From, typename To = From>
            struct slice_bounds
            {
                From from;
                To to;
                slice_bounds(From from, To to)
                  : from(from), to(to)
                {}
            };
        }

        // BUGBUG switch to variable template if it's available
        template<typename Container, typename Rng,
            CONCEPT_REQUIRES_(detail::ConvertibleToContainer<Rng, Container>())>
        Container to_container(Rng && rng)
        {
            static_assert(!is_infinite<Rng>::value,
                "Attempt to convert an infinite range to a container.");
            using I = range_common_iterator_t<Rng>;
            return Container{I{begin(rng)}, I{end(rng)}};
        }

        template<typename Derived, bool Inf /* = false*/>
        struct range_interface
          : private basic_range<Inf>
        {
        private:
            Derived & derived()
            {
                return static_cast<Derived &>(*this);
            }
            Derived const & derived() const
            {
                return static_cast<Derived const &>(*this);
            }
        public:
            // A few ways of testing whether a range can be empty:
            bool empty() const
            {
                return derived().begin() == derived().end();
            }
            bool operator!() const
            {
                return empty();
            }
            explicit operator bool() const
            {
                return !empty();
            }
            // Access the size of the range, if it can be determined:
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() &&
                    SizedIteratorRange<range_iterator_t<D>, range_sentinel_t<D>>())>
            range_size_t<D> size() const
            {
                return iter_size(derived().begin(), derived().end());
            }
            // Access the first element in a range:
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>())>
            range_reference_t<D> front()
            {
                return *derived().begin();
            }
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>())>
            range_reference_t<D const> front() const
            {
                return *derived().begin();
            }
            // Access the last element in a range:
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && BoundedRange<D>() && BidirectionalRange<D>())>
            range_reference_t<D> back()
            {
                return *prev(derived().end());
            }
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && BoundedRange<D const>() && BidirectionalRange<D const>())>
            range_reference_t<D const> back() const
            {
                return *prev(derived().end());
            }
            // Simple indexing:
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && RandomAccessRange<D>())>
            auto operator[](range_difference_t<D> n) ->
                decltype(std::declval<D &>().begin()[n])
            {
                return derived().begin()[n];
            }
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && RandomAccessRange<D const>())>
            auto operator[](range_difference_t<D> n) const ->
                decltype(std::declval<D const &>().begin()[n])
            {
                return derived().begin()[n];
            }
            // Python-ic slicing with rng[{from,to}]
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && InputRange<D>())>
            sliced_view<D &>
            operator[](detail::slice_bounds<range_difference_t<D>> offs)
            {
                return sliced_view<D &>{derived(), offs.from, offs.to};
            }
            template<typename D = Derived,
                CONCEPT_REQUIRES_(Same<D, Derived>() && InputRange<D const>())>
            sliced_view<D const &>
            operator[](detail::slice_bounds<range_difference_t<D>> offs) const
            {
                return sliced_view<D const &>{derived(), offs.from, offs.to};
            }
            // Implicit conversion to something that looks like a container:
            template<typename Container, typename D = Derived,
                typename Alloc = typename Container::allocator_type, // HACKHACK
                CONCEPT_REQUIRES_(detail::ConvertibleToContainer<D, Container>())>
            operator Container ()
            {
                return ranges::to_container<Container>(derived());
            }
            template<typename Container, typename D = Derived,
                typename Alloc = typename Container::allocator_type, // HACKHACK
                CONCEPT_REQUIRES_(detail::ConvertibleToContainer<D const, Container>())>
            operator Container () const
            {
                return ranges::to_container<Container>(derived());
            }
        };
    }
}

#endif
