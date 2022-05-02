#pragma warning(disable:4996)

#include <chrono>
#include <ctime> 
#include <iostream>
#include <string>
#include "MemoryAccess.h"
#include "AlbumManager.h"

void print_openScreen()
{
	std::cout << "========================" << std::endl;
	std::cout << "Made By: Nativ Kolisher" << std::endl;
	auto start = std::chrono::system_clock::now();
	// Some computation here
	auto end = std::chrono::system_clock::now();

	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	std::cout << "     CURRENT TIME:    " << std::endl;
	std::cout << std::ctime(&end_time);
	std::cout << "========================" << std::endl << std::endl;
}

int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");
	
	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}

int main(void)
{
	 print_openScreen();

	 //initialization data access
	 MemoryAccess dataAccess;
	

	 //nitialize album manager
	AlbumManager albumManager(dataAccess);

	std::string albumName;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do {
		int commandNumber = getCommandNumberFromUser();
		
		try	{
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} catch (std::exception& e) {	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);
}


