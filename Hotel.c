#include "hotel.h"

int main() {
    // Initialize variables and structures
    srand(time(NULL)); // Seed the random number generator
    User users[MAX_USERS];
    Room rooms[MAX_ROOMS];
    Booking *bookingHead = NULL;
    LoginLogoutEvent events[MAX_EVENTS];
    int userCount = 0;
    int roomCount = 0;
    int bookingCount = 0;
    int eventCount = 0;
    int choice = 0;

    // Load data from files
    loadUsers(users, &userCount); // Load users from file to the array
    loadRooms(rooms, &roomCount); // Load rooms from file to the array
    bookingHead = loadBookings(&bookingCount); // Load existing bookings
    loadEvents(events, &eventCount); // Load events from file
    addAdministratorInfoToFile(users, &userCount); // Ensure admin info is present
    autoCancelReservations(&bookingHead); // Auto cancel reservations which have check in date are late

    while (choice != 4) {
        displayMainMenu();
        printf("Enter your choice (1-4): ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left in the input buffer

        switch (choice) {
            case 1:
                loginUser(users, &userCount, rooms, &roomCount, &bookingHead, &bookingCount, events, &eventCount);
                break;
            case 2:
                checkRoomAvailabilities(&bookingHead, rooms, &roomCount);
                break;
            case 3:
                searchRooms(rooms, &roomCount);
                break;
            case 4:
                printf("Exiting... Thank you!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    // Free allocated memory for bookings
    freeBookings(bookingHead);

    return 0;
}



// FUNCTION IMPLEMENTATION

// UTILITY FUNCTIONS

// Function to check if a string contains any spaces
bool containSpaces(const char *str) {
    // Iterate through the characters in the string
    while (*str) {
        // Check if the current character is a space
        if (isspace((unsigned char)*str)) {
            return true; // Return true if a space is found
        }
        str++; // Move to the next character in the string
    }
    return false; // Return false if no space is found in the string
}


// Function to check if a string contains any numeric characters
bool containNumeric(const char *str) {
    // Iterate through the characters in the string
    while (*str) {
        // Check if the current character is a numeric character
        if (isdigit((unsigned char)*str))
            return true; // Return true if a numeric character is found
        str++; // Move to the next character in the string
    }
    return false; // Return false if no numeric character is found in the string
}


// Function to check if a string contains any ',' character
bool containComma(const char *str) {
    // Iterate through the characters in the string
    while (*str) {
        // Check if the current character is a comma
        if (*str == ',') {
            return true; // Return true if a comma is found
        }
        str++; // Move to the next character in the string
    }
    return false; // Return false if no comma is found in the string
}


// Function to check if a string is empty
bool isEmpty(const char *str) {
    // Check if the string pointer is NULL or if the first character of the string is the null terminator '\0'
    return (str == NULL || str[0] == '\0');
}


// Function to clear input buffer
void clearInputBuffer() {
    int c;
    // Read characters from the input buffer until a newline character or EOF is encountered
    while ((c = getchar()) != '\n' && c != EOF);
}


// Function to get current date
void getCurrentDate(char* buffer, int bufferSize) {
    // Get the current time
    time_t t = time(NULL);
    // Convert to local time
    struct tm tm = *localtime(&t);
    // Format the date as a string and store it in the buffer
    snprintf(buffer, bufferSize, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}


// Function to validate and get date in the format YYYY-MM-DD
void getValidDate(char* date, const char* prompt, int allowPastDate, const char* referenceDate) {
    regex_t regex;
    int ret;

    // Compile the regex for date format YYYY-MM-DD
    ret = regcomp(&regex, "^[0-9]{4}-(0[1-9]|1[0-2])-([0-2][0-9]|3[01])$", REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("%s", prompt);
        if (scanf("%10s", date) != 1) {
            printf("Error reading date.\n");
            exit(EXIT_FAILURE);
        }
        getchar(); // Consume newline character

        // Execute the regex match
        ret = regexec(&regex, date, 0, NULL, 0);
        if (!ret) {
            // Date format is correct, now apply additional constraints if necessary
            if (!allowPastDate && referenceDate != NULL && strcmp(date, referenceDate) < 0) {
                printf("Date must be the current date or later.\n");
                continue;
            }
            break; // Date is valid
        } else {
            printf("Invalid date format. Please enter the date in YYYY-MM-DD format.\n");
        }
    }

    regfree(&regex);
}


// Function to get the current timestamp in "YYYY-MM-DD HH:MM:SS" format
void getCurrentTimestamp(char *buffer, int bufferSize) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", t);
}
// USER FUNCTIONS

// Function to save users to file
void saveUsers(User users[], int userCount) {
    // Open the users file in write mode
    FILE *file = fopen(USERS_FILE, "w");
    // Check if the file was successfully opened
    if (file == NULL) {
        printf("Error: Unable to open users file for writing.\n");
        return;
    }

    // Write user data to the file
    for (int i = 0; i < userCount; i++) {
        fprintf(file, "%d, %s, %s, %s, %s, %d\n", users[i].orderNum, users[i].name, users[i].id, users[i].password, users[i].role, users[i].active);
    }

    // Close the file
    fclose(file);
}


// Function to load users from file
int loadUsers(User users[], int *userCount) {
    // Open the users file in read mode
    FILE *file = fopen(USERS_FILE, "r");
    // Check if the file was successfully opened
    if (file == NULL) {
        printf("Error: Unable to open users file for reading.\n");
        *userCount = 0;
        return 0; // No users file found, start with empty user list
    }

    *userCount = 0; // Initialize user count
    // Read user data from the file
    while (fscanf(file, "%d, %25[^,], %25[^,], %25[^,], %25[^,], %d\n", &users[*userCount].orderNum, users[*userCount].name, users[*userCount].id, users[*userCount].password, users[*userCount].role, &users[*userCount].active) == 6) {
        (*userCount)++; // Increment user count
    }

    // Close the file
    fclose(file);
    return 1; // Users loaded successfully
}


// Function to add administrator information to the file
void addAdministratorInfoToFile(User users[], int *userCount) {
    
    // Check if administrator information already exists
    for (int i = 0; i < *userCount; i++) {
        if (strcmp(users[i].id, "admin") == 0 && strcmp(users[i].password, "admin") == 0 && strcmp(users[i].role, "Administrator") == 0 && users[i].active == 1) {
            return;
        }
    }

    // Add administrator information to the users array
    users[*userCount].orderNum = 1; // Set the order number for the administrator
    strcpy(users[*userCount].name, "Administrator"); // Set the name for the administrator
    strcpy(users[*userCount].id, "admin"); // Set the ID for the administrator
    strcpy(users[*userCount].password, "admin"); // Set the password for the administrator
    strcpy(users[*userCount].role, "Administrator"); // Set the role for the administrator
    users[*userCount].active = 1; // Set the status as active for the administrator
    (*userCount)++; // Increment the user count

    // Save the updated users array to file
    saveUsers(users, *userCount);
}


// Function to prompt for user name and validate, returning a dynamically allocated string
char* promptValidName() {
    char input_name[NAME_LENGTH];
    bool validInput = false;
    while (!validInput) {
        printf("\nEnter name: ");
        fgets(input_name, NAME_LENGTH, stdin);
        input_name[strcspn(input_name, "\n")] = '\0'; // Remove newline character

        // Check if name exceeds the maximum length
        if (strlen(input_name) > (int)(NAME_LENGTH - 2)) {
            printf("Name is too long. Please enter a name with at most %d characters.\n", NAME_LENGTH - 2);
            clearInputBuffer(); // Clear any extra characters from the input buffer
        }
        // Check if name contains any numeric characters
        else if (containNumeric(input_name)) {
            printf("Name should not contain numeric characters. Please try again.\n");
        }
        // Check if name contains any comma characters
        else if (containComma(input_name)) {
            printf("Name should not contain comma characters. Please try again.\n");
        }
        // Check if name is empty
        else if (isEmpty(input_name)) {
            printf("Name cannot be empty. Please try again.\n");
        }
        else {
            validInput = true;
        }
    }
    return strdup(input_name); // Dynamically allocate memory and copy the name
}


// Function to prompt for user ID and validate, returning a dynamically allocated string
char* promptValidID(User users[], int userCount) {
    char input_id[ID_LENGTH];
    bool validInput = false;
    while (!validInput) {
        printf("\nEnter ID: ");
        fgets(input_id, ID_LENGTH, stdin);
        input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

        // Check if username exceeds the maximum length
        if (strlen(input_id) > (int)(ID_LENGTH - 2)) {
            printf("ID is too long. Please enter an ID with at most %d characters.\n", ID_LENGTH - 2);
            clearInputBuffer(); // Clear any extra characters from the input buffer
        }
        // Check if ID contains spaces
        else if (containSpaces(input_id)) {
            printf("ID should not contain spaces. Please try again.\n");
        }
        // Check if name contains any comma characters
        else if (containComma(input_id)) {
            printf("Name should not contain comma characters. Please try again.\n");
        }
        // Check if ID is empty
        else if (isEmpty(input_id)) {
            printf("ID cannot be empty. Please try again.\n");
        }
        else {
            // Check if ID is already in use
            bool idExists = false;
            for (int i = 0; i < userCount; i++) {
                if (strcmp(users[i].id, input_id) == 0) {
                    idExists = true;
                    break;
                }
            }
            if (idExists) {
                printf("ID is already in use. Please try again.\n");
            } else {
                validInput = true; // ID is valid, break out of the loop
            }
        }
    }
    return strdup(input_id); // Dynamically allocate memory and copy the ID
}


// Function to prompt for user password and validate, returning a dynamically allocated string
char* promptValidPassword() {
    char input_password[PASSWORD_LENGTH];
    bool validInput = false;
    while (!validInput) {
        printf("\nEnter password: ");
        fgets(input_password, PASSWORD_LENGTH, stdin);
        input_password[strcspn(input_password, "\n")] = '\0'; // Remove newline character

        // Check if password exceeds the maximum length
        if (strlen(input_password) > (int)(PASSWORD_LENGTH - 2)) {
            printf("Password is too long. Please enter a password with at most %d characters.\n", PASSWORD_LENGTH - 2);
            clearInputBuffer(); // Clear any extra characters from the input buffer
        }
        // Check if password is empty
        else if (isEmpty(input_password)) {
            printf("Password cannot be empty. Please try again.\n");
        }
        else {
            validInput = true; // Password is valid, break out of the loop
        }
    }
    return strdup(input_password); // Dynamically allocate memory and copy the password
}


// Function to prompt for user role and validate, returning a dynamically allocated string
char* promptValidRole() {
    char input_role[ROLE_LENGTH];
    int numRole = 0;
    bool validInput = false;
    // Prompt user to select role
    do {
        printf("\n1. Hotel Staff\n2. Customer\n");
        printf("Enter number to select role (1-2): ");
        scanf("%d", &numRole);
        getchar(); // Remove newline character
        // Check if the selected number corresponds to a valid role
        if (numRole == 1) {
            strcpy(input_role, "Hotel Staff");
            validInput = true; // Role is valid, exit the loop
        } else if (numRole == 2) {
            strcpy(input_role, "Customer");
            validInput = true; // Role is valid, exit the loop
        } else {
            printf("Invalid number. Please enter a number between 1-2.\n");
        }
    } while (!validInput); // Continue looping until a valid role is selected
    return strdup(input_role); // Dynamically allocate memory and copy the role
}


// Function to register a new user
void registerUser(User users[], int *userCount) {

    char *input_name, *input_id, *input_password, *input_role;
    bool validInput = false;

    while (!validInput) {
        // Prompt user to enter name
        input_name = promptValidName();

        // Prompt user to enter ID
        input_id = promptValidID(users, *userCount);

        // Prompt user to enter password
        input_password = promptValidPassword();

        // Prompt user to select role
        input_role = promptValidRole();

        // Add user details to the users array
        users[*userCount].orderNum = *userCount + 1; // Assign order number based on user count
        strcpy(users[*userCount].name, input_name); // Copy name to user data
        strcpy(users[*userCount].id, input_id); // Copy ID to user data
        strcpy(users[*userCount].password, input_password); // Copy password to user data
        strcpy(users[*userCount].role, input_role); // Copy role to user data
        users[*userCount].active = 1; // Set user as active

        (*userCount)++; // Increment user count
        saveUsers(users, *userCount); // Save the new user to file
        printf("\nUser Account Created Successfully.\n");

        // Free dynamically allocated memory
        free(input_name);
        free(input_id);
        free(input_password);
        free(input_role);

        validInput = true;
    }
}


// Function to update user password
void updateUserPassword(User users[], int *userCount) {

    char input_id[ID_LENGTH];
    
    // Prompt user to enter ID for password update
    printf("Enter ID to update password: ");
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

    if (strcmp(input_id, "admin") == 0) {
        printf("Admin account cannot be updated.\n"); return;
    }

    // Iterate through users to find the user by ID
    for (int i = 0; i < *userCount; i++) {
        // Check if the entered ID matches any user ID
        if (strcmp(users[i].id, input_id) == 0) {
            // Display current password for confirmation
            printf("The current password: %s\n", users[i].password);
            // Prompt user to enter new password
            char* new_password = promptValidPassword();
            if (strcmp(new_password, users[i].password) == 0) {
                printf("New password can not be same as current password. Please try again.");
                free(new_password); // Free dynamically allocated memory
                return; // Exit the function
            }
            // Update the password for the user
            strcpy(users[i].password, new_password);
            saveUsers(users, *userCount); // Save the updated user data to file
            printf("Password updated successfully for user '%s'.\n", input_id);
            free(new_password); // Free dynamically allocated memory
            return; // Exit the function
        }
    }
    // If no user with the entered ID is found, display a message
    printf("User '%s' not found.\n", input_id);
}


// Function to update user status
void updateUserStatus(User users[], int *userCount) {

    char input_id[ID_LENGTH];
    int new_status = -1; // Initialize new_status to an invalid value to enter the while loop

    // Prompt user for ID
    printf("Enter ID to update status: ");
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character
    
    if (strcmp(input_id, "admin") == 0) {
        printf("Admin account can not be updated.\n"); return;
    }

    // Iterate through users to find the user by ID
    for (int i = 0; i < *userCount; i++) {
        // Check if the entered ID matches any user ID
        if (strcmp(users[i].id, input_id) == 0) {
            // Display current user status
            if (users[i].active == 1) {
                printf("The current user status: Active\n");
            } else {
                printf("The current user status: Inactive\n");
            }
            
            // Loop until a valid new status is entered
            while (new_status != 0 && new_status != 1) {
                // Prompt user to enter new status (1 for active, 0 for inactive)
                printf("Enter new status (1 for active, 0 for inactive): ");
                scanf("%d", &new_status); // Get status from user input
                getchar(); // Remove newline character

                // Check if the entered status is valid
                if (new_status != 0 && new_status != 1) {
                    printf("Invalid status. Please enter 1 for active or 0 for inactive.\n");
                }
            }
            
            // Update user status and save changes to file
            users[i].active = new_status;
            saveUsers(users, *userCount);
            printf("Status updated successfully for user '%s'.\n", input_id);
            return; // Exit the function
        }
    }
    // If no user with the entered ID is found, display a message
    printf("User '%s' not found.\n", input_id);
}


// Function to delete a user
void deleteUser(User users[], int *userCount) {
    
    char input_id[ID_LENGTH];
    printf("Enter ID to delete: ");
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

    if (strcmp(input_id, "admin") == 0) {
        printf("Admin account can not be deleted.\n"); return;
    }

    // Iterate through the users to find the user by ID
    for (int i = 0; i < *userCount; i++) {
        // Check if the entered ID matches any user ID
        if (strcmp(users[i].id, input_id) == 0) {
            printf("\n=== User found ===\n");
            printf("Name: %s\n", users[i].name);
            printf("Role: %s\n", users[i].role);

            int choice;
            printf("Do you want to delete this user (Enter 1 for Yes): ");
            scanf("%d", &choice);
            getchar(); // Consume the newline character
            if (choice != 1) {
                printf("User not deleted.\n");
                return; // Exit the function
            }
            
            // Shift remaining users left to fill the gap
            for (int j = i; j < *userCount - 1; j++) {
                users[j] = users[j + 1];
            }
            (*userCount)--; // Decrease the user count
            // Renumber the remaining users
            for (int k = 0; k < *userCount; k++) {
                users[k].orderNum = k + 1;
            }
            saveUsers(users, *userCount); // The * operator is used to dereference the pointer and get the value it points to. Save changes to file
            printf("User '%s' deleted successfully.\n", input_id);
            return; // Exit the function
        }
    }
    // If no user with the entered ID is found, display a message
    printf("User '%s' not found.\n", input_id);
}


// Function to display user details
void displayUser(User users[], int *userCount) {

    char input_id[ID_LENGTH];
    
    printf("\nEnter ID user: "); 
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

    // Iterate through the users to find the user by ID
    for (int i = 0; i < *userCount; i++) {
        // Check if the entered ID matches any user ID
        if (strcmp(users[i].id, input_id) == 0) {
            printf("\n-------------- User Details --------------\n");
            printf("Name: %s\n", users[i].name);
            printf("ID: %s\n", users[i].id);
            printf("Password: %s\n", users[i].password);
            printf("Role: %s\n", users[i].role);
            // Check user status and display accordingly
            if (users[i].active == 1) {
                printf("Status: Active\n");
            } else {
                printf("Status: Inactive\n");
            }
            printf("------------------------------------------\n");
            return; // Exit the function since the ID is found
        }
    }
    // If no user with the entered ID is found, display a message
    printf("ID user not found.\n");
}


// Function to check if a user is active
char* checkUserStatus(User users[], int *userCount) {
    
    char* input_id = (char*)malloc(ID_LENGTH); // Allocate memory for input_id with space for ID_LENGTH characters

    if (input_id == NULL) {
        perror("Failed to allocate memory for input ID");
        return NULL;
    }

    // Prompt user to enter ID
    printf("Enter user ID: ");
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

    // Search for the user by ID and check their status
    for (int i = 0; i < *userCount; i++) {
        if (strcmp(users[i].id, input_id) == 0) {
            printf("User name: %s\n", users[i].name);
            // Check if the user is a customer and active
            if (strcmp(users[i].role, "Customer") == 0 && users[i].active == 1) {
                return input_id; // Return the input ID if the user is active
            } else {
                // Display a message if the user is inactive or not a customer
                printf("User ID '%s' is inactive or not a customer. Please contact the administrator.\n", input_id);
                free(input_id); // Free dynamically allocated memory
                return NULL; // Return NULL if the user is found but not active
            }
        }
    }

    // Display a message if the user ID is not found
    printf("User with ID %s not found.\n", input_id);
    free(input_id); // Free dynamically allocated memory
    return NULL; // Return NULL if the user ID is not found
}


// Display menu manage users
void displayMenuManageUsers() {
    printf("\n========== MANAGE USERS ===========\n");
    printf("1. Register user\n");
    printf("2. Delete user\n");
    printf("3. Update user password\n");
    printf("4. Update user status\n");
    printf("5. Check user details\n");
    printf("6. Exit\n");
    printf("==================================\n");
}


// Function to manage users
void manageUsers(User users[], int *userCount) {
    int choice = 0; // Variable to store user's choice
    while (choice != 6) { // Continue the loop until the user chooses to exit
        displayMenuManageUsers(); // Display the menu options
        printf("Enter your choice: ");
        scanf("%d", &choice); // Get user's choice
        getchar(); // Consume the newline character left in the input buffer
        
        // Perform actions based on user's choice
        switch (choice) {
            case 1: registerUser(users, userCount); break; // Register a new user
            case 2: deleteUser(users, userCount); break; // Delete a user
            case 3: updateUserPassword(users, userCount); break; // Update user password
            case 4: updateUserStatus(users, userCount); break; // Update user status
            case 5: displayUser(users, userCount); break; // Display user details
            case 6: printf("Exiting... Thank you!\n"); break; // Exit the loop
            default: printf("Invalid choice. Please try again.\n"); // Display message for invalid choice
        }
    }
}


// ROOM FUNCTIONS

// Function to save rooms to file
void saveRooms(Room rooms[], int roomCount) {
    FILE *file = fopen(ROOMS_FILE, "w"); // Open the file for writing
    if (file == NULL) {
        printf("Error: Unable to open rooms file for writing.\n");
        return;
    }

    // Write each room's data to the file
    for (int i = 0; i < roomCount; i++) {
        if (rooms[i].type == STANDARD) { // Check if the room is a standard type
            // Write standard room data to the file
            fprintf(file, "%d, %d, %d, %s, %.2f, %.2f, %d\n", 
                    rooms[i].orderNum,
                    rooms[i].id, 
                    rooms[i].type, 
                    rooms[i].details.standard.bedType, 
                    rooms[i].rate, 
                    rooms[i].details.standard.size,
                    rooms[i].active);
        } else if (rooms[i].type == SUITE) { // Check if the room is a suite type
            // Write suite room data to the file
            fprintf(file, "%d, %d, %d, %s, %.2f, %.2f, %s, %d\n", 
                    rooms[i].orderNum,
                    rooms[i].id, 
                    rooms[i].type, 
                    rooms[i].details.suite.bedType, 
                    rooms[i].rate, 
                    rooms[i].details.suite.size, 
                    rooms[i].details.suite.amenities,
                    rooms[i].active);
        }
    }

    fclose(file); // Close the file
}


// Function to load rooms from file
void loadRooms(Room rooms[], int *roomCount) {
    FILE *file = fopen(ROOMS_FILE, "r"); // Open the file for reading
    if (file == NULL) {
        printf("Error: Unable to open rooms file for reading.\n");
        *roomCount = 0;
        return; // No rooms file found, start with an empty room list
    }

    *roomCount = 0; // Initialize room count to zero
    while (!feof(file) && *roomCount < MAX_ROOMS) { // Read data until end of file or maximum rooms reached
        Room room;
        int type;
        // Read room data from the file
        if (fscanf(file, "%d, %d, %d,", &room.orderNum, &room.id, &type) != 3) break;
        room.type = (RoomType)type;

        if (room.type == STANDARD) { // Check if the room is a standard type
            // Read standard room data from the file
            if (fscanf(file, " %25[^,], %f, %f, %d\n",
                       room.details.standard.bedType,
                       &room.rate,
                       &room.details.standard.size,
                       &room.active) != 4) break;

        } else if (room.type == SUITE) { // Check if the room is a suite type
            // Read suite room data from the file
            if (fscanf(file, " %25[^,], %f, %f, %99[^,], %d\n",
                       room.details.suite.bedType,
                       &room.rate,
                       &room.details.suite.size,
                       room.details.suite.amenities,
                       &room.active) != 5) break;
        }

        rooms[*roomCount] = room; // Add the room to the rooms array
        (*roomCount)++; // Increment room count
    }

    fclose(file); // Close the file
}


// Function to prompt for and validate room ID
int promptValidRoomID(Room rooms[], int roomCount) {
    int roomID = 0;
    bool idExists = false;
    
    do {
        // Prompt user to enter room ID
        printf("Enter room ID (between 101 and 151): ");
        scanf("%d", &roomID); 
        getchar(); // Remove newline character
        
        if (roomID < 101 || roomID > 151) {
            printf("Room ID must be between 101 and 151. Please try again.\n");
            continue; // Prompt user to enter again
        }

        // Check for duplicate room ID
        idExists = false; // Reset the flag
        for (int i = 0; i < roomCount; i++) {
            if (rooms[i].id == roomID) {
                idExists = true;
                break;
            }
        }

        if (idExists) {
            printf("Room ID already exists. Please enter a different ID.\n");
        }
    } while (roomID < 101 || roomID > 151 || idExists); // Repeat until a valid ID is entered

    return roomID;
}


// Function to prompt for and validate room size
float promptValidRoomSize() {
    float roomSize = 0;

    do {
        // Prompt user to enter room size
        printf("Enter size of the room (in square meters, between 15.0 and 35.0): ");
        scanf("%f", &roomSize);
        getchar(); // Remove newline character

        // Check if room size is within the valid range
        if (roomSize < 15.0 || roomSize > 35.0) {
            printf("Room size must be between 15.0 sq meters and 35.0 sq meters. Please try again.\n");
        }
    } while (roomSize < 15.0 || roomSize > 35.0); // Repeat until a valid room size is entered

    return roomSize;
}


// Function to prompt for and validate room rate
float promptValidRoomRate() {
    float roomRate = 0;

    do {
        // Prompt user to enter room rate
        printf("Enter room rate (between 100.0RM and 1000.0RM): ");
        scanf("%f", &roomRate);
        getchar(); // Remove newline character

        // Check if room rate is within the valid range
        if (roomRate < 100.0 || roomRate > 1000.0) {
            printf("Room rate must be between 100.0RM and 1000.0RM. Please try again.\n");
        }
    } while (roomRate < 100.0 || roomRate > 1000.0); // Repeat until a valid room rate is entered

    return roomRate;
}


// Function to prompt for and validate room bed type
char* promptValidRoomBedType() {
    char bedType[BEDTYPE_LENGTH];
    int bedType_choice = 0;

    do {
        // Prompt user to select bed type
        printf("Enter bed type (1 for Single, 2 for Double): ");
        scanf("%d", &bedType_choice);
        getchar(); // Remove newline character

        if (bedType_choice != 1 && bedType_choice != 2) {
            printf("Please enter 1 or 2 for bed type.\n");
        }
    } while (bedType_choice != 1 && bedType_choice != 2);

    // Assign the bed type based on the user's choice
    if (bedType_choice == 1) {
        strcpy(bedType, "Single");
    } else {
        strcpy(bedType, "Double");
    }
    return strdup(bedType); // Dynamically allocate memory and copy the bedtype
}


// Function to prompt for and validate room type
int promptValidRoomType() {
    int roomType_choice = 0;

    do {
        // Prompt user to select room type
        printf("Enter room type (1 for Standard, 2 for Suite): ");
        scanf("%d", &roomType_choice); getchar();

        if (roomType_choice != 1 && roomType_choice != 2) {
            printf("Please enter 1 or 2 for room type.\n");
            continue;
        }
    } while (roomType_choice != 1 && roomType_choice != 2);

    return roomType_choice;
}


// Function to prompt for and validate room amenities
char* promptValidAmenities() {
    int numAmenities = 0;
    char amenitiesList[MAX_AMENITIES][AMENITY_LENGTH]; // Assuming each amenity can have up to 25 characters

    // Prompt user to enter the number of amenities
    do { 
        printf("How many amenities (1-%d): ", MAX_AMENITIES);
        scanf("%d", &numAmenities);
        getchar(); // Remove newline character

        if (numAmenities < 1 || numAmenities > MAX_AMENITIES) {
            printf("Please enter a number between 1 and %d.\n", MAX_AMENITIES);
        }
    } while (numAmenities < 1 || numAmenities > MAX_AMENITIES);

    // Prompt user to enter each amenity
    for (int i = 0; i < numAmenities; i++) {
        printf("Enter room amenity %d: ", i + 1);
        fgets(amenitiesList[i], AMENITY_LENGTH, stdin);
        amenitiesList[i][strcspn(amenitiesList[i], "\n")] = '\0'; // Remove newline character

        // Validate the entered amenity
        if (isEmpty(amenitiesList[i])) {
            printf("Amenity cannot be empty. Please try again!\n");
            i--; // Re-prompt for the current amenity
            continue;
        }

        if (containComma(amenitiesList[i])) {
            printf("Amenity cannot contain a comma character. Please try again!\n");
            i--; // Re-prompt for the current amenity
            continue;
        }

        if (containNumeric(amenitiesList[i])) {
            printf("Amenity cannot contain numeric characters. Please try again!\n");
            i--; // Re-prompt for the current amenity
            continue;
        }
    }

    // Concatenate amenities into a single string separated by '|'
    char* concatenatedAmenities = malloc(MAX_AMENITIES * (AMENITY_LENGTH)); // Allocate memory
    if (concatenatedAmenities == NULL) {
        // Handle memory allocation failure
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }

    strcpy(concatenatedAmenities, amenitiesList[0]);
    for (int i = 1; i < numAmenities; i++) {
        strcat(concatenatedAmenities, "|");
        strcat(concatenatedAmenities, amenitiesList[i]);
    }

    return concatenatedAmenities; // Dynamically allocated memory containing the concatenated amenities
}


// Function to add a new room
void addRoom(Room rooms[], int *roomCount) {

    if (*roomCount >= MAX_ROOMS) { // Check if maximum room capacity has been reached
        printf("Error: Cannot add more rooms. The maximum capacity of %d rooms has been reached.\n", MAX_ROOMS);
        return;
    }

    int roomID, roomType;
    float roomSize, roomRate;
    char *amenities = NULL, *bedType = NULL;
    bool validInput = false;

    while (!validInput) {
        Room newRoom;
        roomID = promptValidRoomID(rooms, *roomCount);
        roomSize = promptValidRoomSize();
        roomRate = promptValidRoomRate();
        bedType = promptValidRoomBedType();
        roomType = promptValidRoomType();

        if (roomType == 1) {
            newRoom.type = STANDARD;
            strcpy(newRoom.details.standard.bedType, bedType);
            newRoom.details.standard.size = roomSize;
        } else {
            newRoom.type = SUITE;
            strcpy(newRoom.details.suite.bedType, bedType);
            newRoom.details.suite.size = roomSize;
            if (roomType == 2) {
                amenities = promptValidAmenities();
                strcpy(newRoom.details.suite.amenities, amenities);
                free(amenities); // Free dynamically allocated memory for amenities
            }
        }

        newRoom.orderNum = *roomCount + 1; // Assign order number based on room count
        newRoom.id = roomID;
        newRoom.rate = roomRate;
        newRoom.active = 1;

        rooms[*roomCount] = newRoom; // Add the new room to the rooms array
        (*roomCount)++; // Increment room count
        saveRooms(rooms, *roomCount); // Save the updated rooms to file
        printf("Room added successfully.\n");
        free(bedType); // Free dynamically allocated memory for bedType
        validInput = true; // Break out of the loop as input is valid
    }
}


// Function to delete a room
void deleteRoom(Room rooms[], int *roomCount) {

    int roomIDToDelete;
    printf("Enter room ID to delete: ");
    scanf("%d", &roomIDToDelete);
    getchar(); // Consume the newline character

    bool roomFound = false;
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].id == roomIDToDelete) {
            // Shift remaining rooms left to fill the gap
            for (int j = i; j < *roomCount - 1; j++) {
                rooms[j] = rooms[j + 1];
            }
            (*roomCount)--;
            roomFound = true;
            break;
        }
    }
    if (!roomFound) {
        printf("Room with ID %d not found.\n", roomIDToDelete);
        return;
    }

    // Renumber the remaining rooms
    for (int i = 0; i < *roomCount; i++) {
        rooms[i].orderNum = i + 1;
    }

    // Save the updated rooms back to the file
    saveRooms(rooms, *roomCount);
    printf("Room with ID %d deleted successfully.\n", roomIDToDelete);
}


// Function to update room status
void updateRoomStatus(Room rooms[], int *roomCount) {

    int roomIDToUpdate;
    int newStatus = -1;

    printf("Enter room ID to update status: ");
    scanf("%d", &roomIDToUpdate);
    getchar(); // Consume the newline character

    bool roomFound = false;
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].id == roomIDToUpdate) {
            roomFound = true;
            // Display current room status
            printf("Current status of room ID %d: %d\n", roomIDToUpdate, rooms[i].active);

            // Prompt for new status until a valid input is given
            while (newStatus != 1 && newStatus != 0) {
                printf("Enter new status (1 for Active, 0 for Inactive): ");
                scanf("%d", &newStatus);
                getchar(); // Consume the newline character

                if (newStatus != 1 && newStatus != 0) {
                    printf("Invalid status. Please enter 1 for Active or 0 for Inactive.\n");
                }
            }

            rooms[i].active = newStatus;
            break;
        }
    }

    if (!roomFound) {
        printf("Room with ID %d not found.\n", roomIDToUpdate);
        return;
    }

    // Save the updated rooms back to the file
    saveRooms(rooms, *roomCount);
    printf("Room with ID %d status updated successfully.\n", roomIDToUpdate);
}


