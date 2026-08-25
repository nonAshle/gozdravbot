#include <iostream>
#include <dpp/dpp.h>
#include "discord.h"
#include "filemanager.h"

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    std::srand(std::time(0));

    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    dpp::cluster bot("Токен вашего бота", dpp::i_default_intents | dpp::i_message_content);
    
    discord abc = discord();
    filemanager folder = filemanager();

    std::vector<std::string> emptyvector;

    folder.preparefilefolder();

    std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Prepare folder end" << "\033[0m" << std::endl;
    folder.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Prepare folder end", 3, emptyvector);

    abc.preparebot(bot);
    abc.botgetevent(bot);

    std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Prepare bot end" << "\033[0m" << std::endl;
    folder.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Prepare bot end", 3, emptyvector);

    bot.start(dpp::st_wait);
}