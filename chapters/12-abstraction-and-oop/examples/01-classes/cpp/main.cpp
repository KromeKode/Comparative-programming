#include <algorithm>
#include <compare>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  public:
    Fraction(long numerator, long denominator) {
        if (denominator == 0) {
            throw std::invalid_argument("denominator is zero");
        }
        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
        long divisor = std::gcd(numerator, denominator);
        numerator_ = numerator / divisor;
        denominator_ = denominator / divisor;
    }

    Fraction operator+(const Fraction &other) const {
        return Fraction(numerator_ * other.denominator_ +
                            other.numerator_ * denominator_,
                        denominator_ * other.denominator_);
    }

    // Lowest terms make equal fractions look the same, so the default ==,
    // which compares the fields one by one, is right.
    bool operator==(const Fraction &) const = default;

    // The default <=> would compare the numerators first, which is wrong for
    // 1/3 and 1/2. This one compares a/b with c/d as a*d with c*b. The
    // compiler writes <, <=, >, and >= from it.
    std::strong_ordering operator<=>(const Fraction &other) const {
        return numerator_ * other.denominator_ <=>
               other.numerator_ * denominator_;
    }

    friend std::ostream &operator<<(std::ostream &out, const Fraction &f) {
        out << f.numerator_;
        if (f.denominator_ != 1) {
            out << '/' << f.denominator_;
        }
        return out;
    }

  private:
    long numerator_;
    long denominator_; // always positive
};

const char *yes_no(bool value) { return value ? "yes" : "no"; }

int main() {
    Fraction half(1, 2), third(1, 3);
    std::cout << "1/2 + 1/3 = " << half + third << '\n';
    std::cout << "2/-4 is stored as " << Fraction(2, -4) << '\n';
    std::cout << "6/3 is stored as " << Fraction(6, 3) << '\n';
    try {
        Fraction(1, 0);
    } catch (const std::invalid_argument &error) {
        std::cout << "1/0: error: " << error.what() << '\n';
    }
    std::cout << "1/2 == 2/4: " << yes_no(half == Fraction(2, 4)) << '\n';
    std::cout << "1/3 < 1/2: " << yes_no(third < half) << '\n';
    std::vector<Fraction> fractions{Fraction(3, 4), third, Fraction(-1, 2),
                                    Fraction(2, 4)};
    std::sort(fractions.begin(), fractions.end());
    std::cout << "sorted: ";
    for (std::size_t i = 0; i < fractions.size(); i++) {
        std::cout << (i > 0 ? ", " : "") << fractions[i];
    }
    std::cout << '\n';
}
