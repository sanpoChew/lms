/*
 * Copyright (C) 2016 Emeric Poupon
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

#include <set>

#include <boost/filesystem.hpp>
#include <libconfig.h++>

// Used to get config values from configuration files
class Config final
{
	public:

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(Config&&) = delete;

		static Config& instance();

		void		setFile(const boost::filesystem::path& p);

		// Default values are returned in case of setting not found
		std::string	getString(const std::string& setting, const std::string& def = "", const std::set<std::string>& allowedValues = {});
		boost::filesystem::path getPath(const std::string& setting, const boost::filesystem::path& def = boost::filesystem::path());
		unsigned long	getULong(const std::string& setting, unsigned long def = 0);
		long		getLong(const std::string& setting, long def = 0);
		bool		getBool(const std::string& setting, bool def = false);

	private:

		Config() = default;

		std::unique_ptr<libconfig::Config> _config;
};

