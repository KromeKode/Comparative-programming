#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr int max_attempts = 3;

// The base class. Each kind says whether to try again or stop everything.
class DownloadError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
    virtual bool retry() const { return false; }
    virtual bool fatal() const { return false; }
};

class TimedOut : public DownloadError {
  public:
    TimedOut() : DownloadError("timed out") {}
    bool retry() const override { return true; }
};

class ServerBusy : public DownloadError {
  public:
    ServerBusy() : DownloadError("server busy") {}
    bool retry() const override { return true; }
};

class NotFound : public DownloadError {
  public:
    NotFound() : DownloadError("not found") {}
};

class LoginRefused : public DownloadError {
  public:
    LoginRefused() : DownloadError("login refused") {}
    bool fatal() const override { return true; }
};

class GaveUp : public DownloadError {
  public:
    GaveUp(int attempts, const DownloadError &last)
        : DownloadError("gave up after " + std::to_string(attempts) +
                        " attempts: " + last.what()) {}
};

struct Job {
    std::string name;
    std::vector<std::string> answers;
};

// The pretend server: throw the error for this attempt's answer, if any.
void download(const Job &job, int attempt) {
    int count = static_cast<int>(job.answers.size());
    const std::string &answer = job.answers[std::min(attempt, count) - 1];
    if (answer == "timeout") {
        throw TimedOut();
    } else if (answer == "busy") {
        throw ServerBusy();
    } else if (answer == "missing") {
        throw NotFound();
    } else if (answer == "auth") {
        throw LoginRefused();
    }
}

// Return the number of attempts it took, or throw the error that stopped it.
int download_with_retries(const Job &job) {
    int wait = 1;
    for (int attempt = 1;; attempt++) {
        try {
            download(job, attempt);
            return attempt;
        } catch (const DownloadError &error) {
            if (!error.retry()) {
                throw; // rethrow the same exception, with its real type
            }
            if (attempt == max_attempts) {
                throw GaveUp(attempt, error);
            }
            std::cout << job.name << ": " << error.what() << ", retrying in "
                      << wait << "s\n";
            wait *= 2;
        }
    }
}

int main() {
    std::vector<Job> jobs;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        Job job;
        if (!(words >> job.name)) {
            continue;
        }
        for (std::string answer; words >> answer;) {
            job.answers.push_back(answer);
        }
        jobs.push_back(job);
    }

    int downloaded = 0, skipped = 0, failed = 0, not_tried = 0;
    for (std::size_t i = 0; i < jobs.size(); i++) {
        const std::string &name = jobs[i].name;
        try {
            int attempts = download_with_retries(jobs[i]);
            std::cout << name << ": ok after " << attempts << " attempt"
                      << (attempts == 1 ? "" : "s") << '\n';
            downloaded++;
        } catch (const GaveUp &error) {
            std::cout << name << ": failed: " << error.what() << '\n';
            failed++;
        } catch (const DownloadError &error) {
            if (error.fatal()) {
                std::cout << name << ": stopped: " << error.what() << '\n';
                failed++;
                not_tried = static_cast<int>(jobs.size() - i - 1);
                break;
            }
            std::cout << name << ": skipped: " << error.what() << '\n';
            skipped++;
        }
    }
    std::cout << "downloaded " << downloaded << ", skipped " << skipped
              << ", failed " << failed << ", not tried " << not_tried << '\n';
}
