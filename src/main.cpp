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

int askNumber(string text, int low, int high) {
    while (true) {
        stringstream input(ask(text));
        int value;
        if (input >> value && value >= low && value <= high) return value;
        cout << "Enter " << low << " to " << high << ".\n";
    }
}

string safe(string text) {
    for (int i = 0; i < (int)text.size(); i++)
        if (text[i] == '|') text[i] = '/';
    return text;
}

vector<string> split(string line) {
    vector<string> data;
    string part;
    stringstream input(line);
    while (getline(input, part, '|')) data.push_back(part);
    return data;
}

enum Status { SUBMITTED, UNDER_REVIEW, ASSIGNED, IN_PROGRESS, RESOLVED, CLOSED };

string statusName(Status status) {
    string names[] = {"Submitted", "Under Review", "Assigned",
                      "In Progress", "Resolved", "Closed"};
    return names[status];
}

// Abstraction: every child must provide its role.
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

// Multilevel inheritance.
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
    string message;

public:
    Notification(int id, string text) : userId(id), message(text) {}
    void show(int id) const {
        if (id == userId) cout << "- " << message << "\n";
    }
};

// Abstraction + encapsulation: data is private, category/risk are virtual.
class Complaint {
private:
    int id, citizenId, supports, rating;
    string title, description, location, department, officer, resolution;
    Status status;

public:
    Complaint(int number, int owner, string heading, string details, string place)
        : id(number), citizenId(owner), supports(0), rating(0), title(heading),
          description(details), location(place), department("Not assigned"),
          officer("Not assigned"), resolution("None"), status(SUBMITTED) {}

    virtual ~Complaint() {}
    virtual string category() const = 0;
    virtual int riskScore() const = 0; // Runtime polymorphism

    int getId() const { return id; }
    int getCitizenId() const { return citizenId; }
    string getLocation() const { return location; }
    Status getStatus() const { return status; }

    string priority() const {
        int score = riskScore() + supports / 3;
        if (score >= 5) return "Critical";
        if (score >= 4) return "High";
        if (score >= 2) return "Medium";
        return "Low";
    }

    void addSupport() { supports++; }

    bool nextStatus() {
        if (status == CLOSED) return false;
        status = (Status)(status + 1);
        return true;
    }

    void assign(string departmentName, string officerName) {
        department = departmentName;
        officer = officerName;
    }

    void resolve(string note) { resolution = note; }

    bool feedback(int stars) {
        if (status < RESOLVED) return false;
        rating = stars;
        return true;
    }

    void restore(Status savedStatus, int savedSupports, string dept,
                 string assignedOfficer, string note, int stars) {
        status = savedStatus;
        supports = savedSupports;
        department = dept;
        officer = assignedOfficer;
        resolution = note;
        rating = stars;
    }

    void display() const {
        cout << "\nComplaint ID : CC-" << id
             << "\nCategory     : " << category()
             << "\nTitle        : " << title
             << "\nLocation     : " << location
             << "\nDescription  : " << description
             << "\nPriority     : " << priority()
             << "\nStatus       : " << statusName(status)
             << "\nSupport      : " << supports
             << "\nDepartment   : " << department
             << "\nOfficer      : " << officer
             << "\nResolution   : " << resolution
             << "\nRating       : " << (rating ? to_string(rating) + "/5" : "Not given")
             << "\n----------------------------------------\n";
    }

