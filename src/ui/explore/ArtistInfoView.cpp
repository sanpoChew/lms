/*
 * Copyright (C) 2018 Emeric Poupon
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

#include "ArtistInfoView.hpp"

#include "database/Artist.hpp"
#include "main/Service.hpp"
#include "similarity/SimilaritySearcher.hpp"
#include "utils/Utils.hpp"

#include "ArtistLink.hpp"
#include "LmsApplication.hpp"

using namespace Database;

namespace UserInterface {

ArtistInfo::ArtistInfo()
: Wt::WTemplate(Wt::WString::tr("Lms.Explore.ArtistInfo.template"))
{
	addFunction("tr", &Wt::WTemplate::Functions::tr);

	_similarArtistsContainer = bindNew<Wt::WContainerWidget>("similar-artists");

	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	LmsApp->getEvents().dbScanned.connect([=]
	{
		refresh();
	});

	refresh();
}

void
ArtistInfo::refresh()
{
	_similarArtistsContainer->clear();

	if (!wApp->internalPathMatches("/artist/"))
		return;

	auto artistId = readAs<Database::IdType>(wApp->internalPathNextPart("/artist/"));
	if (!artistId)
		return;

	auto artistsIds = getService<Similarity::Searcher>()->getSimilarArtists(LmsApp->getDboSession(), *artistId, 5);

	Wt::Dbo::Transaction transaction(LmsApp->getDboSession());

	std::vector<Database::Artist::pointer> artists;
	for (auto artistId : artistsIds)
	{
		auto artist = Database::Artist::getById(LmsApp->getDboSession(), artistId);

		if (artist)
			artists.push_back(artist);
	}

	for (auto artist : artists)
		_similarArtistsContainer->addNew<ArtistLink>(artist);
}

} // namespace UserInterface

