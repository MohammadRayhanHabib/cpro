#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// -------------------- Simple input helpers --------------------

string clean(string text) {
    for (size_t i = 0; i < text.size(); i++)
        if (text[i] == '|' || text[i] == '\n' || text[i] == '\r') text[i] = ' ';
    return text;
}

string ask(const string& message) {
    string value;
    do {
        cout << message;
        getline(cin, value);
        value = clean(value);
    } while (value.empty());
    return value;
}

int askInt(const string& message, int low, int high) {
    while (true) {
        cout << message;
        string line;
        getline(cin, line);
        stringstream input(line);
        int value;
        char extra;
        if ((input >> value) && !(input >> extra) && value >= low && value <= high)
            return value;
        cout << "Please enter a number from " << low << " to " << high << ".\n";
    }
}

string lower(string text) {
    for (size_t i = 0; i < text.size(); i++)
        text[i] = static_cast<char>(tolower(static_cast<unsigned char>(text[i])));
    return text;
}

vector<string> split(const string& line) {
    vector<string> parts;
    string part;
    stringstream input(line);
    while (getline(input, part, '|')) parts.push_back(part);
    return parts;
}

int toInt(const string& text) {
    stringstream input(text);
    int value;
    char extra;
    if ((input >> value) && !(input >> extra)) return value;
    return -1;
}

// -------------------- Status and urgency --------------------

enum Status { PENDING, APPROVED, REJECTED, CLAIMED, COMPLETED };
enum Urgency { LOW = 1, MEDIUM = 2, HIGH = 3 };

string statusName(Status status) {
    const string names[] = {"Pending", "Approved", "Rejected", "Claimed", "Completed"};
    return names[status];
}

string urgencyName(int urgency) {
    const string names[] = {"", "Low", "Medium", "High"};
    return names[urgency];
}

// -------------------- User classes --------------------
// Abstraction: User has a pure virtual role() function.

class User {
private:
    string name; // Encapsulation

public:
    User(const string& userName) : name(userName) {}
    virtual ~User() {}
    string getName() const { return name; }
    virtual string role() const = 0;
};

// Inheritance: all roles inherit from User.
class Volunteer : public User {
public:
    Volunteer(const string& name) : User(name) {}
    string role() const override { return "Volunteer"; }
};

class Moderator : public User {
public:
    Moderator(const string& name) : User(name) {}
    string role() const override { return "Moderator"; }
};

class Organization : public User {
public:
    Organization(const string& name) : User(name) {}
    string role() const override { return "Organization"; }
};

class Admin : public User {
public:
    Admin(const string& name) : User(name) {}
    string role() const override { return "Admin"; }
};

struct RequestData {
    int id;
    string area;
    int urgency;
    int people;
    string description;
    string beneficiary;
    string phone;
    string address;
    bool centralReview;
};

class FileManager;

// -------------------- Help request classes --------------------
// Abstraction: HelpRequest has pure virtual type() and priorityScore().

class HelpRequest {
private:
    int id;
    string area;
    int urgency;
    int people;
    string description;
    string beneficiary;
    string phone;
    string address;
    bool centralReview;
    Status status;
    string claimedBy;
    bool privateAccess;

    friend class FileManager;

protected:
    int basePriority() const {
        int peopleBonus = 0;
        if (people > 20) peopleBonus = 2;
        else if (people > 5) peopleBonus = 1;
        return urgency + peopleBonus;
    }

public:
    HelpRequest(const RequestData& data)
        : id(data.id), area(data.area), urgency(data.urgency), people(data.people),
          description(data.description), beneficiary(data.beneficiary), phone(data.phone),
          address(data.address), centralReview(data.centralReview), status(PENDING),
          claimedBy(""), privateAccess(false) {}

    virtual ~HelpRequest() {}
    virtual string type() const = 0;
    virtual int priorityScore() const = 0; // Polymorphism

    int getId() const { return id; }
    string getArea() const { return area; }
    Status getStatus() const { return status; }
    string getClaimedBy() const { return claimedBy; }
    bool needsCentralReview() const { return centralReview; }

    bool isPublic() const {
        return status == APPROVED || status == CLAIMED || status == COMPLETED;
    }

