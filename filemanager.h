#pragma once
#include <string>
#include <mutex>
#include <future>
#include <shared_mutex>
#include <fstream>
#include <filesystem>
#include <iostream>

class filemanager
{
	private:
		std::shared_mutex lockstream;

		int writeorcheck(std::string, std::string, int, std::vector<std::string>&);

	public:
		void preparefilefolder();
		int callfile(std::string, std::string, int, std::vector<std::string>&);
};