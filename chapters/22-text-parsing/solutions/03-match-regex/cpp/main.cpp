// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// What a "\" can make a literal: the characters that mean something, inside brackets or
// out.
constexpr std::u32string_view special = U"\\.[]()*+?|^$-";

struct PatternError : std::runtime_error {
    std::size_t at;  // an index into the pattern's characters
    PatternError(std::size_t at, const std::string& message)
        : std::runtime_error(message), at(at) {}
};

// UTF-8, which the input and the pattern are, as code points.
static std::u32string decode(std::string_view s) {
    std::u32string out;
    for (std::size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        int n = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
        char32_t code = n == 1 ? c : c & (0x7F >> n);  // the bits after the 1s and the 0
        for (int k = 1; k < n && i + k < s.size(); k++) code = code << 6 | (s[i + k] & 0x3F);
        out += code;
        i += n;
    }
    return out;
}

using Ranges = std::vector<std::pair<char32_t, char32_t>>;

// The tree. A character, a ".", and brackets are each a set of characters: "a" is the
// range a to a, and "." is the empty set, negated.
struct Node {
    enum Kind { chars, anchor, concat, alternation, repeat } kind;
    Ranges ranges;              // a set's
    bool negated = false;       // a set's
    char32_t op = 0;            // '^' or '$' for an anchor; '*', '+', or '?' for a repeat
    std::vector<Node> children;  // a concat's items, an alternation's options, or a repeat's item
    explicit Node(Kind kind) : kind(kind) {}
};

// Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?].
class Parser {
  public:
    explicit Parser(std::u32string pattern) : pattern_(std::move(pattern)) {}

    Node parse() {
        Node node = alternation();
        // alternation() stops early only at a ")".
        if (i_ < pattern_.size()) throw PatternError(i_, "unmatched \")\"");
        return node;
    }

  private:
    static constexpr char32_t end = 0xFFFFFFFF;  // stands for the end of the pattern
    std::u32string pattern_;  // code points, so that an index counts characters
    std::size_t i_ = 0;

    char32_t peek() const { return i_ < pattern_.size() ? pattern_[i_] : end; }

    static bool is_repeat(char32_t c) { return c == U'*' || c == U'+' || c == U'?'; }

    Node alternation() {
        Node first = concat();
        if (peek() != U'|') return first;
        Node node(Node::alternation);
        node.children.push_back(std::move(first));
        while (peek() == U'|') {
            i_++;
            node.children.push_back(concat());
        }
        return node;
    }

    Node concat() {
        Node node(Node::concat);
        for (char32_t c = peek(); c != end && c != U'|' && c != U')'; c = peek()) {
            node.children.push_back(repeat());
        }
        return node;
    }

    Node repeat() {
        char32_t c = peek();
        // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
        if (is_repeat(c)) throw PatternError(i_, "nothing to repeat");
        if (c == U'^' || c == U'$') {
            i_++;
            Node node(Node::anchor);
            node.op = c;
            return node;
        }
        Node item = atom();
        if (!is_repeat(peek())) return item;
        Node node(Node::repeat);
        node.op = pattern_[i_++];
        node.children.push_back(std::move(item));
        return node;
    }

    Node atom() {
        if (peek() == U'(') {
            std::size_t start = i_++;
            Node node = alternation();
            if (peek() != U')') throw PatternError(start, "unclosed \"(\"");
            i_++;
            return node;
        }
        if (peek() == U'[') return brackets();
        Node node(Node::chars);
        if (peek() == U'.') {
            i_++;
            node.negated = true;
        } else {
            char32_t c = character();
            node.ranges.emplace_back(c, c);
        }
        return node;
    }

    // A character, or a "\" and the character it makes a literal.
    char32_t character() {
        char32_t c = pattern_[i_];
        if (c != U'\\') {
            i_++;
            return c;
        }
        if (i_ + 1 == pattern_.size()) throw PatternError(i_, "trailing backslash");
        c = pattern_[i_ + 1];
        if (special.find(c) == special.npos) throw PatternError(i_, "invalid escape");
        i_ += 2;
        return c;
    }

    Node brackets() {
        Node node(Node::chars);
        std::size_t start = i_++;  // the [
        if (peek() == U'^') {
            node.negated = true;
            i_++;
        }
        if (peek() == U']') throw PatternError(start, "empty brackets");
        while (peek() != U']') {
            if (peek() == end) throw PatternError(start, "unclosed \"[\"");
            std::size_t at = i_;
            char32_t low = character(), high = low;
            // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
            if (peek() == U'-' && i_ + 1 < pattern_.size() && pattern_[i_ + 1] != U']') {
                i_++;
                high = character();
                if (high < low) throw PatternError(at, "range out of order");
            }
            node.ranges.emplace_back(low, high);
        }
        i_++;  // the ]
        return node;
    }
};