// Function to check room's details
void displayRoom(Room rooms[], int *roomCount, int roomId) {

    // Search for the room with the specified ID
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].id == roomId) {
            // Print room details
            printf("\n------------ Room Details ------------\n");
            printf("ID: %d\n", rooms[i].id);
            if (rooms[i].type == STANDARD) {
                printf("Type: Standard\n");
                printf("Bed type: %s\n", rooms[i].details.standard.bedType);
                printf("Room size: %.2f\n", rooms[i].details.standard.size);
            } else {
                printf("Type: Suite\n");
                printf("Bed type: %s\n", rooms[i].details.suite.bedType);
                printf("Room size: %.2f\n", rooms[i].details.suite.size);
                printf("Amenities: %s\n", rooms[i].details.suite.amenities);
            }
            printf("Rate: %.2f RM\n", rooms[i].rate);
            if (rooms[i].active == 1) {
                printf("Status: Active\n");
            } else {
                printf("Status: Inactive\n");
            }
            printf("\n--------------------------------------\n");
            return;
        }
    }

    // If the room with the specified ID is not found
    printf("Room ID %d not found.\n", roomId);
}


// Function to search for rooms by type
void searchByRoomType(Room rooms[], int *roomCount) {
    int roomType = 0;
    printf("Enter room type (1 for Standard, 2 for Suite): ");
    scanf("%d", &roomType); getchar();

    // Determine the room type string based on user input
    char *roomTypeStr;
    if (roomType == 1) {
        roomTypeStr = "Standard";
    } else if (roomType == 2) {
        roomTypeStr = "Suite";
    } else {
        printf("Invalid room type entered.\n");
        return;
    }

    // Print header for the available rooms of the specified type
    printf("\n================ %s Rooms ===============\n", roomTypeStr);
    int found = 0; // Flag to check if any rooms are found
    for (int i = 0; i < *roomCount; i++) {
        if ((roomType == 1 && rooms[i].type == STANDARD) || 
            (roomType == 2 && rooms[i].type == SUITE)) {
            if (rooms[i].active == 1) {
                displayRoom(rooms, roomCount, rooms[i].id); // Display the details of the room
                found = 1; // Set the flag when a room is found
            }
        }
    }

    // If no rooms of the specified type are found
    if (!found) {
        printf("No available rooms of the specified type.\n");
    }
}


