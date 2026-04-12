#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_TODO 100
#define MAX_POMODOROS 100
#define USER_FILE "users.dat"
#define SESSION_FILE "sessions.dat"
#define TODO_FILE "todos.dat"

// Define structures for user accounts, study sessions, and to-do items
typedef struct {
    char description[100];
    int completed;
} ToDoItem;

typedef struct {
    char category[50];
    struct tm startTime;
    struct tm endTime;
    int duration; // in seconds
    int sessionActive; // Indicates if a session is currently active
    int countdownActive; // Indicates if a countdown is currently active
    int countdownDuration; // in seconds
    char sessionType[20]; // Type of the session (Timer, Countdown, Pomodoro)
} StudySession;

typedef struct {
    int studyDuration; // in seconds
    int breakDuration; // in seconds
} PomodoroSession;

typedef struct {
    char username[50];
    int sessionCount;
    StudySession sessions[MAX_SESSIONS];
    ToDoItem todos[MAX_TODO];
    int todoCount;
    PomodoroSession pomodoros[MAX_POMODOROS];
    int pomodoroCount;
} User;

// Global array to store users and sessions
User users[MAX_USERS];
int userCount = 0;
int loggedIn = -1; // Index of logged-in user, -1 if no user is logged in

// Function prototypes
void clearScreen();
void displayHeader(const char* header);
void displayMainMenu();
void displayUserMenu();
void createUser();
void loginUser();
void startStudyOption();
void endSession();
void showStatistics();
void manageToDoList();
int findUserIndex(const char* username);
void pressEnterToContinue();
void startTimer();
void startCountdown();
int updateCountdown(int totalSeconds);
void pomodoro(int userIndex);
void loadUsers();
void saveUsers();
void loadSessions();
void saveSessions();
void loadToDos();
void saveToDos();



// Function to clear the screen
void clearScreen() {

        system("cls");
}
// Function to display a header
void displayHeader(const char* header) {
    clearScreen();
    printf("== %s ==\n\n", header);
}

