#ifndef HOTEL_H
#define HOTEL_H

#include <stdio.h>     // Standard input-output library
#include <stdlib.h>    // Standard library for general functions
#include <string.h>    // String manipulation functions
#include <stdbool.h>   // Boolean type and values
#include <ctype.h>     // Functions for character classification
#include <time.h>      // Functions for manipulating date and time
#include <regex.h>     // POSIX regular expression library


#define NAME_LENGTH 26 // Maximum length of a name
#define ID_LENGTH 26 // Maximum length of a id
#define PASSWORD_LENGTH 26 // Maximum length of a password
#define ROLE_LENGTH 26 // Maximum length of a role
#define MAX_USERS 100 // Maximum number of users that can be stored
#define MAX_ROOMS 50 // Maximum number of rooms that can be stored
#define BEDTYPE_LENGTH 26 // Maximum length of a bedtype
#define MAX_AMENITIES 3 // Maximum amenities of a room
#define AMENITY_LENGTH 26 // Maximum amenities of a room
#define AMENITIES_LENGTH 100 // Maximum length of amenities description
#define MAX_BOOKINGS 200 // Maximum number of bookings that can be stored
#define STATUS_LENGTH 26 // Maximum length of a booking status
#define DATE_LENGTH 11 // Maximum length of a date
#define MAX_EVENTS 500 // Maximum number of login/logout events
#define EVENT_TYPE_LENGTH 7 // Maximum length of an event type ("login" or "logout")
#define DATE_TIME_LENGTH 26 // Maximum length of a timestamp


// File paths for storing data
#define USERS_FILE "users.txt"
#define ROOMS_FILE "rooms.txt"
#define BOOKINGS_FILE "bookings.txt"
#define EVENTS_FILE "login_logout_events.txt"


// Define structures
typedef struct {
    int orderNum;
    char name[NAME_LENGTH];
    char id[ID_LENGTH];
    char password[PASSWORD_LENGTH];
    char role[ROLE_LENGTH];
    int active; // 1 if active, 0 if inactive
} User;

typedef struct {
    int orderNum;
    char userId[ID_LENGTH];
    char eventType[EVENT_TYPE_LENGTH]; // "login" or "logout"
    char timestamp[DATE_TIME_LENGTH];  // Format: YYYY-MM-DD HH:MM:SS
} LoginLogoutEvent;

typedef enum {
    STANDARD, // This is assigned the value 0
    SUITE     // This is assigned the value 1
} RoomType;

typedef union {
    struct {
        char bedType[BEDTYPE_LENGTH]; // Single or Double
        float size; // Size of the room in square meters
    } standard;
    struct {
        char bedType[BEDTYPE_LENGTH]; // Single or Double
        float size; // Size of the room in square meters
        char amenities[AMENITIES_LENGTH]; // Amenities available in the suite
    } suite;
} RoomDetails;

typedef struct {
    int orderNum;
    int id;
    RoomType type; // Enum to indicate whether it's Standard or Suite
    RoomDetails details; // Union representing different details of rooms
    float rate;
    int active; // 1 if active, 0 if inactive
} Room;

typedef struct Booking {
    int orderNum;
    int id; // A unique identifier for the booking
    char userId[ID_LENGTH]; // Customer ID whose books rooms
    int roomId;
    char status[STATUS_LENGTH]; // "Reserved", "Checked-In", "Checked-Out", "Cancelled"
    char bookingDate[DATE_LENGTH]; // Assuming date format YYYY-MM-DD (including null terminator)
    char checkInDate[DATE_LENGTH]; // Assuming date format YYYY-MM-DD (including null terminator)
    char checkOutDate[DATE_LENGTH]; // Assuming date format YYYY-MM-DD (including null terminator)
    struct Booking *next; // Pointer to next node in the list
} Booking;


// FUNCTION PROTOTYPE

// Uitily Functions

bool containSpaces(const char *str); // 1
bool containNumeric(const char *str); // 2
bool containComma(const char *str); // 3
bool isEmpty(const char *str);
void clearInputBuffer();
void getCurrentDate(char* buffer, int bufferSize);
void getValidDate(char* date, const char* prompt, int allowPastDate, const char* referenceDate);
void getCurrentTimestamp(char *buffer, int bufferSize);