// Function to search for rooms by bed type
void searchByBedType(Room rooms[], int *roomCount) {
    int bedType;
    printf("Enter bed type (1 for Single or 2 for Double): ");
    scanf("%d", &bedType); getchar();

    // Convert the integer bed type to the corresponding string value
    char *bedTypeStr;
    if (bedType == 1) {
        bedTypeStr = "Single";
    } else if (bedType == 2) {
        bedTypeStr = "Double";
    } else {
        printf("Invalid bed type entered.\n");
        return;
    }

    // Print header for the available rooms with the specified bed type
    printf("\n============ %s Bedtype Rooms ===========\n", bedTypeStr);
    int found = 0; // Flag to check if any rooms are found
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].active == 1) {
            if ((rooms[i].type == STANDARD && strcmp(rooms[i].details.standard.bedType, bedTypeStr) == 0) || 
                (rooms[i].type == SUITE && strcmp(rooms[i].details.suite.bedType, bedTypeStr) == 0)) {
                displayRoom(rooms, roomCount, rooms[i].id); // Display the details of the room
                found = 1; // Set the flag when a room is found
            }
        }
    }

    // If no rooms with the specified bed type are found
    if (!found) {
        printf("No available rooms with the specified bed type.\n");
    }
}


// Function to search for rooms by rate
void searchByRate(Room rooms[], int *roomCount) {
    float minRate, maxRate;
    printf("Enter minimum rate (RM): ");
    scanf("%f", &minRate); getchar(); // Consume newline character
    printf("Enter maximum rate (RM): ");
    scanf("%f", &maxRate); getchar(); // Consume newline character

    // Print header for the available rooms within the specified rate range
    printf("\n====== Rooms from %.2fRM to %.2fRM ======\n", minRate, maxRate);
    int found = 0; // Flag to check if any rooms are found
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].active == 1 && rooms[i].rate >= minRate && rooms[i].rate <= maxRate) {
            displayRoom(rooms, roomCount, rooms[i].id); // Display the details of the room
            found = 1; // Set the flag when a room is found
        }
    }

    // If no rooms within the specified rate range are found
    if (!found) {
        printf("No available rooms within the specified rate range.\n");
    }
}


