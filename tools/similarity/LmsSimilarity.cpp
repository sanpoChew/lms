#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <chrono>
#include <random>

#include "database/DatabaseHandler.hpp"
#include "database/Track.hpp"
#include "database/Artist.hpp"
#include "database/Cluster.hpp"
#include "database/Release.hpp"
#include "database/TrackFeatures.hpp"
#include "utils/Config.hpp"
#include "similarity/features/som/DataNormalizer.hpp"
#include "similarity/features/som/Network.hpp"

static
std::ostream& operator<<(std::ostream& os, const Database::Track::pointer& track)
{
	auto genreClusterType = Database::ClusterType::getByName(*track->session(), "GENRE");

	os << "[";
	auto genreClusters = track->getClusterGroups({genreClusterType}, 1);
	for (auto genreCluster : genreClusters)
		os << genreCluster.front()->getName() << " - ";
	for (auto artist : track->getArtists())
		os << artist->getName() << " - ";
	if (track->getRelease())
		os << track->getRelease()->getName() << " - ";
	os << track->getName() << "]";

	return os;
}

static
bool
getTrackFeatures(Wt::Dbo::Session &session, const Database::Track::pointer& track, const std::map<std::string, std::size_t>& featuresSettings, SOM::InputVector& res)
{
	std::map<std::string, std::vector<double>> features;
	for (const auto& featureSettings : featuresSettings)
		features[featureSettings.first] = {};

	if (!track->getTrackFeatures()->getFeatures(features))
	{
		std::cout << "Skipping track '" << track->getMBID() << "': missing item" << std::endl;
		return false;
	};

	std::size_t index {};
	for (const auto& feature : features)
	{
		auto it = featuresSettings.find(feature.first);
		if (it == featuresSettings.end() || (feature.second.size() != it->second))
			return false;

		for (double value : feature.second)
			res[index++] = value;
	}

	return true;
}


