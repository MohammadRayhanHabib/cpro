#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

string ask(string text) {
    string value;
    cout << text;
    getline(cin, value);
    return value;
}

string askRequired(string text) {
    while (true) {
        string value = ask(text);
        if (!value.empty()) return value;
        cout << "This field cannot be empty.\n";
    }
}

int askNumber(string text, int low, int high) {
    while (true) {
        stringstream input(ask(text));
        int value;
        char extra;
        if (input >> value && !(input >> extra) && value >= low && value <= high)
            return value;
        cout << "Enter a number from " << low << " to " << high << ".\n";
    }
}

string safe(string text) {
    for (int i = 0; i < (int)text.size(); i++)
        if (text[i] == '|') text[i] = '/';
    return text;
}

vector<string> split(string line, char separator = '|') {
    vector<string> parts;
    string part;
    stringstream input(line);
    while (getline(input, part, separator)) parts.push_back(part);
    return parts;
}

string lowerText(string text) {
    transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

enum Status { SUBMITTED, UNDER_REVIEW, ASSIGNED, IN_PROGRESS, RESOLVED, CLOSED };
enum Priority { LOW, MEDIUM, HIGH, CRITICAL };

string statusName(Status value) {
    string names[] = {"Submitted", "Under Review", "Assigned",
                      "In Progress", "Resolved", "Closed"};
    return names[value];
}

string priorityName(Priority value) {
    string names[] = {"Low", "Medium", "High", "Critical"};
    return names[value];
}

// Abstraction: every user type must provide its role.
class User {
private:
    int id;
    string name;

public:
    User(int userId, string userName) : id(userId), name(userName) {}
    virtual ~User() {}
    int getId() const { return id; }
    string getName() const { return name; }
    virtual string role() const = 0;
};

class Citizen : public User {
public:
    Citizen(int id, string name) : User(id, name) {}
    string role() const { return "Citizen"; }
};

class Authority : public User {
public:
    Authority(int id, string name) : User(id, name) {}
    string role() const { return "Authority"; }
};

class Admin : public Authority {
public:
    Admin(int id, string name) : Authority(id, name) {}
    string role() const { return "Admin"; }
};

class Officer : public Authority {
public:
    Officer(int id, string name) : Authority(id, name) {}
    string role() const { return "Officer"; }
};

class Notification {
private:
    int userId;
    bool read;
    string message;

public:
    Notification(int id, string text, bool seen = false)
        : userId(id), read(seen), message(text) {}
    int getUserId() const { return userId; }
    bool isRead() const { return read; }
    string getMessage() const { return message; }
    void markRead() { read = true; }
    string saveLine() const {
        return to_string(userId) + "|" + to_string(read) + "|" + safe(message);
    }
};

// Encapsulation: complaint data can only change through validated methods.
class Complaint {
private:
    int id, citizenId, rating;
    string title, description, location, department, officer;
    string resolution, feedbackComment;
    vector<int> supporters;
    Status status;
    Priority priority;

public:
    Complaint(int number, int owner, string heading, string details, string place)
        : id(number), citizenId(owner), rating(0), title(heading),
          description(details), location(place), department("Not assigned"),
          officer("Not assigned"), resolution("None"), feedbackComment("None"),
          status(SUBMITTED), priority(LOW) {}

    virtual ~Complaint() {}
    virtual string category() const = 0;
    virtual int riskScore() const = 0; // Runtime polymorphism

    int getId() const { return id; }
    int getCitizenId() const { return citizenId; }
    string getLocation() const { return location; }
    string getOfficer() const { return officer; }
    Status getStatus() const { return status; }

    void setAutomaticPriority() {
        int score = riskScore();
        priority = score >= 4 ? CRITICAL : score == 3 ? HIGH : score == 2 ? MEDIUM : LOW;
    }
    void setPriority(Priority value) { priority = value; }

    bool changeStatus(Status expected, Status next) {
        if (status != expected) return false;
        status = next;
        return true;
    }

    bool assign(string departmentName, string officerName) {
        if (!changeStatus(UNDER_REVIEW, ASSIGNED)) return false;
        department = departmentName;
        officer = officerName;
        return true;
    }

    bool resolve(string note) {
        if (!changeStatus(IN_PROGRESS, RESOLVED)) return false;
        resolution = note;
        return true;
    }

    bool addSupport(int userId) {
        if (status == CLOSED) return false;
        for (int i = 0; i < (int)supporters.size(); i++)
            if (supporters[i] == userId) return false;
        supporters.push_back(userId);
        return true;
    }

    bool addFeedback(int stars, string comment) {
        if (status < RESOLVED || rating != 0) return false;
        rating = stars;
        feedbackComment = comment;
        return true;
    }

    void restore(Priority savedPriority, Status savedStatus, vector<int> savedSupporters,
                 string dept, string assignedOfficer, string note,
                 int stars, string comment) {
        priority = savedPriority;
        status = savedStatus;
        supporters = savedSupporters;
        department = dept;
        officer = assignedOfficer;
        resolution = note;
        rating = stars;
        feedbackComment = comment;
    }

    void display() const {
        cout << "\n----------------------------------------"
             << "\nComplaint ID : CC-" << id
             << "\nCategory     : " << category()
             << "\nTitle        : " << title
             << "\nDescription  : " << description
             << "\nLocation     : " << location
             << "\nPriority     : " << priorityName(priority)
             << "\nStatus       : " << statusName(status)
             << "\nSupports     : " << supporters.size()
             << "\nDepartment   : " << department
             << "\nOfficer      : " << officer
             << "\nResolution   : " << resolution
             << "\nRating       : " << (rating ? to_string(rating) + "/5" : "Not given")
             << "\nComment      : " << feedbackComment << "\n";
    }

    string saveLine() const {
        string supporterIds;
        for (int i = 0; i < (int)supporters.size(); i++) {
            if (i) supporterIds += ',';
            supporterIds += to_string(supporters[i]);
        }
        return to_string(id) + "|" + to_string(citizenId) + "|" + category() +
               "|" + safe(title) + "|" + safe(description) + "|" + safe(location) +
               "|" + to_string(priority) + "|" + to_string(status) + "|" +
               supporterIds + "|" + safe(department) + "|" + safe(officer) +
               "|" + safe(resolution) + "|" + to_string(rating) + "|" +
               safe(feedbackComment);
    }
};

class RoadComplaint : public Complaint {
public:
    RoadComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string category() const { return "Road"; }
    int riskScore() const { return 2; }
};

class WasteComplaint : public Complaint {
public:
    WasteComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string category() const { return "Waste"; }
    int riskScore() const { return 1; }
};

class DrainageComplaint : public Complaint {
public:
    DrainageComplaint(int id, int owner, string title, string details, string location)
        : Complaint(id, owner, title, details, location) {}
    string category() const { return "Drainage"; }
    int riskScore() const { return 3; }
};

class GeneralComplaint : public Complaint {
private:
    string type;
    int risk;

public:
    GeneralComplaint(int id, int owner, string title, string details, string location,
                     string complaintType, int score)
        : Complaint(id, owner, title, details, location), type(complaintType), risk(score) {}
    string category() const { return type; }
    int riskScore() const { return risk; }
};

Complaint* makeComplaint(string type, int id, int owner, string title,
                         string details, string location) {
    if (type == "Road") return new RoadComplaint(id, owner, title, details, location);
    if (type == "Waste") return new WasteComplaint(id, owner, title, details, location);
    if (type == "Drainage") return new DrainageComplaint(id, owner, title, details, location);
    int risk = (type == "Electricity" || type == "Public Health") ? 3 : 2;
    return new GeneralComplaint(id, owner, title, details, location, type, risk);
}

class Poll {
private:
    vector<string> options;
    vector<int> voters, choices;

public:
    Poll() {
        options.push_back("Road");
        options.push_back("Waste");
        options.push_back("Drainage");
    }

    void load(string fileName) {
        ifstream file(fileName.c_str());
        int userId, option;
        while (file >> userId >> option)
            if (option >= 1 && option <= 3) {
                voters.push_back(userId);
                choices.push_back(option);
            }
    }

    void save(string fileName) const {
        ofstream file(fileName.c_str());
        for (int i = 0; i < (int)voters.size(); i++)
            file << voters[i] << " " << choices[i] << "\n";
    }

    void show() const {
        int votes[] = {0, 0, 0};
        for (int i = 0; i < (int)choices.size(); i++) votes[choices[i] - 1]++;
        cout << "\nWhich public issue needs attention first?\n";
        for (int i = 0; i < 3; i++) {
            int percent = choices.empty() ? 0 : votes[i] * 100 / choices.size();
            cout << i + 1 << ". " << options[i] << " - " << votes[i]
                 << " vote(s), " << percent << "%\n";
        }
    }

    bool vote(int citizenId, int option) {
        for (int i = 0; i < (int)voters.size(); i++)
            if (voters[i] == citizenId) return false;
        voters.push_back(citizenId);
        choices.push_back(option);
        return true;
    }
};

class FileManager {
public:
    static void saveComplaints(const vector<Complaint*>& data, string fileName) {
        ofstream file(fileName.c_str());
        for (int i = 0; i < (int)data.size(); i++) file << data[i]->saveLine() << "\n";
    }

    static int loadComplaints(vector<Complaint*>& data, string fileName) {
        ifstream file(fileName.c_str());
        string line;
        int nextId = 1001;
        while (getline(file, line)) {
            vector<string> d = split(line);
            try {
                Complaint* item = NULL;
                if (d.size() == 12) { // Also supports the old saved format.
                    item = makeComplaint(d[2], stoi(d[0]), stoi(d[1]), d[3], d[4], d[5]);
                    vector<int> oldSupports;
                    for (int i = 0; i < stoi(d[7]); i++) oldSupports.push_back(-i - 1);
                    item->restore(LOW, (Status)stoi(d[6]), oldSupports,
                                  d[8], d[9], d[10], stoi(d[11]), "None");
                } else if (d.size() == 14) {
                    item = makeComplaint(d[2], stoi(d[0]), stoi(d[1]), d[3], d[4], d[5]);
                    vector<int> supporters;
                    vector<string> ids = split(d[8], ',');
                    for (int i = 0; i < (int)ids.size(); i++)
                        if (!ids[i].empty()) supporters.push_back(stoi(ids[i]));
                    item->restore((Priority)stoi(d[6]), (Status)stoi(d[7]), supporters,
                                  d[9], d[10], d[11], stoi(d[12]), d[13]);
                }
                if (item) {
                    data.push_back(item);
                    nextId = max(nextId, item->getId() + 1);
                }
            } catch (...) {
                cout << "Warning: one damaged complaint record was skipped.\n";
            }
        }
        return nextId;
    }

    static void saveNotifications(const vector<Notification>& data, string fileName) {
        ofstream file(fileName.c_str());
        for (int i = 0; i < (int)data.size(); i++) file << data[i].saveLine() << "\n";
    }

    static void loadNotifications(vector<Notification>& data, string fileName) {
        ifstream file(fileName.c_str());
        string line;
        while (getline(file, line)) {
            vector<string> d = split(line);
            try {
                if (d.size() == 3)
                    data.push_back(Notification(stoi(d[0]), d[2], stoi(d[1])));
            } catch (...) {}
        }
    }

    static void addHistory(string fileName, string text) {
        ofstream file(fileName.c_str(), ios::app);
        file << safe(text) << "\n";
    }
};

class CivicCareSystem {
private:
    vector<Complaint*> complaints;
    vector<Notification> notifications;
    Poll poll;
    int nextId;
    string complaintFile, notificationFile, historyFile, voteFile;

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

    void save() {
        FileManager::saveComplaints(complaints, complaintFile);
        FileManager::saveNotifications(notifications, notificationFile);
        poll.save(voteFile);
    }

    void notify(int userId, string text) {
        notifications.push_back(Notification(userId, text));
    }

    void history(string text) { FileManager::addHistory(historyFile, text); }

    Complaint* chooseComplaint() {
        Complaint* item = searchComplaint(
            askNumber("Complaint number (without CC-): ", 1, 999999));
        if (!item) cout << "Complaint not found.\n";
        return item;
    }

public:
    CivicCareSystem()
        : complaintFile("complaints.txt"), notificationFile("notifications.txt"),
          historyFile("history.txt"), voteFile("votes.txt") {
        nextId = FileManager::loadComplaints(complaints, complaintFile);
        FileManager::loadNotifications(notifications, notificationFile);
        poll.load(voteFile);
    }

    ~CivicCareSystem() {
        save();
        for (int i = 0; i < (int)complaints.size(); i++) delete complaints[i];
    }

    Complaint* searchComplaint(int id) { // Function overloading 1
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getId() == id) return complaints[i];
        return NULL;
    }

    void searchComplaint(string category, string location) { // Function overloading 2
        bool found = false;
        category = lowerText(category);
        location = lowerText(location);
        for (int i = 0; i < (int)complaints.size(); i++) {
            if (lowerText(complaints[i]->category()).find(category) != string::npos &&
                lowerText(complaints[i]->getLocation()).find(location) != string::npos) {
                complaints[i]->display();
                found = true;
            }
        }
        if (!found) cout << "No matching complaint.\n";
    }

    void report(const Citizen& citizen) {
        string categories[] = {"", "Road", "Waste", "Drainage", "Water",
                               "Street Light", "Traffic", "Environment",
                               "Public Health", "Electricity", "Other"};
        cout << "\n1. Road  2. Waste  3. Drainage  4. Water  5. Street Light\n"
             << "6. Traffic  7. Environment  8. Public Health  9. Electricity  10. Other\n";
        int type = askNumber("Category: ", 1, 10);
        Complaint* item = makeComplaint(categories[type], nextId, citizen.getId(),
                                        askRequired("Title: "),
                                        askRequired("Description: "),
                                        askRequired("Location: "));
        item->setAutomaticPriority();
        complaints.push_back(item);
        notify(citizen.getId(), "CC-" + to_string(nextId) + " was submitted.");
        history("CC-" + to_string(nextId) + ": complaint submitted");
        cout << "Complaint created successfully. Your ID is CC-" << nextId++ << ".\n";
        save();
    }

    void showAll() const {
        if (complaints.empty()) cout << "No complaints found.\n";
        for (int i = 0; i < (int)complaints.size(); i++) complaints[i]->display();
    }

    void showMyComplaints(int citizenId) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getCitizenId() == citizenId) {
                complaints[i]->display();
                found = true;
            }
        if (!found) cout << "You have no complaints.\n";
    }

    void showStatus(Status wanted) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getStatus() == wanted) {
                complaints[i]->display();
                found = true;
            }
        if (!found) cout << "No " << statusName(wanted) << " complaints.\n";
    }

    void showAssigned(const Officer& officer) const {
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getOfficer() == officer.getName() &&
                complaints[i]->getStatus() >= ASSIGNED &&
                complaints[i]->getStatus() <= RESOLVED) {
                complaints[i]->display();
                found = true;
            }
        if (!found) cout << "No complaints assigned to you.\n";
    }

    void track(int citizenId) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (item->getCitizenId() != citizenId) cout << "This is not your complaint.\n";
        else item->display();
    }

    void review(const Admin& admin) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (!item->changeStatus(SUBMITTED, UNDER_REVIEW)) {
            cout << "Only Submitted complaints can be reviewed.\n";
            return;
        }
        notify(item->getCitizenId(), "Your complaint is under review.");
        history("CC-" + to_string(item->getId()) + ": reviewed by " + admin.getName());
        cout << "Complaint moved to Under Review.\n";
        save();
    }

    void changePriority() {
        Complaint* item = chooseComplaint();
        if (!item) return;
        int value = askNumber("1. Low  2. Medium  3. High  4. Critical: ", 1, 4);
        item->setPriority((Priority)(value - 1));
        history("CC-" + to_string(item->getId()) + ": priority changed to " +
                priorityName((Priority)(value - 1)));
        cout << "Priority updated.\n";
        save();
    }

    void assign(const Officer& officer) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        string department = departmentFor(item->category());
        if (!item->assign(department, officer.getName())) {
            cout << "Review the complaint before assignment.\n";
            return;
        }
        notify(item->getCitizenId(), "Assigned to " + department + ".");
        history("CC-" + to_string(item->getId()) + ": assigned to " + officer.getName());
        cout << "Assigned to " << department << " and " << officer.getName() << ".\n";
        save();
    }

    void startWork(const Officer& officer) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (item->getOfficer() != officer.getName()) {
            cout << "This complaint is not assigned to you.\n";
            return;
        }
        if (!item->changeStatus(ASSIGNED, IN_PROGRESS)) {
            cout << "Only Assigned complaints can be started.\n";
            return;
        }
        notify(item->getCitizenId(), "Work on your complaint has started.");
        history("CC-" + to_string(item->getId()) + ": work started");
        cout << "Complaint is now In Progress.\n";
        save();
    }

    void resolve(const Officer& officer) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (item->getOfficer() != officer.getName()) {
            cout << "This complaint is not assigned to you.\n";
            return;
        }
        string note = askRequired("Resolution note: ");
        if (!item->resolve(note)) {
            cout << "Start the work before resolving it.\n";
            return;
        }
        notify(item->getCitizenId(), "Your complaint has been resolved.");
        history("CC-" + to_string(item->getId()) + ": resolved - " + note);
        cout << "Complaint resolved successfully.\n";
        save();
    }

    void closeComplaint() {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (!item->changeStatus(RESOLVED, CLOSED)) {
            cout << "Only Resolved complaints can be closed.\n";
            return;
        }
        notify(item->getCitizenId(), "Your complaint has been closed.");
        history("CC-" + to_string(item->getId()) + ": closed");
        cout << "Complaint closed.\n";
        save();
    }

    void support(const Citizen& citizen) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (item->addSupport(citizen.getId())) {
            cout << "Support added.\n";
            save();
        } else cout << "You already supported it, or it is closed.\n";
    }

    void feedback(const Citizen& citizen) {
        Complaint* item = chooseComplaint();
        if (!item) return;
        if (item->getCitizenId() != citizen.getId()) {
            cout << "You can only review your own complaint.\n";
            return;
        }
        int stars = askNumber("Rating (1-5): ", 1, 5);
        string comment = askRequired("Comment: ");
        if (item->addFeedback(stars, comment)) {
            history("CC-" + to_string(item->getId()) + ": citizen feedback added");
            cout << "Feedback saved.\n";
            save();
        } else cout << "Feedback needs a resolved complaint and can be given once.\n";
    }

    void showNotifications(int userId) {
        bool found = false;
        cout << "\n=== NOTIFICATIONS ===\n";
        for (int i = 0; i < (int)notifications.size(); i++)
            if (notifications[i].getUserId() == userId) {
                cout << (notifications[i].isRead() ? "[Read] " : "[New]  ")
                     << notifications[i].getMessage() << "\n";
                notifications[i].markRead();
                found = true;
            }
        if (!found) cout << "No notifications.\n";
        save();
    }

    void usePoll(int citizenId) {
        poll.show();
        int option = askNumber("Vote (1-3): ", 1, 3);
        cout << (poll.vote(citizenId, option) ? "Vote accepted.\n" : "You already voted.\n");
        poll.save(voteFile);
        poll.show();
    }

    void analytics() const {
        int count[6] = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < (int)complaints.size(); i++)
            count[complaints[i]->getStatus()]++;
        cout << "\n=== ANALYTICS ===\nTotal         : " << complaints.size();
        for (int i = 0; i < 6; i++)
            cout << "\n" << statusName((Status)i) << " : " << count[i];
        int finished = count[RESOLVED] + count[CLOSED];
        int rate = complaints.empty() ? 0 : finished * 100 / complaints.size();
        cout << "\nResolution rate: " << rate << "%\n";
    }

    void showHistory() const {
        ifstream file(historyFile.c_str());
        string line;
        bool found = false;
        cout << "\n=== COMPLAINT HISTORY ===\n";
        while (getline(file, line)) {
            cout << "- " << line << "\n";
            found = true;
        }
        if (!found) cout << "No history yet.\n";
    }
};