// The program. chars reads a character, if it's in the set, and goes on to the next
// instruction; split goes on to both x and y; jump goes on to x; line_start and line_end
// go on if the position is the start or the end of the line; and match is a match.
struct Instruction {
    enum Op { chars, split, jump, line_start, line_end, match } op;
    Ranges ranges = {};
    bool negated = false;
    std::size_t x = 0, y = 0;

    bool accepts(char32_t c) const {
        auto has = [c](auto range) { return range.first <= c && c <= range.second; };
        return std::ranges::any_of(ranges, has) != negated;
    }
};

// Appends node's instructions to the program. They refer to each other by index, since
// push_back can move them.
static void compile(const Node& node, std::vector<Instruction>& program) {
    std::size_t start = program.size();
    switch (node.kind) {
        case Node::chars:
            program.push_back({.op = Instruction::chars, .ranges = node.ranges,
                               .negated = node.negated});
            break;
        case Node::anchor:
            program.push_back({.op = node.op == U'^' ? Instruction::line_start
                                                     : Instruction::line_end});
            break;
        case Node::concat:
            for (const Node& item : node.children) compile(item, program);
            break;
        case Node::alternation: {
            // split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
            std::vector<std::size_t> jumps;
            for (std::size_t k = 0; k + 1 < node.children.size(); k++) {
                std::size_t split = program.size();
                program.push_back({.op = Instruction::split, .x = split + 1});
                compile(node.children[k], program);
                jumps.push_back(program.size());
                program.push_back({.op = Instruction::jump});
                program[split].y = program.size();
            }
            compile(node.children.back(), program);
            for (std::size_t jump : jumps) program[jump].x = program.size();
            break;
        }
        case Node::repeat:
            if (node.op == U'*') {
                // L1: split L2, L3; L2: item; jump L1; L3:
                program.push_back({.op = Instruction::split, .x = start + 1});
                compile(node.children[0], program);
                program.push_back({.op = Instruction::jump, .x = start});
                program[start].y = program.size();
            } else if (node.op == U'+') {
                // L1: item; split L1, L2; L2:
                compile(node.children[0], program);
                std::size_t after = program.size() + 1;
                program.push_back({.op = Instruction::split, .x = start, .y = after});
            } else {
                // split L1, L2; L1: item; L2:
                program.push_back({.op = Instruction::split, .x = start + 1});
                compile(node.children[0], program);
                program[start].y = program.size();
            }
            break;
    }
}

// The chars instructions that pcs lead to without reading a character, at the position
// at, and whether one of them leads to match.
static std::pair<std::vector<std::size_t>, bool> follow(const std::vector<Instruction>& program,
                                                        std::vector<std::size_t> pcs,
                                                        std::size_t at, std::size_t length) {
    std::vector<std::size_t> waiting;
    std::vector<bool> seen(program.size());
    std::vector<std::size_t>& stack = pcs;
    while (!stack.empty()) {
        std::size_t pc = stack.back();
        stack.pop_back();
        if (seen[pc]) continue;  // so that a loop such as (a*)* doesn't go around forever
        seen[pc] = true;
        const Instruction& in = program[pc];
        switch (in.op) {
            case Instruction::chars:
                waiting.push_back(pc);
                break;
            case Instruction::split:
                stack.push_back(in.y);
                stack.push_back(in.x);
                break;
            case Instruction::jump:
                stack.push_back(in.x);
                break;
            case Instruction::line_start:
                if (at == 0) stack.push_back(pc + 1);
                break;
            case Instruction::line_end:
                if (at == length) stack.push_back(pc + 1);
                break;
            case Instruction::match:
                return {{}, true};
        }
    }
    return {waiting, false};
}

// Whether a match starts anywhere in the line: instruction 0 starts at each position.
static bool search(const std::vector<Instruction>& program, std::u32string_view line) {
    std::vector<std::size_t> waiting;
    for (std::size_t at = 0; at <= line.size(); at++) {
        waiting.push_back(0);
        auto [reached, matched] = follow(program, waiting, at, line.size());
        if (matched) return true;
        waiting.clear();
        if (at == line.size()) break;
        for (std::size_t pc : reached) {
            if (program[pc].accepts(line[at])) waiting.push_back(pc + 1);
        }
    }
    return false;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: rematch PATTERN\n";
        return 2;
    }
    std::vector<Instruction> program;
    try {
        compile(Parser(decode(argv[1])).parse(), program);
    } catch (const PatternError& err) {
        std::cerr << "rematch: column " << err.at + 1 << ": " << err.what() << '\n';
        return 2;
    }
    program.push_back({.op = Instruction::match});
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    std::string out;
    // Each line ends at a "\n", or at the end of the text if it doesn't end with one.
    for (std::size_t start = 0; start < text.size();) {
        std::size_t stop = std::min(text.find('\n', start), text.size());
        std::string_view line(text.data() + start, stop - start);
        if (search(program, decode(line))) {
            out += line;
            out += '\n';
        }
        start = stop + 1;
    }
    std::cout << out;
    return out.empty() ? 1 : 0;
}
