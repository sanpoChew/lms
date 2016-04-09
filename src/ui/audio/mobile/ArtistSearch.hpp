/*
 * Copyright (C) 2015 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UI_MOBILE_ARTIST_SEARCH_HPP
#define UI_MOBILE_ARTIST_SEARCH_HPP

#include <Wt/WContainerWidget>

#include "database/Types.hpp"
#include "database/SearchFilter.hpp"

namespace UserInterface {
namespace Mobile {

class ArtistSearch : public Wt::WContainerWidget
{
	public:

		ArtistSearch(Wt::WString title, Wt::WContainerWidget *parent = 0);

		void search(Database::SearchFilter filter, std::size_t nb);
		void addResults(std::size_t nb);

		// Slots
		Wt::Signal<void>& showMore() { return _sigShowMore;}

	private:

		Wt::Signal<void> _sigShowMore;

		void clear(void);

		Wt::WTemplate*	_showMore;

		Database::SearchFilter	_filter;
		Wt::WContainerWidget*	_contents;
		std::size_t		_count;
};

} // namespace Mobile
} // namespace UserInterface

#endif