    string priorityName() const {
        int score = priorityScore();
        if (score >= 7) return "Critical";
        if (score >= 5) return "High";
        if (score >= 3) return "Medium";
        return "Low";
    }

    // Encapsulation: status can change only through these methods.
    bool approve() {
        if (status != PENDING) return false;
        status = APPROVED;
        return true;
    }

    bool reject() {
        if (status != PENDING) return false;
        status = REJECTED;
        return true;
    }

    bool claim(const Organization& organization) {
        if (status != APPROVED) return false;
        status = CLAIMED;
        claimedBy = organization.getName();
        return true;
    }

    bool complete(const Organization& organization) {
        if (status != CLAIMED || claimedBy != organization.getName()) return false;
        status = COMPLETED;
        return true;
    }

    bool authorizePrivateData() {
        if (status != CLAIMED && status != COMPLETED) return false;
        privateAccess = true;
        return true;
    }

    void show(bool revealPrivate = false) const {
        cout << "\n----------------------------------------\n";
        cout << "Case ID     : SH-" << id << "\n";
        cout << "Type        : " << type() << "\n";
        cout << "Area        : " << area << "\n";
        cout << "Urgency     : " << urgencyName(urgency) << "\n";
        cout << "People      : " << people << "\n";
        cout << "Priority    : " << priorityName() << "\n";
        cout << "Status      : " << statusName(status) << "\n";
        cout << "Description : " << description << "\n";
        cout << "Central     : " << (centralReview ? "Yes" : "No") << "\n";
        if (!claimedBy.empty()) cout << "Claimed by  : " << claimedBy << "\n";

        if (revealPrivate) {
            cout << "Beneficiary : " << beneficiary << "\n";
            cout << "Phone       : " << phone << "\n";
            cout << "Address     : " << address << "\n";
        } else {
            cout << "Beneficiary : [PROTECTED]\n";
            cout << "Phone       : [PROTECTED]\n";
            cout << "Address     : [PROTECTED]\n";
        }
    }

    void showForOrganization() const { show(privateAccess); }
};

// Inheritance + Polymorphism: every child calculates priority differently.
class FoodRequest : public HelpRequest {
public:
    FoodRequest(const RequestData& data) : HelpRequest(data) {}
    string type() const override { return "Food"; }
    int priorityScore() const override { return basePriority(); }
};

class MedicalRequest : public HelpRequest {
public:
    MedicalRequest(const RequestData& data) : HelpRequest(data) {}
    string type() const override { return "Medical"; }
    int priorityScore() const override { return basePriority() + 2; }
};

class DisasterRequest : public HelpRequest {
public:
    DisasterRequest(const RequestData& data) : HelpRequest(data) {}
    string type() const override { return "Disaster"; }
    int priorityScore() const override { return basePriority() + 3; }
};

HelpRequest* makeRequest(const string& type, const RequestData& data) {
    if (type == "Food") return new FoodRequest(data);
    if (type == "Medical") return new MedicalRequest(data);
    if (type == "Disaster") return new DisasterRequest(data);
    return NULL;
}

// -------------------- File handling --------------------

class FileManager {
public:
    static bool save(const vector<HelpRequest*>& requests, const string& fileName) {
        ofstream file(fileName.c_str());
        if (!file) return false;

        for (size_t i = 0; i < requests.size(); i++) {
            HelpRequest* r = requests[i];
            file << r->id << '|' << r->type() << '|' << clean(r->area) << '|'
                 << r->urgency << '|' << r->people << '|' << clean(r->description) << '|'
                 << r->status << '|' << clean(r->beneficiary) << '|' << clean(r->phone) << '|'
                 << clean(r->address) << '|' << r->centralReview << '|'
                 << clean(r->claimedBy) << '|' << r->privateAccess << '\n';
        }
        file.flush();
        return file.good();
    }

