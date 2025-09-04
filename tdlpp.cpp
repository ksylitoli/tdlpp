#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
// Linux/macOS color codes
#define FOREGROUND_RED      0x01
#define FOREGROUND_GREEN    0x02
#define FOREGROUND_BLUE     0x04
#define FOREGROUND_INTENSE  0x08
#define FOREGROUND_WHITE    (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#endif

// Cross-platform console color
void setConsoleColor(int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
#else
    // ANSI escape codes for Unix-like systems
    const char* codes[] = {
        "\033[0m",    // 7: White
        "\033[31m",   // 12: Red
        "\033[32m",   // 10: Green
        "\033[33m",   // 14: Yellow
        "\033[34m",   // 9: Blue
        "\033[35m",   // 13: Purple/Magenta
        "\033[36m"    // 11: Cyan
    };
    
    int index = 0;
    switch(color) {
        case 12: index = 1; break; // Red
        case 10: index = 2; break; // Green
        case 14: index = 3; break; // Yellow
        case 9:  index = 4; break; // Blue
        case 13: index = 5; break; // Purple
        case 11: index = 6; break; // Cyan
        default: index = 0; break; // White
    }
    std::cout << codes[index];
#endif
}

// Cross-platform console clear
void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

}

// Task struct representing both regular and scheduled tasks
struct Task {
    std::string description;
    bool completed;
    bool scheduled;
    std::string time; // valid if scheduled

    Task(std::string desc, bool comp = false, bool sched = false, std::string t = "")
        : description(std::move(desc)), completed(comp), scheduled(sched), time(std::move(t)) {}
};

#include <sstream>  // Make sure this include is present near the top of the file


std::vector<std::string> split(const std::string& s, char delimiter) {

    std::vector<std::string> tokens;

    std::stringstream ss(s);

    std::string token;

    while (getline(ss, token, delimiter)) {

        tokens.push_back(token);

    }

    return tokens;
}

class ArchTodo {
private:
    std::string dataFile = "tdlpp.txt";
    std::vector<Task> tasks;

    void loadTasks() {
    tasks.clear();
    std::ifstream file(dataFile);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == '1' || line[0] == '0') {
            bool completed = (line[0] == '1');
            std::string desc = line.substr(2);
            tasks.emplace_back(desc, completed, false);
            continue;
        }

        if (line[0] == 'S') {
            auto parts = split(line, '|');
            if (parts.size() == 4) {
                bool completed = (parts[2] == "1");
                tasks.emplace_back(parts[3], completed, true, parts[1]);
            }
            continue;
        }

        // Optionally log/handle unrecognized lines or corrupt data
    }
}

    void saveTasks() {
        std::ofstream file(dataFile);
        if (!file.is_open()) {
            setConsoleColor(12);
            std::cerr << "Error: Could not save tasks to file!" << std::endl;
            setConsoleColor(7);
            return;
        }
        for (const auto& t : tasks) {
            if (t.scheduled) {
                file << "S|" << t.time << "|" << (t.completed ? "1" : "0") << "|" << t.description << "\n";
            } else {
                file << (t.completed ? "1 " : "0 ") << t.description << "\n";
            }
        }
    }

    void displayHeader() {
        setConsoleColor(11);
        std::cout << R"(

)" << std::endl;
        setConsoleColor(7);
        std::cout << "=========================================================\n";
        std::cout << "|                 tdl++ - Simple Todo List          	|\n";
        std::cout << "|                     Version 1.0.1                   	|\n";
        std::cout << "=========================================================\n\n";
    }

