#include <algorithm>
#include <cctype>
#include <cstdlib>
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

// -------------------- Monochrome terminal UI --------------------

const int UI_WIDTH = 72;

string numberText(int value) {
    stringstream output;
    output << value;
    return output.str();
}

void border(char fill = '-') {
    cout << '+' << string(UI_WIDTH - 2, fill) << "+\n";
}

void boxText(string text) {
    const int space = UI_WIDTH - 4;
    if (text.empty()) text = " ";

    while (!text.empty()) {
        int cut = static_cast<int>(text.size());
        if (cut > space) {
            cut = space;
            size_t lastSpace = text.rfind(' ', space);
            if (lastSpace != string::npos && lastSpace > 0)
                cut = static_cast<int>(lastSpace);
        }

        string part = text.substr(0, cut);
        cout << "| " << part << string(space - part.size(), ' ') << " |\n";
        text.erase(0, cut);
        while (!text.empty() && text[0] == ' ') text.erase(0, 1);
    }
}

void field(const string& name, const string& value) {
    boxText(name + value);
}

void clearScreen() {
    if (getenv("SHOHAY_TEST") == NULL) system("cls");
}

void pageHeader(const string& pageName) {
    clearScreen();
    border('=');
    boxText("SHOHAY");
    boxText("Community Help Request Management System");
    border('=');
    boxText(pageName);
    border('-');
}

void section(const string& title) {
    border('-');
    boxText(title);
    border('-');
}

void notice(const string& title, const string& message) {
    border('-');
    boxText(title);
    boxText(message);
    border('-');
}

void pauseScreen() {
    if (getenv("SHOHAY_TEST") != NULL) return;
    cout << "\nPress Enter to return to the dashboard...";
    string line;
    getline(cin, line);
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
        border('=');
        boxText("CASE SH-" + numberText(id) + "  |  " + type() + " REQUEST");
        border('-');
        field("Status         : ", statusName(status));
        field("Priority       : ", priorityName());
        field("Urgency        : ", urgencyName(urgency));
        field("Area           : ", area);
        field("Affected people: ", numberText(people));
        field("Description    : ", description);
        field("Central review : ", centralReview ? "Required" : "Not required");
        if (!claimedBy.empty()) field("Claimed by     : ", claimedBy);
        border('-');
        boxText("BENEFICIARY INFORMATION");

        if (revealPrivate) {
            field("Name           : ", beneficiary);
            field("Phone          : ", phone);
            field("Exact address  : ", address);
        } else {
            field("Name           : ", "[PROTECTED]");
            field("Phone          : ", "[PROTECTED]");
            field("Exact address  : ", "[PROTECTED]");
        }
        border('=');
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

    int countStatus(Status status) const {
        int count = 0;
        for (size_t i = 0; i < requests.size(); i++)
            if (requests[i]->getStatus() == status) count++;
        return count;
    }

public:
    RequestManager(const string& file) : nextId(1001), fileName(file) {
        nextId = FileManager::load(requests, fileName) + 1;
    }

    ~RequestManager() {
        for (size_t i = 0; i < requests.size(); i++) delete requests[i];
    }

    bool save() const { return FileManager::save(requests, fileName); }

    void showDashboard() const {
        int central = 0;
        for (size_t i = 0; i < requests.size(); i++)
            if (requests[i]->needsCentralReview()) central++;

        boxText("REQUEST OVERVIEW");
        border('-');
        boxText("Total: " + numberText(static_cast<int>(requests.size())) +
                "   Pending: " + numberText(countStatus(PENDING)) +
                "   Approved: " + numberText(countStatus(APPROVED)));
        boxText("Claimed: " + numberText(countStatus(CLAIMED)) +
                "   Completed: " + numberText(countStatus(COMPLETED)) +
                "   Rejected: " + numberText(countStatus(REJECTED)));
        boxText("Central review required: " + numberText(central));
        border('=');
    }

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
        if (!found) notice("EMPTY STATE", "No pending requests are waiting for review.");
    }

    void review(int id, bool approved, const Moderator& moderator) {
        HelpRequest* r = find(id);
        if (r == NULL) {
            notice("NOT FOUND", "The requested case does not exist.");
            return;
        }
        bool changed = approved ? r->approve() : r->reject();
        if (!changed) {
            notice("ACTION BLOCKED", "Only Pending requests can be reviewed.");
            return;
        }
        notice("REQUEST UPDATED", moderator.getName() + " changed SH-" +
               numberText(id) + " to " + statusName(r->getStatus()) + ".");
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
        if (!found) notice("EMPTY STATE", "No approved requests are available.");
    }

    void claim(int id, const Organization& organization) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->claim(organization)) {
            notice("ACTION BLOCKED", "Only an Approved request can be claimed.");
            return;
        }
        notice("REQUEST CLAIMED", organization.getName() + " claimed SH-" +
               numberText(id) + ".");
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
        if (!found) notice("EMPTY STATE", "This organization has no claimed requests.");
    }

    void complete(int id, const Organization& organization) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->complete(organization)) {
            notice("ACTION BLOCKED", "Only the claiming organization can complete this case.");
            return;
        }
        notice("ASSISTANCE COMPLETED", "SH-" + numberText(id) + " is now Completed.");
        save();
    }

    void authorize(int id, const Admin& admin) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->authorizePrivateData()) {
            notice("ACTION BLOCKED", "The case must be Claimed before authorization.");
            return;
        }
        notice("ACCESS AUTHORIZED", admin.getName() + " authorized private data for SH-" +
               numberText(id) + ".");
        save();
    }

    void searchById(int id) {
        HelpRequest* r = find(id);
        if (r == NULL || !r->isPublic()) {
            notice("NOT FOUND", "No verified public request matched that case ID.");
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
        if (!found) notice("NOT FOUND", "No verified request matched that area.");
    }

    void showAll() const {
        if (requests.empty()) {
            notice("EMPTY STATE", "No requests are stored in the system.");
            return;
        }
        for (size_t i = 0; i < requests.size(); i++) requests[i]->show(true);
    }
};

