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

#include "TranscodeResource.hpp"

#include <Wt/Http/Response.h>

#include "utils/Logger.hpp"

#include "database/Track.hpp"

#include "LmsApplication.hpp"

namespace UserInterface {

TranscodeResource::TranscodeResource(Database::Handler& db)
: _db(db)
{
	LMS_LOG(UI, DEBUG) << "CONSTRUCTING RESOURCE";
}

TranscodeResource:: ~TranscodeResource()
{
	LMS_LOG(UI, DEBUG) << "DESTRUCTING RESOURCE";
	beingDeleted();
}

std::string
TranscodeResource::getUrl(Database::IdType trackId, Av::Encoding encoding) const
{
	std::string res = url()+ "&trackid=" + std::to_string(trackId) + "&encoding=" + std::to_string(Av::encodingToInt(encoding));

	return res;
}

void
TranscodeResource::handleRequest(const Wt::Http::Request& request,
		Wt::Http::Response& response)
{
	std::shared_ptr<Av::Transcoder> transcoder;

	LMS_LOG(UI, DEBUG) << "Handling new request...";

	// First, see if this request is for a continuation
	Wt::Http::ResponseContinuation *continuation = request.continuation();
	if (continuation)
	{
		LMS_LOG(UI, DEBUG) << "Continuation! " << continuation ;
		transcoder = Wt::cpp17::any_cast<std::shared_ptr<Av::Transcoder>>(continuation->data());
	}
	else
	{
		Database::IdType trackId;
		Av::TranscodeParameters parameters;

		LMS_LOG(UI, DEBUG) << "No continuation yet";
		try
		{
			auto trackIdStr = request.getParameter("trackid");
			if (!trackIdStr)
			{
				LMS_LOG(UI, ERROR) << "Missing trackid transcode parameter!";
				return;
			}
			trackId = std::stol(*request.getParameter("trackid"));

			auto offsetStr = request.getParameter("offset");
			if (offsetStr)
				parameters.offset = std::chrono::seconds(std::stol(*offsetStr));

			auto encodingStr = request.getParameter("encoding");
			if (encodingStr)
				parameters.encoding = Av::encodingFromInt(std::stol(*encodingStr));

			auto streamStr = request.getParameter("stream");
			if (streamStr)
				parameters.stream = std::stol(*streamStr);
		}
		catch (std::exception &e)
		{
			LMS_LOG(UI, ERROR) << "Exception while handling URL parameters: " << e.what();
			return;
		}

		// transactions are not thread safe
		{
			Wt::WApplication::UpdateLock lock(LmsApplication::instance());

			Wt::Dbo::Transaction transaction(_db.getSession());

			Database::User::pointer user = _db.getCurrentUser();
			Database::Track::pointer track = Database::Track::getById(_db.getSession(), trackId);

			if (!track)
			{
				LMS_LOG(UI, ERROR) << "Missing track";
				return;
			}

			parameters.bitrate = user->getAudioBitrate();
			transcoder = std::make_shared<Av::Transcoder>(track->getPath(), parameters);
		}

		std::string mimeType = Av::encodingToMimetype(transcoder->getParameters().encoding);

		LMS_LOG(UI, DEBUG) << "Mime type set to '" << mimeType << "'";
		response.setMimeType(mimeType);

		if (!transcoder->start())
		{
			LMS_LOG(UI, ERROR) << "Cannot start transcoder";
			return;
		}

		LMS_LOG(UI, DEBUG) << "Transcoder started";
	}

	if (!transcoder->isComplete())
	{
		std::vector<unsigned char> data;
		data.reserve(_chunkSize);

		transcoder->process(data, _chunkSize);

		response.out().write(reinterpret_cast<char*>(&data[0]), data.size());
		LMS_LOG(UI, DEBUG) << "Written " << data.size() << " bytes! complete = " << std::boolalpha << transcoder->isComplete();

		if (!response.out())
		{
			LMS_LOG(UI, ERROR) << "Write failed!";
		}
	}

	if (!transcoder->isComplete() && response.out())
	{
		continuation = response.createContinuation();
		continuation->setData(transcoder);
	}
	else
		LMS_LOG(UI, DEBUG) << "No more data!";
}

} // namespace UserInterface


