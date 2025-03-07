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

#include "MediaPlayer.hpp"

#include "av/AvInfo.hpp"
#include "utils/Logger.hpp"

#include "database/Artist.hpp"
#include "database/Release.hpp"
#include "database/Track.hpp"

#include "resource/ImageResource.hpp"
#include "resource/AudioResource.hpp"

#include "LmsApplication.hpp"

namespace UserInterface {

MediaPlayer::MediaPlayer()
: Wt::WTemplate(Wt::WString::tr("Lms.MediaPlayer.template")),
  playbackEnded(this, "playbackEnded"),
  playPrevious(this, "playPrevious"),
  playNext(this, "playNext")
{
	_title = bindNew<Wt::WText>("title");
	_title->setTextFormat(Wt::TextFormat::Plain);

	_artist = bindNew<Wt::WAnchor>("artist");
	_artist->setTextFormat(Wt::TextFormat::Plain);

	_release = bindNew<Wt::WAnchor>("release");
	_release->setTextFormat(Wt::TextFormat::Plain);

	wApp->doJavaScript("LMS.mediaplayer.init(" + jsRef() + ")");

	LmsApp->getEvents().trackLoaded.connect(this, &MediaPlayer::loadTrack);
	LmsApp->getEvents().trackUnloaded.connect(this, &MediaPlayer::stop);
}

void
MediaPlayer::loadTrack(Database::IdType trackId, bool play)
{
	LMS_LOG(UI, DEBUG) << "Playing track ID = " << trackId;

	Wt::Dbo::Transaction transaction(LmsApp->getDboSession());
	auto track = Database::Track::getById(LmsApp->getDboSession(), trackId);

	try
	{
		Av::MediaFile mediaFile(track->getPath());

		auto resource = LmsApp->getAudioResource()->getUrl(trackId);
		auto imgResource = LmsApp->getImageResource()->getTrackUrl(trackId, 64);

		std::ostringstream oss;
		oss
			<< "var params = {"
			<< " resource: \"" << resource << "\","
			<< " duration: " << std::chrono::duration_cast<std::chrono::seconds>(track->getDuration()).count() << ","
			<< " imgResource: \"" << imgResource << "\","
			<< "};";
		oss << "LMS.mediaplayer.loadTrack(params, " << (play ? "true" : "false") << ")"; // true to autoplay

		LMS_LOG(UI, DEBUG) << "Running js = '" << oss.str() << "'";

		_title->setText(Wt::WString::fromUTF8(track->getName()));

		auto artists = track->getArtists();
		if (!artists.empty())
		{
			_artist->setText(Wt::WString::fromUTF8(artists.front()->getName()));
			_artist->setLink(LmsApp->createArtistLink(artists.front()));
		}
		else
		{
			_artist->setText("");
			_artist->setLink(Wt::WLink());
		}

		if (track->getRelease())
		{
			_release->setText(Wt::WString::fromUTF8(track->getRelease()->getName()));
			_release->setLink(LmsApp->createReleaseLink(track->getRelease()));
		}
		else
		{
			_release->setText("");
			_release->setLink(Wt::WLink());
		}

		wApp->doJavaScript(oss.str());
	}
	catch (Av::MediaFileException& e)
	{
		LMS_LOG(UI, ERROR) << "MediaFileException: " << e.what();
	}
}

void
MediaPlayer::stop()
{
	wApp->doJavaScript("LMS.mediaplayer.stop()");
}

} // namespace UserInterface