int main(int argc, char *argv[])
{
	try
	{
		const std::size_t width = 5;
		const std::size_t height = 5;
		const std::size_t nbIterations = 10;
		std::size_t nbTracks = 5000;

		const std::map<std::string, std::size_t> featuresSettings =
		{
//			{ "lowlevel.average_loudness",			1 },
//			{ "lowlevel.dynamic_complexity",		1 },
			{ "lowlevel.spectral_contrast_coeffs.median",	6 },
			{ "lowlevel.erbbands.median",			40 },
			{ "tonal.hpcp.median",				36 },
			{ "lowlevel.melbands.median",			40 },
			{ "lowlevel.barkbands.median",			27 },
			{ "lowlevel.mfcc.mean",				13 },
			{ "lowlevel.gfcc.mean",				13 },
		};
		std::size_t nbDims = 0;
		for (const auto& featureSettings : featuresSettings)
			nbDims += featureSettings.second;

		boost::filesystem::path configFilePath = "/etc/lms.conf";
		if (argc >= 2)
			configFilePath = std::string(argv[1], 0, 256);

		Config::instance().setFile(configFilePath);

		Database::Handler::configureAuth();
		auto connectionPool = Database::Handler::createConnectionPool(Config::instance().getPath("working-dir") / "lms.db");
		Database::Handler db(*connectionPool);

		std::cout << "Getting all features..." << std::endl;
		Wt::Dbo::Transaction transaction(db.getSession());

		std::vector<Database::IdType> trackIds {Database::Track::getAllIdsWithFeatures(db.getSession(), nbTracks)};

		nbTracks = trackIds.size();
		std::cout << "Getting features DONE (" << nbTracks << " tracks)" << std::endl;

		std::cout << "Reading features..." << std::endl;
		std::vector<SOM::InputVector> tracksFeatures;

		for (Database::IdType trackId : trackIds)
		{
			Database::Track::pointer track {Database::Track::getById(db.getSession(), trackId)};
			if (!track)
				continue;

			SOM::InputVector features {nbDims};
			if (!getTrackFeatures(db.getSession(), track, featuresSettings, features))
				continue;

			tracksFeatures.emplace_back(std::move(features));
		}
		std::cout << "Reading features DONE" << std::endl;

		SOM::Network network {width, height, nbDims};
		SOM::DataNormalizer normalizer {nbDims};

		SOM::InputVector weights {nbDims};
		{
			std::size_t index {};
			for (const auto& featureSettings : featuresSettings)
			{
				for (std::size_t i {}; i < featureSettings.second; ++i)
					weights[index++] = SOM::InputVector::value_type{1. / featureSettings.second};
			}
		}

		network.setDataWeights(weights);

		std::cout << "Weights: " << weights << std::endl;

		std::cout << "Normalizing..." << std::endl;
		normalizer.computeNormalizationFactors(tracksFeatures);

		std::cout << "Dumping normalizer: " << std::endl;
		normalizer.dump(std::cout);
		std::cout << "Dumping normalizer DONE" << std::endl;

		for (SOM::InputVector& features : tracksFeatures)
			normalizer.normalizeData(features);
		std::cout << "Normalizing DONE" << std::endl;

		auto progress {[](const SOM::Network::CurrentIteration& iteration)
		{
			std::cout << "Iteration " << iteration.idIteration + 1 << " of " << iteration.iterationCount << std::endl;;
		}};

		std::cout << "Training..." << std::endl;
		network.train(tracksFeatures, nbIterations, progress);
		std::cout << "Training DONE" << std::endl;

		auto meanDistance = network.computeRefVectorsDistanceMean();
		std::cout << "MEAN distance = " << meanDistance << std::endl;
		auto medianDistance = network.computeRefVectorsDistanceMedian();
		std::cout << "MEDIAN distance = " << medianDistance << std::endl;

		std::cout << "Classifying tracks..." << std::endl;

		SOM::Matrix< std::vector<Database::Track::pointer> > tracksMap(width, height);
		for (Database::IdType trackId : trackIds)
		{
			Database::Track::pointer track {Database::Track::getById(db.getSession(), trackId)};
			if (!track)
				continue;

			SOM::InputVector features {nbDims};
			if (!getTrackFeatures(db.getSession(), track, featuresSettings, features))
				continue;

			normalizer.normalizeData(features);

			SOM::Position position = network.getClosestRefVectorPosition(features);
			tracksMap[position].push_back(track);
		}

		std::cout << "Classifying tracks DONE" << std::endl;

		// Dump tracks

		for (SOM::Coordinate y = 0; y < tracksMap.getHeight(); ++y)
		{
			for (SOM::Coordinate x = 0; x < tracksMap.getWidth(); ++x)
			{
				std::cout << "{" << x << ", " << y << "}" << std::endl;
				const auto& tracks = tracksMap[{x, y}];

				for (const auto& track : tracks)
				{
					std::cout << " - " << track << std::endl;
				}
			}
		}

		// For each track, get the nearest tracks
		for (Database::IdType trackId : trackIds)
		{
			Database::Track::pointer track {Database::Track::getById(db.getSession(), trackId)};
			if (!track)
				continue;

			SOM::InputVector features {nbDims};
			if (!getTrackFeatures(db.getSession(), track, featuresSettings, features))
				continue;

			normalizer.normalizeData(features);

			SOM::Position refVectorPosition {network.getClosestRefVectorPosition(features)};

			std::cout << "Getting nearest songs for track " << track << " in {" << refVectorPosition.x << ", " << refVectorPosition.y << "}:" << std::endl;
			for (auto similarTrack : tracksMap[refVectorPosition])
				std::cout << " - " << similarTrack << std::endl;

			std::set<SOM::Position> neighbourPosition {refVectorPosition};
			for (std::size_t i {}; i < 3; ++i)
			{
				auto position = network.getClosestRefVectorPosition(neighbourPosition, medianDistance);
				if (!position)
					break;

				std::cout << " - in {" << position->x << ", " << position->y << "}, dist = " << network.getRefVectorsDistance(*position, refVectorPosition) << std::endl;
				for (const auto& similarTrack : tracksMap[*position])
					std::cout << "    - " << similarTrack << std::endl;

				neighbourPosition.insert(*position);
			}

		}
	}
	catch( std::exception& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

