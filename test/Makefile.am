
TESTS = som database

check_PROGRAMS = som database

som_SOURCES = \
	$(srcdir)/som/SomTest.cpp					\
	$(top_srcdir)/src/similarity/features/som/DataNormalizer.cpp	\
	$(top_srcdir)/src/similarity/features/som/Network.cpp

som_CXXFLAGS=-std=c++14 -Wall -I${top_srcdir}/src/ -I${top_srcdir}/src/similarity/features/som/


database_SOURCES = \
	$(srcdir)/database/DatabaseTest.cpp			\
	$(top_srcdir)/src/database/Artist.cpp			\
	$(top_srcdir)/src/database/Cluster.cpp			\
	$(top_srcdir)/src/database/DatabaseHandler.cpp		\
	$(top_srcdir)/src/database/TrackArtistLink.cpp		\
	$(top_srcdir)/src/database/TrackFeatures.cpp		\
	$(top_srcdir)/src/database/TrackList.cpp		\
	$(top_srcdir)/src/database/Release.cpp			\
	$(top_srcdir)/src/database/ScanSettings.cpp		\
	$(top_srcdir)/src/database/SimilaritySettings.cpp	\
	$(top_srcdir)/src/database/SqlQuery.cpp			\
	$(top_srcdir)/src/database/Track.cpp			\
	$(top_srcdir)/src/database/User.cpp			\
	$(top_srcdir)/src/utils/Logger.cpp			\
	$(top_srcdir)/src/utils/Utils.cpp

database_CXXFLAGS=-std=c++14 -Wall -I${top_srcdir}/src/