    static int load(vector<HelpRequest*>& requests, const string& fileName) {
        ifstream file(fileName.c_str());
        if (!file) return 1000;

        int highestId = 1000;
        string line;
        while (getline(file, line)) {
            vector<string> f = split(line);
            if (f.size() != 13) continue;

            RequestData data;
            data.id = toInt(f[0]);
            data.area = f[2];
            data.urgency = toInt(f[3]);
            data.people = toInt(f[4]);
            data.description = f[5];
            int savedStatus = toInt(f[6]);
            data.beneficiary = f[7];
            data.phone = f[8];
            data.address = f[9];
            int centralValue = toInt(f[10]);
            data.centralReview = centralValue == 1;
            string organization = f[11];
            int accessValue = toInt(f[12]);
            bool access = accessValue == 1;

            if (data.id <= 0 || data.urgency < 1 || data.urgency > 3 || data.people <= 0 ||
                savedStatus < PENDING || savedStatus > COMPLETED ||
                (centralValue != 0 && centralValue != 1) ||
                (accessValue != 0 && accessValue != 1)) continue;

            bool duplicate = false;
            for (size_t i = 0; i < requests.size(); i++)
                if (requests[i]->getId() == data.id) duplicate = true;
            if (duplicate) continue;

            // Reject impossible saved states so privacy cannot be bypassed.
            if (savedStatus <= REJECTED && (!organization.empty() || access)) continue;
            if (savedStatus >= CLAIMED && organization.empty()) continue;

            HelpRequest* request = makeRequest(f[1], data);
            if (request == NULL) continue;
            request->status = static_cast<Status>(savedStatus);
            request->claimedBy = organization;
            request->privateAccess = access;
            requests.push_back(request);
            highestId = max(highestId, data.id);
        }
        return highestId;
    }
};

// -------------------- Request manager --------------------

class RequestManager {
private:
    vector<HelpRequest*> requests;
    int nextId;
    string fileName;

    HelpRequest* find(int id) {
        for (size_t i = 0; i < requests.size(); i++)
            if (requests[i]->getId() == id) return requests[i];
        return NULL;
    }

public:
    RequestManager(const string& file) : nextId(1001), fileName(file) {
        nextId = FileManager::load(requests, fileName) + 1;
        cout << requests.size() << " saved request(s) loaded.\n";
    }

    ~RequestManager() {
        for (size_t i = 0; i < requests.size(); i++) delete requests[i];
    }

    bool save() const { return FileManager::save(requests, fileName); }

    int submit(const string& type, RequestData data) {
        data.id = nextId++;
        requests.push_back(makeRequest(type, data));
        save();
        return data.id;
    }

    void showPending() const {
        bool found = false;
        for (size_t i = 0; i < requests.size(); i++) {
            if (requests[i]->getStatus() == PENDING) {
                requests[i]->show(true); // Moderator internal view
                found = true;
            }
        }
        if (!found) cout << "No pending requests.\n";
    }

    void review(int id, bool approved, const Moderator& moderator) {
        HelpRequest* r = find(id);
        if (r == NULL) {
            cout << "Case not found.\n";
            return;
        }
        bool changed = approved ? r->approve() : r->reject();
        if (!changed) {
            cout << "Only Pending requests can be reviewed.\n";
            return;
        }
        cout << moderator.getName() << " changed SH-" << id << " to "
             << statusName(r->getStatus()) << ".\n";
        save();
    }

    void showVerified() const {
        bool found = false;
        for (size_t i = 0; i < requests.size(); i++) {
            if (requests[i]->getStatus() == APPROVED) {
                requests[i]->show();
                found = true;
            }
        }
        if (!found) cout << "No approved requests.\n";
    }

    void claim(int id, const Organization& organization) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->claim(organization)) {
            cout << "Only an Approved request can be claimed.\n";
            return;
        }
        cout << organization.getName() << " claimed SH-" << id << ".\n";
        save();
    }

    void showOrganizationCases(const Organization& organization) const {
        bool found = false;
        for (size_t i = 0; i < requests.size(); i++) {
            if (requests[i]->getClaimedBy() == organization.getName()) {
                requests[i]->showForOrganization();
                found = true;
            }
        }
        if (!found) cout << "This organization has no claimed requests.\n";
    }

    void complete(int id, const Organization& organization) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->complete(organization)) {
            cout << "Only the claiming organization can complete this case.\n";
            return;
        }
        cout << "SH-" << id << " is now Completed.\n";
        save();
    }

    void authorize(int id, const Admin& admin) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->authorizePrivateData()) {
            cout << "The case must be Claimed before authorization.\n";
            return;
        }
        cout << admin.getName() << " authorized private data for SH-" << id << ".\n";
        save();
    }

    void searchById(int id) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->isPublic()) {
            cout << "No verified public request found.\n";
            return;
        }
        r->show();
    }

    void searchByArea(const string& area) const {
        bool found = false;
        string key = lower(area);
        for (size_t i = 0; i < requests.size(); i++) {
            if (requests[i]->isPublic() && lower(requests[i]->getArea()).find(key) != string::npos) {
                requests[i]->show();
                found = true;
            }
        }
        if (!found) cout << "No verified request found in that area.\n";
    }

    void showAll() const {
        if (requests.empty()) {
            cout << "No requests.\n";
            return;
        }
        for (size_t i = 0; i < requests.size(); i++) requests[i]->show(true);
    }
};

