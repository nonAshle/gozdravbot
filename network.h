#pragma once
#include <string>
#include <future>
#include <array>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include "include/webdriverxx.h"
#include "filemanager.h"

class network
{
	private:
		std::mutex pool_mutex;
		std::array<std::future<bool>, 5> pool;

		bool openwebandget(std::string, std::vector<std::string>&, std::int16_t);

		filemanager file = filemanager();

	public:
		bool webrequest(std::string, std::vector<std::string>&, std::int16_t);
};