// Function to display the main menu
void displayMainMenu() {
    system("color 3F");
    displayHeader("Welcome to Study Tracker");
    printf("Main Menu\n");
    printf("1. Create new user\n");
    printf("2. Login\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}

// Function to display the user menu
void displayUserMenu() {
    displayHeader("User Menu");
    printf("Welcome back %s! Ready to start the day? ^V^\n",users[loggedIn].username);
    printf("1. Start a study session\n");
    printf("2. Show statistics\n");
    printf("3. Manage to-do list\n");
    printf("4. Logout\n");
    printf("Enter your choice: ");
}

// Function to create a new user
void createUser() {
    displayHeader("Create New User");
    if (userCount >= MAX_USERS) {
        printf("Error: Maximum number of users reached.\n");
        pressEnterToContinue();
        return;
    }
    printf("Enter username: ");
    scanf("%49s", users[userCount].username);
    users[userCount].sessionCount = 0;
    users[userCount].todoCount = 0;
    userCount++;
    printf("User created successfully.\n");
    pressEnterToContinue();
    saveUsers(); // Save user data after creation
}

// Function to login a user
void loginUser() {
    displayHeader("Login");
    printf("Enter username: ");
    char username[50];
    scanf("%49s", username);
    int index = findUserIndex(username);
    if (index == -1) {
        printf("Error: User not found.\n");
        pressEnterToContinue();
    } else {
        loggedIn = index;
        printf("Login successful.\n");
        pressEnterToContinue();
    }
}

// Function to find a user index
int findUserIndex(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to press enter to continue
void pressEnterToContinue()
{
    printf("\nPress Enter to continue...");
    getchar(); // Wait for Enter key
    while (getchar() != '\n'); // Ensure buffer is empty
}

// Function to start a study session option
void startStudyOption() {
    if (loggedIn == -1 || users[loggedIn].sessionCount >= MAX_SESSIONS) {
        printf("Error: No user logged in or maximum sessions reached.\n");
        pressEnterToContinue();
        return;
    }
    displayHeader("Study Session Options");
    printf("1. Timer\n");
    printf("2. Countdown\n");
    printf("3. Pomodoro\n");
    printf("4. Exit to User Menu\n");
    printf("Enter your choice: ");
    int choice;
    scanf("%d", &choice);
    getchar(); // Clear the newline character after the number input

    switch (choice) {
        case 1:
            startTimer();
            break;
        case 2:
            startCountdown();
            break;
        case 3:
            pomodoro(loggedIn);
            break;
        case 4:
            return; // Exit to user menu
        default:
            printf("Invalid choice. Please try again.\n");
            pressEnterToContinue();
    }
}

// Function to start a timer
void startTimer() {
    if (users[loggedIn].sessions[users[loggedIn].sessionCount].sessionActive) {
        printf("Error: A session is already active. Please end the current session before starting a new one.\n");
        pressEnterToContinue();
        return;
    }

    displayHeader("Starting a New Study Session");
    printf("Enter study category: ");
    scanf("%49s", users[loggedIn].sessions[users[loggedIn].sessionCount].category);
    getchar(); // Clear the newline character after the input

    users[loggedIn].sessions[users[loggedIn].sessionCount].sessionActive = 1;
    strcpy(users[loggedIn].sessions[users[loggedIn].sessionCount].sessionType, "Timer");
    time_t start = time(NULL);
    users[loggedIn].sessions[users[loggedIn].sessionCount].startTime = *localtime(&start);
    displayHeader("Timer is on");
    printf("%s\n", users[loggedIn].sessions[users[loggedIn].sessionCount].category);
    printf("Press 'p' to pause the timer, 'r' to resume, 'q' to quit, and Enter to end the timer...\n");

    int paused = 0;
    time_t pauseStart, pauseEnd;
    int pauseDuration = 0;

    while (1) {
        if (!_kbhit()) {
            if (!paused) {
                time_t current = time(NULL);
                int elapsed = (int)difftime(current, start) - pauseDuration;

                int hours = elapsed / 3600;
                int minutes = (elapsed % 3600) / 60;
                int seconds = elapsed % 60;

                printf("\rElapsed time: %02d:%02d:%02d", hours, minutes, seconds);
                fflush(stdout);

                Sleep(100); // Use a shorter sleep to reduce drift
            }
        } else {
            char key = _getch();
            if (key == 'p') {
                if (!paused) {
                    paused = 1;
                    pauseStart = time(NULL);
                    printf("\nTimer paused. Press 'r' to resume, 'q' to quit, and Enter to end...\n");
                }
            } else if (key == 'r') {
                if (paused) {
                    paused = 0;
                    pauseEnd = time(NULL);
                    pauseDuration += (int)difftime(pauseEnd, pauseStart);
                    printf("\nTimer resumed. Press 'p' to pause, 'q' to quit, and Enter to end...\n");
                }
            } else if (key == 'q') {
                printf("\nTimer stopped.\n");
                saveSessions(); // Save session data before returning
                return;
            } else if (key == '\r') {
                printf("\nTimer stopped.\n");
                break;
            }
        }
    }

    time_t end = time(NULL);
    users[loggedIn].sessions[users[loggedIn].sessionCount].endTime = *localtime(&end);
    users[loggedIn].sessions[users[loggedIn].sessionCount].duration = (int)difftime(end, start) - pauseDuration;
    users[loggedIn].sessions[users[loggedIn].sessionCount].sessionActive = 0;
    users[loggedIn].sessionCount++;
    printf("\nStudy session ended. Duration: %02d:%02d:%02d.\n",
           users[loggedIn].sessions[users[loggedIn].sessionCount - 1].duration / 3600,
           (users[loggedIn].sessions[users[loggedIn].sessionCount - 1].duration % 3600) / 60,
           users[loggedIn].sessions[users[loggedIn].sessionCount - 1].duration % 60);

    // Beep 3 times
    for (int i = 0; i < 3; i++) {
        Beep(750, 300);
        Sleep(200); // Add a short delay between beeps
    }

    pressEnterToContinue();
    saveSessions(); // Save session data before returning
}


// Function to start a countdown
void startCountdown() {
    if (users[loggedIn].sessions[users[loggedIn].sessionCount].countdownActive) {
        printf("Error: A countdown is already active. Please wait for it to finish before starting a new one.\n");
        pressEnterToContinue();
        return;
    }
    displayHeader("Countdown:");
    printf("Enter study time in HH MM SS format: ");
    int hours, minutes, seconds;
    scanf("%d %d %d", &hours, &minutes, &seconds);
    getchar(); // Clear the newline character after the number input
    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    if (totalSeconds <= 0 || hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60) {
        printf("Error: Invalid time.\n");
        pressEnterToContinue();
        return;
    }
    users[loggedIn].sessions[users[loggedIn].sessionCount].countdownDuration = totalSeconds;
    users[loggedIn].sessions[users[loggedIn].sessionCount].countdownActive = 1;
    strcpy(users[loggedIn].sessions[users[loggedIn].sessionCount].sessionType, "Countdown");

    // Start the countdown and measure the duration
    time_t start = time(NULL);
    int totalElapsed = updateCountdown(totalSeconds);
    time_t end = time(NULL);

    users[loggedIn].sessions[users[loggedIn].sessionCount].startTime = *localtime(&start);
    users[loggedIn].sessions[users[loggedIn].sessionCount].endTime = *localtime(&end);
    users[loggedIn].sessions[users[loggedIn].sessionCount].duration = totalElapsed;
    users[loggedIn].sessions[users[loggedIn].sessionCount].countdownActive = 0;

    // Increment session count
    users[loggedIn].sessionCount++;

    // Save session data
    saveSessions();
}


// Function to update countdown
int updateCountdown(int totalSeconds) {
    displayHeader("Countdown");
    int elapsed = 0;

    while (totalSeconds > 0) {
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        printf("Time left: %02d:%02d:%02d\r", hours, minutes, seconds);
        fflush(stdout);
        Sleep(1000);
        totalSeconds--;
        elapsed++;
    }
    printf("\nCountdown finished!\n");
    for (int i = 0; i < 3; i++) {
        Beep(750, 300);
        Sleep(200);
    }
    pressEnterToContinue();
    saveSessions(); // Save session data before returning
    return elapsed;
}

// Function to start a Pomodoro session
void pomodoro(int userIndex) {
    displayHeader("Pomodoro Timer");

    printf("Enter study duration in minutes: ");
    int studyMinutes;
    scanf("%d", &studyMinutes);

    printf("Enter break duration in minutes: ");
    int breakMinutes;
    scanf("%d", &breakMinutes);

    printf("Enter number of Pomodoro sessions: ");
    int sessions;
    scanf("%d", &sessions);

    getchar(); // Clear the newline character after the input

    if (studyMinutes <= 0 || breakMinutes <= 0 || sessions <= 0) {
        printf("Error: Study, break durations, and number of sessions must be positive.\n");
        pressEnterToContinue();
        return;
    }

    int totalStudySeconds = studyMinutes * 60;
    int totalBreakSeconds = breakMinutes * 60;
    int pomodoroSessions = 0;
    int key;

    while (pomodoroSessions < sessions) {
        // Start study session
        users[userIndex].sessions[users[userIndex].sessionCount].sessionActive = 1;
        strcpy(users[userIndex].sessions[users[userIndex].sessionCount].sessionType, "Pomodoro");
        time_t start = time(NULL);
        users[userIndex].sessions[users[userIndex].sessionCount].startTime = *localtime(&start);

        displayHeader("Pomodoro Study Session");


        for (int i = totalStudySeconds; i > 0; i--) {
            printf("\rStudy time remaining: %02d:%02d", i / 60, i % 60);
            fflush(stdout);
            Sleep(1000);

            if (_kbhit()) {
                key = _getch();
                if (key == 'p') {
                    printf("\nPaused. Press 'r' to resume, 'q' to quit.\n");
                    while (1) {
                        if (_kbhit()) {
                            key = _getch();
                            if (key == 'q') {
                                saveSessions(); // Save session data before returning
                                return;
                            } else if (key == 'r') {
                                break;
                            }
                        }
                    }
                } else if (key == 'q') {
                    saveSessions(); // Save session data before returning
                    return;
                }
            }
        }

        time_t end = time(NULL);
        users[userIndex].sessions[users[userIndex].sessionCount].endTime = *localtime(&end);
        users[userIndex].sessions[users[userIndex].sessionCount].duration = (int)difftime(end, start);
        users[userIndex].sessions[users[userIndex].sessionCount].sessionActive = 0;
        users[userIndex].sessionCount++;

        // Bell ringing after study session ends
        for (int i = 0; i < 3; i++) {
            Beep(750, 300);
            Sleep(200);
        }

        saveSessions(); // Save session data

        // Break session
        displayHeader("Pomodoro Break");
        for (int i = totalBreakSeconds; i > 0; i--) {
            printf("\rBreak time remaining: %02d:%02d", i / 60, i % 60);
            fflush(stdout);
            Sleep(1000);

            if (_kbhit()) {
                key = _getch();
                if (key == 'p') {
                    printf("\nPaused. Press 'r' to resume, 'q' to quit.\n");
                    while (1) {
                        if (_kbhit()) {
                            key = _getch();
                            if (key == 'q') {
                                saveSessions(); // Save session data before returning
                                return;
                            } else if (key == 'r') {
                                break;
                            }
                        }
                    }
                } else if (key == 'q') {



                 saveSessions(); // Save session data before returning
                    return;
                }
            }
        }

        pomodoroSessions++;
    }
    for (int i = 0; i < 3; i++) {
            Beep(750, 300);
            Sleep(200);
        }

    // All Pomodoro sessions are complete
    printf("\nDo you want to start another Pomodoro session? (y/n): ");
    char choice = getchar();
    getchar(); // Clear the newline character after the input

    if (choice == 'y' || choice == 'Y') {
        pomodoro(userIndex); // Start another Pomodoro session
    } else {
        printf("Pomodoro sessions completed.\n");

        pressEnterToContinue();
    }
}

// Function to show statistics
void showStatistics() {
    displayHeader("Statistics");
    int timerSessions = 0, countdownSessions = 0, pomodoroSessions = 0;
    int totalTimerDuration = 0, totalCountdownDuration = 0, totalPomodoroDuration = 0;

    // Calculate total duration for each session type
    for (int i = 0; i < users[loggedIn].sessionCount; i++) {
        if (strcmp(users[loggedIn].sessions[i].sessionType, "Timer") == 0) {
            timerSessions++;
            totalTimerDuration += users[loggedIn].sessions[i].duration;
        } else if (strcmp(users[loggedIn].sessions[i].sessionType, "Countdown") == 0) {
            countdownSessions++;
            totalCountdownDuration += users[loggedIn].sessions[i].duration;
        } else if (strcmp(users[loggedIn].sessions[i].sessionType, "Pomodoro") == 0) {
            pomodoroSessions++;
            totalPomodoroDuration += users[loggedIn].sessions[i].duration;
        }
    }

    // Convert total timer duration to hours, minutes, and seconds
    int totalTimerHours = totalTimerDuration / 3600;
    int totalTimerMinutes = (totalTimerDuration % 3600) / 60;
    int totalTimerSeconds = totalTimerDuration % 60;

    printf("Total sessions: %d\n", users[loggedIn].sessionCount);
    printf("Timer sessions: %d (Total duration: %02d:%02d:%02d)\n", timerSessions,
           totalTimerHours, totalTimerMinutes, totalTimerSeconds);
    printf("Countdown sessions: %d (Total duration: %02d:%02d:%02d)\n", countdownSessions,
           totalCountdownDuration / 3600, (totalCountdownDuration % 3600) / 60, totalCountdownDuration % 60);
    printf("Pomodoro sessions: %d (Total duration: %02d:%02d:%02d)\n", pomodoroSessions,
           totalPomodoroDuration / 3600, (totalPomodoroDuration % 3600) / 60, totalPomodoroDuration % 60);
           if (users[loggedIn].sessionCount==0)
           {
               printf("Get to work!>:(");
           }
           else
           {
                printf("Well done! Keep going! ");
           }
    pressEnterToContinue();
}

// Function to manage to-do list
void manageToDoList() {
    loadToDos(); // Load the to-do list data when the function starts

    while (1) {
        displayHeader("To-Do List");
        printf("1. Add to-do item(s)\n");
        printf("2. Mark item as completed\n");
        printf("3. View all to-do items\n");
        printf("4. Delete a to-do item\n"); // Added delete option
        printf("5. Exit to User Menu\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        getchar(); // Clear the newline character after the number input

        switch (choice) {
            case 1: {
                while (1) {
                    if (users[loggedIn].todoCount >= MAX_TODO) {
                        printf("Error: Maximum number of to-do items reached.\n");
                        pressEnterToContinue();
                        break;
                    }
                    displayHeader("Add To-Do Item");
                    printf("Enter description: ");
                    fgets(users[loggedIn].todos[users[loggedIn].todoCount].description, 100, stdin);
                    users[loggedIn].todos[users[loggedIn].todoCount].description[strcspn(users[loggedIn].todos[users[loggedIn].todoCount].description, "\n")] = '\0'; // Remove trailing newline
                    users[loggedIn].todos[users[loggedIn].todoCount].completed = 0;
                    users[loggedIn].todoCount++;
                    printf("To-do item added.\n");

                    printf("Do you want to add another item? (y/n): ");
                    char addMore;
                    scanf(" %c", &addMore);
                    getchar(); // Clear the newline character after the input
                    if (addMore == 'n' || addMore == 'N') {
                        break;
                    }
                }
                saveToDos(); // Save the updated to-do list
                break;
            }
            case 2: {
                if (users[loggedIn].todoCount == 0) {
                    printf("No to-do items to mark as completed. Get to work RIGHT NOW >:( \n");
                    pressEnterToContinue();
                    break;
                }
                displayHeader("Mark To-Do Item as Completed");
                printf("To-do items:\n");
                for (int i = 0; i < users[loggedIn].todoCount; i++) {
                    printf("%d. [%c] %s\n", i + 1, users[loggedIn].todos[i].completed ? 'x' : ' ', users[loggedIn].todos[i].description);
                }
                printf("Enter the number of the item to mark as completed: ");
                int itemNumber;
                scanf("%d", &itemNumber);
                getchar(); // Clear the newline character after the number input
                if (itemNumber < 1 || itemNumber > users[loggedIn].todoCount) {
                    printf("Error: Invalid item number.\n");
                    pressEnterToContinue();
                    break;
                }
                users[loggedIn].todos[itemNumber - 1].completed = 1;
                printf("To-do item marked as completed.\n");
                pressEnterToContinue();
                saveToDos(); // Save the updated to-do list
                break;
            }
            case 3: {
                displayHeader("View All To-Do Items");
                if (users[loggedIn].todoCount == 0) {
                    printf("No to-do items.\n");
                } else {
                    printf("To-do items:\n");
                    for (int i = 0; i < users[loggedIn].todoCount; i++) {
                        printf("%d. [%c] %s\n", i + 1, users[loggedIn].todos[i].completed ? 'x' : ' ', users[loggedIn].todos[i].description);
                    }
                }
                pressEnterToContinue();
                break;
            }
            case 4: {
                if (users[loggedIn].todoCount == 0) {
                    printf("No to-do items to delete.\n");
                    pressEnterToContinue();
                    break;
                }
                displayHeader("Delete To-Do Item");
                printf("To-do items:\n");
                for (int i = 0; i < users[loggedIn].todoCount; i++) {
                    printf("%d. %s\n", i + 1, users[loggedIn].todos[i].description);
                }
                printf("Enter the number of the item to delete: ");
                int itemNumber;
                scanf("%d", &itemNumber);
                getchar(); // Clear the newline character after the number input
                if (itemNumber < 1 || itemNumber > users[loggedIn].todoCount) {
                    printf("Error: Invalid item number.\n");
                    pressEnterToContinue();
                    break;
                }
                // Shift remaining items to overwrite the deleted item
                for (int i = itemNumber - 1; i < users[loggedIn].todoCount - 1; i++) {
                    users[loggedIn].todos[i] = users[loggedIn].todos[i + 1];
                }
                users[loggedIn].todoCount--;
                printf("To-do item deleted.\n");
                pressEnterToContinue();
                saveToDos(); // Save the updated to-do list
                break;
            }
            case 5:
                saveToDos(); // Save the to-do list before exiting
                return; // Exit to user menu
            default:
                printf("Invalid choice. Please try again.\n");
                pressEnterToContinue();
        }
    }
}

// Function to load users from file
void loadUsers() {
    FILE* file = fopen(USER_FILE, "rb");
    if (file != NULL) {
        fread(&userCount, sizeof(int), 1, file);
        fread(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

// Function to save users to file
void saveUsers() {
    FILE* file = fopen(USER_FILE, "wb");
    if (file != NULL) {
        fwrite(&userCount, sizeof(int), 1, file);
        fwrite(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

// Function to load sessions from file
void loadSessions() {
    FILE* file = fopen(SESSION_FILE, "rb");
    if (file != NULL) {
        fread(&userCount, sizeof(int), 1, file);
        fread(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

// Function to save sessions to file
void saveSessions() {
    FILE* file = fopen(SESSION_FILE, "wb");
    if (file != NULL) {
        fwrite(&userCount, sizeof(int), 1, file);
        fwrite(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

// Function to load to-dos from file
void loadToDos() {
    FILE* file = fopen(TODO_FILE, "rb");
    if (file != NULL) {
        fread(&userCount, sizeof(int), 1, file);
        for (int i = 0; i < userCount; i++) {
            fread(&users[i].todoCount, sizeof(int), 1, file);
            fread(users[i].todos, sizeof(ToDoItem), users[i].todoCount, file);
        }
        fclose(file);
    }
}

void saveToDos() {
    FILE* file = fopen(TODO_FILE, "wb");
    if (file != NULL) {
        fwrite(&userCount, sizeof(int), 1, file);
        for (int i = 0; i < userCount; i++) {
            fwrite(&users[i].todoCount, sizeof(int), 1, file);
            fwrite(users[i].todos, sizeof(ToDoItem), users[i].todoCount, file);
        }
        fclose(file);
    }
}


int main() {
    loadUsers();  // Load users from file
    loadSessions();// Load users from file
    loadToDos(); // Load to-do data at startup
    while (1) {
        if (loggedIn == -1) {
            displayMainMenu();
            int choice;
            scanf("%d", &choice);
            getchar(); // Clear the newline character after the number input
            switch (choice) {
                case 1:
                    createUser();
                    break;
                case 2:
                    loginUser();
                    break;
                case 3:
                    saveUsers(); // Save users before exiting
                    return 0; // Exit the program
                default:
                    printf("Invalid choice. Please try again.\n");
                    pressEnterToContinue();
            }
        } else {
            displayUserMenu();
            int choice;
            scanf("%d", &choice);
            getchar(); // Clear the newline character after the number input
            switch (choice) {
                case 1:
                    startStudyOption();
                    break;
                case 2:
                    showStatistics();
                    break;
                case 3:
                    manageToDoList();
                    break;
                case 4:
                    loggedIn = -1; // Logout
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                    pressEnterToContinue();
            }
        }
    }
    saveUsers(); // Save user data before exiting
    saveSessions(); // Save session data before exiting
    saveToDos(); // Save to-do data before exiting
    return 0;
}