    string saveLine() const {
        return to_string(id) + "|" + to_string(citizenId) + "|" + category() +
               "|" + safe(title) + "|" + safe(description) + "|" + safe(location) +
               "|" + to_string(status) + "|" + to_string(supports) + "|" +
               safe(department) + "|" + safe(officer) + "|" + safe(resolution) +
               "|" + to_string(rating);
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

Complaint* makeComplaint(string type, int id, int owner, string title,
                         string details, string location) {
    if (type == "Road") return new RoadComplaint(id, owner, title, details, location);
    if (type == "Waste") return new WasteComplaint(id, owner, title, details, location);
    return new DrainageComplaint(id, owner, title, details, location);
}

class Poll {
private:
    string question;
    vector<string> options;
    vector<int> votes, voters;

public:
    Poll() {
        question = "Which issue needs attention first?";
        options.push_back("Road");
        options.push_back("Waste");
        options.push_back("Drainage");
        votes.assign(3, 0);
    }

    void show() const {
        cout << "\n" << question << "\n";
        for (int i = 0; i < 3; i++)
            cout << i + 1 << ". " << options[i] << " - " << votes[i] << " vote(s)\n";
    }

    bool vote(int citizenId, int option) {
        for (int i = 0; i < (int)voters.size(); i++)
            if (voters[i] == citizenId) return false;
        voters.push_back(citizenId);
        votes[option - 1]++;
        return true;
    }
};

class FileManager {
public:
    static void save(const vector<Complaint*>& complaints, string fileName) {
        ofstream file(fileName.c_str());
        for (int i = 0; i < (int)complaints.size(); i++)
            file << complaints[i]->saveLine() << "\n";
    }

    static int load(vector<Complaint*>& complaints, string fileName) {
        ifstream file(fileName.c_str());
        string line;
        int nextId = 1001;
        while (getline(file, line)) {
            vector<string> d = split(line);
            if (d.size() != 12) continue;
            int id = stoi(d[0]);
            Complaint* complaint = makeComplaint(d[2], id, stoi(d[1]), d[3], d[4], d[5]);
            complaint->restore((Status)stoi(d[6]), stoi(d[7]), d[8], d[9], d[10], stoi(d[11]));
            complaints.push_back(complaint);
            if (id >= nextId) nextId = id + 1;
        }
        return nextId;
    }
};

class CivicCareSystem {
private:
    vector<Complaint*> complaints;
    vector<Notification> notifications;
    Poll poll;
    int nextId;
    string fileName;

    string departmentFor(string category) const {
        if (category == "Road") return "Road & Maintenance";
        if (category == "Waste") return "Waste Management";
        return "Drainage Department";
    }

public:
    CivicCareSystem(string file) : fileName(file) {
        nextId = FileManager::load(complaints, fileName);
    }

    ~CivicCareSystem() {
        FileManager::save(complaints, fileName);
        for (int i = 0; i < (int)complaints.size(); i++) delete complaints[i];
    }

    Complaint* searchComplaint(int id) { // Overloaded search 1
        for (int i = 0; i < (int)complaints.size(); i++)
            if (complaints[i]->getId() == id) return complaints[i];
        return NULL;
    }

    void searchComplaint(string category, string location) { // Overloaded search 2
        bool found = false;
        for (int i = 0; i < (int)complaints.size(); i++) {
            if (complaints[i]->category() == category &&
                complaints[i]->getLocation().find(location) != string::npos) {
                complaints[i]->display();
                found = true;
            }
        }
        if (!found) cout << "No matching complaint.\n";
    }

    void report(const Citizen& citizen) {
        int choice = askNumber("1. Road  2. Waste  3. Drainage: ", 1, 3);
        string types[] = {"", "Road", "Waste", "Drainage"};
        string title = ask("Title: ");
        string description = ask("Description: ");
        string location = ask("Location: ");
        complaints.push_back(makeComplaint(types[choice], nextId, citizen.getId(),
                                            title, description, location));
        notifications.push_back(Notification(citizen.getId(),
                                "CC-" + to_string(nextId) + " was submitted."));
        cout << "Created complaint CC-" << nextId++ << ".\n";
        FileManager::save(complaints, fileName);
    }

    void showAll() const {
        if (complaints.empty()) cout << "No complaints.\n";
        for (int i = 0; i < (int)complaints.size(); i++) complaints[i]->display();
    }

    void advance(const Admin& admin, const Officer& officer) {
        Complaint* complaint = searchComplaint(askNumber("Complaint number: ", 1, 999999));
        if (!complaint) {
            cout << "Complaint not found.\n";
            return;
        }
        Status old = complaint->getStatus();
        if (!complaint->nextStatus()) {
            cout << "Already Closed.\n";
            return;
        }
        if (complaint->getStatus() == ASSIGNED)
            complaint->assign(departmentFor(complaint->category()), officer.getName());
        if (complaint->getStatus() == RESOLVED)
            complaint->resolve(ask("Resolution note: "));
        notifications.push_back(Notification(complaint->getCitizenId(),
                                "Status: " + statusName(complaint->getStatus())));
        cout << admin.getName() << " changed " << statusName(old) << " to "
             << statusName(complaint->getStatus()) << ".\n";
        FileManager::save(complaints, fileName);
    }

    void support() {
        Complaint* complaint = searchComplaint(askNumber("Complaint number: ", 1, 999999));
        if (!complaint) cout << "Complaint not found.\n";
        else {
            complaint->addSupport();
            cout << "Support added.\n";
        }
    }

    void giveFeedback() {
        Complaint* complaint = searchComplaint(askNumber("Complaint number: ", 1, 999999));
        int rating = askNumber("Rating (1-5): ", 1, 5);
        if (!complaint || !complaint->feedback(rating))
            cout << "Resolve the complaint first.\n";
        else cout << "Feedback saved.\n";
    }

    void showNotifications(int userId) const {
        cout << "\nNotifications\n";
        for (int i = 0; i < (int)notifications.size(); i++) notifications[i].show(userId);
    }

    void analytics() const {
        int open = 0, done = 0;
        for (int i = 0; i < (int)complaints.size(); i++)
            complaints[i]->getStatus() >= RESOLVED ? done++ : open++;
        cout << "\nTotal    : " << complaints.size()
             << "\nOpen     : " << open << "\nResolved : " << done << "\n";
    }

    void usePoll(int citizenId) {
        poll.show();
        int option = askNumber("Vote (1-3): ", 1, 3);
        cout << (poll.vote(citizenId, option) ? "Vote accepted.\n" : "Already voted.\n");
        poll.show();
    }
};

void menu() {
    cout << "\n=== CIVICCARE BANGLADESH ===\n"
         << "1. Report complaint\n2. View complaints\n3. Move to next status\n"
         << "4. Support complaint\n5. Search by ID\n6. Search category/location\n"
         << "7. Civic poll\n8. Give feedback\n9. Notifications\n"
         << "10. Analytics\n11. Show user roles\n0. Exit\n";
}

int main() {
    Citizen citizen(1, "Rahim");
    Admin admin(2, "System Admin");
    Officer officer(3, "Local Officer");
    CivicCareSystem system("complaints.txt");
    vector<User*> users = {&citizen, &admin, &officer};

    int choice;
    do {
        menu();
        choice = askNumber("Choose: ", 0, 11);
        if (choice == 1) system.report(citizen);
        else if (choice == 2) system.showAll();
        else if (choice == 3) system.advance(admin, officer);
        else if (choice == 4) system.support();
        else if (choice == 5) {
            Complaint* complaint = system.searchComplaint(askNumber("Complaint number: ", 1, 999999));
            if (complaint) complaint->display();
            else cout << "Complaint not found.\n";
        } else if (choice == 6)
            system.searchComplaint(ask("Category (Road/Waste/Drainage): "), ask("Location: "));
        else if (choice == 7) system.usePoll(citizen.getId());
        else if (choice == 8) system.giveFeedback();
        else if (choice == 9) system.showNotifications(citizen.getId());
        else if (choice == 10) system.analytics();
        else if (choice == 11)
            for (int i = 0; i < (int)users.size(); i++)
                cout << users[i]->role() << ": " << users[i]->getName() << "\n";
    } while (choice != 0);

    cout << "Data saved. Goodbye.\n";
    return 0;
}
