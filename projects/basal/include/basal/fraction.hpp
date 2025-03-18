#pragma once

#include <cstdint>
#include <iostream>
#include <type_traits>

namespace basal {

template <typename NumeratorType, typename DenominatorType>
class fraction_ {
    static_assert(std::is_integral<NumeratorType>::value, "Must be integral numerator_");
    static_assert(std::is_integral<DenominatorType>::value, "Must be integral denominator_");
    static_assert(std::is_unsigned<DenominatorType>::value, "Must be unsigned denominator_");

protected:
    NumeratorType numerator_;
    DenominatorType denominator_;

    static DenominatorType gcd(NumeratorType a, DenominatorType b) {
        DenominatorType c = static_cast<DenominatorType>(std::abs(a));  // ensure positive
        DenominatorType gcd = std::min(c, b);
        while (gcd > 0) {
            if ((c % gcd) == 0 && (b % gcd) == 0)
                return gcd;
            gcd -= 1;
        }
        return 0;
    }

    static DenominatorType lcm(DenominatorType a, DenominatorType b) {
        return (a * b) / gcd(a, b);
    }

    void reduce() {
        auto g = gcd(numerator_, denominator_);
        if (g > 1U) {
            exception::throw_if(g > std::numeric_limits<NumeratorType>::max(), __FILE__, __LINE__,
                                "GCD is larger than numerator_ max");
            numerator_ /= static_cast<NumeratorType>(g);
            denominator_ /= g;
        }
    }

public:
    fraction_() : numerator_{0}, denominator_{1} {
    }
    explicit fraction_(NumeratorType n) : numerator_{n}, denominator_{1U} {
    }
    explicit fraction_(NumeratorType n, DenominatorType d) : numerator_{n}, denominator_{d} {
        exception::throw_if(d == 0U, __FILE__, __LINE__, "Denominator cannot be zero");
        reduce();
    }

    bool operator==(fraction_ const& other) const {
        bool same = (numerator_ == other.numerator_ and denominator_ == other.denominator_);
        // this assumes that the fractions are reduced
        return same;
    }

    bool operator!=(fraction_ const& other) const {
        return not operator==(other);
    }

    bool operator>(NumeratorType m) const {
        // convert to fraction_ and compare
        fraction_ f{m};
        return (*this > f);
    }

    bool operator>(fraction_ const& other) const {
        // cross multiply to avoid floating point issues
        return ((numerator_ * other.denominator_) > (other.numerator_ * denominator_));
    }

    bool operator<(NumeratorType m) const {
        // convert to fraction_ and compare
        fraction_ f{m};
        return (*this < f);
    }

    bool operator<(fraction_ const& other) const {
        // cross multiply to avoid floating point issues
        return ((numerator_ * other.denominator_) < (other.numerator_ * denominator_));
    }

    bool operator>=(NumeratorType m) const {
        // convert to fraction_ and compare
        fraction_ f{m};
        return (*this >= f);
    }

    bool operator>=(fraction_ const& other) const {
        // cross multiply to avoid floating point issues
        return ((numerator_ * other.denominator_) >= (other.numerator_ * denominator_));
    }

    bool operator<=(NumeratorType m) const {
        // convert to fraction_ and compare
        fraction_ f{m};
        return (*this <= f);
    }

    bool operator<=(fraction_ const& other) const {
        // cross multiply to avoid floating point issues
        return ((numerator_ * other.denominator_) <= (other.numerator_ * denominator_));
    }

    template <typename AdditiveType>
    fraction_& operator+=(AdditiveType m) {
        static_assert(std::is_integral<AdditiveType>::value, "AdditiveType must be integral");
        numerator_ += (m * denominator_);
        reduce();
        return (*this);
    }

    template <typename AdditiveType>
    fraction_& operator-=(AdditiveType m) {
        static_assert(std::is_integral<AdditiveType>::value, "AdditiveType must be integral");
        numerator_ -= (m * denominator_);
        reduce();
        return (*this);
    }

    template <typename MultiplicativeType>
    fraction_& operator*=(MultiplicativeType m) {
        static_assert(std::is_integral<MultiplicativeType>::value, "MultiplicativeType must be integral");
        numerator_ = numerator_ * m;
        reduce();
        return (*this);
    }

    template <typename MultiplicativeType>
    fraction_& operator/=(MultiplicativeType m) {
        exception::throw_if(m == 0, __FILE__, __LINE__, "Cannot divide by zero");
        static_assert(std::is_integral<MultiplicativeType>::value, "MultiplicativeType must be integral");
        denominator_ *= m;
        reduce();
        return (*this);
    }

