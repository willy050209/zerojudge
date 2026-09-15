#pragma once
#include <string>
#include <string_view>
#include <cstdlib>
#include <utility>

namespace solver {

struct Input {
    std::string s1;
    std::string s2;
};

struct Output {
    std::string diff_roman;
};

/**
 * 將單一羅馬數字符號轉為整數值
 */
inline constexpr int char_to_val(char c) {
    switch (c) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:  return 0;
    }
}

/**
 * 純函數：將標準羅馬數字字串轉換為十進位整數
 */
inline int roman_to_int(std::string_view s) {
    int total = 0;
    const int n = static_cast<int>(s.size());
    for (int i = 0; i < n; ++i) {
        int cur = char_to_val(s[i]);
        if (i + 1 < n && cur < char_to_val(s[i + 1])) {
            total -= cur;
        } else {
            total += cur;
        }
    }
    return total;
}

/**
 * 純函數：將十進位整數轉換為標準羅馬數字字串 (0 輸出 "ZERO")
 */
inline std::string int_to_roman(int val) {
    if (val == 0) {
        return "ZERO";
    }

    static constexpr std::pair<int, const char*> ROMAN_TABLE[] = {
        {1000, "M"},
        {900,  "CM"},
        {500,  "D"},
        {400,  "CD"},
        {100,  "C"},
        {90,   "XC"},
        {50,   "L"},
        {40,   "XL"},
        {10,   "X"},
        {9,    "IX"},
        {5,    "V"},
        {4,    "IV"},
        {1,    "I"}
    };

    std::string res;
    res.reserve(16);
    for (const auto& [num, str] : ROMAN_TABLE) {
        while (val >= num) {
            res += str;
            val -= num;
        }
    }
    return res;
}

/**
 * 純函數：計算兩羅馬數字之差的絕對值，並轉為羅馬數字
 */
inline Output solve(const Input& in) {
    int val1 = roman_to_int(in.s1);
    int val2 = roman_to_int(in.s2);
    int diff = std::abs(val1 - val2);
    return Output{int_to_roman(diff)};
}

} // namespace solver
