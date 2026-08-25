#include "discord.h"

void  discord::preparebot(dpp::cluster& bot)
{
	bot.on_ready([&](const dpp::ready_t& event)
	{
		if (dpp::run_once<struct register_bot_command>())
		{
			dpp::slashcommand registerdoctor("register_doctor", "Зарегистрируй врача", bot.me.id);
			registerdoctor.add_option(dpp::command_option(dpp::co_string, "link", "ссылка на специализацию врача в чем угодно", true));
			registerdoctor.set_interaction_contexts({ dpp::itc_guild, dpp::itc_bot_dm });

			dpp::slashcommand deletedoctor("delete_doctor", "Удали врача", bot.me.id);
			deletedoctor.set_interaction_contexts({ dpp::itc_guild, dpp::itc_bot_dm });

			dpp::slashcommand checkdoctor("check_doctor", "Посмотри своих врачей", bot.me.id);
			checkdoctor.set_interaction_contexts({ dpp::itc_guild, dpp::itc_bot_dm });

			bot.global_bulk_command_create({registerdoctor, deletedoctor, checkdoctor});
		}

		if (dpp::run_once<struct start_bot_timer>())
		{
			bot.start_timer([&bot, this](dpp::timer t)
			{
					std::vector<std::string> empty_vector;

					std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Start ping users"  << "\033[0m" << std::endl;
					filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Start ping users", 3, empty_vector);

					std::thread worker([this, &bot]()
					{
						pingdoctors(bot);
					});
					
					worker.detach();

			}, 3600);
		}
	});
}

void discord::botgetevent(dpp::cluster& bot)
{
	bot.on_slashcommand([&](const dpp::slashcommand_t& event)
	{
		if (event.command.guild_id == 0)
		{
			if (event.command.get_command_name() == "register_doctor")
			{
				event.thinking();

				std::vector<std::string> empty_vector;

				std::string url = std::get<std::string>(event.get_parameter("link"));

				if (!url.empty())
				{
					if (url.starts_with("https://gorzdrav.spb.ru/service-free-schedule#%5B%7B%22district%22:%22"))
					{
						std::vector<std::string> wordsfromfile;

						int resultfromfile = filehandler.callfile(std::to_string(event.command.usr.id), "", 4, wordsfromfile);

						if (wordsfromfile.size() <= 25)
						{
							std::unique_lock<std::mutex> lock(discord::operationmutex);

							discord::useroperation& temp = discord::mapforoperation[event.command.usr.id];

							if (!temp.flagregistration && !temp.flagdeleting)
							{
								temp.flagregistration = true;
								lock.unlock();

								std::vector<std::string> list;

								if (browser.webrequest(url, list, 1))
								{
									if (!list.empty())
									{
										dpp::component selectdoctors;
										selectdoctors.set_type(dpp::cot_selectmenu);
										selectdoctors.set_id(discord::idselectmenu);
										selectdoctors.set_placeholder("Нажми, чтобы выбрать доктора");
										selectdoctors.set_max_values(1);
										selectdoctors.set_min_values(1);

										for (int i = 0; i < list.size(); i++)
										{
											selectdoctors.add_select_option(dpp::select_option(list[i], std::to_string(i), "Нажми, чтобы выбрать"));
										}

										lock.lock();
										discord::mapforoperation[event.command.usr.id].data = std::move(list);
										discord::mapforoperation[event.command.usr.id].url = url;
										lock.unlock();

										dpp::component actionrow;
										actionrow.set_type(dpp::cot_action_row);
										actionrow.add_component(selectdoctors);

										dpp::message msg("Выберите врача, чтобы его зарегистровать");
										msg.add_component(actionrow);

										std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Create poll for registration by " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
										filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Create poll for registration by " + std::to_string(event.command.usr.id), 3, empty_vector);

										event.edit_response(msg);
									}
									else
									{
										lock.lock();
										discord::mapforoperation.erase(event.command.usr.id);
										lock.unlock();

										std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " List doctors empty for " << "\033[33m" << event.command.usr.id << "\033[31m" << " URL:" << "\033[33m" << url << "\033[0m" << std::endl;
										filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " List doctors empty for " + std::to_string(event.command.usr.id) + " URL: " + url, 3, empty_vector);

										event.edit_original_response(dpp::message("Нету врачей в выбранной специализации"));
									}
								}
								else
								{
									lock.lock();
									discord::mapforoperation.erase(event.command.usr.id);
									lock.unlock();

									event.edit_original_response(dpp::message("Ошибка в загрузке браузера"));
								}
							}
							else
							{
								lock.unlock();

								event.edit_original_response(dpp::message("Сначала завершите другую операцию или попробуйте еще раз"));
							}
						}
						else
						{
							if (resultfromfile == 1)
							{
								if (wordsfromfile.size() > 25)
								{
									std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Reached limit doctors by " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
									filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Reached limit doctors by " + std::to_string(event.command.usr.id), 3, empty_vector);

									event.edit_original_response(dpp::message("Освободите сначала место для врача"));
								}
							}
						}
					}
					else
					{
						std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Url invalid by  " << "\033[33m" << event.command.usr.id << "\033[31m" << " URL: " << "\033[33m" << url << "\033[0m" << std::endl;
						filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Url invalid by " + std::to_string(event.command.usr.id) + " URL: " + url, 3, empty_vector);

						event.edit_original_response(dpp::message("Вы отправили ссылку не на тот сайт"));
					}
				}
				else
				{
					std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Url not entered by " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
					filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Url not entered by " + std::to_string(event.command.usr.id), 3, empty_vector);

					event.edit_original_response(dpp::message("Не написали ссылку"));
				}
			}
			else if (event.command.get_command_name() == "delete_doctor")
			{
				event.thinking();

				std::unique_lock<std::mutex> lock(discord::operationmutex);

				discord::useroperation& temp = discord::mapforoperation[event.command.usr.id];

				if (!temp.flagregistration && !temp.flagdeleting)
				{
					temp.flagdeleting = true;
					lock.unlock();

					std::vector<std::string> wordsfromfile;

					int result = filehandler.callfile(std::to_string(event.command.usr.id), "", 4, wordsfromfile);

					if (result == 2)
					{
						lock.lock();
						discord::mapforoperation.erase(event.command.usr.id);
						lock.unlock();

						event.edit_original_response(dpp::message("У вас нету врачей"));
					}
					else if (result == 1)
					{
						if (wordsfromfile.empty())
						{
							lock.lock();
							discord::mapforoperation.erase(event.command.usr.id);
							lock.unlock();

							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Vector with words from file empty by  " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
							filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Vector with words from file empty by " + std::to_string(event.command.usr.id), 3, wordsfromfile);

							event.edit_original_response(dpp::message("У вас нету врачей"));
						}
						else
						{
							std::vector<std::string> empty_vector;

							dpp::component selectdoctors;
							selectdoctors.set_type(dpp::cot_selectmenu);
							selectdoctors.set_id(discord::idselectmenu);
							selectdoctors.set_placeholder("Нажми, чтобы выбрать доктора");
							selectdoctors.set_max_values(1);
							selectdoctors.set_min_values(1);

							std::vector<std::string> processednamefromfile;

							for (const auto& word : wordsfromfile)
							{
								size_t pos = word.find('|');

								if (pos != std::string::npos)
								{
									processednamefromfile.push_back(word.substr(pos + 1));
								}
							}

							for (int i = 0; i < processednamefromfile.size(); i++)
							{
								selectdoctors.add_select_option(dpp::select_option(processednamefromfile[i], std::to_string(i), "Нажми, чтобы выбрать"));
							}

							lock.lock();
							discord::mapforoperation[event.command.usr.id].data = std::move(wordsfromfile);
							lock.unlock();

							dpp::component actionrow;
							actionrow.set_type(dpp::cot_action_row);
							actionrow.add_component(selectdoctors);

							dpp::message msg("Выберите врача, чтобы его удалить");
							msg.add_component(actionrow);

							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Create poll for deleting by " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
							filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Create poll for deleting by " + std::to_string(event.command.usr.id), 3, empty_vector);

							event.edit_response(msg);
						}
					}
				}
				else
				{
					lock.unlock();

					event.edit_original_response(dpp::message("Сначала завершите другую операцию или попробуйте еще раз"));
				}
			}
			else if (event.command.get_command_name() == "check_doctor")
			{
				event.thinking();

				std::vector<std::string> wordsfromfile;

				int result = filehandler.callfile(std::to_string(event.command.usr.id), "", 4, wordsfromfile);

				if (result == 2)
				{
					event.edit_original_response(dpp::message("У вас нету врачей"));
				}
				else if (result == 1)
				{
					if (wordsfromfile.empty())
					{
						event.edit_original_response(dpp::message("У вас нету врачей"));
					}
					else
					{
						std::stringstream tempstorage;
						tempstorage << "Ваши врачи:" << "\n";

						for (const auto& word : wordsfromfile)
						{
							tempstorage << word << "\n";
						}

						event.edit_response(tempstorage.str());
					}
				}

				std::vector<std::string> empty_vector;

				std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Give users list with doctores for " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
				filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Give users list with doctores for  " + std::to_string(event.command.usr.id), 3, empty_vector);
			}
		}
	});

	bot.on_select_click([&bot, this](const dpp::select_click_t& event)
	{
		event.thinking();

		if (event.custom_id == discord::idselectmenu)
		{
			if (event.command.msg.author == bot.me.id)
			{
				if (event.command.guild_id == 0)
				{
					bot.message_delete(event.command.message_id, event.command.channel_id);

					std::vector<std::string> empty_vector;

					std::unique_lock<std::mutex> lock(discord::operationmutex);

					discord::useroperation& temp = discord::mapforoperation[event.command.usr.id];

					if (temp.flagregistration && !temp.flagdeleting)
					{
						if (temp.data.empty())
						{
							discord::mapforoperation.erase(event.command.usr.id);
							lock.unlock();

							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Empty vector in voice handler in operation registration for " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
							filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Empty vector in voice handler in operation registration for " + std::to_string(event.command.usr.id), 3, empty_vector);

							event.edit_original_response(dpp::message("Я не смог записать врача, попробуй еще раз"));

							return;
						}

						std::string namedoctor = temp.data[std::stoi(event.values[0])];
						std::string url = temp.url;
						discord::mapforoperation.erase(event.command.usr.id);
						lock.unlock();

						std::string resultstring = url + "|" + namedoctor;

						int fileresponse = filehandler.callfile(std::to_string(event.command.usr.id), resultstring, 1, empty_vector);

						if (fileresponse == 2)
						{
							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " File not exist at user " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;

							if (filehandler.callfile(std::to_string(event.command.usr.id), resultstring, 2, empty_vector) == 1)
							{
								std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Doctor  register for user " << "\033[33m" << event.command.usr.id << "\033[32m" << " namely:" << "\033[33m" << resultstring << "\033[0m" << std::endl;
								filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Doctor  register for user " + std::to_string(event.command.usr.id) + " namely:  " + resultstring, 3, empty_vector);
							}
						}
						else if (fileresponse == 0)
						{
							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Doctor already register for user " << "\033[33m" << event.command.usr.id << "\033[32m" << " namely:" << "\033[33m" << resultstring << "\033[0m" << std::endl;
							filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Doctor already register for user " + std::to_string(event.command.usr.id) + " namely:  " + resultstring, 3, empty_vector);
						}
						else if (fileresponse == 1)
						{
							if (filehandler.callfile(std::to_string(event.command.usr.id), resultstring, 2, empty_vector) == 1)
							{
								std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Doctor  register for user " << "\033[33m" << event.command.usr.id << "\033[32m" << " namely:" << "\033[33m" << resultstring << "\033[0m" << std::endl;
								filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Doctor  register for user " + std::to_string(event.command.usr.id) + " namely:  " + resultstring, 3, empty_vector);
							}
						}

						event.edit_original_response(dpp::message("Я записал врача"));
					}
					else if (temp.flagdeleting && !temp.flagregistration)
					{
						if (temp.data.empty())
						{
							discord::mapforoperation.erase(event.command.usr.id);
							lock.unlock();

							std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Empty vector in voice handler in operation deleting for " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
							filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Empty vector in voice handler in operation registration for " + std::to_string(event.command.usr.id), 3, empty_vector);

							event.edit_original_response(dpp::message("Я не смог удалить врача, попробуй еще раз"));

							return;
						}

						std::vector<std::string> wordsinfile;
						std::string deletedoctor;

						for (int i = 0; i < temp.data.size(); i++)
						{
							if (i == std::stoi(event.values[0]))
							{
								deletedoctor = temp.data[i];
								continue;
							}
							else
							{
								wordsinfile.push_back(temp.data[i]);
							}
						}

						discord::mapforoperation.erase(event.command.usr.id);
						lock.unlock();

						filehandler.callfile(std::to_string(event.command.usr.id), "", 5, wordsinfile);

						std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Deleting doctor successfully for " << "\033[33m" << event.command.usr.id << "\033[32m" << " doctors has been deleting " << "\033[33m" << deletedoctor << "\033[0m" << std::endl;
						filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Deleting doctor successfully for " + std::to_string(event.command.usr.id) + " doctors has been deleting " + deletedoctor, 3, empty_vector);

						event.edit_original_response(dpp::message("Я удалил врача"));
					}
					else
					{
						discord::mapforoperation.erase(event.command.usr.id);
						lock.unlock();

						std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Two flags are been false by " << "\033[33m" << event.command.usr.id << "\033[0m" << std::endl;
						filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Two flags are been false by " + std::to_string(event.command.usr.id), 3, empty_vector);

						event.edit_original_response(dpp::message("Попробуй еще раз"));
					}
				}
			}
		}
	});		
}

void discord::pingdoctors(dpp::cluster& bot)
{
	std::filesystem::path directoryname = "userdata";
	std::string extension = ".gozdravbot";

	if (std::filesystem::exists(directoryname) && std::filesystem::is_directory(directoryname))
	{
		for (const auto& file : std::filesystem::directory_iterator(directoryname))
		{
			if (file.is_regular_file())
			{
				if (file.path().extension() == extension)
				{
					std::vector<std::string> wordsfromfile;

					int resultcheckfile = filehandler.callfile(file.path().stem().string(), "", 4, wordsfromfile);

					if (resultcheckfile == 1)
					{
						if (!wordsfromfile.empty())
						{
							for (const auto& word : wordsfromfile)
							{
								size_t pos = word.find('|');

								if (pos != std::string::npos)
								{
									std::string url = word.substr(0, pos);
									std::string namedoctor = word.substr(pos + 1);

									std::vector<std::string> list;

									if (browser.webrequest(url, list, 2))
									{
										if (!list.empty())
										{
											std::vector<std::string> empty_vector;

											auto iterator = std::find(list.begin(), list.end(), namedoctor);

											if (iterator != list.end() && std::next(iterator) != list.end())
											{
												std::string appointment = *std::next(iterator);

												if (appointment == "")
												{
													bot.direct_message_create(dpp::snowflake(file.path().stem().string()), dpp::message("У врача " + namedoctor + " нету номерков"));
												}
												else
												{
													bot.direct_message_create(dpp::snowflake(file.path().stem().string()), dpp::message("Информация о номерке у врача " + namedoctor + ": " + appointment));
												}

												std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[32m" << " Give user " << "\033[33m" << file.path().stem().string() << "\033[32m" << " information about doctor " << "\033[33m" << namedoctor <<  "\033[0m" << std::endl;
												filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Give user " + file.path().stem().string() + " information about doctor " + namedoctor, 3, empty_vector);
											}
											else
											{
												bot.direct_message_create(dpp::snowflake(file.path().stem().string()), dpp::message("Врач " + namedoctor + " не был найден, проверьте есть ли он на сайте гоздрава " + url));

												std::cout << "\033[34m" << std::chrono::system_clock::now() << "\033[31m" << " Dont find for user " << "\033[33m" << file.path().stem().string() << "\033[31m" << " doctor " << "\033[33m" << namedoctor << "\033[0m" << std::endl;
												filehandler.callfile("", std::format("{:%Y-%m-%d %X}", std::chrono::system_clock::now()) + " Dont find for user " + file.path().stem().string() + " doctor " + namedoctor, 3, empty_vector);
											}
										}
									}
									else
									{
										bot.direct_message_create(dpp::snowflake(file.path().stem().string()), dpp::message("Ошибка в работе с браузером, скажите кому нибудь"));
									}
								}
							}
						}
					}
				}
			}
		}
	}
}