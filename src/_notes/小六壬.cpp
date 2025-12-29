/*
TODO:农历转换
*/

import std;

struct NameWithInfo {
    std::string name;
    std::string info;

    NameWithInfo(std::string name, std::string info)
        : name(std::move(name)), info(std::move(info)) {}

    friend std::ostream& operator<<(std::ostream& stream, const NameWithInfo& val) {
        stream << val.name << "-(" << val.info << ")";
        return stream;
    }
};

std::array<std::string, 10> branch_heavenly = {
    "甲", "乙", "丙", "丁",
    "戊", "己", "庚", "辛",
    "壬", "癸"
}; // 天干

std::array<NameWithInfo, 12> branch_earthly = {{
    {"子", "鼠"}, {"丑", "牛"},
    {"寅", "虎"}, {"卯", "兔"},
    {"辰", "龙"}, {"巳", "蛇"},
    {"午", "马"}, {"未", "羊"},
    {"申", "猴"}, {"酉", "鸡"},
    {"戌", "狗"}, {"亥", "猪"}
}}; // 地支

std::array<NameWithInfo, 6> shen_tong_6 = {{
    {"大安", "静止, 心安, 吉祥之意"},
    {"流连", "晦暗不明, 拖延, 纠缠"},
    {"速喜", "快速, 喜庆, 时机已到"},
    {"赤口", "惊恐, 凶险, 口舌是非"},
    {"小吉", "家, 回归, 好运将到来"},
    {"空亡", "忧虑, 不吉, 没有结果"}
}}; // 掐指神通术的六神

int main() {
    std::array<int, 3> lunar_date;
    std::cout <<
R"(示例:
    阴历3月5日辰 (5) 时
    输入3 5 5
)";

    std::cout << "阴历日期:\n";
    std::cout << "    月: ";
    std::cin >> lunar_date[0];
    std::cout << "    日: ";
    std::cin >> lunar_date[1];
    std::cout << "    时: ";
    std::cin >> lunar_date[2];
    std::cout << shen_tong_6[
        // -1是为了使1能映射到shen_tong_6的第一项
        (lunar_date[0] + lunar_date[1] + lunar_date[2] - 2  -1) % 6
    ];
}
