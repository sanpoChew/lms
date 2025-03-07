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

#pragma once

#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WSignal.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#include <boost/optional.hpp>

#include "database/TrackList.hpp"
#include "database/Track.hpp"

namespace Similarity
{
	class Finder;
}

namespace UserInterface {

class PlayQueue : public Wt::WTemplate
{
	public:
		PlayQueue();

		void addTracks(const std::vector<Database::Track::pointer>& tracks);
		void playTracks(const std::vector<Database::Track::pointer>& tracks);

		// play the next track in the queue
		void playNext();

		// play the previous track in the queue
		void playPrevious();

		// Signal emitted when a track is to be load(and optionally played)
		Wt::Signal<Database::IdType /*trackId*/, bool /*play*/> loadTrack;

		// Signal emitted when play has to be stopped
		Wt::Signal<> trackUnload;

	private:
		Database::TrackList::pointer getTrackList();

		void clearTracks();
		void enqueueTracks(const std::vector<Database::Track::pointer>& tracks);
		void enqueueTrack(Database::Track::pointer track);
		void addSome();
		void addRadioTrack();
		void updateInfo();
		void updateCurrentTrack(bool selected);
		void updateRepeatBtn();
		void updateRadioBtn();

		void load(std::size_t pos, bool play);
		void stop();

		void addRadioTrackFromSimilarity(std::shared_ptr<Similarity::Finder> similarityFinder);
		void addRadioTrackFromClusters();

		bool _repeatAll = false;
		bool _radioMode = false;
		boost::optional<Database::IdType> _tracklistId;
		Wt::WContainerWidget* _entriesContainer = nullptr;
		Wt::WPushButton* _showMore = nullptr;
		Wt::WText* _nbTracks = nullptr;
		Wt::WText* _repeatBtn = nullptr;
		Wt::WText* _radioBtn = nullptr;
		boost::optional<std::size_t> _trackPos;	// current track position, if set
};

} // namespace UserInterface

