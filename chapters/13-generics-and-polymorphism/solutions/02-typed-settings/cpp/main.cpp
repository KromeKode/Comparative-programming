#include <charconv>
#include <concepts>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct SettingError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// What the command loop needs from a setting, without knowing its T.
class AnySetting {
public:
    virtual ~AnySetting() = default;
    virtual const std::string& name() const = 0;
    virtual void set_text(const std::string& text) = 0;
    virtual void reset() = 0;
    virtual std::string describe() const = 0;
    virtual bool is_default() const = 0;
};

// One setting whose value is a T. It knows how to read a T from text and how
// to show one. T must support ==, for is_default.
template <std::equality_comparable T>
class Setting final : public AnySetting {
public:
    using Parse = std::function<std::optional<T>(const std::string&)>;
    using Show = std::function<std::string(const T&)>;

    Setting(std::string name, T def, Parse parse, Show show, std::string needs)
        : name_(std::move(name)), default_(def), value_(std::move(def)),
          parse_(std::move(parse)), show_(std::move(show)), needs_(std::move(needs)) {}

    const T& value() const { return value_; }

    const std::string& name() const override { return name_; }

    void set_text(const std::string& text) override {
        std::optional<T> value = parse_(text);
        if (!value) throw SettingError(name_ + " needs " + needs_ + ", not \"" + text + "\"");
        value_ = std::move(*value);
    }

    void reset() override { value_ = default_; }
    std::string describe() const override { return name_ + " = " + show_(value_); }
    bool is_default() const override { return value_ == default_; }

private:
    std::string name_;
    T default_, value_;
    Parse parse_;
    Show show_;
    std::string needs_;
};

std::optional<int> parse_int(const std::string& text) {
    // from_chars takes a leading '-' but not '+', and reports values that
    // don't fit in an int.
    int value = 0;
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size()) return std::nullopt;
    return value;
}

std::optional<bool> parse_bool(const std::string& text) {
    if (text == "true") return true;
    if (text == "false") return false;
    return std::nullopt;
}

std::vector<std::string> split(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> words;
    for (std::string word; in >> word;) words.push_back(word);
    return words;
}

std::string show_tags(const std::vector<std::string>& tags) {
    std::string text = "[";
    for (std::size_t i = 0; i < tags.size(); i++) text += (i ? ", " : "") + tags[i];
    return text + "]";
}

// Typed members: volume.value() is an int and muted.value() a bool, with no casts.
struct Settings {
    Setting<int> volume{"volume", 5, parse_int,
                        [](int n) { return std::to_string(n); }, "a whole number"};
    Setting<bool> muted{"muted", false, parse_bool,
                        [](bool b) { return std::string(b ? "true" : "false"); },
                        "true or false"};
    Setting<std::string> name{"name", "guest", [](const std::string& t) { return t; },
                              [](const std::string& t) { return t; }, ""};
    Setting<std::vector<std::string>> tags{"tags", {}, split, show_tags, ""};
    // all points into this object, so a Settings must never be copied.
    std::vector<AnySetting*> all{&volume, &muted, &name, &tags};

    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    AnySetting& find(const std::string& wanted) {
        for (AnySetting* setting : all) {
            if (setting->name() == wanted) return *setting;
        }
        throw SettingError("no setting named \"" + wanted + "\"");
    }

    std::string summary() const {
        if (muted.value()) return name.value() + " has muted the sound";
        long long percent = volume.value() * 10LL;
        return name.value() + " hears the sound at " + std::to_string(percent) + "%";
    }
};

void run(Settings& settings, const std::vector<std::string>& words) {
    if (words.empty()) return;
    const std::string& command = words[0];
    if (command == "set" || command == "get" || command == "reset") {
        AnySetting& setting = settings.find(words[1]);
        if (command == "set") {
            std::string value;
            for (std::size_t i = 2; i < words.size(); i++) value += (i > 2 ? " " : "") + words[i];
            setting.set_text(value);
        } else if (command == "reset") {
            setting.reset();
        }
        std::cout << setting.describe() << "\n";
    } else if (command == "list") {
        for (const AnySetting* setting : settings.all) {
            std::cout << setting->describe() << (setting->is_default() ? " (default)" : "") << "\n";
        }
    } else if (command == "summary") {
        std::cout << settings.summary() << "\n";
    }
}

int main() {
    Settings settings;
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            run(settings, split(line));
        } catch (const SettingError& error) {
            std::cout << "error: " << error.what() << "\n";
        }
    }
}