// Function to search for available rooms
void searchRooms(Room rooms[], int *roomCount) {
    int choice = 0;    
    do {
        // Display the menu for searching available rooms
        printf("\n========= Search For Room Details ==========\n");
        printf("1. Search by Room Type\n");
        printf("2. Search by Bed Type\n");
        printf("3. Search by Rate\n");
        printf("4. Back to Main Menu\n");
        printf("============================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice); getchar(); // Consume the newline character
        
        // Perform actions based on user's choice
        switch (choice) {
            case 1:
                searchByRoomType(rooms, roomCount);
                break;
            case 2:
                searchByBedType(rooms, roomCount);
                break;
            case 3:
                searchByRate(rooms, roomCount);
                break;
            case 4:
                printf("Returning to main menu...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 4); // Continue the loop until the user chooses to go back to the main menu
}


// Function to check if a room is active and display its details
int checkRoomStatus(Room rooms[], int *roomCount) {

    int roomIDToCheck;

    // Prompt user to enter room ID
    printf("Enter room ID: ");
    scanf("%d", &roomIDToCheck);
    getchar(); // Consume the newline character

    // Search for the room by ID and check its status
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].id == roomIDToCheck) {
            if (rooms[i].active == 1) {
                // Display room details if it's active
                printf("\n------------ Room Details ------------\n");
                printf("ID: %d\n", rooms[i].id);
                if (rooms[i].type == STANDARD) {
                    printf("Type: Standard\n");
                    printf("Bed type: %s\n", rooms[i].details.standard.bedType);
                    printf("Room size: %.2f\n", rooms[i].details.standard.size);
                } else {
                    printf("Type: Suite\n");
                    printf("Bed type: %s\n", rooms[i].details.suite.bedType);
                    printf("Room size: %.2f\n", rooms[i].details.suite.size);
                    printf("Amenities: %s\n", rooms[i].details.suite.amenities);
                }
                printf("Rate: %.2f RM\n", rooms[i].rate);
                printf("\n--------------------------------------\n");
                return roomIDToCheck; // Return the room ID if it's active
            } else {
                // Inform the user if the room is not active
                printf("This room is unavailable at the moment.\n");
                return -1; // Return -1 if the room is found but not active
            }
        }
    }

    // Inform the user if the room ID is not found
    printf("Room with ID %d not found.\n", roomIDToCheck);
    return -1; // Return -1 if the room ID is not found
}


// Function to display menu manage rooms
void displayMenuManageRooms() {
    printf("\n========== MANAGE ROOMS ==========\n");
    printf("1. Add a new room\n");
    printf("2. Delete room\n");
    printf("3. Update room status\n");
    printf("4. Check for room details\n");
    printf("5. Exit\n");
    printf("==================================\n");
}


// Function to manage rooms
void manageRooms(Room rooms[], int roomCount) {
    int choice = 0;
    int roomId;
    while (choice != 5) {
        displayMenuManageRooms();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left in the input buffer
        
        switch (choice) {
            case 1: 
                addRoom(rooms, &roomCount); // Add a new room
                break;
            case 2: 
                deleteRoom(rooms, &roomCount); // Delete a room
                break;
            case 3: 
                updateRoomStatus(rooms, &roomCount); // Update room status
                break;
            case 4: 
                printf("\nEnter room ID to check: "); 
                scanf("%d", &roomId); 
                getchar(); // Consume the newline character
                displayRoom(rooms, &roomCount, roomId); // Check room details
                break;
            case 5: 
                printf("Exiting... Thank you!\n"); 
                break;
            default: 
                printf("Invalid choice. Please try again.\n");
        }
    }
}


// BOOKING FUNCTIONS

// Function to save bookings to file
void saveBookings(Booking *head) {
    FILE *file = fopen(BOOKINGS_FILE, "w");
    if (file == NULL) {
        perror("Error: Unable to open file for writing");
        return;
    }

    // Traverse the linked list and write each booking data to the file
    Booking *current = head;
    while (current != NULL) {
        fprintf(file, "%d, %d, %s, %d, %s, %s, %s, %s\n", 
                current->orderNum, current->id, current->userId, current->roomId, current->status, 
                current->bookingDate, current->checkInDate, current->checkOutDate);
        current = current->next;
    }

    fclose(file);
}


// Function to load bookings from file
Booking* loadBookings(int *bookingCount) {
    FILE *file = fopen(BOOKINGS_FILE, "r");
    if (file == NULL) {
        perror("Error: Unable to open bookings file for reading.\n");
        return NULL; // Return NULL to indicate no bookings
    }

    Booking *head = NULL; // Initialize head of linked list to NULL
    Booking *tail = NULL; // Initialize tail of linked list to NULL
    *bookingCount = 0; // Initialize booking count to 0

    // Read data from file and create linked list nodes
    while (1) {
        Booking *newNode = (Booking*)malloc(sizeof(Booking));
        if (!newNode) {
            perror("Failed to allocate memory for new booking");
            return head; // Return the current state of the linked list
        }

        // Read data from file into the newNode
        if (fscanf(file, "%d, %d, %[^,], %d, %[^,], %[^,], %[^,], %[^,\n]",
                   &newNode->orderNum,
                   &newNode->id,
                   newNode->userId,
                   &newNode->roomId,
                   newNode->status,
                   newNode->bookingDate,
                   newNode->checkInDate,
                   newNode->checkOutDate) != 8) {
            free(newNode); // Free memory allocated for newNode
            break; // Exit the loop if fscanf fails
        }

        // Set the next pointer of the newNode to NULL
        newNode->next = NULL;

        // Update the linked list
        if (head == NULL) {
            head = newNode; // If head is NULL, newNode becomes the head
            tail = newNode; // If tail is NULL, newNode becomes the tail
        } else {
            tail->next = newNode; // Append newNode to the end of the list
            tail = newNode; // Update the tail to newNode
        }

        (*bookingCount)++; // Increment the booking count
    }

    fclose(file); // Close the file
    return head; // Return the head of the linked list
}


// Function to generate a unique ID by checking against the list of bookings
int generateUniqueId(Booking* head) {
    int id;
    Booking* current;

    do {
        id = (rand() % 900) + 100; // Generate ID between 100 and 999
        current = head;
        while (current != NULL) {
            if (current->id == id) {
                id = -1; // ID is not unique, set it to an invalid value to continue loop
                break;
            }
            current = current->next;
        }
    } while (id == -1);

    return id;
}


// Function to add a booking node
void addBookingNode(Booking **head, int *bookingCount, int uniqueId, const char *userId, int roomId, const char *bookingDate, const char *checkInDate, const char *checkOutDate) {
    Booking *newBooking = (Booking*)malloc(sizeof(Booking));
    if (!newBooking) {
        perror("Failed to allocate memory for new booking");
        return;
    }
    newBooking->orderNum = *bookingCount + 1; // Increment booking order number
    newBooking->id = uniqueId;
    strcpy(newBooking->userId, userId);
    newBooking->roomId = roomId;
    strcpy(newBooking->status, "Reserved"); // Assuming status of newly created booking is 'Reserved'
    strcpy(newBooking->bookingDate, bookingDate);
    strcpy(newBooking->checkInDate, checkInDate);
    strcpy(newBooking->checkOutDate, checkOutDate);
    newBooking->next = NULL;

    if (*head == NULL) {
        *head = newBooking;
    } else {
        Booking *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newBooking;
    }

    (*bookingCount)++;
}


// Function to check room availability for booking
int isRoomAvailableForBooking(Booking *head, int roomId, const char *checkInDate, const char *checkOutDate) {
    Booking *current = head;
    
    while (current != NULL) {
        if (current->roomId == roomId) {
            // Check if the room is currently Reserved or Checked-In
            if (strcmp(current->status, "Reserved") == 0 || strcmp(current->status, "Checked-In") == 0) {
                // Check for overlap:
                if (strcmp(checkInDate, current->checkInDate) >= 0 && strcmp(checkInDate, current->checkOutDate) < 0) {
                    return 0; // Room is not available
                }
                if (strcmp(checkOutDate, current->checkInDate) > 0 && strcmp(checkOutDate, current->checkOutDate) <= 0) {
                    return 0; // Room is not available
                }
            }
        }
        current = current->next;
    }

    return 1; // Room is available
}


// Function to display booking details
void displayBookingIdDetails(int *id, const char *userId, int *roomId, char *status, char *bookingDate, char *checkInDate, char *checkOutDate) {
    printf("\n------------ Booking Details ------------");
    printf("\nBooking ID: %d\n", *id);
    printf("User ID: %s\n", userId);
    printf("Room ID: %d\n", *roomId);
    printf("Status: %s\n", status);
    printf("Booking Date: %s\n", bookingDate);
    printf("Check-In Date: %s\n", checkInDate);
    printf("Check-Out Date: %s\n", checkOutDate);
    printf("-----------------------------------------\n");
}


// Function to free memory allocated for booking nodes
void freeBookings(Booking *head) {
    Booking *current = head;
    while (current != NULL) {
        Booking *next = current->next; // Store the next node to prevent losing reference
        free(current); // Free memory for the current node
        current = next; // Move to the next node
    }
}


// Function to update order numbers of bookings
void updateOrderNumbers(Booking *head) {
    Booking *current = head;
    int orderNum = 1;

    // Traverse the list of bookings and update order numbers sequentially
    while (current != NULL) {
        current->orderNum = orderNum;
        orderNum++;
        current = current->next;
    }
}


// Function to check room availability and make a reservation
void makeReservation(Booking **head, int *bookingCount, Room rooms[], int *roomCount, const char* userId) {

    int roomId;
    char checkInDate[DATE_LENGTH], checkOutDate[DATE_LENGTH];

    // Check if room is active
    roomId = checkRoomStatus(rooms, roomCount);
    if (roomId == -1) {
        return; // Exit if the room is not active
    }

    // Get current date
    char currentDate[DATE_LENGTH]; // Assuming YYYY-MM-DD format
    getCurrentDate(currentDate, DATE_LENGTH); 

    // Get valid check-in date (must be current date or later)
    getValidDate(checkInDate, "Enter check-in date (YYYY-MM-DD): ", 0, currentDate);
    
    // Get valid check-out date (must be later than check-in date)
    do {
        getValidDate(checkOutDate, "Enter check-out date (YYYY-MM-DD): ", 0, checkInDate);

        // Check if check-out date is the same as or earlier than check-in date
        if (strcmp(checkInDate, checkOutDate) >= 0) {
            printf("Check-out date must be later than check-in date. Please enter again.\n");
        }
    } while (strcmp(checkInDate, checkOutDate) >= 0);

    // Check if room is available for the specified duration
    if (!isRoomAvailableForBooking(*head, roomId, checkInDate, checkOutDate)) {
        printf("Room is not available for the specified duration.\n");
        return; // Exit if the room is not available
    }

    // Ask the user if they want to make a reservation
    int choice;
    printf("Room is available. Do you want to make a reservation? (1 for yes, 0 for no): ");
    scanf("%d", &choice);

    if (choice != 1) {
        printf("Reservation not made.\n");
        return; // Exit if the user decides not to make a reservation
    }

    // Generate unique ID for the new booking
    int uniqueId = generateUniqueId(*head);
    if (uniqueId == -1) {
        printf("Error: Unable to generate a unique ID for the reservation.\n");
        return; // Exit if unable to generate a unique ID
    }

    // Add the new booking node to the list
    addBookingNode(head, bookingCount, uniqueId, userId, roomId, currentDate, checkInDate, checkOutDate);

    // Save updated bookings to file
    saveBookings(*head);

    // Display booking details
    displayBookingIdDetails(&uniqueId, userId, &roomId, "Reserved", currentDate, checkInDate, checkOutDate);
    printf("Reservation successfully made with booking ID: %d\n", uniqueId);
}


// Function to cancel a reservation
void cancelReservation(Booking **head, int *bookingCount, const char *userId) {
    // If no bookings or error loading bookings, return
    if (*head == NULL || *bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Get input for booking ID
    int bookingId;
    printf("Enter booking ID to cancel: ");
    scanf("%d", &bookingId);
    getchar(); // Consume newline character

    // Traverse the list of bookings to find the matching booking ID
    Booking *current = *head;
    while (current != NULL) {
        if (current->id == bookingId) {
            // Check if the booking is in "Reserved" status and user ID matches
            if (strcmp(current->status, "Reserved") == 0 && strcmp(current->userId, userId) == 0) {
                // Update the status to "Cancelled" and save the bookings
                strcpy(current->status, "Cancelled");
                saveBookings(*head);
                printf("Reservation cancelled successfully for booking ID: %d\n", bookingId);
                return; // No need to free memory here
            } else {
                printf("Cancellation failed. Either the booking ID or user ID is incorrect, or the booking is not in 'Reserved' status.\n");
                return; // No need to free memory here
            }
        }
        current = current->next;
    }
    printf("Booking ID: %d not found.\n", bookingId);
}


// Function to cancel a reservation based on check-in date less than current date
void autoCancelReservations(Booking **head) {
    char currentDate[DATE_LENGTH];
    getCurrentDate(currentDate, DATE_LENGTH);

    Booking *current = *head;
    while (current != NULL) {
        // Compare check-in date with current date
        if (strcmp(current->status, "Reserved") == 0 && strcmp(current->checkInDate, currentDate) < 0) {
            // Update status to "Cancelled"
            strcpy(current->status, "Cancelled");
            saveBookings(*head);
        }
        current = current->next;
    }
}


// Function to perform checking for a booking
void checkin(Booking *head, int bookingCount) {
    // If no bookings or error loading bookings, return
    if (head == NULL || bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Get input for booking ID and user ID
    int bookingId;
    char userId[ID_LENGTH];
    printf("Enter booking ID to check in: ");
    scanf("%d", &bookingId);
    getchar(); // Consume newline character
    printf("Enter user ID: ");
    fgets(userId, ID_LENGTH, stdin);
    userId[strcspn(userId, "\n")] = '\0'; // Remove newline character

    // Get the current date
    char currentDate[DATE_LENGTH];
    getCurrentDate(currentDate, DATE_LENGTH);

    // Traverse the list of bookings to find the matching booking ID
    Booking *current = head;
    while (current != NULL) {
        if (current->id == bookingId) {
            // Check if the status allows for check-in
            if (strcmp(current->status, "Reserved") == 0) {
                // Check if the user ID matches the one associated with the booking
                if (strcmp(current->userId, userId) == 0) {
                    // Check if the check-in date matches the current date
                    if (strcmp(current->checkInDate, currentDate) == 0) {
                        // Update the status to "Checked-In" and save the bookings
                        strcpy(current->status, "Checked-In");
                        saveBookings(head); // Save the updated bookings to file
                        printf("Check-in successful for booking ID: %d\n", bookingId);
                    } else {
                        printf("Check-in date mismatch for booking ID: %d. Cannot check-in.\n", bookingId);
                    }
                } else {
                    printf("Booking ID: %d cannot be checked in. Reserved by another user.\n", bookingId);
                }
            } else if (strcmp(current->status, "Checked-In") == 0) {
                printf("Booking ID: %d is already checked in.\n", bookingId);
            } else {
                printf("Booking ID: %d cannot be checked in. Status must be 'Reserved'.\n", bookingId);
            }
            return;
        }
        current = current->next;
    }
    printf("Booking ID: %d not found.\n", bookingId);
}


// Function to perform checkout for a booking
void checkout(Booking *head, int bookingCount) {
    // If no bookings or error loading bookings, return
    if (head == NULL || bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Get input for booking ID and user ID
    int bookingId;
    char userId[ID_LENGTH];
    printf("Enter booking ID to check out: ");
    scanf("%d", &bookingId);
    getchar(); // Consume newline character
    printf("Enter user ID: ");
    fgets(userId, sizeof(userId), stdin);
    userId[strcspn(userId, "\n")] = '\0'; // Remove newline character

    // Traverse the list of bookings to find the matching booking ID
    Booking *current = head;
    while (current != NULL) {
        if (current->id == bookingId) {
            // Check if the booking is in "Checked-In" status and user ID matches
            if (strcmp(current->status, "Checked-In") == 0 && strcmp(current->userId, userId) == 0) {
                // Update the status to "Checked-Out" and save the bookings
                strcpy(current->status, "Checked-Out");
                saveBookings(head); // Save the updated bookings to file
                printf("Checkout successful for booking ID: %d\n", bookingId);
                return;
            } else {
                printf("Checkout failed. Either the booking ID or user ID is incorrect, or the booking is not in 'Checked-In' status.\n");
                return;
            }
        }
        current = current->next;
    }
    printf("Booking ID: %d not found.\n", bookingId);
}


// Function to view booking history within a specified date range
void viewBookingHistory(Booking *head, int bookingCount) {
    // Check if there are any bookings
    if (head == NULL || bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Get start and end dates from the user
    char startDate[DATE_LENGTH], endDate[DATE_LENGTH];
    // Prompt user to enter start date and validate
    getValidDate(startDate, "Enter start date (YYYY-MM-DD): ", 1, NULL); // allowPastDate is set to 1 (true)

    // Prompt user to enter end date and validate, ensuring it's later than start date
    do {
        getValidDate(endDate, "Enter end date (YYYY-MM-DD): ", 1, NULL); // allowPastDate is set to 1 (true)

        // Check if end date is later than start date
        if (strcmp(startDate, endDate) >= 0) {
            printf("End date must be later than start date. Please enter again.\n");
        }
    } while (strcmp(startDate, endDate) >= 0);

    // Display booking history within the specified date range
    printf("Booking History within Date Range (%s - %s):\n", startDate, endDate);
    printf("===========================================\n");
    Booking *current = head;
    while (current != NULL) {
        // Check if booking date falls within the specified range
        if (strcmp(current->bookingDate, startDate) >= 0 && strcmp(current->bookingDate, endDate) <= 0) {
            displayBookingIdDetails(&current->id, current->userId, &current->roomId, current->status, current->bookingDate, current->checkInDate, current->checkOutDate);
        }
        current = current->next;
    }
}


// Function to check current bookings for a specific user
void checkCurrentBooking(Booking *head, int bookingCount, const char* userId) {
    int found = 0; // Flag to indicate if any matching booking is found

    // Check if there are any bookings
    if (head == NULL || bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Display current bookings for the specified user
    printf("Current Bookings for User ID: %s\n", userId);
    printf("================================\n");
    Booking *current = head;
    while (current != NULL) {
        // Check if the booking is reserved or checked-out and matches the user ID
        if ((strcmp(current->userId, userId) == 0) && (strcmp(current->status, "Reserved") == 0 || strcmp(current->status, "Checked-In") == 0)) {
            displayBookingIdDetails(&current->id, current->userId, &current->roomId, current->status, current->bookingDate, current->checkInDate, current->checkOutDate);
            found = 1; // Set the flag to indicate at least one matching booking is found
        }
        current = current->next;
    }
    
    // If no matching bookings are found, display a message
    if (!found) {
        printf("No current bookings available for your user ID.\n");
    }
}


// Function to check booking history for a specific user
void checkBookingHistory(Booking *head, int bookingCount, const char* userId) {
    int found = 0; // Flag to indicate if any matching booking is found

    // Check if there are any bookings
    if (head == NULL || bookingCount == 0) {
        printf("No bookings available.\n");
        return;
    }

    // Display booking history for the specified user
    printf("Booking History for User ID: %s\n", userId);
    printf("================================\n");
    Booking *current = head;
    while (current != NULL) {
        // Check if the booking matches the user ID and is either cancelled or checked-out
        if (strcmp(current->userId, userId) == 0 && (strcmp(current->status, "Cancelled") == 0 || strcmp(current->status, "Checked-Out") == 0)) {
            displayBookingIdDetails(&current->id, current->userId, &current->roomId, current->status, current->bookingDate, current->checkInDate, current->checkOutDate);
            found = 1; // Set the flag to indicate at least one matching booking is found
        }
        current = current->next;
    }

    // If no matching bookings are found, display a message
    if (!found) {
        printf("You do not have any booking history at our hotel.\n");
    }
}


// Function to view room availability for a specific duration
void checkRoomAvailabilities(Booking **head, Room rooms[], int *roomCount) {
    char startDate[DATE_LENGTH], endDate[DATE_LENGTH], currentDate[DATE_LENGTH];

    // Get valid dates for the specified duration
    getCurrentDate(currentDate, DATE_LENGTH);
    
    // Get valid start date (must be current date or later)
    getValidDate(startDate, "Enter start date (YYYY-MM-DD): ", 0, currentDate); 
    
    // Get valid end date (must be later than start date)
    do {
        getValidDate(endDate, "Enter end date (YYYY-MM-DD): ", 0, startDate);

        // Check if end date is the same as or earlier than start date
        if (strcmp(startDate, endDate) >= 0) {
            printf("End date must be later than start date. Please enter again.\n");
        }
    } while (strcmp(startDate, endDate) >= 0);
      
    // Load existing rooms from file
    loadRooms(rooms, roomCount);

    // Iterate over each room and check availability for the specified duration
    printf("\nRoom Availabilities for the Duration %s to %s:\n", startDate, endDate);
    printf("--------------------------------------------------------------\n");
    int anyRoomAvailable = 0;
    for (int i = 0; i < *roomCount; i++) {
        if (rooms[i].active == 1 && isRoomAvailableForBooking(*head, rooms[i].id, startDate, endDate)) {
            displayRoom(rooms, roomCount, rooms[i].id);
            anyRoomAvailable++;
        }
    }
    printf("\n%d available room.\n", anyRoomAvailable);
}


// FUNCTION USER LOGIN TRACKING 

// Function to save events to a file
void saveEvents(LoginLogoutEvent events[], int eventCount) {
    // Open the event file in write mode
    FILE *file = fopen(EVENTS_FILE, "w");
    if (file == NULL) {
        printf("Error opening events file for writing.\n");
        return;
    }

    // Write event data to the file
    for (int i = 0; i < eventCount; i++) {
        fprintf(file, "%d, %s, %s, %s\n", events[i].orderNum, events[i].userId, events[i].eventType, events[i].timestamp);
    }

    // Close the file
    fclose(file);
}


// Function to load events from a file
void loadEvents(LoginLogoutEvent events[], int *eventCount) {
    // Open the event file in read mode
    FILE *file = fopen(EVENTS_FILE, "r");
    if (file == NULL) {
        printf("Error opening events file for reading.\n");
        *eventCount = 0;
        return; // No event file found, start with empty event list
    }

    *eventCount = 0; // Initialize event count
    // Read event data from the file
    while (fscanf(file, "%d, %25[^,], %6[^,], %25[^\n]\n", 
                  &events[*eventCount].orderNum, 
                  events[*eventCount].userId, 
                  events[*eventCount].eventType, 
                  events[*eventCount].timestamp) == 4) {
        (*eventCount)++;
    }

    fclose(file);
}


// Function to log an event 
void logEvent(LoginLogoutEvent events[], int *eventCount, const char *userId, const char *eventType) {
    if (*eventCount >= MAX_EVENTS) {
        printf("Event log is full. Cannot log more events.\n");
        return;
    }

    // Log the event
    events[*eventCount].orderNum = *eventCount + 1; // Assign a unique order number
    strncpy(events[*eventCount].userId, userId, ID_LENGTH); // Copy user ID
    strncpy(events[*eventCount].eventType, eventType, EVENT_TYPE_LENGTH); // Copy event type
    getCurrentTimestamp(events[*eventCount].timestamp, DATE_TIME_LENGTH); // Get current time 
    (*eventCount)++; // Increment event count

    // Save all events to the file
    saveEvents(events, *eventCount);
}


// Function to search for events by User ID
void searchEventsByUserId(LoginLogoutEvent events[], int eventCount) {
    if (eventCount == 0) {
        printf("No events available.\n");
        return;
    }

    // Get User ID from the user
    char userId[ID_LENGTH];
    printf("Enter User ID to search: ");
    fgets(userId, ID_LENGTH, stdin);
    userId[strcspn(userId, "\n")] = '\0'; // Remove newline character

    // Display events for the specified User ID
    printf("Event History for User ID '%s':\n", userId);
    printf("===========================================\n");
    for (int i = 0; i < eventCount; i++) {
        if (strcmp(events[i].userId, userId) == 0) {
            printf("Order Number: %d\n", events[i].orderNum);
            printf("User ID: %s\n", events[i].userId);
            printf("Event Type: %s\n", events[i].eventType);
            printf("Timestamp: %s\n", events[i].timestamp);
            printf("-------------------------------------------\n");
        }
    }
}


// Function to search for events within a specified date range
void searchEventsByDateRange(LoginLogoutEvent events[], int eventCount) {
    if (eventCount == 0) {
        printf("No events available.\n");
        return;
    }

    // Get start and end dates from the user
    char startDate[DATE_LENGTH], endDate[DATE_LENGTH];
    // Prompt user to enter start date and validate
    getValidDate(startDate, "Enter start date (YYYY-MM-DD): ", 1, NULL); // allowPastDate is set to 1 (true)

    // Prompt user to enter end date and validate, ensuring it's later than start date
    do {
        getValidDate(endDate, "Enter end date (YYYY-MM-DD): ", 1, NULL); // allowPastDate is set to 1 (true)

        // Check if end date is later than start date
        if (strcmp(startDate, endDate) >= 0) {
            printf("End date must be later than start date. Please enter again.\n");
        }
    } while (strcmp(startDate, endDate) >= 0);

    // Display events within the specified date range
    printf("Event History within Date Range (%s - %s):\n", startDate, endDate);
    printf("===========================================\n");
    for (int i = 0; i < eventCount; i++) {
        // Check if event timestamp falls within the specified range
        if (strcmp(events[i].timestamp, startDate) >= 0 && strcmp(events[i].timestamp, endDate) <= 0) {
            printf("Order Number: %d\n", events[i].orderNum);
            printf("User ID: %s\n", events[i].userId);
            printf("Event Type: %s\n", events[i].eventType);
            printf("Timestamp: %s\n", events[i].timestamp);
            printf("-------------------------------------------\n");
        }
    }
}


// Function to manage events
void manageEvents(LoginLogoutEvent events[], int eventCount) {
    int choice = 0;
    while (choice != 3) {
        printf("\n========== MANAGE EVENTS =========\n");
        printf("1. Search for events by User ID\n");
        printf("2. Search for events by specificied duration\n");
        printf("3. Exit\n");
        printf("=================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left in the input buffer
        
        switch (choice) {
            case 1: 
                searchEventsByUserId(events, eventCount);
                break;
            case 2: 
                searchEventsByDateRange(events, eventCount);
                break;
            case 3: 
                printf("Exiting... Thank you!\n"); 
                break;
            default: 
                printf("Invalid choice. Please try again.\n");
        }
    }
}


// Function to display administrator menu
void adminMenu(User users[], int *userCount, Room rooms[], int *roomCount, LoginLogoutEvent events[], int *eventCount, const char *userId) {
    int choice = 0;
    while (choice != 4) {
        printf("\n======= ADMINISTRATOR MENU =======\n");
        printf("1. Manage User\n");
        printf("2. Manage Room\n");
        printf("3. Manage Login-Logout Event\n");
        printf("4. Logout\n");
        printf("==================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left in the input buffer
        
        switch (choice) {
        case 1: manageUsers(users, userCount); break;
        case 2: manageRooms(rooms, *roomCount); break;
        case 3: manageEvents(events, *eventCount); break;
        case 4: printf("Logging out... Thank you!\n"); 
                logEvent(events, eventCount, userId, "Logout");
                break;
        default: printf("Invalid choice. Please try again.\n");
        }
    }
}


// Function to display the staff menu options
void displayStaffMenuOptions() {
    printf("\n======= HOTEL STAFF MENU =======\n");
    printf("1. Make reservation\n");
    printf("2. Cancel reservation\n");
    printf("3. Check-In\n");
    printf("4. Check-Out\n");
    printf("5. Search booking history\n");
    printf("6. Check room availabilities\n");
    printf("7. Search for room details\n");
    printf("8. Logout\n");
    printf("=================================\n");
}


// Function to display hotel staff menu
void hotelStaffMenu(User users[], int *userCount, Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount, const char *userId) {
    int choice = 0;
    char customerUserID[ID_LENGTH];; char* result;
    int roomId;

    do {
        displayStaffMenuOptions(); // Extracted the menu display into a separate function
        printf("Enter your choice: "); 
        scanf("%d", &choice); 
        getchar(); // Consume newline character

        switch (choice) {
            case 1: {
                result = checkUserStatus(users, userCount);
                if (result != NULL) {
                    strcpy(customerUserID, result);
                    free(result); // Free the dynamically allocated memory
                    // Proceed with further operations using userId
                    makeReservation(head, bookingCount, rooms, roomCount, customerUserID);
                } 
                break;
            }
            
            case 2: 
                result = checkUserStatus(users, userCount);
                if (result != NULL) {
                    strcpy(customerUserID, result);
                    free(result); // Free the dynamically allocated memory
                    cancelReservation(head, bookingCount, customerUserID);
                }
                break;
            case 3: checkin(*head, *bookingCount); break;
            case 4: checkout(*head, *bookingCount); break;
            case 5: viewBookingHistory(*head, *bookingCount); break;
            case 6: checkRoomAvailabilities(head, rooms, roomCount); break;
            case 7: searchRooms(rooms, roomCount); break;
            case 8: 
                printf("Logging out... Thank you!\n"); 
                logEvent(events, eventCount, userId, "Logout");
                break;
            default: 
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 8);
}


// Function to display the customer menu options
void displayCustomerMenuOptions() {
    printf("\n========= CUSTOMER MENU =========\n");
    printf("1. Make reservation\n");
    printf("2. Cancel reservation\n");
    printf("3. View reservation history\n");
    printf("4. View current bookings\n");
    printf("5. Check room availabilities\n");
    printf("6. Search for room details\n");
    printf("7. Logout\n");
    printf("=================================\n");
}


// Function to display customer menu
void hotelCustomerMenu(Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount, const char *userId) {
    int choice = 0;
    int roomId;

    do {
        displayCustomerMenuOptions(); // Extracted the menu display into a separate function
        printf("Enter your choice: "); 
        scanf("%d", &choice); 
        getchar(); // Consume newline character

        switch (choice) {
            case 1: makeReservation(head, bookingCount, rooms, roomCount, userId); break;
            case 2: cancelReservation(head, bookingCount, userId); break;
            case 3: checkBookingHistory(*head, *bookingCount, userId); break;
            case 4: checkCurrentBooking(*head, *bookingCount, userId); break;
            case 5: checkRoomAvailabilities(head, rooms, roomCount); break;
            case 6: searchRooms(rooms, roomCount); break;
            case 7: 
                printf("Logging out... Thank you!\n"); 
                logEvent(events, eventCount, userId, "Logout");
                break;
            default: 
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);
}


// Function to display main menu
void displayMainMenu() {
    printf("\n====== WELCOME TO OUR HOTEL ======\n");
        printf("1. Login\n");
        printf("2. View room availability\n");
        printf("3. Search for room details\n");
        printf("4. Exit\n");
        printf("==================================\n");
}


// Function to authenticate a user
int authenticateUser(User users[], int *userCount, const char *input_id, const char *input_password) {
    // Load user data if not already loaded
    if (*userCount == 0) {
        if (!loadUsers(users, userCount)) {
            printf("Error: Unable to load user data.\n");
            return -1;
        }
    }

    for (int i = 0; i < *userCount; i++) {
        if (strcmp(users[i].id, input_id) == 0 && strcmp(users[i].password, input_password) == 0) {
            return i; // User authenticated successfully   
        }
    }
    return -1; // User not found or password incorrect
}


// Function to login
void loginUser(User users[], int *userCount, Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount) {
    char input_id[ID_LENGTH], input_password[PASSWORD_LENGTH];
    int orderNum;

    printf("\n======== LOGIN PROCESSING ========\n");
    printf("Enter ID: ");
    fgets(input_id, ID_LENGTH, stdin);
    input_id[strcspn(input_id, "\n")] = '\0'; // Remove newline character

    printf("Enter password: ");
    fgets(input_password, PASSWORD_LENGTH, stdin);
    input_password[strcspn(input_password, "\n")] = '\0'; // Remove newline character

    // Authenticate user
    orderNum = authenticateUser(users, userCount, input_id, input_password);
    if (orderNum >= 0) {
        if (users[orderNum].active == 1) {
            printf("\nLogged in successful. Welcome, %s!\n", users[orderNum].name);
            // Log the login event
            logEvent(events, eventCount, users[orderNum].id, "Login");

            // Display menu based on role
            if (strcmp(users[orderNum].role, "Administrator") == 0) {
                adminMenu(users, userCount, rooms, roomCount, events, eventCount, users[orderNum].id);
            } else if (strcmp(users[orderNum].role, "Hotel Staff") == 0) {
                hotelStaffMenu(users, userCount, rooms, roomCount, head, bookingCount, events, eventCount, users[orderNum].id);
            } else {
                hotelCustomerMenu(rooms, roomCount, head, bookingCount, events, eventCount, users[orderNum].id);
            }
        } else {
            printf("User ID '%s' is inactive. Please contact the administrator.\n", input_id);
        }
    } else {
        printf("Login failed. Invalid username or password.\n");
    }
}