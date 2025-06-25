# Hotel Management System

This is a C-based Hotel Management System for managing hotel operations such as user accounts, room inventory, bookings, and login/logout event tracking. The system uses plain text files for persistent storage and provides a menu-driven interface for administrators, staff, and customers.

## Features

- User Management: Register, update, delete, and view users. Supports roles: Administrator, Hotel Staff, Customer.
- Room Management: Add, update, delete, and search rooms by type, bed, or rate.
- Booking Management: Make, cancel, check-in, and check-out bookings. View booking history and current bookings.
- Event Logging: Tracks login and logout events for all users.
- Data Persistence: All data is stored in text files (`users.txt`, `rooms.txt`, `bookings.txt`, `login_logout_events.txt`).

## File Structure

- `Hotel.c` — Main source code and function implementations.
- `hotel.h` — Header file with structure definitions and function prototypes.
- `users.txt` — Stores user account data.
- `rooms.txt` — Stores room inventory and details.
- `bookings.txt` — Stores all booking records.
- `login_logout_events.txt` — Stores login/logout event logs.

## How to Build

Compile with GCC:
```sh
gcc Hotel.c -o Hotel -Wall
```

## How to Run

```sh
./Hotel
```

## Usage

- On startup, the system loads all data from the text files.
- Follow the on-screen menus to log in as an administrator, staff, or customer.
- Administrators can manage users, rooms, and view event logs.
- Staff can manage bookings for customers.
- Customers can make reservations, view their bookings, and check room availability.

## Data Format

- users.txt:  
  `orderNum, name, id, password, role, active`
- rooms.txt:
  `orderNum, id, type, bedType, rate, size, [amenities], active`
- bookings.txt:
  `orderNum, id, userId, roomId, status, bookingDate, checkInDate, checkOutDate`
- login_logout_events.txt:
  `orderNum, userId, eventType, timestamp`

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License

MIT License
