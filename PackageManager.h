#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

class Storage // or container
{};

class TempBuffer // or container
{};

class Parser
{};

class PackageManager
{
    public:
    PackageManager(std::size_t packages, std::size_t bulkSize = 3) 
        : packages{packages},
          bulkSize{bulkSize}
    { }
    
    void run() {
        std::string line;
        while(getline(std::cin, line))
        {
            std::size_t foundCmd = line.find_first_of("cmd");
            if (foundCmd != std::string::npos)
            {
                if(bracketOccured == false)
                {
                    ++currentTimeReceivedPackages;

                    std::cout << "Found cmd." << '\n';

                    packageStorage.push_back(line);

                    if(currentTimeReceivedPackages == bulkSize)
                    {
                        currentTimeReceivedPackages = 0;

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
                std::cout << "Found {" << '\n';
                bracketOccured = true;
                
                currentTimeReceivedPackages = 0;
                
                printStorage();
            }
            else if (line == "}")
            {
                bracketOccured = false;
                printStorage();
            }
            //std::cout << "currentTimeReceivedPackages: " << currentTimeReceivedPackages;
            //std::cout << "line: " << line;
            //
            //std::cout << '\n';
            
            
        }
        
        std::cout << "End: " << '\n';
        printStorage();
    }
    
    void setBulkSize (std::size_t bulkSize) {
        this->bulkSize = bulkSize;
    }
    
    private:
    void printStorage() {
        std::ostringstream oss;
        
        oss << "bulk: ";
        for(std::size_t i = 0; i < packageStorage.size(); ++i)
        {
            oss << packageStorage[i];
            
            if(i + 1 != packageStorage.size())
                oss << ", ";
        }
            
        std::cout << oss.str();
        
        packageStorage.clear();
        std::cout << '\n';
    }
    
    std::size_t packages;
    std::size_t bulkSize;
    std::size_t currentTimeReceivedPackages{0};
    std::vector<std::string> packageStorage;
    bool bracketOccured{false};
};

int main(int, char**)
{
    std::size_t N{};
    std::cin >> N;
    std::cin.ignore();
    
    PackageManager manager{N};
    
    manager.run();
}
