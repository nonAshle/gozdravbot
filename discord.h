#pragma once
#include <dpp/dpp.h>
#include "network.h"
#include "filemanager.h"
#include <string>
#include <iostream>
#include <filesystem>

class discord
{
	private:
		struct useroperation
		{
			bool flagregistration = false;
			bool flagdeleting = false;

			std::string url;

			std::vector<std::string> data;
		};

		std::string idselectmenu = "gozdravbot." + std::to_string(dpp::utility::time_f());

		std::map<dpp::snowflake, useroperation> mapforoperation;

		std::mutex operationmutex;

		network browser = network();
		filemanager filehandler = filemanager();

	public:
		void preparebot(dpp::cluster&);
		void botgetevent(dpp::cluster&);
		void pingdoctors(dpp::cluster&);
};