void citizenMenu(CivicCareSystem& system, const Citizen& citizen) {
    int choice;
    do {
        cout << "\n=== CITIZEN DASHBOARD ===\n"
             << "1. Report complaint\n2. My complaints\n3. Track complaint\n"
             << "4. Support complaint\n5. Civic poll\n6. Give feedback\n"
             << "7. Notifications\n0. Back\n";
        choice = askNumber("Choose: ", 0, 7);
        if (choice == 1) system.report(citizen);
        else if (choice == 2) system.showMyComplaints(citizen.getId());
        else if (choice == 3) system.track(citizen.getId());
        else if (choice == 4) system.support(citizen);
        else if (choice == 5) system.usePoll(citizen.getId());
        else if (choice == 6) system.feedback(citizen);
        else if (choice == 7) system.showNotifications(citizen.getId());
    } while (choice != 0);
}

void authorityMenu(CivicCareSystem& system, const Admin& admin, const Officer& officer) {
    int choice;
    do {
        cout << "\n=== AUTHORITY / ADMIN DASHBOARD ===\n"
             << "1. View submitted\n2. Review complaint\n3. Set priority\n"
             << "4. Assign department and officer\n5. Search category/location\n"
             << "6. View all complaints\n7. Analytics\n8. Close resolved complaint\n"
             << "9. View history\n0. Back\n";
        choice = askNumber("Choose: ", 0, 9);
        if (choice == 1) system.showStatus(SUBMITTED);
        else if (choice == 2) system.review(admin);
        else if (choice == 3) system.changePriority();
        else if (choice == 4) system.assign(officer);
        else if (choice == 5)
            system.searchComplaint(ask("Category (blank = all): "),
                                   ask("Location (blank = all): "));
        else if (choice == 6) system.showAll();
        else if (choice == 7) system.analytics();
        else if (choice == 8) system.closeComplaint();
        else if (choice == 9) system.showHistory();
    } while (choice != 0);
}

