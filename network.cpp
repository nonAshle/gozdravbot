#include "network.h"

bool network::openwebandget(std::string url, std::vector<std::string>& list, std::int16_t operation)
{
	if (operation == 1)
	{
		try
		{
			webdriverxx::Capabilities capabilities = webdriverxx::Chrome();

			std::vector<std::string> args = {
				"--headless=new",
				"--disable-gpu",
				"--window-size=1920,1080",
				"--ignore-certificate-errors",
				"--allow-running-insecure-content"
			};

			webdriverxx::JsonObject chromeOptions;
			chromeOptions.Set("args", args);

			capabilities.Set("goog:chromeOptions", chromeOptions);

			webdriverxx::WebDriver driver = webdriverxx::Start(capabilities, "http://localhost:55717");
			driver.Navigate(url);

			std::this_thread::sleep_for(std::chrono::seconds(5));

			std::string js_script =
				"var names = [];"
				"var elements = document.querySelectorAll('.service-doctor__title');"
				"elements.forEach(function(el) { names.push(el.innerText.trim()); });"
				"return names.join(';');";

			std::string result = driver.Eval<std::string>(js_script);
			std::string temp = "";

			for (const auto& symbol : result)
			{
				if (symbol == ';')
				{
					list.push_back(std::exchange(temp, ""));
					continue;
				}
				
				temp += symbol;
			}

			list.push_back(std::move(temp));

			return true;
		}
		catch (const std::exception& e)
		{
			std::vector<std::string> empty_vector;

			std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Error in work browser: " << "\033[33m" << e.what() << "\033[0m" << std::endl;
			file.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Error in work browser: " + e.what(), 3, empty_vector);

			return false;
		}
	}
	else if (operation == 2)
	{
		try
		{
			webdriverxx::Capabilities capabilities = webdriverxx::Chrome();

			std::vector<std::string> args = {
				"--headless=new",
				"--disable-gpu",
				"--window-size=1920,1080",
				"--ignore-certificate-errors",
				"--allow-running-insecure-content"
			};

			webdriverxx::JsonObject chromeOptions;
			chromeOptions.Set("args", args);

			capabilities.Set("goog:chromeOptions", chromeOptions);

			webdriverxx::WebDriver driver = webdriverxx::Start(capabilities, "http://localhost:55717");
			driver.Navigate(url);

			std::this_thread::sleep_for(std::chrono::seconds(5));

			std::string js_script =
				"var names = [];"
				"var elements = document.querySelectorAll('.service-doctor__title, .service-doctor-top__list_numbers');"
				"elements.forEach(function(el) {"
				"  var text = el.innerText.trim().replace(/\\n/g, \" \");"
				"  names.push(text);"
				"});"
				"return names.join(';');";

			std::string result = driver.Eval<std::string>(js_script);
			std::string temp = "";

			for (const auto& symbol : result)
			{
				if (symbol == ';')
				{
					list.push_back(std::exchange(temp, ""));
					continue;
				}

				temp += symbol;
			}

			list.push_back(std::move(temp));

			return true;
		}
		catch (const std::exception& e)
		{
			std::vector<std::string> empty_vector;

			std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Error in work browser: " << "\033[33m" << e.what() << "\033[0m" << std::endl;
			file.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Error in work browser: " + e.what(), 3, empty_vector);

			return false;
		}
	}
	else
	{
		return false;
	}
}

bool network::webrequest(std::string url, std::vector<std::string>& list, std::int16_t operation)
{
	int task;

	{
		std::unique_lock<std::mutex> lock(network::pool_mutex);

	jump:

		for (int i = 0; i < network::pool.size(); i++)
		{

			if (!pool[i].valid() || pool[i].wait_for(std::chrono::seconds::zero()) == std::future_status::ready)
			{
				pool[i] = std::async(std::launch::async, &network::openwebandget, this, url, std::ref(list), operation);

				task = i;

				break;
			}
			else if (pool[i].wait_for(std::chrono::seconds::zero()) == std::future_status::timeout)
			{
				if (i == network::pool.size() - 1)
				{
					lock.unlock();

					std::this_thread::sleep_for(std::chrono::seconds((std::rand() % 10) + 1));

					lock.lock();

					goto jump;
				}
			}
		}
	}

	return pool[task].get();
}