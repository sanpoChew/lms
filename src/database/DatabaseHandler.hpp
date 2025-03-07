/*
 * Copyright (C) 2013 Emeric Poupon
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

#include <boost/filesystem.hpp>
#include <memory>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/PasswordService.h>

#include "User.hpp"

namespace Database {

using UserDatabase = Wt::Auth::Dbo::UserDatabase<AuthInfo>;

// Session living class handling the database and the login
class Handler
{
	public:

		Handler(Wt::Dbo::SqlConnectionPool& connectionPool);
		~Handler();

		Wt::Dbo::Session& getSession() { return _session; }

		Wt::Dbo::ptr<User> getCurrentUser();	// get the current user, may return empty
		Wt::Dbo::ptr<User> getUser(const std::string& loginName);
		Wt::Dbo::ptr<User> getUser(const Wt::Auth::User& authUser);
		Wt::Dbo::ptr<User> createUser(const Wt::Auth::User& authUser);

		Wt::Auth::AbstractUserDatabase& getUserDatabase();
		Wt::Auth::Login& getLogin() { return _login; } // TODO move

		// Long living shared associated services
		static void configureAuth();

		static const Wt::Auth::AuthService& getAuthService();
		static const Wt::Auth::PasswordService& getPasswordService();

		static std::unique_ptr<Wt::Dbo::SqlConnectionPool> createConnectionPool(boost::filesystem::path db);

	private:

		Wt::Dbo::Session		_session;
		UserDatabase*			_users;
		Wt::Auth::Login 		_login;

};

} // namespace Database


