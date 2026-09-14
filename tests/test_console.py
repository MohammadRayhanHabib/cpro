"""Run with: python tests/test_console.py (requires g++).

Every test uses a temporary folder, never the project's saved citizen data.
"""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ConsoleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory(prefix="civiccare-build-")
        cls.exe = pathlib.Path(cls.build.name) / "civiccare.exe"
        subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-pedantic",
                        str(ROOT / "src/main.cpp"), "-o", str(cls.exe)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.build.cleanup()

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="civiccare-test-")
        self.addCleanup(self.temp.cleanup)
        self.folder = pathlib.Path(self.temp.name)

    def run_app(self, lines):
        result = subprocess.run([str(self.exe)], input="\n".join(lines) + "\n",
                                text=True, capture_output=True, cwd=self.folder, timeout=5)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout

    def records(self):
        return (self.folder / "complaints.txt").read_text().splitlines()

    def signup(self):
        return ["1", "1234567890", "test@example.com", "01712345678",
                "Test Citizen", "Dhaka", "test123"]

    def login(self):
        return ["3", "test@example.com", "test123"]

    def test_menu_spacing(self):
        output = self.run_app(["0"])
        self.assertIn("1. Citizen Sign Up\n2. Authority Sign Up\n", output)

    def test_exit_and_eof_in_validation(self):
        for lines in (["1", "invalid", "EXIT"], ["1"], ["3", "admin", "/exit"]):
            self.assertIn("Goodbye", self.run_app(lines))

    def test_back_cancels_signup(self):
        output = self.run_app(["1", "1234567890", "back", "0"])
        self.assertIn("Back to menu", output)
        self.assertEqual(len((self.folder / "users.txt").read_text().splitlines()), 2)

    def test_normal_emergency_and_reload(self):
        output = self.run_app(self.signup() + self.login() +
                              ["1", "1", "Normal title", "Normal details", "Dhaka",
                               "8", "9", "Danger", "Live wire", "Mirpur", "exit"])
        self.assertIn("Emergency priority: Critical", output)
        normal, emergency = [r.split("|") for r in self.records()]
        self.assertEqual(normal[3:8], ["Normal title", "Normal details", "Dhaka", "0", "0"])
        self.assertEqual(emergency[2:8], ["Electricity", "Danger", "Live wire", "Mirpur", "3", "0"])
        output = self.run_app(["4", "0"])
        self.assertIn("Priority     : Critical", output)

    def test_back_and_exit_do_not_submit_partial_complaint(self):
        self.run_app(self.signup() + ["0"])
        for command in ("back", "exit"):
            self.run_app(self.login() + ["8", "1", "Partial title", command, "exit"])
            self.assertEqual(self.records(), [])

    def test_staff_back_returns_to_own_dashboard(self):
        output = self.run_app(["3", "admin", "admin123", "4", "back", "exit"])
        self.assertEqual(output.count("=== Admin DASHBOARD ==="), 2)
        output = self.run_app(["3", "officer", "officer123", "3", "back", "exit"])
        self.assertEqual(output.count("=== OFFICER DASHBOARD ==="), 2)

    def seed_poll(self, count, complaint_id=1001):
        (self.folder / "complaint_votes.txt").write_text("".join(
            f"{i + 100} {complaint_id}\n" for i in range(count)))

    def seed_complaints(self, categories=("Road",), status=0):
        (self.folder / "complaints.txt").write_text("".join(
            f"{1001+i}|3|{category}|Case {1001+i}|Details|Dhaka|0|{status}||Not assigned|0|Not assigned|None|0|None\n"
            for i, category in enumerate(categories)))

    def test_poll_threshold_and_closed_complaint(self):
        for votes, status, expected in [(0, 0, "0"), (49, 0, "0"),
                (50, 0, "0"), (51, 0, "3"), (52, 0, "3"), (51, 5, "0")]:
            with self.subTest(votes=votes, status=status):
                self.seed_complaints(status=status)
                self.seed_poll(votes)
                self.run_app(["0"])
                self.assertEqual(self.records()[0].split("|")[6], expected)

    def test_live_51st_vote_duplicate_and_other_complaint(self):
        self.run_app(self.signup() + ["0"])
        self.seed_complaints(("Road", "Road"))
        self.seed_poll(50)
        output = self.run_app(self.login() +
                              ["4", "1001", "4", "1001", "4", "1002", "exit"])
        self.assertIn("Total votes: 51", output)
        self.assertIn("poll priority changed to Critical", output)
        self.assertIn("Already voted for this complaint", output)
        self.assertEqual([r.split("|")[6] for r in self.records()], ["3", "0"])
        votes = (self.folder / "complaint_votes.txt").read_text().splitlines()
        self.assertEqual(len(votes), 52)
        self.assertEqual(votes.count("3 1001"), 1)
        self.assertIn("3 1002", votes)
        before = (self.folder / "history.txt").read_text()
        notices = (self.folder / "notifications.txt").read_text()
        output = self.run_app(self.login() + ["4", "1001", "exit"])
        self.assertIn("Already voted", output)
        self.assertEqual((self.folder / "history.txt").read_text(), before)
        self.assertEqual((self.folder / "notifications.txt").read_text(), notices)

    def test_all_categories_listed_and_each_complaint_can_receive_votes(self):
        categories = ("Road", "Waste", "Drainage", "Water", "Street Light",
                      "Traffic", "Environment", "Public Health", "Electricity", "Other")
        self.run_app(self.signup() + ["0"])
        self.seed_complaints(categories)
        actions = []
        for i in range(10):
            actions += ["4", str(1001+i)]
        output = self.run_app(self.login() + actions + ["exit"])
        for i, category in enumerate(categories):
            self.assertIn("=== " + category + " ===", output)
            self.assertIn("Case " + str(1001+i), output)
            self.assertIn("Vote accepted for CC-" + str(1001+i), output)
        self.assertEqual(len((self.folder / "complaint_votes.txt").read_text().splitlines()), 10)
        self.assertTrue(all(r.split("|")[6] == "0" for r in self.records()))

    def test_new_complaint_does_not_inherit_other_complaints_votes(self):
        self.run_app(self.signup() + ["0"])
        self.seed_complaints()
        self.seed_poll(51)
        self.run_app(self.login() + ["1", "1", "New road", "Details", "Dhaka", "exit"])
        self.assertEqual([r.split("|")[6] for r in self.records()], ["3", "0"])

    def test_duplicate_and_damaged_saved_votes(self):
        self.seed_complaints()
        self.seed_poll(50)
        with (self.folder / "complaint_votes.txt").open("a") as file:
            file.write("100 1001\n100 1001\ninvalid\n-2 1001\n0 1001\n500 1001 extra\n")
        self.run_app(["0"])
        self.assertEqual(self.records()[0].split("|")[6], "0")
        with (self.folder / "complaint_votes.txt").open("a") as file:
            file.write("999 1001\n")
        self.run_app(["0"])
        self.assertEqual(self.records()[0].split("|")[6], "3")

    def test_old_category_votes_are_preserved_and_not_counted(self):
        self.seed_complaints()
        old = "".join(f"{i+100} 1\n" for i in range(80))
        (self.folder / "votes.txt").write_text(old)
        self.run_app(["0"])
        self.assertEqual(self.records()[0].split("|")[6], "0")
        self.assertEqual((self.folder / "votes.txt").read_text(), old)

    def test_closed_missing_cancelled_and_empty_poll(self):
        self.run_app(self.signup() + ["0"])
        output = self.run_app(self.login() + ["4", "exit"])
        self.assertEqual(output.count("No complaints in this category."), 10)
        self.seed_complaints(status=5)
        output = self.run_app(self.login() +
                              ["4", "1001", "4", "9999", "4", "back", "4", "exit"])
        self.assertIn("Closed complaints cannot receive votes", output)
        self.assertIn("Complaint not found", output)
        self.assertIn("Back to menu", output)
        self.assertFalse((self.folder / "complaint_votes.txt").exists())

    def test_admin_cannot_downgrade_complaint_with_51_votes(self):
        self.seed_complaints()
        self.seed_poll(51)
        output = self.run_app(["3", "admin", "admin123", "3", "1001", "1", "exit"])
        self.assertIn("priority must stay Critical", output)
        self.assertEqual(self.records()[0].split("|")[6], "3")


if __name__ == "__main__":
    unittest.main(verbosity=2)
