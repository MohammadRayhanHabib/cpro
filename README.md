# CivicCare Bangladesh

A beginner-friendly C++ console project for managing public complaints. The
complete program intentionally stays in one `main.cpp` file and uses simple
functions, loops, classes and text files.

The current source in `src/main.cpp` is based on the team's edited
`complaintmgmt.cpp`, with the complaint poll changes applied.

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
- Citizen dashboard option 8: Emergency complaint starts with `Critical` priority
- Menu options are displayed on separate lines
- At any input, type `back` to cancel the unfinished form and return to the menu,
  or `exit` to save completed work and quit (case-insensitive; `/back` and `/exit` also work)
- Authority can manually change priority to Low, Medium, High or Critical
- Validated workflow: review, assign, start work, resolve and close
- Automatic department mapping and officer assignment
- My complaints, ID tracking and category/location search
- Citizen dashboard option 4: Poll (the separate Support option has been removed).
  Shows complaints grouped under all 10 categories, with their
  IDs, details, status, priority and poll vote counts. Empty categories are shown too.
  Enter the complaint number (for example, `1001`) to vote for that complaint.
- One poll vote per citizen per complaint, including their own complaint;
  the same citizen can vote for other complaints. Duplicate votes are rejected,
  including after restarting.
- More than 50 distinct user votes (51 or more) makes only that complaint
  Critical; exactly 50 votes does not. Categories and percentages no longer
  determine priority. Qualified complaints cannot be manually downgraded.
  Promotion is checked on restart and recorded once in history/notifications.
- Closed complaints remain visible but cannot receive votes or change priority.
- Resolution feedback, read/unread notifications and history
- Status analytics and resolution-rate calculation
- Input checks, duplicate-action prevention and invalid-status protection
- Persistent local files: `users.txt`, `complaints.txt`, `notifications.txt`,
  `complaint_votes.txt` and `history.txt`

Old `votes.txt` category votes are left untouched and are not treated as complaint
votes. New poll records use `userId complaintId` in `complaint_votes.txt`.
Existing saved priorities are preserved; the old rule is no longer applied.
The old support field is retained only for saved-file compatibility. It is
hidden from complaint details and does not count toward poll votes.

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