// -------------------- Main program --------------------

void menu() {
    cout << "\n=== SHOHAY ===\n"
         << "1. Submit request\n"
         << "2. View pending\n"
         << "3. Approve / reject\n"
         << "4. View verified requests\n"
         << "5. Claim request\n"
         << "6. View organization cases\n"
         << "7. Complete request\n"
         << "8. Search by ID / area\n"
         << "9. Authorize private data\n"
         << "10. Admin view all\n"
         << "11. Show user roles\n"
         << "0. Save and exit\n";
}

int main() {
    Volunteer volunteer("Local Volunteer");
    Moderator moderator("Central Moderator");
    Organization organization("Shohay Foundation");
    Admin admin("System Admin");

    vector<User*> users;
    users.push_back(&volunteer);
    users.push_back(&moderator);
    users.push_back(&organization);
    users.push_back(&admin);

    RequestManager manager("help_requests.txt");
    int choice;

    do {
        menu();
        choice = askInt("Choose: ", 0, 11);

        if (choice == 1) {
            cout << "1. Food  2. Medical  3. Disaster\n";
            int typeNumber = askInt("Type: ", 1, 3);
            string types[] = {"", "Food", "Medical", "Disaster"};

            RequestData data;
            data.area = ask("Approximate area: ");
            data.urgency = askInt("Urgency (1=Low, 2=Medium, 3=High): ", 1, 3);
            data.people = askInt("Affected people: ", 1, 1000000);
            data.description = ask("Short description: ");
            data.beneficiary = ask("Beneficiary name: ");
            data.phone = ask("Phone: ");
            data.address = ask("Exact address: ");
            data.centralReview = askInt("Local volunteer available? (1=Yes, 2=No): ", 1, 2) == 2;

            int id = manager.submit(types[typeNumber], data);
            cout << volunteer.getName() << " submitted SH-" << id << " with Pending status.\n";
            if (data.centralReview) cout << "Forwarded to the central moderator.\n";
        } else if (choice == 2) {
            manager.showPending();
        } else if (choice == 3) {
            manager.showPending();
            int id = askInt("Case ID without SH-: ", 1, 999999999);
            bool approve = askInt("1=Approve, 2=Reject: ", 1, 2) == 1;
            manager.review(id, approve, moderator);
        } else if (choice == 4) {
            manager.showVerified();
        } else if (choice == 5) {
            manager.showVerified();
            manager.claim(askInt("Case ID to claim: ", 1, 999999999), organization);
        } else if (choice == 6) {
            manager.showOrganizationCases(organization);
        } else if (choice == 7) {
            manager.showOrganizationCases(organization);
            manager.complete(askInt("Case ID to complete: ", 1, 999999999), organization);
        } else if (choice == 8) {
            int searchType = askInt("1=Search ID, 2=Search area: ", 1, 2);
            if (searchType == 1) manager.searchById(askInt("Case ID: ", 1, 999999999));
            else manager.searchByArea(ask("Area: "));
        } else if (choice == 9) {
            manager.authorize(askInt("Claimed case ID: ", 1, 999999999), admin);
        } else if (choice == 10) {
            manager.showAll();
        } else if (choice == 11) {
            // Polymorphism: correct child role() runs through a User pointer.
            for (size_t i = 0; i < users.size(); i++)
                cout << users[i]->role() << ": " << users[i]->getName() << "\n";
        }
    } while (choice != 0);

    if (manager.save()) cout << "Saved. Goodbye.\n";
    else cout << "Save failed.\n";
    return 0;
}
