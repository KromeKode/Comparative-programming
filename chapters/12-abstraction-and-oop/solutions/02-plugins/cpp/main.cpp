#include <cctype>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Words = std::vector<std::string>;

class PluginError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

// Every plugin has a name and changes a list of words. describe is virtual but
// not pure, so it has a default that a plugin may replace.
class Plugin {
  public:
    virtual ~Plugin() = default;
    virtual std::string name() const = 0;
    virtual Words apply(Words words) const = 0;
    virtual std::string describe() const { return name(); }
};

std::string lowercase(std::string text) {
    for (char &c : text) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return text;
}

class Upper : public Plugin {
  public:
    std::string name() const override { return "upper"; }

    Words apply(Words words) const override {
        for (std::string &word : words) {
            for (char &c : word) {
                c = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(c)));
            }
        }
        return words;
    }
};

class Lower : public Plugin {
  public:
    std::string name() const override { return "lower"; }

    Words apply(Words words) const override {
        for (std::string &word : words) {
            word = lowercase(word);
        }
        return words;
    }
};

class Reverse : public Plugin {
  public:
    std::string name() const override { return "reverse"; }

    Words apply(Words words) const override {
        return Words(words.rbegin(), words.rend());
    }
};

class Censor : public Plugin {
  public:
    explicit Censor(std::string word) : word_(std::move(word)) {}

    std::string name() const override { return "censor"; }

    Words apply(Words words) const override {
        std::string hidden = lowercase(word_);
        for (std::string &word : words) {
            if (lowercase(word) == hidden) {
                word = std::string(word.size(), '*');
            }
        }
        return words;
    }

    std::string describe() const override { return "censor(" + word_ + ")"; }

  private:
    std::string word_;
};

struct Entry {
    std::string name, usage, help;
    std::function<std::unique_ptr<Plugin>(const Words &args)> make;
};

// Maps a plugin's name to a way to make one. The rest of the program finds
// plugins only through here.
class Registry {
  public:
    void add(Entry entry) { entries_.push_back(std::move(entry)); }

    std::unique_ptr<Plugin> create(const std::string &name,
                                   const Words &args) const {
        for (const Entry &entry : entries_) {
            if (entry.name == name) {
                return entry.make(args);
            }
        }
        throw PluginError("no plugin named " + name);
    }

    const std::vector<Entry> &entries() const { return entries_; }

  private:
    std::vector<Entry> entries_;
};

// One template for every plugin that takes no arguments. Each use, such as
// no_arguments<Upper>, is a separate function made for that type.
template <typename P> std::unique_ptr<Plugin> no_arguments(const Words &args) {
    auto plugin = std::make_unique<P>();
    if (!args.empty()) {
        throw PluginError(plugin->name() + " takes no arguments");
    }
    return plugin;
}

std::unique_ptr<Plugin> make_censor(const Words &args) {
    if (args.size() != 1) {
        throw PluginError("censor needs exactly one word");
    }
    return std::make_unique<Censor>(args[0]);
}

Registry make_registry() {
    Registry registry;
    registry.add({"upper", "upper", "make every letter uppercase",
                  no_arguments<Upper>});
    registry.add({"lower", "lower", "make every letter lowercase",
                  no_arguments<Lower>});
    registry.add({"reverse", "reverse", "reverse the order of the words",
                  no_arguments<Reverse>});
    registry.add({"censor", "censor WORD", "hide WORD wherever it appears",
                  make_censor});
    return registry;
}

int main() {
    Registry registry = make_registry();
    std::vector<std::unique_ptr<Plugin>> pipeline;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        Words words;
        for (std::string word; in >> word;) {
            words.push_back(word);
        }
        if (words.empty()) {
            continue;
        }
        const std::string &command = words[0];
        if (command == "list" && words.size() == 1) {
            for (const Entry &entry : registry.entries()) {
                std::cout << entry.usage << ": " << entry.help << '\n';
            }
        } else if (command == "add" && words.size() >= 2) {
            try {
                Words args(words.begin() + 2, words.end());
                pipeline.push_back(registry.create(words[1], args));
                std::cout << "added " << pipeline.back()->describe() << '\n';
            } catch (const PluginError &error) {
                std::cout << "error: " << error.what() << '\n';
            }
        } else if (command == "pipeline" && words.size() == 1) {
            std::cout << "pipeline: ";
            if (pipeline.empty()) {
                std::cout << "(empty)";
            }
            for (std::size_t i = 0; i < pipeline.size(); i++) {
                std::cout << (i > 0 ? " | " : "") << pipeline[i]->describe();
            }
            std::cout << '\n';
        } else if (command == "run") {
            Words text(words.begin() + 1, words.end());
            for (const auto &plugin : pipeline) {
                text = plugin->apply(std::move(text));
            }
            for (std::size_t i = 0; i < text.size(); i++) {
                std::cout << (i > 0 ? " " : "") << text[i];
            }
            std::cout << '\n';
        }
    }
}
