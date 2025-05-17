//
// Created by Kxq03 on 25-5-16.
//

#ifndef BIGINT_H
#define BIGINT_H
#include <algorithm>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <vector>
// TODO: 模运算, FFT乘法
class BigInt {
private:
    using i64 = long long;
    using ui64 = unsigned long long;
    const static int BASE = 1e8;    // 数组中一个数字的大小不会超过BASE
    int len;                    // 当前数字的位数
    bool neg;                   // 是否是负数
    std::deque<int> digits;     // 双端队列存大整数, 前面是高位, 后面是低位

private:
    void initWithString(const char *str, int length) {
        // 清空原先的数据
        digits.clear();
        len = 0;
        neg = false;
        // 从后往前初始化数字
        auto func = [this, &str, &length](const int start) {
            // 从第一个非0的字符开始
            int non_zero = -1;
            for (int i = start; i < length; ++i) {
                if (str[i] != '0') {
                    non_zero = i;
                    break;
                }
            }
            if (non_zero == -1) {
                this->neg = false;
                this->len = 1;
                this->digits.push_front(0);
                return;
            }
            // 从后往前, 每隔8位填入一个数字
            for (int i = length - 1, k = 1, num = 0; i >= non_zero; --i) {
                num = num + k * (str[i] - '0');
                k *= 10;
                if (k == BigInt::BASE || i == non_zero) {
                    this->len += 1;
                    this->digits.push_front(num);
                    k = 1;
                    num = 0;
                }
            }
        };
        if (str[0] == '-') {
            neg = true;
            func(1);
        } else {
            neg = false;
            func(0);
        }
    }
public:
    // 无参默认为0
    BigInt() {
        len = 1;
        neg = false;
        digits.push_back(0);
    }
#ifdef __cplusplus
#if __cplusplus >= 202002L
    template<std::integral T>
#elif __cplusplus >= 201103L
    template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
#else
    template<typename T>
#endif
    explicit BigInt(T val) : len(0), neg(false) {
        if (val < 0) {
            val = -val;
            neg = true;
        }
        while (val != 0) {
            digits.push_front(val % BASE);
            len += 1;
            val /= BASE;
        }
        // 特例: val == 0的时候
        if (len == 0) {
            digits.push_front(val);
            len += 1;
        }
    }
#endif
    BigInt(char *str, int length) :len(0), neg(false) {
        initWithString(str, length);
    }
    BigInt(const char *str, int length) :len(0), neg(false) {
        initWithString(str, length);
    }
    BigInt(const std::string& str) :len(0), neg(false) {
        initWithString(str.c_str(), str.length());
    }
    // 比较运算
    bool operator == (const BigInt& rhs) const {
        // 如果位数不等或者符号不相等, 两个数不相等
        if (len != rhs.len || neg != rhs.neg) {
            return false;
        }
        // 两个数位数和符号位都相同, 依次比较
        for (int i = 0; i < len; ++i) {
            if (digits[i] != rhs.digits[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator != (const BigInt& rhs) const {
        return !(*this == rhs);
    }

    bool operator > (const BigInt& rhs) const {
        // 正数大于负数(0视为正数)
        if (neg != rhs.neg) {
            return neg;
        }
        // 两数同为正数或负数, 其比较是反着来的
        // 如果是正数, 位数多的大
        // 负数, 位数少的大
        if (len != rhs.len) {
            if (!neg) return len > rhs.len;
            else return len < rhs.len;
        }
        // 位数相同的情况下, 从高到低位依次比较
        // 正数, 高位大的就大
        // 负数, 高位小的就大
        for (int i = 0; i < len; ++i) {
            if (digits[i] != rhs.digits[i]) {
                if (!neg) return digits[i] > rhs.digits[i];
                else return digits[i] < rhs.digits[i];
            }
        }
        return false;
    }

    bool operator < (const BigInt& rhs) const {
        // 两数符号位不同: 如果this.neg是正数, 返回false, 否则返回true
        if (neg != rhs.neg) {
            return rhs.neg;
        }
        // 同>的比较, 但反过来
        // 位数不同, 位数低的更小
        if (len != rhs.len) {
            if (!neg) return len < rhs.len;
            else return len > rhs.len;
        }
        // 位数相同
        for (int i = 0; i < len; ++i) {
            if (digits[i] != rhs.digits[i]) {
                if (!neg) return digits[i] < rhs.digits[i];
                else return digits[i] > rhs.digits[i];
            }
        }
        return false;
    }

    bool operator <= (const BigInt& rhs) const {
        return !(*this > rhs);
    }

    bool operator >= (const BigInt& rhs) const {
        return !(*this < rhs);
    }
    // 高精度加减(通常将低精度转为高精度计算, 不会损失太多性能)
    BigInt operator + (const BigInt& rhs) const {
        // 符号位有4种情况
        // +, -
        if (!neg && rhs.neg) {
            return *this - (-rhs);
        }
        // -, +
        if (neg && !rhs.neg) {
            return rhs - (-(*this));
        }
        // +, + 符号位不变
        // -, - 符号位不变
        // res初始化后, digits有一个0存在, 先去掉
        BigInt res{};
        res.digits.pop_back();
        res.len = std::max(len, rhs.len);

        i64 ex_add = 0;
        // 逆向迭代器, 从低位到高位计算
        for (auto this_it = digits.rbegin(), rhs_it = rhs.digits.rbegin();
            this_it != digits.rend() || rhs_it != rhs.digits.rend();) {
            i64 num = (this_it == digits.rend() ? 0 : (*this_it)) +
                (rhs_it == rhs.digits.rend() ? 0 : (*rhs_it)) + ex_add;
            ex_add = 0;
            if (num >= BASE) {
                num -= BASE;
                ex_add = 1;
            }
            res.digits.push_front(num);
            if (this_it != digits.rend()) ++this_it;
            if (rhs_it != rhs.digits.rend()) ++rhs_it;
        }
        if (ex_add != 0) {
            res.len += 1;
            res.digits.push_front(1);
        }
        res.neg = neg;
        return res;
    }
    // 高精度减
    BigInt operator - (const BigInt& rhs) const {
        // +, -
        if (!neg && rhs.neg) {
            return *this + (-rhs);
        }
        // -, +
        if (neg && !rhs.neg) {
            return -(rhs + (-(*this)));
        }
        // +, +
        // -, -
        BigInt res{};
        // 同符号比较, 两个数大小相同
        if (*this == rhs) {
            return res;
        }
        // 不同进行实际的减法
        // res初始化后, digits有一个0存在, 先去掉
        res.digits.pop_back();
        // subFunc是绝对值大的减绝对值小的
        auto subFunc = [&res](const BigInt &a, const BigInt &b) {
            // a > b
            // forward 存借位, 模拟竖式减法从低位开始减
            i64 forward = 0;
            for (auto a_it = a.digits.rbegin(), b_it = b.digits.rbegin();
                a_it != a.digits.rend() || b_it != b.digits.rend();) {
                i64 num = *a_it - forward - (b_it == b.digits.rend() ? 0 : *b_it);
                forward = 0;
                if (num < 0) {
                    // 向前借位
                    forward = 1;
                    num += BASE;
                }
                res.digits.push_front(num);
                if (a_it != a.digits.rend()) ++a_it;
                if (b_it != b.digits.rend()) ++b_it;
            }
            while (res.digits.size() > 1 && res.digits.front() == 0) {
                res.digits.pop_front();
            }
            res.len = res.digits.size();
        };
        // 正数-正数
        if (!neg) {
            if (*this > rhs) {
                subFunc(*this, rhs);
            } else {
                subFunc(rhs, *this);
                res.neg = true;
            }
        } else {
            // 负数-负数
            // 两个负数, 小的那个绝对值更大
            if (*this < rhs) {
                subFunc(*this, rhs);
                res.neg = true;
            } else {
                subFunc(rhs, *this);
            }
        }
        return res;
    }
    // 负号重载(成员没有用到指针, 不用担心拷贝构造)
    BigInt operator - () const {
        BigInt res(*this);
        res.neg ^= true;
        return res;
    }

    // 高精度乘除低精度(int)类型
    // 为什么不用long long, 考虑到int * i64极限会爆i64
    // 支持i64需要用int128类型, 后续可以再改
    BigInt operator * (int rhs) const {
        BigInt res{};
        if (rhs == 0 || (*this) == res) {
            return res;
        }
        // 设置符号位
        bool rhs_neg = (rhs < 0);
        res.neg = (neg != rhs_neg);
        // if ((!neg && rhs > 0) || (neg && rhs < 0)) {
        //     res.neg = false;
        // } else {
        //     res.neg = true;
        // }

        // 将rhs转为正数
        if (rhs < 0) rhs = -rhs;
        // 清空res.digits
        res.digits.pop_back();
        // 从低位向高位计算
        i64 ex_add = 0;
        for (auto this_it = digits.rbegin(); this_it != digits.rend(); ++this_it) {
            ex_add += static_cast<i64>(*this_it) * rhs;
            res.digits.push_front(ex_add % BASE);
            ex_add /= BASE;
        }
        while (ex_add != 0) {
            res.digits.push_front(ex_add % BASE);
            ex_add /= BASE;
        }
        while (res.digits.size() > 1 && res.digits.front() == 0) {
            res.digits.pop_front();
        }
        res.len = res.digits.size();
        return res;
    }

    BigInt operator / (int rhs) const {
        assert(rhs != 0);
        BigInt res{};
        // rhs > this 或this为0
        if ((*this == res) || BigInt(::abs(rhs)) > abs(*this)) {
            return res;
        }
        // 设置符号位
        bool rhs_neg = (rhs < 0);
        res.neg = (neg != rhs_neg);
        // if ((!neg && rhs > 0) || (neg && rhs < 0)) {
        //     res.neg = false;
        // } else {
        //     res.neg = true;
        // }

        // 将rhs转为正数
        if (rhs < 0) rhs = -rhs;
        // 清空res.digits
        res.digits.pop_back();
        // 模拟竖式除法, 从高位到低位往下除
        i64 num = 0;
        for (auto this_it = digits.begin(); this_it != digits.end(); ++this_it) {
            num = num * BASE + (*this_it);
            res.digits.push_back(num / rhs);
            num %= rhs;
        }

        while (res.digits.size() > 1 && res.digits.front() == 0) {
            res.digits.pop_front();
        }
        res.len = res.digits.size();
        // if (res.digits.size() == 1 && res.digits[0] == 0) {
        //     res.neg = false;
        // }
        return res;
    }
    // 高精度乘高精度
    BigInt operator * (const BigInt &rhs) const {
        BigInt res{};
        // 两者有一个为0, 直接返回0
        if (rhs == res || (*this) == res) {
            return res;
        }
        // 设置符号位(两个数符号位不同, 为负, 否则为正)
        res.neg = (neg != rhs.neg);
        // 模拟竖式计算
        i64 tmp = 0;
        res.digits = std::deque<int>(len + rhs.len, 0);
        // 低位到高位计算
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < rhs.len; j++) {
                int it1 = len - i - 1;
                int it2 = rhs.len - j - 1;
                // res.digits[i + j]在计算之前存的是进位
                tmp = 1ll * digits[it1] * rhs.digits[it2] + res.digits[i + j];
                if (tmp >= BASE) {
                    res.digits[i + j + 1] += tmp / BASE;
                    res.digits[i + j] = tmp % BASE;
                } else {
                    res.digits[i + j] = tmp;
                }
            }
        }
        std::reverse(res.digits.begin(), res.digits.end());
        while (res.digits.size() > 1 && res.digits.front() == 0) {
            res.digits.pop_front();
        }
        res.len = res.digits.size();
        return res;
    }
    // 高精度除以高精度
    BigInt operator / (const BigInt &rhs) const {
        BigInt res{};
        // rhs != 0
        assert(rhs != res);
        // 0除以任何数都为0 或者 如果rhs的绝对值比this大, 结果为0
        if ((*this == res) || abs(rhs) > abs(*this)) {
            return res;
        }
        // 不能改变this, 用res代替
        res = (*this);
        // 设置符号位(两个数符号位不同, 为负, 否则为正)
        bool t_neg = (neg != rhs.neg);
        // 除数, 除数的个数, 商
        BigInt divisor(rhs), tmp(1), quotient{};
        // 除数和被除数都当做正数来做, 是为了后面方便比较运算
        res.neg = divisor.neg = false;
        // 大整数的除法求的是 res / divisor = quotient
        // → quotient * divisor <= res, 求的是满足左式的quotient的最大值
        // 相当于是res减去quotient个divisor

        // 倍增: 将divisor不断*2, 对应减去的个数也*2
        while (res > divisor) {
            divisor = divisor * 2;
            tmp = tmp * 2;
        }
        // res还能够减去一个divisor时, 继续减
        while (tmp > BigInt(0)) {
            if (res >= divisor) {
                res -= divisor;
                // 对应的商要加上之前减去的个数
                quotient += tmp;
            }
            divisor = divisor / 2;
            tmp = tmp / 2;
        }
        // res 里剩余的就是模数, quotient就是商
        while (quotient.digits.size() > 1 && quotient.digits.front() == 0) {
            quotient.digits.pop_front();
        }
        quotient.neg = t_neg;
        return quotient;
    }
    BigInt operator += (const BigInt &rhs) {
        *this = (*this) + rhs;
        return *this;
    }
    BigInt operator -= (const BigInt &rhs) {
        *this = (*this) - rhs;
        return *this;
    }
    BigInt operator *= (int rhs) {
        *this = (*this) * rhs;
        return *this;
    }
    BigInt operator *= (const BigInt &rhs) {
        *this = (*this) * rhs;
        return *this;
    }
    BigInt operator /= (int rhs) {
        assert(rhs != 0);
        BigInt res{};
        // rhs > this 或this为0
        if ((*this == res) || BigInt(::abs(rhs)) > abs(*this)) {
            *this = res;
            return *this;
        }
        // 复制this, 将res当做this, 将计算结果直接放入this
        res = (*this);
        // 设置符号位
        bool t_neg, rhs_neg = (rhs < 0);
        t_neg = (neg != rhs_neg);
        // if ((!res.neg && rhs > 0) || (res.neg && rhs < 0)) {
        //     t_neg = false;
        // } else {
        //     t_neg = true;
        // }
        if (rhs < 0) rhs = -rhs;
        // 初始化答案
        this->digits.clear();
        this->len = 0;
        // 模拟竖式除法, 从高位到低位往下除
        i64 num = 0;
        for (auto this_it = res.digits.begin(); this_it != res.digits.end(); ++this_it) {
            num = num * BASE + (*this_it);
            digits.push_back(num / rhs);
            num %= rhs;
        }

        while (digits.size() > 1 && digits.front() == 0) {
            digits.pop_front();
        }
        len = digits.size();
        neg = t_neg;
        return *this;
    }
    BigInt operator /= (const BigInt &rhs) {
        BigInt res{};
        // rhs != 0
        assert(rhs != res);
        // 0除以任何数都为0 或者 如果rhs的绝对值比this大, 结果为0
        if ((*this == res) || abs(rhs) > abs(*this)) {
            *this = res;
            return *this;
        }
        res = (*this);
        // 设置符号位(两个数符号位不同, 为负, 否则为正)
        bool t_neg = (neg != rhs.neg);
        // 除数divisor, 除数的个数tmp, 商*this
        (*this) = BigInt(0);
        BigInt divisor(rhs), tmp(1);
        // 被除数和除数的符号位设为正数, 方便比较
        res.neg = divisor.neg = false;

        while (res > divisor) {
            divisor *=  2;
            tmp *= 2;
        }
        // res还能够减去一个divisor时, 继续减
        while (tmp > BigInt(0)) {
            if (res >= divisor) {
                res -= divisor;
                // 对应的商要加上之前减去的个数
                *this += tmp;
            }
            divisor /=  2;
            tmp /= 2;
        }
        // res 里剩余的就是模数, quotient就是商
        while (digits.size() > 1 && digits.front() == 0) {
            digits.pop_front();
        }
        neg = t_neg;
        return *this;
    }
    static BigInt abs(const BigInt &big_int) {
        BigInt res = big_int;
        if (res.neg) res.neg = false;
        return res;
    }
    // 输入
    friend std::istream& operator >> (std::istream& is, BigInt& rhs) {
        rhs.digits.clear();
        std::string str;
        is >> str;
        rhs.initWithString(str.c_str(), str.length());
        return is;
    }
    // 输出
    friend std::ostream& operator << (std::ostream& os, const BigInt& rhs) {
        if (rhs.neg) std::cout << '-';
        os << rhs.digits[0];
        for (int i = 1; i < rhs.len; ++i) {
            os << std::setw(8) << std::setfill('0') << rhs.digits[i];
        }
        return os;
    }
};


#endif //BIGINT_H