// User Functions

void saveUsers(User users[], int userCount);
int loadUsers(User users[], int *userCount);
void addAdministratorInfoToFile(User users[], int *userCount);
char* promptValidName();
char* promptValidID(User users[], int userCount);
char* promptValidPassword();
char* promptValidRole();
void registerUser(User users[], int *userCount);
void updateUserPassword(User users[], int *userCount);
void updateUserStatus(User users[], int *userCount);
void deleteUser(User users[], int *userCount);
void displayUser(User users[], int *userCount);
char* checkUserStatus(User users[], int *userCount);
void displayMenuManageUsers();
void manageUsers(User users[], int *userCount);


// Room Functions

void saveRooms(Room rooms[], int roomCount);
void loadRooms(Room rooms[], int *roomCount);
int promptValidRoomID(Room rooms[], int roomCount);
float promptValidRoomSize();
float promptValidRoomRate();
char* promptValidRoomBedType();
int promptValidRoomType();
char* promptValidAmenities();
void addRoom(Room rooms[], int *roomCount);
void deleteRoom(Room rooms[], int *roomCount);
void updateRoomStatus(Room rooms[], int *roomCount);
void displayRoom(Room rooms[], int *roomCount, int roomId);
void searchByRoomType(Room rooms[], int *roomCount);
void searchByBedType(Room rooms[], int *roomCount);
void searchByRate(Room rooms[], int *roomCount);
void searchRooms(Room rooms[], int *roomCount);
int checkRoomStatus(Room rooms[], int *roomCount);
void displayMenuManageRooms();
void manageRooms(Room rooms[], int roomCount);


// Booking Functions

void saveBookings(Booking *head);
Booking* loadBookings(int *bookingCount);
int generateUniqueId(Booking* head);
void addBookingNode(Booking **head, int *bookingCount, int uniqueId, const char *userId, int roomId, const char *bookingDate, const char *checkInDate, const char *checkOutDate);
int isRoomAvailableForBooking(Booking *head, int roomId, const char *checkInDate, const char *checkOutDate);
void displayBookingIdDetails(int *id, const char *userId, int *roomId, char *status, char *bookingDate, char *checkInDate, char *checkOutDate);
void makeReservation(Booking **head, int *bookingCount, Room rooms[], int *roomCount, const char *userId);
void cancelReservation(Booking **head, int *bookingCount, const char *userId);
void autoCancelReservations(Booking **head);
void freeBookings(Booking *head);
void checkin(Booking *head, int bookingCount);
void checkout(Booking *head, int bookingCount);
void updateOrderNumbers(Booking *head);
void viewBookingHistory(Booking *head, int bookingCount);
void checkCurrentBooking(Booking *head, int bookingCount, const char* userId);
void checkBookingHistory(Booking *head, int bookingCount, const char* userId);
void checkRoomAvailabilities(Booking **head, Room rooms[], int *roomCount);


// Login Logout Event Functions

void saveEvents(LoginLogoutEvent events[], int eventCount);
void loadEvents(LoginLogoutEvent events[], int *eventCount);
void logEvent(LoginLogoutEvent events[], int *eventCount, const char *userId, const char *eventType);
void searchEventsByUserId(LoginLogoutEvent events[], int eventCount);
void searchEventsByDateRange(LoginLogoutEvent events[], int eventCount);
void manageEvents(LoginLogoutEvent events[], int eventCount);


// Menu Functions

void adminMenu(User users[], int *userCount, Room rooms[], int *roomCount, LoginLogoutEvent events[], int *eventCount, const char *userId);
void displayStaffMenuOptions();
void hotelStaffMenu(User users[], int *userCount, Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount, const char *userId);
void displayCustomerMenuOptions();
void hotelCustomerMenu(Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount, const char *userId);
void displayMainMenu();
int authenticateUser(User users[], int *userCount, const char *input_id, const char *input_password);
void loginUser(User users[], int *userCount, Room rooms[], int *roomCount, Booking **head, int *bookingCount, LoginLogoutEvent events[], int *eventCount);

#endif