public:
    ArchTodo() {
        loadTasks();
    }

    void addTask(const std::string& desc) {
        tasks.emplace_back(desc);
        saveTasks();
        setConsoleColor(10);
        std::cout << "[+] Added: ";
        setConsoleColor(7);
        std::cout << desc << "\n";
    }

    void addScheduledTask(const std::string& time, const std::string& desc) {
        tasks.emplace_back(desc, false, true, time);
        saveTasks();
        setConsoleColor(10);
        std::cout << "[+] Scheduled: ";
        setConsoleColor(7);
        std::cout << "[" << time << "] " << desc << "\n";
    }

    void listTasks() {
        clearConsole();
        displayHeader();

        if (tasks.empty()) {
            setConsoleColor(14);
            std::cout << "  No tasks found. Use 'add <task>' to create one.\n\n";
            setConsoleColor(7);
            return;
        }

        // Regular tasks
        bool hasRegular = std::any_of(tasks.begin(), tasks.end(), [](const Task& t) { return !t.scheduled; });
        if (hasRegular) {
            setConsoleColor(14);
            std::cout << "  REGULAR TASKS:\n  --------------------------------\n";
            setConsoleColor(7);
            int idx = 1;
            for (const auto& t : tasks) {
                if (!t.scheduled) {
                    std::cout << "  " << idx++ << ".   ";
                    setConsoleColor(t.completed ? 10 : 12);
                    std::cout << (t.completed ? "[DONE] " : "[PEND] ");
                    setConsoleColor(7);
                    std::cout << t.description << "\n";
                }
            }
            std::cout << "\n";
        }

        // Scheduled tasks
        bool hasScheduled = std::any_of(tasks.begin(), tasks.end(), [](const Task& t) { return t.scheduled; });
        if (hasScheduled) {
            setConsoleColor(14);
            std::cout << "  SCHEDULED TASKS:\n  --------------------------------\n";
            setConsoleColor(7);
            int idx = 1;
            for (const auto& t : tasks) {
                if (t.scheduled) {
                    std::cout << "  " << idx++ << ".   ";
                    setConsoleColor(t.completed ? 10 : 13);
                    std::cout << (t.completed ? "[DONE] " : "[TIME] ");
                    setConsoleColor(7);
                    std::cout << "[" << t.time << "] " << t.description << "\n";
                }
            }
            std::cout << "\n";
        }

        showStats();
    }

    void markComplete(int id) {
        if (id < 1 || id > static_cast<int>(tasks.size())) {
            setConsoleColor(12);
            std::cout << "Error: Invalid task ID\n";
            setConsoleColor(7);
            return;
        }
        Task& t = tasks[id - 1];
        if (t.completed) {
            setConsoleColor(14);
            std::cout << "Task is already completed!\n";
            setConsoleColor(7);
            return;
        }
        t.completed = true;
        saveTasks();
        setConsoleColor(10);
        std::cout << "[OK] Marked as complete: ";
        setConsoleColor(7);
        if (t.scheduled) std::cout << "[" << t.time << "] ";
        std::cout << t.description << "\n";
    }

    void removeTask(int id) {
        if (id < 1 || id > static_cast<int>(tasks.size())) {
            setConsoleColor(12);
            std::cout << "Error: Invalid task ID\n";
            setConsoleColor(7);
            return;
        }
        Task t = tasks[id - 1];
        tasks.erase(tasks.begin() + id - 1);
        saveTasks();
        setConsoleColor(14);
        std::cout << "[-] Removed: ";
        setConsoleColor(7);
        if (t.scheduled) std::cout << "[" << t.time << "] ";
        std::cout << t.description << "\n";
    }

    void clearCompleted() {
        clearConsole();
        displayHeader();

        int clearedCount = std::count_if(tasks.begin(), tasks.end(), [](const Task& t) { return t.completed; });
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(), [](const Task& t) { return t.completed; }), tasks.end());
        saveTasks();

        setConsoleColor(10);
        std::cout << "[OK] Cleared " << clearedCount << " completed tasks\n\n";
        setConsoleColor(7);

        showStats();
    }

