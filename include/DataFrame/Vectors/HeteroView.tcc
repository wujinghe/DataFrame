// Hossein Moein
// October 24, 2018
/*
Copyright (c) 2019-2022, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the DataFrame nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Hossein Moein BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <DataFrame/Vectors/HeteroVector.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

// ----------------------------------------------------------------------------

namespace hmdf
{

template<typename T>
HeteroView::HeteroView(T *begin_ptr, T *end_ptr)
    : clear_function_([](HeteroView &hv) { views_<T>.erase(&hv); }),
      copy_function_([](const HeteroView &from, HeteroView &to)  {
              views_<T>[&to] = views_<T>[&from]; }),
      move_function_([](HeteroView &from, HeteroView &to)  {
              views_<T>[&to] = std::move(views_<T>[&from]); })  {

    views_<T>.emplace(this, VectorView<T>(begin_ptr, end_ptr));
}

// ----------------------------------------------------------------------------

template<typename T>
void HeteroView::set_begin_end_special(T *bp, T *ep_1)  {

    clear_function_ = [](HeteroView &hv) { views_<T>.erase(&hv); };
    copy_function_  = [](const HeteroView &from, HeteroView &to)  {
                          views_<T>[&to] = views_<T>[&from];
                      };
    move_function_ = [](HeteroView &from, HeteroView &to)  {
                         views_<T>[&to] = std::move(views_<T>[&from]);
                     };

    VectorView<T>   vv;

    vv.set_begin_end_special(bp, ep_1);
    views_<T>.emplace(this, vv);
}

// ----------------------------------------------------------------------------

template<typename T>
VectorView<T> &HeteroView::get_vector()  {

    auto    iter = views_<T>.find (this);

    if (iter == views_<T>.end())
        throw std::runtime_error("HeteroView::get_vector(): ERROR: "
                                 "Cannot find view");

    return (iter->second);
}

// ----------------------------------------------------------------------------

template<typename T>
const VectorView<T> &HeteroView::get_vector() const  {

    return (const_cast<HeteroView *>(this)->get_vector<T>());
}

// ----------------------------------------------------------------------------

template<typename T, typename U>
void HeteroView::visit_impl_help_ (T &visitor)  {

    auto    iter = views_<U>.find (this);

    if (iter != views_<U>.end())
        for (auto &&element : iter->second)
            visitor(element);
}

// ----------------------------------------------------------------------------

template<typename T, typename U>
void HeteroView::visit_impl_help_ (T &visitor) const  {

    const auto  citer = views_<U>.find (this);

    if (citer != views_<U>.end())
        for (auto &&element : citer->second)
            visitor(element);
}

// ----------------------------------------------------------------------------

template<typename T, typename U>
void HeteroView::sort_impl_help_ (T &functor)  {

    auto    iter = views_<U>.find (this);

    if (iter != views_<U>.end())
        std::sort (iter->second.begin(), iter->second.end(), functor);
}

// ----------------------------------------------------------------------------

template<typename T, typename U>
void HeteroView::change_impl_help_ (T &functor)  {

    auto    iter = views_<U>.find (this);

    if (iter != views_<U>.end())
        functor(iter->second);
}

// ----------------------------------------------------------------------------

template<typename T, typename U>
void HeteroView::change_impl_help_ (T &functor) const  {

    const auto  citer = views_<U>.find (this);

    if (citer != views_<U>.end())
        functor(citer->second);
}

// ----------------------------------------------------------------------------

template<class T, template<class...> class TLIST, class... TYPES>
void HeteroView::visit_impl_ (T &&visitor, TLIST<TYPES...>)  {

    // (..., visit_impl_help_<std::decay_t<T>, TYPES>(visitor)); // C++17
    using expander = int[];
    (void) expander { 0, (visit_impl_help_<T, TYPES>(visitor), 0) ... };
}

// ----------------------------------------------------------------------------

template<class T, template<class...> class TLIST, class... TYPES>
void HeteroView::visit_impl_ (T &&visitor, TLIST<TYPES...>) const  {

    // (..., visit_impl_help_<std::decay_t<T>, TYPES>(visitor)); // C++17
    using expander = int[];
    (void) expander { 0, (visit_impl_help_<T, TYPES>(visitor), 0) ... };
}

// ----------------------------------------------------------------------------

template<class T, template<class...> class TLIST, class... TYPES>
void HeteroView::sort_impl_ (T &&functor, TLIST<TYPES...>)  {

    using expander = int[];
    (void) expander { 0, (sort_impl_help_<T, TYPES>(functor), 0) ... };
}

// ----------------------------------------------------------------------------

template<class T, template<class...> class TLIST, class... TYPES>
void HeteroView::change_impl_ (T &&functor, TLIST<TYPES...>)  {

    using expander = int[];
    (void) expander { 0, (change_impl_help_<T, TYPES>(functor), 0) ... };
}

// ----------------------------------------------------------------------------

template<class T, template<class...> class TLIST, class... TYPES>
void HeteroView::change_impl_ (T &&functor, TLIST<TYPES...>) const  {

    using expander = int[];
    (void) expander { 0, (change_impl_help_<T, TYPES>(functor), 0) ... };
}

// ----------------------------------------------------------------------------

template<typename T>
bool HeteroView::empty() const noexcept  {

    return (get_vector<T>().empty ());
}

// ----------------------------------------------------------------------------

template<typename T>
T &HeteroView::at(size_type idx)  {

    return (get_vector<T>()[idx]);
}

// ----------------------------------------------------------------------------

template<typename T>
const T &HeteroView::at(size_type idx) const  {

    return (get_vector<T>()[idx]);
}

// ----------------------------------------------------------------------------

template<typename T>
T &HeteroView::back()  { return (get_vector<T>().back ()); }

// ----------------------------------------------------------------------------

template<typename T>
const T &HeteroView::back() const  { return (get_vector<T>().back ()); }

// ----------------------------------------------------------------------------

template<typename T>
T &HeteroView::front()  { return (get_vector<T>().front ()); }

// ----------------------------------------------------------------------------

template<typename T>
const T &HeteroView::front() const  { return (get_vector<T>().front ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::iterator<T>
HeteroView::begin()  { return (get_vector<T>().begin ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::const_iterator<T>
HeteroView::begin() const  { return (get_vector<T>().begin ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::iterator<T>
HeteroView::end()  { return (get_vector<T>().end ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::const_iterator<T>
HeteroView::end() const  { return (get_vector<T>().end ()); }


// ----------------------------------------------------------------------------

template<typename T>
HeteroView::reverse_iterator<T>
HeteroView::rbegin()  { return (get_vector<T>().rbegin ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::const_reverse_iterator<T>
HeteroView::rbegin() const  { return (get_vector<T>().rbegin ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::reverse_iterator<T>
HeteroView::rend()  { return (get_vector<T>().rend ()); }

// ----------------------------------------------------------------------------

template<typename T>
HeteroView::const_reverse_iterator<T>
HeteroView::rend() const  { return (get_vector<T>().rend ()); }

} // namespace hmdf

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
