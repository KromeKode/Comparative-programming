#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// A pretend file that opens in its constructor and closes in its destructor.
// That pairing is called RAII: resource acquisition is initialization.
class File {
  public:
    File(std::string name, bool fails) : name_(std::move(name)) {
        if (fails) {
            throw std::runtime_error("cannot open " + name_);
        }
        std::cout << "  open " << name_ << '\n';
    }
    ~File() { std::cout << "  close " << name_ << '\n'; }

    // Two File objects closing one file would close it twice, so no copies.
    File(const File &) = delete;
    File &operator=(const File &) = delete;

  private:
    std::string name_;
};

// No cleanup code at all. When the function ends, by returning or by an
// exception, the files it opened are destroyed in the reverse order. A
// constructor that throws never made an object, so there is nothing to close.
void run_job(bool output_fails, bool copy_fails) {
    File input("input", false);
    File output("output", output_fails);
    if (copy_fails) {
        throw std::runtime_error("disk full");
    }
    std::cout << "  copy input to output\n";
}

int main() {
    struct Job {
        std::string title;
        bool output_fails;
        bool copy_fails;
    };
    Job jobs[] = {
        {"everything works", false, false},
        {"opening output fails", true, false},
        {"copying fails", false, true},
    };
    int number = 0;
    for (const Job &job : jobs) {
        number++;
        std::cout << "job " << number << ": " << job.title << '\n';
        try {
            run_job(job.output_fails, job.copy_fails);
            std::cout << "  job " << number << " done\n";
        } catch (const std::exception &e) {
            std::cout << "  job " << number << " failed: " << e.what() << '\n';
        }
    }
}