void showStats() {
    int total = tasks.size();
    int completedCount = 0;
    int scheduledPendingCount = 0;
    
    // Single pass through tasks instead of multiple count_if calls
    for (const Task& t : tasks) {
        if (t.completed) {
            completedCount++;
        } else if (t.scheduled) {
            scheduledPendingCount++;
        }
    }
    
    int pendingCount = total - completedCount;
    
    setConsoleColor(11);
    std::cout << "  STATISTICS:\n  --------------------------------\n";
    
    setConsoleColor(13);
    std::cout << "  Pending (Scheduled): " << scheduledPendingCount << "\n";
    
    setConsoleColor(12);
    std::cout << "  Pending: " << pendingCount << "\n";
    
    setConsoleColor(7);
    std::cout << "\n";
}
    void showHelp() {
        clearConsole();
        displayHeader();

        setConsoleColor(14);
        std::cout << "  Available Commands:\n  -------------------\n";
        setConsoleColor(7);

        std::cout << "  "; setConsoleColor(10); std::cout << "list"; setConsoleColor(7); std::cout << "          - Show all tasks\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "stats"; setConsoleColor(7); std::cout << "         - Show task statistics\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "add <task>"; setConsoleColor(7); std::cout << "     - Add a new task\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "schedule <time> <task>"; setConsoleColor(7); std::cout << " - Add a scheduled task\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "done <id>"; setConsoleColor(7); std::cout << "      - Mark task as complete\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "rm <id>"; setConsoleColor(7); std::cout << "        - Remove a task\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "clear"; setConsoleColor(7); std::cout << "         - Remove completed tasks\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "help"; setConsoleColor(7); std::cout << "         - Show this help message\n";
        std::cout << "  "; setConsoleColor(10); std::cout << "exit"; setConsoleColor(7); std::cout << "         - Exit the program\n\n";

        setConsoleColor(14);
        std::cout << "  Examples:\n  ---------\n";
        setConsoleColor(7);
        std::cout << "  add \"Update system packages\"\n";
        std::cout << "  schedule 14:30 \"Team meeting\"\n";
        std::cout << "  done 1\n";
        std::cout << "  rm 2\n";
        std::cout << "  clear\n";
        std::cout << "  stats\n\n";
    }
};

int main() {
    ArchTodo todo;
    std::string command;

    SetConsoleTitle("tdl++ v 1.0");
    system("mode con: cols=80 lines=30");
    todo.showHelp();

    while (true) {
        setConsoleColor(10); std::cout << "user";
        setConsoleColor(12); std::cout << "@";
        setConsoleColor(11); std::cout << "tdl++";
        setConsoleColor(7); std::cout << "> ";
        std::getline(std::cin, command);

        if (command == "exit" || command == "quit") {
            break;
        } else if (command == "list") {
            todo.listTasks();
        } else if (command == "stats") {
            clearConsole();
            todo.showHelp();
            todo.showStats();
        } else if (command == "help") {
            todo.showHelp();
        } else if (command == "clear") {
            todo.clearCompleted();
        } else if (command.find("add ") == 0) {
            if (command.length() > 4) {
                todo.addTask(command.substr(4));
            } else {
                setConsoleColor(12); std::cout << "Error: Usage: add <task>\n"; setConsoleColor(7);
            }
        } else if (command.find("schedule ") == 0) {
            size_t spacePos = command.find(' ', 9);
            if (spacePos != std::string::npos && command.length() > spacePos + 1) {
                std::string time = command.substr(9, spacePos - 9);
                std::string task = command.substr(spacePos + 1);
                todo.addScheduledTask(time, task);
            } else {
                setConsoleColor(12); std::cout << "Error: Usage: schedule <time> <task>\n"; setConsoleColor(7);
            }
        } else if (command.find("done ") == 0) {
            try {
                int id = std::stoi(command.substr(5));
                todo.markComplete(id);
            } catch (...) {
                setConsoleColor(12); std::cout << "Error: Usage: done <id>\n"; setConsoleColor(7);
            }
        } else if (command.find("rm ") == 0) {
            try {
                int id = std::stoi(command.substr(3));
                todo.removeTask(id);
            } catch (...) {
                setConsoleColor(12); std::cout << "Error: Usage: rm <id>\n"; setConsoleColor(7);
            }
        } else if (!command.empty()) {
            setConsoleColor(12);
            std::cout << "Unknown command: '" << command << "'\nType 'help' for available commands.\n";
            setConsoleColor(7);
        }
    }

    setConsoleColor(11);
    std::cout << "Goodbye! May your packages be updated.\n";
    setConsoleColor(7);
    return 0;
}
