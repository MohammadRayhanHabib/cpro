# CivicCare Bangladesh

A beginner-friendly C++ console project for managing public complaints. The
complete program intentionally stays in one `main.cpp` file and uses simple
functions, loops, classes and text files.

## Workflow

```text
Submitted -> Under Review -> Assigned -> In Progress -> Resolved -> Closed
```

## Working features

- Citizen signup with NID, email, phone, address and password
- Authority, Officer and Admin signup with Job ID, NID, email, phone and passport
- Persistent login using Job ID, NID, email, phone or passport plus password
- Duplicate identity checks, wrong-password protection and role-based dashboards
- Separate Citizen, Authority/Admin and Officer dashboards
- 10 complaint categories; normal complaints start with `Low` priority
- Citizen dashboard option 9: Emergency complaint starts with `Critical` priority
- Menu options are separated by blank lines
- At any input, type `back` to cancel the unfinished form and return to the menu,
  or `exit` to save completed work and quit (case-insensitive; `/back` and `/exit` also work)
- Authority can manually change priority to Low, Medium, High or Critical
- Validated workflow: review, assign, start work, resolve and close
- Automatic department mapping and officer assignment
- My complaints, ID tracking and category/location search
- One support and one poll vote per citizen
- Category poll: at least 70% of votes cast makes that category's non-closed
  complaints Critical (Road, Waste or Drainage). No votes means no promotion.
  One vote out of one is 100%; there is no minimum turnout. Complaint Support
  is separate from this poll. Existing and newly submitted complaints are checked,
  including on restart. Falling below 70% does not automatically lower priority.
- Resolution feedback, read/unread notifications and history
- Status analytics and resolution-rate calculation
- Input checks, duplicate-action prevention and invalid-status protection
- Persistent local files: `users.txt`, `complaints.txt`, `notifications.txt`, `votes.txt` and `history.txt`

## OOP used

- **Abstraction:** `User` and `Complaint` are abstract classes.
- **Encapsulation:** user and complaint data are private.
- **Inheritance:** `Citizen` and `Authority` inherit from `User`.
- **Multilevel inheritance:** `Admin` and `Officer` inherit from `Authority`.
- **Polymorphism:** complaint classes override `getCategory()`.
- **Overloading:** `findComplaint()` works with an ID or category/location.

## Build and run

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic src\main.cpp -o civiccare.exe
.\civiccare.exe
```

For a complete demo, submit a complaint from the Citizen dashboard, review and
assign it from the Authority dashboard, then start and resolve it from the
Officer dashboard. No external library, database or internet connection is needed.

`0` keeps its usual menu meaning (Logout or Exit). Unlike `0`, the reserved
commands `back` and `exit` work inside forms too, so they cannot be used as entire
field values. Closing the input stream also exits instead of looping forever.
An unfinished signup or complaint is discarded; already completed work stays saved.

Run isolated regression tests with `python tests/test_console.py`. They compile
the C++ app and use temporary data folders, leaving your real text files untouched.

## First login

Two demo staff accounts are created automatically when `users.txt` does not exist:

```text
Admin   : admin / admin123
Officer : officer / officer123
```

New users can sign up from the main menu. Login accepts a saved Job ID, NID,
email, phone or passport together with the correct password. All account
information stays in the local `users.txt` file.