// -------------------- Main program --------------------

void menu(const RequestManager& manager) {
    pageHeader("MAIN DASHBOARD");
    manager.showDashboard();

    section("VOLUNTEER");
    boxText("[1] Submit a new help request");

    section("MODERATOR");
    boxText("[2] View pending requests");
    boxText("[3] Approve or reject a request");

    section("ORGANIZATION");
    boxText("[4] View verified requests");
    boxText("[5] Claim an approved request");
    boxText("[6] View organization cases");
    boxText("[7] Mark assistance as completed");

    section("PUBLIC AND ADMIN");
    boxText("[8] Search by case ID or area");
    boxText("[9] Authorize private information");
    boxText("[10] View all internal records");
    boxText("[11] Show system user roles");
    border('=');
    boxText("[0] Save and exit");
    border('=');
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
        menu(manager);
        choice = askInt("Choose: ", 0, 11);

        if (choice == 1) {
            pageHeader("VOLUNTEER / NEW HELP REQUEST");
            section("SELECT REQUEST TYPE");
            boxText("[1] Food support");
            boxText("[2] Medical support");
            boxText("[3] Disaster relief");
            int typeNumber = askInt("Type: ", 1, 3);
            string types[] = {"", "Food", "Medical", "Disaster"};

            section("REQUEST DETAILS");
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
            string message = volunteer.getName() + " submitted SH-" + numberText(id) +
                             " with Pending status.";
            if (data.centralReview) message += " Forwarded to the central moderator.";
            notice("REQUEST CREATED", message);
        } else if (choice == 2) {
            pageHeader("MODERATOR / PENDING REQUESTS");
            manager.showPending();
        } else if (choice == 3) {
            pageHeader("MODERATOR / REVIEW REQUEST");
            manager.showPending();
            section("MODERATOR DECISION");
            int id = askInt("Case ID without SH-: ", 1, 999999999);
            bool approve = askInt("1=Approve, 2=Reject: ", 1, 2) == 1;
            manager.review(id, approve, moderator);
        } else if (choice == 4) {
            pageHeader("VERIFIED HELP REQUESTS");
            manager.showVerified();
        } else if (choice == 5) {
            pageHeader("ORGANIZATION / CLAIM REQUEST");
            manager.showVerified();
            section("CLAIM ACTION");
            manager.claim(askInt("Case ID to claim: ", 1, 999999999), organization);
        } else if (choice == 6) {
            pageHeader("ORGANIZATION / MY CASES");
            manager.showOrganizationCases(organization);
        } else if (choice == 7) {
            pageHeader("ORGANIZATION / COMPLETE ASSISTANCE");
            manager.showOrganizationCases(organization);
            section("COMPLETION ACTION");
            manager.complete(askInt("Case ID to complete: ", 1, 999999999), organization);
        } else if (choice == 8) {
            pageHeader("PUBLIC REQUEST SEARCH");
            section("SEARCH OPTIONS");
            boxText("[1] Search using case ID");
            boxText("[2] Search using approximate area");
            int searchType = askInt("1=Search ID, 2=Search area: ", 1, 2);
            if (searchType == 1) manager.searchById(askInt("Case ID: ", 1, 999999999));
            else manager.searchByArea(ask("Area: "));
        } else if (choice == 9) {
            pageHeader("ADMIN / PRIVACY AUTHORIZATION");
            boxText("Private information can be shared only after an organization claims the case.");
            border('-');
            manager.authorize(askInt("Claimed case ID: ", 1, 999999999), admin);
        } else if (choice == 10) {
            pageHeader("ADMIN / ALL INTERNAL RECORDS");
            manager.showAll();
        } else if (choice == 11) {
            pageHeader("SYSTEM USER ROLES");
            // Polymorphism: correct child role() runs through a User pointer.
            for (size_t i = 0; i < users.size(); i++) {
                border('-');
                field("Role : ", users[i]->role());
                field("User : ", users[i]->getName());
                border('-');
            }
        }

        if (choice != 0) pauseScreen();
    } while (choice != 0);

    pageHeader("SESSION CLOSED");
    if (manager.save()) notice("SAVED", "All records were saved. Goodbye.");
    else notice("SAVE FAILED", "The latest records could not be saved.");
    return 0;
}
