#include "stdafx.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <chrono>

class Storage // or container
{};

class TempBuffer // or container
{};

class Parser
{};

class PackageManager
{
public:
	PackageManager(std::size_t bulkSize = 3)
		: bulkSize{ bulkSize }
	{ }

	void run() {
		std::string line;
		while (getline(std::cin, line))
		{
			std::size_t foundCmd = line.find_first_of("cmd");
			if (foundCmd != std::string::npos)
			{
				if (brackets == 0)
				{
					//std::cout << "Found cmd." << '\n';

					if(packageStorage.empty())
						firstPackageOfTheBulkCameTime = std::chrono::system_clock::now();

					packageStorage.push_back(line);

					if (packageStorage.size() == bulkSize)
					{
						printStorage();
					}
				}
				else
				{
					packageStorage.push_back(line);
				}
			}
			else if (line == "{")
			{
				//std::cout << "Found {" << '\n';
				++brackets;

				if (brackets == 1)
				{
					printStorage();
					firstPackageOfTheBulkCameTime = std::chrono::system_clock::now();
				}
			}
			else if (line == "}")
			{
				//std::cout << "Found }" << '\n';
				--brackets;

				if (brackets == 0)
					printStorage();
			}
		}

		std::cout << "End: " << '\n';
		printStorage();
	}

	void setBulkSize(std::size_t bulkSize) {
		this->bulkSize = bulkSize;
	}

private:
	void printStorage() {
		if (packageStorage.empty())
			return;

		std::ostringstream oss;

		oss << "bulk: ";
		for (std::size_t i = 0; i < packageStorage.size(); ++i)
		{
			oss << packageStorage[i];

			if (i + 1 != packageStorage.size())
				oss << ", ";
		}

		std::cout << oss.str();

		std::chrono::system_clock::duration dtn = firstPackageOfTheBulkCameTime.time_since_epoch();

		auto timeStamp = dtn.count();
		std::string timeStampStr = "bulk" + std::to_string(timeStamp) + ".log";

		std::ofstream myfile;
		myfile.open(timeStampStr);
		myfile << oss.str();
		myfile.close();

		packageStorage.clear();
		std::cout << '\n';
	}

	std::size_t bulkSize;
	std::vector<std::string> packageStorage;
	std::size_t brackets{ 0 };
	std::chrono::system_clock::time_point firstPackageOfTheBulkCameTime;
};

int main(int argc, char* argv[])
{
	std::size_t bulkSize = 3;

	if (argc == 2)
		bulkSize = atoi(argv[1]);
	else
		std::cout << "Invalid input" << '\n';

	PackageManager manager{ bulkSize };

	manager.run();
}