void officerMenu(CivicCareSystem& system, const Officer& officer) {
    int choice;
    do {
        cout << "\n=== OFFICER DASHBOARD ===\n"
             << "1. My assigned complaints\n2. Start work\n3. Resolve complaint\n"
             << "4. View history\n0. Back\n";
        choice = askNumber("Choose: ", 0, 4);
        if (choice == 1) system.showAssigned(officer);
        else if (choice == 2) system.startWork(officer);
        else if (choice == 3) system.resolve(officer);
        else if (choice == 4) system.showHistory();
    } while (choice != 0);
}

int main() {
    Citizen citizen(1, "Rahim");
    Admin admin(2, "System Admin");
    Officer officer(3, "Local Officer");
    CivicCareSystem system;
    vector<User*> users;
    users.push_back(&citizen);
    users.push_back(&admin);
    users.push_back(&officer);

    int choice;
    do {
        cout << "\n========================================\n"
             << "       CIVICCARE BANGLADESH\n"
             << "========================================\n"
             << "1. Citizen dashboard\n2. Authority / Admin dashboard\n"
             << "3. Officer dashboard\n4. Public complaint list\n"
             << "5. Show OOP user roles\n0. Save and exit\n";
        choice = askNumber("Choose: ", 0, 5);
        if (choice == 1) citizenMenu(system, citizen);
        else if (choice == 2) authorityMenu(system, admin, officer);
        else if (choice == 3) officerMenu(system, officer);
        else if (choice == 4) system.showAll();
        else if (choice == 5)
            for (int i = 0; i < (int)users.size(); i++)
                cout << users[i]->role() << ": " << users[i]->getName() << "\n";
    } while (choice != 0);

    cout << "Data saved. Goodbye.\n";
    return 0;
}
