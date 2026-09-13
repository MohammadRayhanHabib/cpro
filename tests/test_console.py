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
        self.assertIn("1. Citizen sign up\n\n2. Government", output)

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
                               "9", "9", "Danger", "Live wire", "Mirpur", "exit"])
        self.assertIn("Emergency priority: Critical", output)
        normal, emergency = [r.split("|") for r in self.records()]
        self.assertEqual(normal[3:8], ["Normal title", "Normal details", "Dhaka", "0", "0"])
        self.assertEqual(emergency[2:8], ["Electricity", "Danger", "Live wire", "Mirpur", "3", "0"])
        output = self.run_app(["4", "0"])
        self.assertIn("Priority     : Critical", output)

    def test_back_and_exit_do_not_submit_partial_complaint(self):
        self.run_app(self.signup() + ["0"])
        for command in ("back", "exit"):
            self.run_app(self.login() + ["9", "1", "Partial title", command, "exit"])
            self.assertEqual(self.records(), [])

    def test_staff_back_returns_to_own_dashboard(self):
        output = self.run_app(["3", "admin", "admin123", "4", "back", "exit"])
        self.assertEqual(output.count("=== Admin DASHBOARD ==="), 2)
        output = self.run_app(["3", "officer", "officer123", "3", "back", "exit"])
        self.assertEqual(output.count("=== OFFICER DASHBOARD ==="), 2)

    def seed_poll(self, road, total):
        (self.folder / "votes.txt").write_text("".join(
            f"{i + 100} {1 if i < road else 2}\n" for i in range(total)))

    def seed_complaint(self, status=0):
        (self.folder / "complaints.txt").write_text(
            f"1001|3|Road|Test|Details|Dhaka|0|{status}||Not assigned|0|Not assigned|None|0|None\n")

    def test_poll_threshold_and_closed_complaint(self):
        for votes, total, status, expected in [(0, 0, 0, "0"), (69, 100, 0, "0"),
                (70, 100, 0, "3"), (71, 100, 0, "3"), (70, 100, 5, "0")]:
            self.seed_complaint(status)
            self.seed_poll(votes, total)
            self.run_app(["0"])
            self.assertEqual(self.records()[0].split("|")[6], expected)

    def test_live_vote_reaches_70_and_duplicate_is_rejected(self):
        self.run_app(self.signup() + ["0"])
        self.seed_complaint()
        self.seed_poll(6, 9)
        output = self.run_app(self.login() + ["5", "1", "5", "1", "exit"])
        self.assertIn("poll priority changed to Critical", output)
        self.assertIn("Already voted", output)
        self.assertEqual(self.records()[0].split("|")[6], "3")
        self.assertEqual(len((self.folder / "votes.txt").read_text().splitlines()), 10)

    def test_new_complaint_and_restart_do_not_duplicate_poll_history(self):
        self.run_app(self.signup() + ["0"])
        self.seed_poll(7, 10)
        self.run_app(self.login() + ["1", "1", "Road title", "Details", "Dhaka", "exit"])
        self.assertEqual(self.records()[0].split("|")[6], "3")
        before = (self.folder / "history.txt").read_text()
        self.run_app(["0"])
        self.assertEqual((self.folder / "history.txt").read_text(), before)

    def test_duplicate_saved_votes_do_not_inflate_percentage(self):
        self.seed_complaint()
        (self.folder / "votes.txt").write_text("100 1\n100 1\n100 1\n101 2\n")
        self.run_app(["0"])
        self.assertEqual(self.records()[0].split("|")[6], "0")


if __name__ == "__main__":
    unittest.main(verbosity=2)
