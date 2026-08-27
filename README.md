# CivicCare Bangladesh

A small, beginner-friendly C++ console project for managing public complaints. The complete program is kept in one `main.cpp` file so a new student can understand it easily.

## Workflow

```text
Submitted -> Under Review -> Assigned -> In Progress -> Resolved -> Closed
```

## Features

- Road, Waste and Drainage complaints
- Automatic complaint IDs and priorities
- Department and officer assignment
- Support/upvote and complaint search
- Civic poll with duplicate-vote prevention
- Feedback, notifications and analytics
- Local file saving in `complaints.txt`

## OOP used

- **Abstraction:** `User` and `Complaint` are abstract classes.
- **Encapsulation:** user and complaint data are private.
- **Inheritance:** `Citizen` and `Authority` inherit from `User`.
- **Multilevel inheritance:** `Admin` and `Officer` inherit from `Authority`.
- **Polymorphism:** complaint classes override `category()` and `riskScore()`.
- **Overloading:** `searchComplaint()` works with an ID or category/location.

## Build and run

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic src\main.cpp -o civiccare.exe
.\civiccare.exe
```

No external library, database or internet connection is needed.
