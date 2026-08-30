#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
string input(string message) {
    string value;
    cout << message;
    getline(cin, value);
    return value;
}
string requiredInput(string message) {
    while (true) {
        string value = input(message);
        if (!value.empty()) return value;
        cout << "Value cannot be empty.\n";
    }
}
int numberInput(string message, int minimum, int maximum) {
    while (true) {
        stringstream stream(input(message));
        int value;
        char extra;
        if (stream >> value && !(stream >> extra) && value >= minimum && value <= maximum)
            return value;
        cout << "Enter " << minimum << " to " << maximum << ".\n";
    }
}
string clean(string value) {
    for (int i = 0; i < (int)value.size(); i++)
        if (value[i] == '|') value[i] = '/';
    return value;
}
vector<string> split(string line, char separator = '|') {
    vector<string> parts;
    string part;
    stringstream stream(line);
    while (getline(stream, part, separator)) parts.push_back(part);
    return parts;
}
string lowerCase(string value) {
    for (int i = 0; i < (int)value.size(); i++)
        value[i] = (char)tolower((unsigned char)value[i]);
    return value;
}
bool onlyDigits(string value) {
    if (value.empty()) return false;
    for (int i = 0; i < (int)value.size(); i++)
        if (!isdigit((unsigned char)value[i])) return false;
    return true;
}
string validPhoneInput() {
    while (true) {
        string phone = input("Phone number: ");
        if (onlyDigits(phone) && phone.size() >= 10 && phone.size() <= 15)
            return phone;
        cout << "Use 10 to 15 digits.\n";
    }
}
string validNidInput() {
    while (true) {
        string nid = input("NID number: ");
        if (onlyDigits(nid) && nid.size() >= 10 && nid.size() <= 17) return nid;
        cout << "Use 10 to 17 digits.\n";
    }
}
string validEmailInput() {
    while (true) {
        string email = input("Email: ");
        if (email.find('@') != string::npos && email.find('.') != string::npos && email.find('|') == string::npos)
            return lowerCase(email);
        cout << "Enter a valid email.\n";
    }
}
string identifierInput(string message) {
    while (true) {
        string value = input(message);
        if (!value.empty() && value.find('|') == string::npos) return value;
        cout << "Value cannot be empty or contain |.\n";
    }
}
string validPasswordInput() {
    while (true) {
        string password = input("Password (minimum 4 characters): ");
        if (password.size() >= 4 && password.find('|') == string::npos)
            return password;
        cout << "Use at least 4 characters and do not use |.\n";
    }
}
enum Status { SUBMITTED, UNDER_REVIEW, ASSIGNED, IN_PROGRESS, RESOLVED, CLOSED };
enum Priority { LOW, MEDIUM, HIGH, CRITICAL };
string statusText(Status status) {
    string names[] = {"Submitted", "Under Review", "Assigned",
                      "In Progress", "Resolved", "Closed"};
    return names[status];
}
string priorityText(Priority priority) {
    string names[] = {"Low", "Medium", "High", "Critical"};
    return names[priority];
}
// Abstraction and inheritance
class User {
private:
    int id;
    string name, nid, email, phone, address, password;
    string governmentId, passport;
public:
    User(int userId, string userName, string userNid, string userEmail, string userPhone, string userAddress, string userPassword,
         string govtId = "None", string passportNumber = "None")
        : id(userId), name(userName), nid(userNid), email(lowerCase(userEmail)),
          phone(userPhone), address(userAddress), password(userPassword),
          governmentId(govtId), passport(passportNumber) {}
    virtual ~User() {}
    int getId() const { return id; }
    string getName() const { return name; }
    string getNid() const { return nid; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    string getGovernmentId() const { return governmentId; }
    string getPassport() const { return passport; }
    virtual string getRole() const = 0;
    bool matches(string key, string enteredPassword) const {
        key = lowerCase(key);
        return password == enteredPassword && (key == lowerCase(nid) || key == email || key == lowerCase(phone) ||
                key == lowerCase(governmentId) || key == lowerCase(passport));
    }
    string saveData() const {
        return to_string(id) + "|" + getRole() + "|" + clean(name) + "|" + clean(nid) + "|" + clean(email) + "|" + clean(phone) + "|" +
               clean(address) + "|" + clean(password) + "|" +
               clean(governmentId) + "|" + clean(passport);
    }
    virtual void showProfile() const {
        cout << "\nID      : U-" << id << "\nName    : " << name << "\nRole    : " << getRole() << "\nNID     : " << nid
             << "\nEmail   : " << email << "\nPhone   : " << phone
             << "\nAddress : " << address << "\n";
        if (getRole() != "Citizen")
            cout << "Govt ID : " << governmentId << "\nPassport: " << passport << "\n";
    }
};
class Citizen : public User {
public:
    Citizen(int id, string name, string nid, string email, string phone, string address, string password)
        : User(id, name, nid, email, phone, address, password) {}
    string getRole() const { return "Citizen"; }
};
class Authority : public User {
public:
    Authority(int id, string name, string nid, string email, string phone, string address, string password, string govtId, string passport)
        : User(id, name, nid, email, phone, address, password, govtId, passport) {}
    string getRole() const { return "Authority"; }
};
class Admin : public Authority {
public:
    Admin(int id, string name, string nid, string email, string phone, string address, string password, string govtId, string passport)
        : Authority(id, name, nid, email, phone, address, password, govtId, passport) {}
    string getRole() const { return "Admin"; }
};
class Officer : public Authority {
public:
    Officer(int id, string name, string nid, string email, string phone, string address, string password, string govtId, string passport)
        : Authority(id, name, nid, email, phone, address, password, govtId, passport) {}
    string getRole() const { return "Officer"; }
};
User* createUser(int id, string role, string name, string nid, string email, string phone, string address, string password, string govtId, string passport) {
    if (role == "Citizen")
        return new Citizen(id, name, nid, email, phone, address, password);
    if (role == "Admin")
        return new Admin(id, name, nid, email, phone, address, password, govtId, passport);
    if (role == "Officer")
        return new Officer(id, name, nid, email, phone, address, password, govtId, passport);
    return new Authority(id, name, nid, email, phone, address, password, govtId, passport);
}
class UserManager {
private:
    vector<User*> users;
    int nextId;
    bool duplicate(string nid, string email, string phone, string govtId = "None", string passport = "None") const {
        for (int i = 0; i < (int)users.size(); i++)
            if (users[i]->getNid() == nid || users[i]->getEmail() == lowerCase(email) || users[i]->getPhone() == phone ||
                (govtId != "None" && lowerCase(users[i]->getGovernmentId()) == lowerCase(govtId)) ||
                (passport != "None" && lowerCase(users[i]->getPassport()) == lowerCase(passport)))
                return true;
        return false;
    }
    void addDemoUsers() {
        users.push_back(createUser(1, "Admin", "System Admin", "0000000001", "admin", "01000000001", "Dhaka", "admin123", "GOV-ADMIN-001", "A0000001"));
        users.push_back(createUser(2, "Officer", "Local Officer", "0000000002", "officer", "01000000002", "Dhaka", "officer123",
                        "GOV-OFFICER-001", "B0000001"));
        nextId = 3;
        save();
    }
public:
    UserManager() : nextId(1) {
        ifstream file("users.txt");
        string line;
        while (getline(file, line)) {
            vector<string> data = split(line);
            try {
                if (data.size() == 10) {
                    User* user = createUser(stoi(data[0]), data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
                    users.push_back(user);
                    nextId = max(nextId, user->getId() + 1);
                }
            } catch (...) {
                cout << "Warning: damaged user data skipped.\n";
            }
        }
        if (users.empty()) addDemoUsers();
    }
    ~UserManager() {
        save();
        for (int i = 0; i < (int)users.size(); i++) delete users[i];
    }
    void save() const {
        ofstream file("users.txt");
        for (int i = 0; i < (int)users.size(); i++)
            file << users[i]->saveData() << "\n";
    }
    void citizenSignUp() {
        cout << "\n=== CITIZEN SIGN UP ===\n";
        string nid = validNidInput();
        string email = validEmailInput();
        string phone = validPhoneInput();
        if (duplicate(nid, email, phone)) {
            cout << "[ERROR] NID, email or phone is already registered.\n";
            return;
        }
        string name = requiredInput("Full name: ");
        string address = requiredInput("Address: ");
        string password = validPasswordInput();
        users.push_back(new Citizen(nextId++, name, nid, email, phone, address, password));
        save();
        cout << "[SUCCESS] Citizen account created. You can now login.\n";
    }
    void staffSignUp() {
        cout << "\n=== GOVERNMENT STAFF SIGN UP ===\n" << "1. Authority  2. Officer  3. Admin\n";
        int choice = numberInput("Role: ", 1, 3);
        string roles[] = {"", "Authority", "Officer", "Admin"};
        string nid = validNidInput();
        string govtId = identifierInput("Government ID: ");
        string passport = identifierInput("Passport number: ");
        string email = validEmailInput();
        string phone = validPhoneInput();
        if (duplicate(nid, email, phone, govtId, passport)) {
            cout << "[ERROR] Identity information is already registered.\n";
            return;
        }
        string name = requiredInput("Full name: ");
        string address = requiredInput("Address: ");
        string password = validPasswordInput();
        users.push_back(createUser(nextId++, roles[choice], name, nid, email, phone, address, password, govtId, passport));
        save();
        cout << "[SUCCESS] " << roles[choice] << " account created.\n";
    }
    User* login() {
        cout << "\n=== LOGIN ===\n";
        string key = requiredInput("NID, email, phone, Govt ID or passport: ");
        string password = input("Password: ");
        for (int i = 0; i < (int)users.size(); i++)
            if (users[i]->matches(key, password)) {
                cout << "[SUCCESS] Welcome, " << users[i]->getName() << ".\n";
                return users[i];
            }
        cout << "[ERROR] Login information did not match.\n";
        return NULL;
    }
    Officer* selectOfficer() const {
        vector<Officer*> officers;
        for (int i = 0; i < (int)users.size(); i++)
            if (users[i]->getRole() == "Officer")
                officers.push_back(dynamic_cast<Officer*>(users[i]));
        if (officers.empty()) {
            cout << "No registered officer.\n";
            return NULL;
        }
        cout << "\nRegistered officers\n";
        for (int i = 0; i < (int)officers.size(); i++)
            cout << i + 1 << ". U-" << officers[i]->getId() << " - " << officers[i]->getName() << "\n";
        int choice = numberInput("Officer: ", 1, (int)officers.size());
        return officers[choice - 1];
    }
    void showUsers() const {
        cout << "\n=== REGISTERED USERS ===\n";
        for (int i = 0; i < (int)users.size(); i++)
            cout << "U-" << users[i]->getId() << " | " << users[i]->getRole() << " | " << users[i]->getName() << " | " << users[i]->getEmail() << "\n";
    }
};
class Notification {
public:
    int userId;
    bool isRead;
    string message;
    Notification(int id, string text, bool read = false)
        : userId(id), isRead(read), message(text) {}
    string saveData() const {
        return to_string(userId) + "|" + to_string(isRead) + "|" + clean(message);
    }
};
// Encapsulation and runtime polymorphism
class Complaint {
private:
    int id, citizenId, officerId, rating;
    string title, description, location, department, officer;
    string resolution, comment;
    vector<int> supporters;
    Status status;
    Priority priority;
public:
    Complaint(int complaintId, int ownerId, string complaintTitle, string details, string place)
        : id(complaintId), citizenId(ownerId), officerId(0), rating(0),
          title(complaintTitle), description(details), location(place),
          department("Not assigned"), officer("Not assigned"),
          resolution("None"), comment("None"),
          status(SUBMITTED), priority(LOW) {}
    virtual ~Complaint() {}
    virtual string getCategory() const = 0;
    int getId() const { return id; }
    int getCitizenId() const { return citizenId; }
    string getLocation() const { return location; }
    Status getStatus() const { return status; }
    bool assignedTo(int userId, string userName) const {
        return officerId > 0 ? officerId == userId : officer == userName;
    }
    bool setPriority(Priority newPriority) {
        if (status == CLOSED) return false;
        priority = newPriority;
        return true;
    }
    bool moveStatus(Status current, Status next) {
        if (status != current) return false;
        status = next;
        return true;
    }
    bool assignTo(string departmentName, int assignedOfficerId, string officerName) {
        if (!moveStatus(UNDER_REVIEW, ASSIGNED)) return false;
        department = departmentName;
        officerId = assignedOfficerId;
        officer = officerName;
        return true;
    }
    bool resolve(string note) {
        if (!moveStatus(IN_PROGRESS, RESOLVED)) return false;
        resolution = note;
        return true;
    }
    bool support(int userId) {
        if (status == CLOSED) return false;
        for (int i = 0; i < (int)supporters.size(); i++)
            if (supporters[i] == userId) return false;
        supporters.push_back(userId);
        return true;
    }
    bool feedback(int stars, string feedbackComment) {
        if (status != RESOLVED || rating != 0) return false;
        rating = stars;
        comment = feedbackComment;
        return true;
    }
    void restore(Priority oldPriority, Status oldStatus, vector<int> oldSupporters,
                 string oldDepartment, int oldOfficerId, string oldOfficer, string oldResolution,
                 int oldRating, string oldComment) {
        priority = oldPriority;
        status = oldStatus;
        supporters = oldSupporters;
        department = oldDepartment;
        officerId = oldOfficerId;
        officer = oldOfficer;
        resolution = oldResolution;
        rating = oldRating;
        comment = oldComment;
    }
    void show() const {
        cout << "\n----------------------------------------" << "\nComplaint ID : CC-" << id << "\nCategory     : " << getCategory()
             << "\nTitle        : " << title
             << "\nDescription  : " << description
             << "\nLocation     : " << location
             << "\nPriority     : " << priorityText(priority)
             << "\nStatus       : " << statusText(status)
             << "\nSupports     : " << supporters.size()
             << "\nDepartment   : " << department
             << "\nOfficer      : " << officer
             << "\nResolution   : " << resolution
             << "\nRating       : " << (rating ? to_string(rating) + "/5" : "Not given")
             << "\nComment      : " << comment << "\n";
    }
    string saveData() const {
        string supporterData;
        for (int i = 0; i < (int)supporters.size(); i++) {
            if (i > 0) supporterData += ",";
            supporterData += to_string(supporters[i]);
        }
        return to_string(id) + "|" + to_string(citizenId) + "|" + getCategory() + "|" + clean(title) + "|" + clean(description) + "|" + clean(location) +
               "|" + to_string(priority) + "|" + to_string(status) + "|" +
               supporterData + "|" + clean(department) + "|" + to_string(officerId) +
               "|" + clean(officer) + "|" + clean(resolution) + "|" +
               to_string(rating) + "|" + clean(comment);
    }
};
class RoadComplaint : public Complaint {
public:
    RoadComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string getCategory() const { return "Road"; }
};
class WasteComplaint : public Complaint {
public:
    WasteComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string getCategory() const { return "Waste"; }
};
class DrainageComplaint : public Complaint {
public:
    DrainageComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string getCategory() const { return "Drainage"; }
};
class OtherComplaint : public Complaint {
private:
    string category;
public:
    OtherComplaint(int id, int owner, string title, string details, string location, string type)
        : Complaint(id, owner, title, details, location),
          category(type) {}
    string getCategory() const { return category; }
};
Complaint* createComplaint(string category, int id, int owner, string title, string description, string location) {
    if (category == "Road")
        return new RoadComplaint(id, owner, title, description, location);
    if (category == "Waste")
        return new WasteComplaint(id, owner, title, description, location);
    if (category == "Drainage")
        return new DrainageComplaint(id, owner, title, description, location);
    return new OtherComplaint(id, owner, title, description, location, category);
}
class Poll {
private:
    vector<int> voters, choices;
public:
    void load() {
        ifstream file("votes.txt");
        int userId, choice;
        while (file >> userId >> choice)
            if (choice >= 1 && choice <= 3) {
                voters.push_back(userId);
                choices.push_back(choice);
            }
    }
    void save() const {
        ofstream file("votes.txt");
        for (int i = 0; i < (int)voters.size(); i++)
            file << voters[i] << " " << choices[i] << "\n";
    }
    void show() const {
        string options[] = {"Road", "Waste", "Drainage"};
        int count[] = {0, 0, 0};
        for (int i = 0; i < (int)choices.size(); i++) count[choices[i] - 1]++;
        cout << "\nWhich issue needs attention first?\n";
        for (int i = 0; i < 3; i++) {
            int percent = choices.empty() ? 0 : count[i] * 100 / choices.size();
            cout << i + 1 << ". " << options[i] << " - " << count[i] << " vote(s), " << percent << "%\n";
        }
    }
    bool vote(int userId, int choice) {
        for (int i = 0; i < (int)voters.size(); i++)
            if (voters[i] == userId) return false;
        voters.push_back(userId);
        choices.push_back(choice);
        save();
        return true;
    }
};
class FileManager {
public:
    static void saveComplaints(const vector<Complaint*>& complaints) {
        ofstream file("complaints.txt");
        for (int i = 0; i < (int)complaints.size(); i++)
            file << complaints[i]->saveData() << "\n";
    }
    static int loadComplaints(vector<Complaint*>& complaints) {
        ifstream file("complaints.txt");
        string line;
        int nextId = 1001;
        while (getline(file, line)) {
            vector<string> data = split(line);
            try {
                Complaint* complaint = NULL;
                vector<int> supporters;
                if (data.size() == 12) { // Old file support
                    int savedStatus = stoi(data[6]);
                    if (savedStatus < SUBMITTED || savedStatus > CLOSED) continue;
                    complaint = createComplaint(data[2], stoi(data[0]), stoi(data[1]), data[3], data[4], data[5]);
                    for (int i = 0; i < stoi(data[7]); i++) supporters.push_back(-i - 1);
                    complaint->restore(LOW, (Status)savedStatus, supporters, data[8], 0, data[9], data[10], stoi(data[11]), "None");
                } else if (data.size() == 14 || data.size() == 15) {
                    int savedPriority = stoi(data[6]), savedStatus = stoi(data[7]);
                    if (savedPriority < LOW || savedPriority > CRITICAL || savedStatus < SUBMITTED || savedStatus > CLOSED) continue;
                    complaint = createComplaint(data[2], stoi(data[0]), stoi(data[1]), data[3], data[4], data[5]);
                    vector<string> ids = split(data[8], ',');
                    for (int i = 0; i < (int)ids.size(); i++)
                        if (!ids[i].empty()) supporters.push_back(stoi(ids[i]));
                    int extra = data.size() == 15 ? 1 : 0;
                    int officerId = extra ? stoi(data[10]) : 0;
                    complaint->restore((Priority)savedPriority, (Status)savedStatus, supporters, data[9], officerId, data[10 + extra],
                                       data[11 + extra], stoi(data[12 + extra]),
                                       data[13 + extra]);
                }
                if (complaint) {
                    complaints.push_back(complaint);
                    nextId = max(nextId, complaint->getId() + 1);
                }
            } catch (...) {
                cout << "Warning: damaged complaint data skipped.\n";
            }
        }
        return nextId;
    }
    static void saveNotifications(const vector<Notification>& notifications) {
        ofstream file("notifications.txt");
        for (int i = 0; i < (int)notifications.size(); i++)
            file << notifications[i].saveData() << "\n";
    }
    static void loadNotifications(vector<Notification>& notifications) {
        ifstream file("notifications.txt");
        string line;
        while (getline(file, line)) {
            vector<string> data = split(line);
            try {
                if (data.size() == 3)
                    notifications.push_back( Notification(stoi(data[0]), data[2], stoi(data[1])));
            } catch (...) {}
        }
    }
    static void addHistory(string text) {
        ofstream file("history.txt", ios::app);
        file << clean(text) << "\n";
    }
};
class CivicCareSystem {
private:
    vector<Complaint*> complaints;
    vector<Notification> notifications;
    Poll poll;
    int nextId;
    void save() {
        FileManager::saveComplaints(complaints);
        FileManager::saveNotifications(notifications);
    }
    void addNotification(int userId, string message) {
        notifications.push_back(Notification(userId, message));
    }
    void addHistory(int id, string message) {
        FileManager::addHistory("CC-" + to_string(id) + ": " + message);
    }
    string departmentFor(string category) const {
        if (category == "Road") return "Road & Maintenance";
        if (category == "Waste") return "Waste Management";
        if (category == "Drainage") return "Drainage Department";
        if (category == "Water") return "Water Supply";
        if (category == "Street Light" || category == "Electricity") return "Electrical";
        if (category == "Traffic") return "Traffic Department";
        if (category == "Public Health") return "Public Health";
        return "General Services";
    }
    Complaint* selectComplaint() {
        int id = numberInput("Complaint number (without CC-): ", 1, 999999);
        Complaint* complaint = findComplaint(id);
        if (!complaint) cout << "Complaint not found.\n";
        return complaint;
    }
    bool updateStatus(Complaint* complaint, Status from, Status to, string message) {
        if (!complaint->moveStatus(from, to)) {
            cout << "Required current status: " << statusText(from) << ".\n";
            return false;
        }
        addNotification(complaint->getCitizenId(), message);
        addHistory(complaint->getId(), statusText(to));
        cout << "Status changed to " << statusText(to) << ".\n";
        save();
        return true;
    }
public:
    CivicCareSystem() {
        nextId = FileManager::loadComplaints(complaints);
        FileManager::loadNotifications(notifications);
        poll.load();
    }
    ~CivicCareSystem() {
        save();
        for (int i = 0; i < (int)complaints.size(); i++) delete complaints[i];
    }
    Complaint* findComplaint(int id) { // Overload 1
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getId() == id) return complaints[i];
        return NULL;
    }
    void findComplaint(string category, string location) { // Overload 2
        bool found = false;
        category = lowerCase(category);
        location = lowerCase(location);
        for (int i = 0; i < (int)complaints.size(); i++) {
            if (lowerCase(complaints[i]->getCategory()).find(category) != string::npos &&
                lowerCase(complaints[i]->getLocation()).find(location) != string::npos) {
                complaints[i]->show();
                found = true;
            }
        }
        if (!found) cout << "No matching complaint.\n";
    }
    void report(const Citizen& citizen) {
        string categories[] = {"", "Road", "Waste", "Drainage", "Water",
                               "Street Light", "Traffic", "Environment",
                               "Public Health", "Electricity", "Other"};
        cout << "\n1.Road  2.Waste  3.Drainage  4.Water  5.Street Light\n" << "6.Traffic  7.Environment  8.Public Health  9.Electricity  10.Other\n";
        int choice = numberInput("Category: ", 1, 10);
        Complaint* complaint = createComplaint( categories[choice], nextId, citizen.getId(), requiredInput("Title: "), requiredInput("Description: "),
            requiredInput("Location: "));
        complaints.push_back(complaint);
        addNotification(citizen.getId(), "CC-" + to_string(nextId) + " submitted.");
        addHistory(nextId, "Submitted");
        cout << "Created successfully. ID: CC-" << nextId++ << "\n";
        save();
    }
    void showAll() const {
        if (complaints.empty()) cout << "No complaints.\n";
        for (int i = 0; i < (int)complaints.size(); i++) complaints[i]->show();
    }
    void showMyComplaints(int userId) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getCitizenId() == userId) {
                complaints[i]->show();
                found = true;
            }
        if (!found) cout << "No complaints.\n";
    }
    void showByStatus(Status status) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getStatus() == status) {
                complaints[i]->show();
                found = true;
            }
        if (!found) cout << "No " << statusText(status) << " complaints.\n";
    }
    void track(const Citizen& citizen) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        if (complaint->getCitizenId() != citizen.getId())
            cout << "This is not your complaint.\n";
        else complaint->show();
    }
    void review() {
        Complaint* complaint = selectComplaint();
        if (complaint)
            updateStatus(complaint, SUBMITTED, UNDER_REVIEW, "Your complaint is under review.");
    }
    void changePriority() {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        int choice = numberInput("1.Low  2.Medium  3.High  4.Critical: ", 1, 4);
        if (!complaint->setPriority((Priority)(choice - 1))) {
            cout << "Closed complaint cannot be changed.\n";
            return;
        }
        addHistory(complaint->getId(), "Priority: " + priorityText((Priority)(choice - 1)));
        cout << "Priority updated.\n";
        save();
    }
    void assign(const Officer& officer) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        string department = departmentFor(complaint->getCategory());
        if (!complaint->assignTo(department, officer.getId(), officer.getName())) {
            cout << "Review the complaint first.\n";
            return;
        }
        addNotification(complaint->getCitizenId(), "Assigned to " + department + ".");
        addHistory(complaint->getId(), "Assigned to " + officer.getName());
        cout << "Assigned to " << department << " and " << officer.getName() << ".\n";
        save();
    }
    void showAssigned(const Officer& officer) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->assignedTo(officer.getId(), officer.getName()) && complaints[i]->getStatus() >= ASSIGNED) {
                complaints[i]->show();
                found = true;
            }
        if (!found) cout << "No assigned complaints.\n";
    }
    void startWork(const Officer& officer) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        if (!complaint->assignedTo(officer.getId(), officer.getName())) {
            cout << "This complaint is not assigned to you.\n";
            return;
        }
        updateStatus(complaint, ASSIGNED, IN_PROGRESS, "Work on your complaint has started.");
    }
    void resolve(const Officer& officer) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        if (!complaint->assignedTo(officer.getId(), officer.getName())) {
            cout << "This complaint is not assigned to you.\n";
            return;
        }
        string note = requiredInput("Resolution note: ");
        if (!complaint->resolve(note)) {
            cout << "Start the work first.\n";
            return;
        }
        addNotification(complaint->getCitizenId(), "Your complaint was resolved.");
        addHistory(complaint->getId(), "Resolved - " + note);
        cout << "Complaint resolved.\n";
        save();
    }
    void closeComplaint() {
        Complaint* complaint = selectComplaint();
        if (complaint)
            updateStatus(complaint, RESOLVED, CLOSED, "Your complaint was closed.");
    }
    void support(const Citizen& citizen) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        if (complaint->getCitizenId() == citizen.getId()) {
            cout << "You cannot support your own complaint.\n";
            return;
        }
        if (complaint->support(citizen.getId())) {
            cout << "Support added.\n";
            save();
        } else cout << "Already supported or complaint is closed.\n";
    }
    void feedback(const Citizen& citizen) {
        Complaint* complaint = selectComplaint();
        if (!complaint) return;
        if (complaint->getCitizenId() != citizen.getId()) {
            cout << "You can only review your complaint.\n";
            return;
        }
        int stars = numberInput("Rating (1-5): ", 1, 5);
        string comment = requiredInput("Comment: ");
        if (complaint->feedback(stars, comment)) {
            cout << "Feedback saved.\n";
            save();
        } else cout << "Resolve it first. Feedback is allowed once.\n";
    }
    void notificationsFor(int userId) {
        bool found = false;
        cout << "\n=== NOTIFICATIONS ===\n";
        for (int i = 0; i < (int)notifications.size(); i++)
            if (notifications[i].userId == userId) {
                cout << (notifications[i].isRead ? "[Read] " : "[New]  ") << notifications[i].message << "\n";
                notifications[i].isRead = true;
                found = true;
            }
        if (!found) cout << "No notifications.\n";
        save();
    }
    void vote(int userId) {
        poll.show();
        int choice = numberInput("Vote (1-3): ", 1, 3);
        cout << (poll.vote(userId, choice) ? "Vote accepted.\n" : "Already voted.\n");
        poll.show();
    }
    void analytics() const {
        int count[6] = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < (int)complaints.size(); i++)
            count[complaints[i]->getStatus()]++;
        cout << "\n=== ANALYTICS ===\nTotal: " << complaints.size();
        for (int i = 0; i < 6; i++)
            cout << "\n" << statusText((Status)i) << ": " << count[i];
        int done = count[RESOLVED] + count[CLOSED];
        int rate = complaints.empty() ? 0 : done * 100 / complaints.size();
        cout << "\nResolution rate: " << rate << "%\n";
    }
    void history() const {
        ifstream file("history.txt");
        string line;
        bool found = false;
        cout << "\n=== HISTORY ===\n";
        while (getline(file, line)) {
            cout << "- " << line << "\n";
            found = true;
        }
        if (!found) cout << "No history.\n";
    }
    void officerHistory(const Officer& officer) const {
        ifstream file("history.txt");
        string line;
        bool found = false;
        cout << "\n=== MY ASSIGNED HISTORY ===\n";
        while (getline(file, line)) {
            for (int i = 0; i < (int)complaints.size(); i++) {
                string prefix = "CC-" + to_string(complaints[i]->getId()) + ":";
                if (line.find(prefix) == 0 && complaints[i]->assignedTo(officer.getId(), officer.getName())) {
                    cout << "- " << line << "\n";
                    found = true;
                    break;
                }
            }
        }
        if (!found) cout << "No assigned history.\n";
    }
};
void citizenDashboard(CivicCareSystem& system, const Citizen& citizen) {
    int choice;
    do {
        cout << "\n=== CITIZEN DASHBOARD ===\n" << "1.Report  2.My complaints  3.Track  4.Support\n"
             << "5.Poll  6.Feedback  7.Notifications  8.Profile  0.Logout\n";
        choice = numberInput("Choose: ", 0, 8);
        if (choice == 1) system.report(citizen);
        else if (choice == 2) system.showMyComplaints(citizen.getId());
        else if (choice == 3) system.track(citizen);
        else if (choice == 4) system.support(citizen);
        else if (choice == 5) system.vote(citizen.getId());
        else if (choice == 6) system.feedback(citizen);
        else if (choice == 7) system.notificationsFor(citizen.getId());
        else if (choice == 8) citizen.showProfile();
    } while (choice != 0);
}
void authorityDashboard(CivicCareSystem& system, const User& user, const UserManager& userManager) {
    int choice;
    do {
        cout << "\n=== " << user.getRole() << " DASHBOARD ===\n" << "1.Submitted  2.Review  3.Set priority  4.Assign\n"
             << "5.Search  6.All complaints  7.Analytics  8.Close\n"
             << "9.History  10.Profile";
        if (user.getRole() == "Admin") cout << "  11.View users";
        cout << "  0.Logout\n";
        int maximum = user.getRole() == "Admin" ? 11 : 10;
        choice = numberInput("Choose: ", 0, maximum);
        if (choice == 1) system.showByStatus(SUBMITTED);
        else if (choice == 2) system.review();
        else if (choice == 3) system.changePriority();
        else if (choice == 4) {
            Officer* officer = userManager.selectOfficer();
            if (officer) system.assign(*officer);
        }
        else if (choice == 5)
            system.findComplaint(input("Category (blank = all): "), input("Location (blank = all): "));
        else if (choice == 6) system.showAll();
        else if (choice == 7) system.analytics();
        else if (choice == 8) system.closeComplaint();
        else if (choice == 9) system.history();
        else if (choice == 10) user.showProfile();
        else if (choice == 11 && user.getRole() == "Admin") userManager.showUsers();
    } while (choice != 0);
}
void officerDashboard(CivicCareSystem& system, const Officer& officer) {
    int choice;
    do {
        cout << "\n=== OFFICER DASHBOARD ===\n" << "1.Assigned complaints  2.Start work  3.Resolve\n" << "4.History  5.Profile  0.Logout\n";
        choice = numberInput("Choose: ", 0, 5);
        if (choice == 1) system.showAssigned(officer);
        else if (choice == 2) system.startWork(officer);
        else if (choice == 3) system.resolve(officer);
        else if (choice == 4) system.officerHistory(officer);
        else if (choice == 5) officer.showProfile();
    } while (choice != 0);
}
int main() {
    UserManager userManager;
    CivicCareSystem system;
    int choice;
    do {
        cout << "\n========================================\n" << "       CIVICCARE BANGLADESH\n" << "========================================\n"
             << "1.Citizen sign up\n2.Government staff sign up\n"
             << "3.Login\n4.Public complaints\n0.Exit\n";
        choice = numberInput("Choose: ", 0, 4);
        if (choice == 1) userManager.citizenSignUp();
        else if (choice == 2) userManager.staffSignUp();
        else if (choice == 3) {
            User* user = userManager.login();
            if (!user) continue;
            if (user->getRole() == "Citizen")
                citizenDashboard(system, *dynamic_cast<Citizen*>(user));
            else if (user->getRole() == "Officer")
                officerDashboard(system, *dynamic_cast<Officer*>(user));
            else
                authorityDashboard(system, *user, userManager);
        } else if (choice == 4) system.showAll();
    } while (choice != 0);
    cout << "Data saved. Goodbye.\n";
    return 0;
}