    fraction_& operator+=(fraction_ const& other) {
        auto common_denominator = lcm(denominator_, other.denominator_);
        numerator_ = (numerator_ * (common_denominator / denominator_))
                     + (other.numerator_ * (common_denominator / other.denominator_));
        denominator_ = common_denominator;
        reduce();
        return (*this);
    }

    fraction_& operator-=(fraction_ const& other) {
        auto common_denominator = lcm(denominator_, other.denominator_);
        numerator_ = (numerator_ * (common_denominator / denominator_))
                     - (other.numerator_ * (common_denominator / other.denominator_));
        denominator_ = common_denominator;
        reduce();
        return (*this);
    }

    fraction_& operator*=(fraction_ const& other) {
        numerator_ *= other.numerator_;
        denominator_ *= other.denominator_;
        reduce();
        return (*this);
    }

    fraction_& operator/=(fraction_ const& other) {
        exception::throw_if(other.numerator_ == 0, __FILE__, __LINE__, "Cannot divide by zero");
        numerator_ *= other.denominator_;
        denominator_ *= other.numerator_;
        reduce();
        return (*this);
    }

    friend fraction_ operator+(fraction_ const& a, fraction_ const& b) {
        fraction_ c;
        auto common_denominator = lcm(a.denominator_, b.denominator_);
        c.numerator_ = (a.numerator_ * (common_denominator / a.denominator_))
                       + (b.numerator_ * (common_denominator / b.denominator_));
        c.denominator_ = common_denominator;
        c.reduce();
        return c;
    }

    friend fraction_ operator-(fraction_ const& a, fraction_ const& b) {
        fraction_ c;
        auto common_denominator = lcm(a.denominator_, b.denominator_);
        c.numerator_ = (a.numerator_ * (common_denominator / a.denominator_))
                       - (b.numerator_ * (common_denominator / b.denominator_));
        c.denominator_ = common_denominator;
        c.reduce();
        return c;
    }

    friend fraction_ operator*(fraction_ const& a, fraction_ const& b) {
        fraction_ c;
        c.numerator_ = a.numerator_ * b.numerator_;
        c.denominator_ = a.denominator_ * b.denominator_;
        c.reduce();
        return c;
    }

    friend fraction_ operator/(fraction_ const& a, fraction_ const& b) {
        fraction_ c;
        c.numerator_ = a.numerator_ * b.denominator_;
        c.denominator_ = a.denominator_ * b.numerator_;
        c.reduce();
        return c;
    }

    template <typename AdditiveType>
    friend fraction_ operator+(fraction_ const& a, AdditiveType b) {
        fraction_ c;
        c.numerator_ = a.numerator_ + (b * a.denominator_);
        c.denominator_ = a.denominator_;
        c.reduce();
        return c;
    }

    template <typename AdditiveType>
    friend fraction_ operator+(AdditiveType b, fraction_ const& a) {
        fraction_ c;
        c.numerator_ = (b * a.denominator_) + a.numerator_;
        c.denominator_ = a.denominator_;
        c.reduce();
        return c;
    }

    template <typename AdditiveType>
    friend fraction_ operator-(AdditiveType b, fraction_ const& a) {
        fraction_ c;
        c.numerator_ = (b * a.denominator_) - a.numerator_;
        c.denominator_ = a.denominator_;
        c.reduce();
        return c;
    }

    template <typename AdditiveType>
    friend fraction_ operator-(fraction_ const& a, AdditiveType b) {
        fraction_ c;
        c.numerator_ = a.numerator_ - (b * a.denominator_);
        c.denominator_ = a.denominator_;
        c.reduce();
        return c;
    }

    template <typename MultiplicativeType>
    friend fraction_ operator*(fraction_ const& a, MultiplicativeType b) {
        fraction_ c;
        c.numerator_ = a.numerator_ * b;
        c.denominator_ = a.denominator_;
        c.reduce();
        return c;
    }

    template <typename MultiplicativeType>
    friend fraction_ operator/(fraction_ const& a, MultiplicativeType b) {
        fraction_ c;
        c.numerator_ = a.numerator_;
        c.denominator_ = a.denominator_ * b;
        c.reduce();
        return c;
    }

    friend std::ostream& operator<<(std::ostream& os, fraction_ const& f) {
        return os << "fraction=" << f.numerator_ << "/" << f.denominator_;
    }

    template <typename SomeType>
    SomeType as() const {
        return static_cast<SomeType>(numerator_) / denominator_;  // using implicit deduction rules?
    }
};

using fraction = fraction_<int, unsigned int>;

}  // namespace basal
