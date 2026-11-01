#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Lines = std::vector<std::string>;

class EditError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

// An edit that knows how to do itself and how to undo itself.
class Command {
  public:
    virtual ~Command() = default;
    virtual void apply(Lines &lines) = 0;
    virtual void undo(Lines &lines) = 0;
    virtual std::string describe() const = 0;
};

void check_line(const Lines &lines, std::size_t number) {
    if (number < 1 || number > lines.size()) {
        throw EditError("no line " + std::to_string(number));
    }
}

class Add : public Command {
  public:
    explicit Add(std::string text) : text_(std::move(text)) {}

    void apply(Lines &lines) override { lines.push_back(text_); }
    void undo(Lines &lines) override { lines.pop_back(); }
    std::string describe() const override { return "add \"" + text_ + "\""; }

  private:
    std::string text_;
};

class Delete : public Command {
  public:
    explicit Delete(std::size_t number) : number_(number) {}

    void apply(Lines &lines) override {
        check_line(lines, number_);
        removed_ = std::move(lines[number_ - 1]);
        lines.erase(lines.begin() + static_cast<long>(number_ - 1));
    }

    void undo(Lines &lines) override {
        lines.insert(lines.begin() + static_cast<long>(number_ - 1),
                     std::move(removed_));
    }

    std::string describe() const override {
        return "delete line " + std::to_string(number_);
    }

  private:
    std::size_t number_;
    std::string removed_;
};

class Replace : public Command {
  public:
    Replace(std::size_t number, std::string text)
        : number_(number), text_(std::move(text)) {}

    void apply(Lines &lines) override {
        check_line(lines, number_);
        old_ = std::exchange(lines[number_ - 1], text_);
    }

    void undo(Lines &lines) override { lines[number_ - 1] = std::move(old_); }

    std::string describe() const override {
        return "replace line " + std::to_string(number_) + " with \"" +
               text_ + "\"";
    }

  private:
    std::size_t number_;
    std::string text_;
    std::string old_;
};

class Editor {
  public:
    void run(std::unique_ptr<Command> command) {
        command->apply(lines_); // if this throws, nothing is recorded
        done_.push_back(std::move(command));
        undone_.clear();
    }

    void undo() {
        if (done_.empty()) {
            throw EditError("nothing to undo");
        }
        std::unique_ptr<Command> command = std::move(done_.back());
        done_.pop_back();
        command->undo(lines_);
        std::cout << "undo: " << command->describe() << '\n';
        undone_.push_back(std::move(command));
    }

    void redo() {
        if (undone_.empty()) {
            throw EditError("nothing to redo");
        }
        std::unique_ptr<Command> command = std::move(undone_.back());
        undone_.pop_back();
        command->apply(lines_);
        std::cout << "redo: " << command->describe() << '\n';
        done_.push_back(std::move(command));
    }

    void show() const {
        if (lines_.empty()) {
            std::cout << "(empty)\n";
        }
        for (std::size_t i = 0; i < lines_.size(); i++) {
            std::cout << i + 1 << ": " << lines_[i] << '\n';
        }
    }

  private:
    Lines lines_;
    std::vector<std::unique_ptr<Command>> done_;
    std::vector<std::unique_ptr<Command>> undone_;
};

std::string join(const std::vector<std::string> &words, std::size_t from) {
    std::string text;
    for (std::size_t i = from; i < words.size(); i++) {
        text += (i > from ? " " : "") + words[i];
    }
    return text;
}

std::size_t parse_number(const std::string &text) {
    try {
        return std::stoul(text);
    } catch (const std::logic_error &) {
        throw EditError("not a line number: " + text);
    }
}

void run(Editor &editor, const std::vector<std::string> &words) {
    const std::string &command = words[0];
    if (command == "add") {
        editor.run(std::make_unique<Add>(join(words, 1)));
    } else if (command == "delete" && words.size() == 2) {
        editor.run(std::make_unique<Delete>(parse_number(words[1])));
    } else if (command == "replace" && words.size() >= 2) {
        editor.run(
            std::make_unique<Replace>(parse_number(words[1]), join(words, 2)));
    } else if (command == "undo") {
        editor.undo();
    } else if (command == "redo") {
        editor.redo();
    } else if (command == "show") {
        editor.show();
    }
}

int main() {
    Editor editor;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::vector<std::string> words;
        for (std::string word; in >> word;) {
            words.push_back(word);
        }
        if (words.empty()) {
            continue;
        }
        try {
            run(editor, words);
        } catch (const EditError &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
}
