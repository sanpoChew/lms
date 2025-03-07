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

#include <Wt/Dbo/SqlConnectionPool.h>

#include "database/DatabaseHandler.hpp"
#include "scanner/MediaScannerAddon.hpp"

#include "SimilarityFeaturesSearcher.hpp"

namespace Similarity {

class FeaturesScannerAddon final : public Scanner::MediaScannerAddon
{
	public:

		FeaturesScannerAddon(Wt::Dbo::SqlConnectionPool& connectionPool);

		std::shared_ptr<FeaturesSearcher> getSearcher();

	private:

		void refreshSettings() override {}
		void requestStop() override;
		void trackAdded(Database::IdType trackId) override {}
		void trackToRemove(Database::IdType trackId) override {}
		void trackUpdated(Database::IdType trackId) override;
		void preScanComplete() override;

		bool fetchFeatures(Database::IdType trackId, const std::string& MBID);

		void updateSearcher();

		Database::Handler			_db;
		std::shared_ptr<FeaturesSearcher>	_searcher;
		bool					_stopRequested{false};
};

FeaturesScannerAddon* setFeaturesScannerAddon(FeaturesScannerAddon addon);
FeaturesScannerAddon* getFeaturesScannerAddon();

} // namespace Similarity

