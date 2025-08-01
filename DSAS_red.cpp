#include <iostream>
#include <chrono>
#include <tuple>
#include <string>

#include <windows.h> // for HANDLE
#include <psapi.h> // for GetProcessMemoryInfo
#include <list>

// Data Structures and Algorithms Statistics
namespace DSAS_red
{
    class Diagnostics
    {
        HANDLE process;
    public:
        Diagnostics()
        {
            process = GetCurrentProcess();
            if (process == NULL)
                std::cerr << "Failed to get current process handle. Error: " << GetLastError() << std::endl;
        }
        ~Diagnostics()
        {
            CloseHandle(process);
        }
    
        class Stopwatch
        {
            std::chrono::high_resolution_clock::time_point start_time;
            bool running;
        public:
            Stopwatch() : running(false) {}
    
            void Start()
            {
                start_time = std::chrono::high_resolution_clock::now();
                running = true;
            }
    
            void Stop()
            {
                running = false;
            }
    
            long long ElapsedMiliseconds() const
            {
                return running ? std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() :     0;
            }
        };
    
        // Returns a tuple with physical memory usage and virtual memory usage respectively (unit = KB)
        std::tuple<long,long> GetProcessMemory()
        {
            PROCESS_MEMORY_COUNTERS pmc;
    
            if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc)))
                return { pmc.WorkingSetSize / 1024, pmc.PagefileUsage / 1024 };
            
            std::cerr << "Failed to get process memory info. Error: " << GetLastError() << std::endl;
            return { 0 , 0 };
        }
    };
}

using namespace DSAS_red;

int main()
{
    Diagnostics* diagnostics = new Diagnostics();
    std::tuple<long, long> memUsageStart = diagnostics->GetProcessMemory();
    Diagnostics::Stopwatch* sw = new Diagnostics::Stopwatch();
    sw->Start();
    std::list<int> newList;
    for (int i = 0; i < 1000000; i++)
    {
        newList.emplace_back(1);
    }
    long long elapsedMiliseconds = sw->ElapsedMiliseconds();
    std::tuple<long, long> memUsageEnd = diagnostics->GetProcessMemory();
    long diffPhysical = std::get<0>(memUsageEnd) - std::get<0>(memUsageStart);
    long diffVirtual = std::get<1>(memUsageEnd) - std::get<1>(memUsageStart);

    std::cout << "Allocated 1,000,000 integers to list<int>\n\tElapsed Time: " + std::to_string(elapsedMiliseconds) + " ms"
        + "\n\tMemory Usage:\n\t\tPhysical: " + std::to_string(diffPhysical) + " KB\n\t\tVirtual: " + std::to_string(diffVirtual) + " KB";
    delete diagnostics;
    delete sw;
    return 0;
}