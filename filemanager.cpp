#include "filemanager.h"

void filemanager::preparefilefolder()
{
	if (!std::filesystem::exists("userdata"))
	{
		std::filesystem::create_directory("userdata");
	}

	if (!std::filesystem::exists("gozdravbotlog"))
	{
		std::filesystem::create_directory("gozdravbotlog");
	}
}

int filemanager::writeorcheck(std::string userid, std::string text, int operationnumber, std::vector<std::string>& filetext)
{
	if (operationnumber == 1)
	{
		std::ifstream fileread(("userdata/" + userid + ".gozdravbot").c_str());

		if (fileread.is_open())
		{
			std::string line;

			while (std::getline(fileread, line))
			{
				if (line == text)
				{
					return  0;
				}
			}
			
			return 1;
		}
		else
		{
			std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Cant open file for user " << "\033[33m" << userid << "\033[31m" << " with text: " << "\033[33m" << text << "\033[0m" << std::endl;

			return 2;
		}
	}
	else if (operationnumber == 2 || operationnumber == 3)
	{
		if (operationnumber == 2)
		{
			std::ofstream filewrite(("userdata/" + userid + ".gozdravbot").c_str(), std::ios::app);

			if (filewrite.is_open())
			{
				filewrite << text << std::endl;

				filewrite.close();

				return 1;
			}
		}
		else
		{
			std::ofstream filewrite("gozdravbotlog/log.gozdravbot", std::ios::app);

			if (filewrite.is_open())
			{
				filewrite << text << std::endl;

				filewrite.close();

				return 1;
			}
		}
	}
	else if (operationnumber == 4)
	{
		std::ifstream fileread(("userdata/" + userid + ".gozdravbot").c_str());

		if (fileread.is_open())
		{
			std::string line;

			while (std::getline(fileread, line))
			{
				filetext.push_back(line);
			}

			return 1;
		}
		else
		{
			std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Cant open file for user " << "\033[33m" << userid << "\033[31m" << " with text: " << "\033[33m" << text << "\033[0m" << std::endl;

			return 2;
		}
	}
	else if (operationnumber == 5)
	{
		std::ofstream filewrite(("userdata/" + userid + ".gozdravbot").c_str());

		if (filewrite.is_open())
		{
			if (!filetext.empty())
			{
				for (int i = 0; i < filetext.size(); i++)
				{
					filewrite << filetext[i] << std::endl;
				}
			}			

			filewrite.close();

			return 1;
		}
	}
}

int filemanager::callfile(std::string userid, std::string text, int operationnumber, std::vector<std::string>& filetext)
{
	if (operationnumber == 1 || operationnumber == 4)
	{
		int result;

		{
			std::shared_lock<std::shared_mutex> lock(filemanager::lockstream);

			result = filemanager::writeorcheck(userid, text, operationnumber, filetext);
		}

		if (result == 2)
		{
			filemanager::callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Cant open file for user " + userid + " with text: " + text, 3, filetext);
		}

		return result;
	}
	else if (operationnumber == 2 || operationnumber == 3 || operationnumber == 5)
	{
		std::lock_guard<std::shared_mutex> lock(filemanager::lockstream);

		return filemanager::writeorcheck(userid, text, operationnumber, filetext);
	}
}