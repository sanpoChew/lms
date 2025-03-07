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

#include "ArtistView.hpp"

#include <Wt/WAnchor.h>
#include <Wt/WImage.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#include "database/Artist.hpp"
#include "database/Release.hpp"

#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

#include "resource/ImageResource.hpp"

#include "LmsApplication.hpp"
#include "Filters.hpp"

using namespace Database;

namespace UserInterface {

Artist::Artist(Filters* filters)
: _filters(filters)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();

	filters->updated().connect(std::bind([=] {
		refresh();
	}));
}

void
Artist::refresh()
{
	if (!wApp->internalPathMatches("/artist/"))
		return;

	clear();

	auto artistId = readAs<Database::IdType>(wApp->internalPathNextPart("/artist/"));
	if (!artistId)
		return;

        Wt::Dbo::Transaction transaction(LmsApp->getDboSession());
	auto artist = Database::Artist::getById(LmsApp->getDboSession(), *artistId);

	if (!artist)
	{
		LmsApp->goHome();
		return;
	}

	Wt::WTemplate* t = addNew<Wt::WTemplate>(Wt::WString::tr("Lms.Explore.Artist.template"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	Wt::WContainerWidget* clusterContainers = t->bindNew<Wt::WContainerWidget>("clusters");

	{
		auto clusterTypes = ScanSettings::get(LmsApp->getDboSession())->getClusterTypes();
		auto clusterGroups = artist->getClusterGroups(clusterTypes, 3);

		for (auto clusters : clusterGroups)
		{
			for (auto cluster : clusters)
			{
				auto clusterId = cluster.id();
				auto entry = clusterContainers->addWidget(LmsApp->createCluster(cluster));
				entry->clicked().connect([=]
				{
					_filters->add(clusterId);
				});
			}
		}
	}

	t->bindString("name", Wt::WString::fromUTF8(artist->getName()), Wt::TextFormat::Plain);
	{
		Wt::WText* playBtn = t->bindNew<Wt::WText>("play-btn", Wt::WString::tr("Lms.Explore.template.play-btn"), Wt::TextFormat::XHTML);

		playBtn->clicked().connect(std::bind([=]
		{
			artistPlay.emit(*artistId);
		}));
	}

	{
		Wt::WText* addBtn = t->bindNew<Wt::WText>("add-btn", Wt::WString::tr("Lms.Explore.template.add-btn"), Wt::TextFormat::XHTML);

		addBtn->clicked().connect(std::bind([=]
		{
			artistAdd.emit(*artistId);
		}));
	}

	Wt::WContainerWidget* releasesContainer = t->bindNew<Wt::WContainerWidget>("releases");

	auto releases = artist->getReleases(_filters->getClusterIds());

	for (auto release : releases)
	{
		auto releaseId = release.id();

		Wt::WTemplate* entry = releasesContainer->addNew<Wt::WTemplate>(Wt::WString::tr("Lms.Explore.Artist.template.entry"));
		entry->addFunction("tr", Wt::WTemplate::Functions::tr);

		{
			Wt::WAnchor* anchor = entry->bindWidget("cover", LmsApplication::createReleaseAnchor(release, false));

			auto cover = std::make_unique<Wt::WImage>();
			cover->setImageLink(LmsApp->getImageResource()->getReleaseUrl(release.id(), 128));
			// Some images may not be square
			cover->setWidth(128);
			anchor->setImage(std::move(cover));
		}

		entry->bindWidget("name", LmsApplication::createReleaseAnchor(release));

		auto artists {release->getReleaseArtists()};
		LMS_LOG(UI, DEBUG) << "Found " << artists.size() << " release artists";

		for (auto artist : artists)
			LMS_LOG(UI, DEBUG) << "\tArtist = '" << artist->getName() << "'";

		if (artists.empty())
			artists = release->getArtists();

		bool isSameArtist {(std::find(std::cbegin(artists), std::cend(artists), artist) != artists.end())};

		if (artists.size() > 1)
		{
			entry->setCondition("if-has-artist", true);
			entry->bindNew<Wt::WText>("artist", Wt::WString::tr("Lms.Explore.various-artists"));
		}
		else if (artists.size() == 1 && !isSameArtist)
		{
			entry->setCondition("if-has-artist", true);
			entry->bindWidget("artist", LmsApplication::createArtistAnchor(artists.front()));
		}

		boost::optional<int> year = release->getReleaseYear();
		if (year)
		{
			entry->setCondition("if-has-year", true);
			entry->bindInt("year", *year);

			boost::optional<int> originalYear = release->getReleaseYear(true);
			if (originalYear && *originalYear != *year)
			{
				entry->setCondition("if-has-orig-year", true);
				entry->bindInt("orig-year", *originalYear);
			}
		}

		Wt::WText* playBtn = entry->bindNew<Wt::WText>("play-btn", Wt::WString::tr("Lms.Explore.template.play-btn"), Wt::TextFormat::XHTML);
		playBtn->clicked().connect(std::bind([=]
		{
			releasePlay.emit(releaseId);
		}));

		Wt::WText* addBtn = entry->bindNew<Wt::WText>("add-btn", Wt::WString::tr("Lms.Explore.template.add-btn"), Wt::TextFormat::XHTML);
		addBtn->clicked().connect(std::bind([=]
		{
			releaseAdd.emit(releaseId);
		}));
	}
}

} // namespace UserInterface

