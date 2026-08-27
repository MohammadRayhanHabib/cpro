# Shohay - Community Help Request Management System

Shohay is a beginner-friendly C++ console application for a CSE OOP course. It manages food, medical and disaster-relief requests while keeping beneficiary information hidden from public lists.

The whole program is intentionally kept in one file: `src/main.cpp`. There are no header files, web frameworks or database dependencies.

## Workflow

```text
Volunteer submits request
          |
       Pending
          |
Moderator approves or rejects
          |
       Approved
          |
Organization claims request
          |
        Claimed
          |
Assistance is delivered
          |
       Completed
```

Only valid state changes are allowed. For example, an organization cannot claim a Pending request.

## Monochrome terminal dashboard

The interface uses plain ASCII borders, cards and sections, so it works without terminal colors.

```text
+======================================================================+
| SHOHAY                                                               |
| Community Help Request Management System                             |
+======================================================================+
| MAIN DASHBOARD                                                       |
+----------------------------------------------------------------------+
| Total: 3   Pending: 1   Approved: 1                                  |
| Claimed: 0   Completed: 1   Rejected: 0                              |
+======================================================================+
| [1] Submit a new help request                                        |
| [2] View pending requests                                            |
| [3] Approve or reject a request                                      |
| [0] Save and exit                                                    |
+======================================================================+
```

Each request is shown as a bordered card. After completing an action, press Enter to return to the main dashboard.

## Build and run

Open PowerShell in the `Shohay` folder, then run:

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic src\main.cpp -o shohay.exe
.\shohay.exe
```

The application creates `help_requests.txt` in the folder from which it is run. It automatically reloads those records the next time it starts.

## Main classes

- `User` - abstract parent class
- `Volunteer`, `Moderator`, `Organization`, `Admin` - user child classes
- `HelpRequest` - abstract parent class
- `FoodRequest`, `MedicalRequest`, `DisasterRequest` - request child classes
- `RequestManager` - workflow, search, status and ID management
- `FileManager` - saves and loads request records

## Four OOP pillars

1. **Abstraction** - `User` and `HelpRequest` contain pure virtual functions.
2. **Encapsulation** - beneficiary data and request status are private. Status changes only through controlled methods such as `approve()`, `claim()` and `complete()`.
3. **Inheritance** - all roles inherit from `User`; all request types inherit from `HelpRequest`.
4. **Polymorphism** - every request type overrides `calculatePriorityScore()`, while every role overrides its role and responsibility functions.

## Privacy behavior

- Public search and verified-request lists always hide beneficiary name, phone and exact address.
- Moderators and admins can use the internal view for verification.
- An organization sees private details only after it claims the request and an admin authorizes access.
- `help_requests.txt` is a simple local course-project file, not encrypted production storage. A real system should use authentication, encryption and a secure database.

## Priority calculation

- Food requests use urgency plus the affected-person bonus.
- Medical requests receive an additional medical-risk score.
- Disaster requests receive the highest type-risk score.

Calling `calculatePriorityScore()` through a `HelpRequest` pointer runs the correct child-class version at runtime.

## Suggested demo

1. Submit Food, Medical and Disaster requests.
2. Submit one request without a local volunteer to show central forwarding.
3. Approve one request and reject another.
4. Verify that only Approved requests appear in the organization list.
5. Claim an Approved request.
6. Check that beneficiary information stays protected.
7. Authorize the claimed case, view it as the organization and complete it.
8. Exit and reopen the program to demonstrate file handling.
