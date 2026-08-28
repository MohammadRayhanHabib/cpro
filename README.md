# CivicCare Bangladesh

A beginner-friendly C++ console project for managing public complaints. The
complete program intentionally stays in one `main.cpp` file and uses simple
functions, loops, classes and text files.

## Workflow

```text
Submitted -> Under Review -> Assigned -> In Progress -> Resolved -> Closed
```

## Working features

- Separate Citizen, Authority/Admin and Officer dashboards
- 10 complaint categories, automatic IDs and editable priority
- Validated workflow: review, assign, start work, resolve and close
- Automatic department mapping and officer assignment
- My complaints, ID tracking and category/location search
- One support and one poll vote per citizen
- Resolution feedback, read/unread notifications and history
- Status analytics and resolution-rate calculation
- Input checks, duplicate-action prevention and invalid-status protection
- Persistent local files: `complaints.txt`, `notifications.txt`, `votes.txt` and `history.txt`

## OOP used

- **Abstraction:** `User` and `Complaint` are abstract classes.
- **Encapsulation:** user and complaint data are private.
- **Inheritance:** `Citizen` and `Authority` inherit from `User`.
- **Multilevel inheritance:** `Admin` and `Officer` inherit from `Authority`.
- **Polymorphism:** complaint classes override `category()` and `riskScore()`.
- **Overloading:** `findComplaint()` works with an ID or category/location.

## Build and run

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic src\main.cpp -o civiccare.exe
.\civiccare.exe
```

For a complete demo, submit a complaint from the Citizen dashboard, review and
assign it from the Authority dashboard, then start and resolve it from the
Officer dashboard. No external library, database or internet connection is needed.
