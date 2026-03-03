#pragma once

#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <string>
#include <iomanip>
#include <sstream>
#include <concepts>

#include <ctime>

namespace chronoflux
{

const std::int64_t PICO_PER_SEC = 1'000'000'000'000LL; // 10^12
const std::int64_t PICO_PER_MSEC = 1'000'000'000LL; // 10^9
const std::int64_t PICO_PER_USEC = 1'000'000LL; // 10^6
const std::int64_t PICO_PER_NSEC = 1'000LL; // 10^3

class TimePoint
{
	private:
		std::int64_t seconds_;		// signed integer
		std::int64_t milliseconds_;	// 0 ～ 999
		std::int64_t microseconds_;	// 0 ～ 999
		std::int64_t nanoseconds_;	// 0 ～ 999
		std::int64_t picoseconds_;	// 0 ～ 999

	public:
		TimePoint();
		TimePoint(int, int, int, int, int, double);
		TimePoint(std::string, std::string);

		void parse(const std::string &, std::string);
		void normalize();
		void extractCalendarFields(int&, int&, int&, int&, int&, double&) const;
		template<std::integral Int> void addSeconds(Int);
		template<std::floating_point Real> void addSeconds(Real);
		template<std::integral Int> void addMinutes(Int);
		template<std::floating_point Real> void addMinutes(Real);
		template<std::integral Int> void addHours(Int);
		template<std::floating_point Real> void addHours(Real);
		template<std::integral Int> void addDays(Int);
		template<std::floating_point Real> void addDays(Real);
		template<std::integral Int> void addMilliseconds(Int);
		template<std::floating_point Real> void addMilliseconds(Real);
		template<std::integral Int> void addMicroseconds(Int);
		template<std::floating_point Real> void addMicroseconds(Real);
		template<std::integral Int> void addNanoseconds(Int);
		template<std::floating_point Real> void addNanoseconds(Real);
		template<std::integral Int> void addPicoseconds(Int);
		template<std::floating_point Real> void addPicoseconds(Real);
		void extractToStrings(std::string&, std::string&, std::string&, std::string&, std::string&, std::string&, std::string&, int) const;
		std::string format(std::string) const;
		bool isValidDateTime(int, int, int, int, int, double) const;
		std::int64_t seconds(void) const;
		std::int64_t milliseconds(void) const;
		std::int64_t microseconds(void) const;
		std::int64_t nanoseconds(void) const;
		std::int64_t picoseconds(void) const;
		template<std::integral Int> void seconds(Int);
		template<std::integral Int> void milliseconds(Int);
		template<std::integral Int> void microseconds(Int);
		template<std::integral Int> void nanoseconds(Int);
		template<std::integral Int> void picoseconds(Int);

		bool isLeapYear(int) const;
		int daysInMonth(int, int) const;

	private:
		std::int64_t days_from_epoch(int, unsigned, unsigned);
		bool isValidDateTimeString(const std::string&);
};

class Duration
{
	private:
		bool positive_ = true;

		std::int64_t seconds_;		// 0 ~
		std::int64_t milliseconds_;	// 0 ~ 999
		std::int64_t microseconds_;	// 0 ~ 999
		std::int64_t nanoseconds_;	// 0 ~ 999
		std::int64_t picoseconds_;	// 0 ~ 999

	public:
		Duration();
		Duration(std::int64_t, std::int64_t, std::int64_t, std::int64_t, std::int64_t);
		Duration(double);
		[[deprecated]] Duration(std::int64_t, int, int, double);
		Duration(std::string, std::string);

		void parse(std::string, std::string);
		void normalize(void);
		int extractFracWidth(std::string);
		std::string format(std::string);

		template<std::integral Int> void addSeconds(Int);
		template<std::floating_point Real> void addSeconds(Real);
		template<std::integral Int> void addMinutes(Int);
		template<std::floating_point Real> void addMinutes(Real);
		template<std::integral Int> void addHours(Int);
		template<std::floating_point Real> void addHours(Real);
		template<std::integral Int> void addDays(Int);
		template<std::floating_point Real> void addDays(Real);
		template<std::integral Int> void addMilliseconds(Int);
		template<std::floating_point Real> void addMilliseconds(Real);
		template<std::integral Int> void addMicroseconds(Int);
		template<std::floating_point Real> void addMicroseconds(Real);
		template<std::integral Int> void addNanoseconds(Int);
		template<std::floating_point Real> void addNanoseconds(Real);
		template<std::integral Int> void addPicoseconds(Int);
		template<std::floating_point Real> void addPicoseconds(Real);

		bool positive(void) const;
		std::int64_t seconds(void) const;
		std::int64_t milliseconds(void) const;
		std::int64_t microseconds(void) const;
		std::int64_t nanoseconds(void) const;
		std::int64_t picoseconds(void) const;

		void positive(bool);
		template<std::integral Int> void seconds(Int);
		template<std::integral Int> void milliseconds(Int);
		template<std::integral Int> void microseconds(Int);
		template<std::integral Int> void nanoseconds(Int);
		template<std::integral Int> void picoseconds(Int);

		Duration operator+(const Duration&) const;
		Duration operator-(const Duration&) const;
		double operator/(const Duration&) const;
		bool operator==(const Duration&) const;
		bool operator!=(const Duration&) const;
		bool operator<(const Duration&) const;
		bool operator<=(const Duration&) const;
		bool operator>(const Duration&) const;
		bool operator>=(const Duration&) const;
		double toTotalPicoseconds(void);
		double toTotalNanoseconds(void);
		double toTotalMicroseconds(void);
		double toTotalMilliseconds(void);
		double toTotalSeconds(void);
		double toTotalMinutes(void);
		double toTotalHours(void);
		double toTotalDays(void);
};

inline TimePoint::TimePoint()
{
	seconds_ = 0;
	milliseconds_ = 0;
	microseconds_ = 0;
	nanoseconds_ = 0;
	picoseconds_ = 0;
	return;
}

inline TimePoint::TimePoint(int year, int month, int day, int hour, int minute, double second_full)
{
	// std::cout << year << ", " << month << ", " << day << ", " << hour << ", " << minute << ", " << second_full << std::endl;
	if(!isValidDateTime(year, month, day, hour, minute, second_full))
	{
		throw std::invalid_argument("TimePoint: date or time out of valid range");
	}

	//─── (A) まず「年月日 → 通算日数」+「時:分:秒(int)」を整数演算 ───
	std::int64_t days = days_from_epoch(year, month, day);

	// 整数部の「時」「分」「秒」を秒数に換算
	int sec_int = static_cast<int>(std::floor(second_full));  // 秒の整数部（0 ≤ sec_int < 60 が保証されている前提）
	std::int64_t hms_sec = static_cast<std::int64_t>(hour) * 3600LL + static_cast<std::int64_t>(minute) * 60LL + static_cast<std::int64_t>(sec_int);

	// エポックからの通算「秒(整数)」を決定
	seconds_ = days * 86400LL + hms_sec;

	//─── (B) 小数秒部 → ピコ秒単位の整数 に丸め ───
	// 1 秒 = 1,000,000,000,000 ピコ秒
	double frac_sec = second_full - static_cast<double>(sec_int); 
	// frac_sec ∈ [0.0, 1.0)

	// 「最も近い整数ピコ秒」に四捨五入
	// 例: frac_sec = 0.345678901234 → frac_sec * 1e12 = 345678901234.0 → llround → 345678901234
	std::int64_t total_picosec = static_cast<std::int64_t>(std::llround(frac_sec * 1.0e12));

	// もし丸めでちょうど 1e12 ピコ秒 (= 1 秒分) を超えていたら、
	// その秒数分だけ sec_int（および seconds_）にキャリー
	if (total_picosec >= PICO_PER_SEC)
	{
		total_picosec -= PICO_PER_SEC;
		seconds_ += 1;
	}

	// ミリ秒部
	milliseconds_ = static_cast<std::int64_t>(total_picosec / PICO_PER_MSEC);
	std::int64_t rem = total_picosec % PICO_PER_MSEC;

	// マイクロ秒部
	microseconds_ = static_cast<std::int64_t>(rem / PICO_PER_USEC);
	rem %= PICO_PER_USEC;

	// ナノ秒部
	nanoseconds_ = static_cast<std::int64_t>(rem / PICO_PER_NSEC);
	rem %= PICO_PER_NSEC;

	// ピコ秒部（残り）
	picoseconds_ = static_cast<std::int64_t>(rem);

	//─── (D) 念のため normalize ───
	normalize();
}

inline TimePoint::TimePoint(std::string s, std::string fmt = "%Y/%m/%d %H:%M:%S.%f")
{
	// (1) 初期化: 各フィールドをゼロに
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, sec_int = 0;
	std::int64_t picosec = 0;

	size_t i_fmt = 0, i_s = 0;
	const size_t N_fmt = fmt.size(), N_s = s.size();

	// (2) フォーマット文字列を先頭から順に走査
	while (i_fmt < N_fmt)
	{
		if (fmt[i_fmt] != '%')
		{
			// リテラル文字：入力文字列 s でも同じ文字が並んでいなければ例外
			if (i_s >= N_s || s[i_s] != fmt[i_fmt])
				throw std::invalid_argument("TimePoint parse: literal mismatch");
			++i_fmt;
			++i_s;
		}
		else
		{
			// '%' から specifier の読み取り開始
			++i_fmt;
			if (i_fmt >= N_fmt)
				throw std::invalid_argument("TimePoint parse: '%' at end of format");

			// "%%" の場合はリテラル '%' をマッチさせる
			if (fmt[i_fmt] == '%')
			{
				if (i_s >= N_s || s[i_s] != '%')
					throw std::invalid_argument("TimePoint parse: expected '%'");
				++i_fmt;
				++i_s;
				continue;
			}

			// (a) まず幅指定（数字列）があるかどうかをチェック
			bool hasWidth = false;
			int width = 0;
			size_t j = i_fmt;
			while (j < N_fmt && std::isdigit(fmt[j]))
			{
				hasWidth = true;
				++j;
			}
			if (hasWidth)
			{
				width = std::stoi(fmt.substr(i_fmt, j - i_fmt));
				i_fmt = j;
			}
			if (i_fmt >= N_fmt)
				throw std::invalid_argument("TimePoint parse: missing specifier after width");

			char spec = fmt[i_fmt++];
			switch (spec)
			{
				case 'Y':
				{
					//–– 年 (符号＋数字)
					if (hasWidth)
					{
						// 幅だけ文字を読んで符号＋数字としてパース
						if (i_s + (size_t)width > N_s)
							throw std::invalid_argument("TimePoint parse: year width overflow");
						std::string token = s.substr(i_s, width);
						if (token.empty() ||
							(!std::isdigit(token[0]) && token[0] != '-'))
							throw std::invalid_argument("TimePoint parse: invalid year token");
						for (size_t k = 1; k < token.size(); ++k)
							if (!std::isdigit(token[k]))
								throw std::invalid_argument("TimePoint parse: invalid char in year");
						year = std::stoi(token);
						i_s += width;
					}
					else
					{
						// 自動幅：符号(任意)＋数字をリテラル直前まで読み取る
						size_t start = i_s;
						if (i_s < N_s && (s[i_s] == '+' || s[i_s] == '-'))
							++i_s;
						if (i_s >= N_s || !std::isdigit(s[i_s]))
							throw std::invalid_argument("TimePoint parse: expected digit in year");
						while (i_s < N_s && std::isdigit(s[i_s])) ++i_s;
						std::string token = s.substr(start, i_s - start);
						year = std::stoi(token);
					}
					break;
				}
				case 'm':  // –– 月
				case 'd':  // –– 日
				case 'H':  // –– 時
				case 'M':  // –– 分
				case 'S':  // –– 秒(整数)
				{
					int *field = nullptr;
					if (spec == 'm') field = &month;
					if (spec == 'd') field = &day;
					if (spec == 'H') field = &hour;
					if (spec == 'M') field = &minute;
					if (spec == 'S') field = &sec_int;

					if (hasWidth)
					{
						// 固定幅をそのまま数字として読み取り
						if (i_s + (size_t)width > N_s)
							throw std::invalid_argument("TimePoint parse: field width overflow");
						std::string token = s.substr(i_s, width);
						for (char c : token)
							if (!std::isdigit(c))
								throw std::invalid_argument("TimePoint parse: expected digit");
						*field = std::stoi(token);
						i_s += width;
					}
					else
					{
						// 自動幅：次のリテラルまで数字を読み取る
						if (i_s >= N_s || !std::isdigit(s[i_s]))
							throw std::invalid_argument("TimePoint parse: expected digit");
						size_t start = i_s;
						while (i_s < N_s && std::isdigit(s[i_s])) ++i_s;
						std::string token = s.substr(start, i_s - start);
						*field = std::stoi(token);
					}
					break;
				}
				case 'f':
				{
					//–– 小数秒(小数部：ピコ秒) の読み取り
					if (hasWidth)
					{
						if (width <= 12)
						{
							// 幅<=12桁：そのままゼロ埋して12桁にする
							if (i_s + (size_t)width > N_s)
								throw std::invalid_argument("TimePoint parse: frac width overflow");
							std::string token = s.substr(i_s, width);
							for (char c : token)
								if (!std::isdigit(c))
									throw std::invalid_argument("TimePoint parse: expected digit in fraction");
							i_s += width;

							int64_t val = 0;
							for (char c : token)
								val = val * 10 + (c - '0');
							for (int k = 0; k < 12 - width; ++k)
								val *= 10;
							picosec = val;
						}
						else
						{
							// 幅 >12 桁：先頭12桁を取り、13桁目以降で四捨五入
							if (i_s + (size_t)width > N_s)
								throw std::invalid_argument("TimePoint parse: frac width overflow");
							std::string token = s.substr(i_s, width);
							for (char c : token)
								if (!std::isdigit(c))
									throw std::invalid_argument("TimePoint parse: expected digit in fraction");
							i_s += width;

							// 先頭12桁と13桁目
							std::string head12 = token.substr(0, 12);
							char digit13 = token[12];
							int64_t pic = 0;
							for (char c : head12)
								pic = pic * 10 + (c - '0');

							if (digit13 >= '5')
							{
								pic += 1;
								if (pic >= 1000000000000LL)
								{
									// 12 桁が 999...999 +1 → 秒を繰り上げ、ピコ秒を0に
									pic = 0;
									sec_int++;
									if (sec_int >= 60)
									{
										// 秒が 60 以上 → 分へキャリー
										TimePoint tmp(year, month, day, hour, minute, static_cast<double>(sec_int - 1));
										tmp.addSeconds(1);  // 秒を１追加しつつ normalize() も通る
										double dummy;
										tmp.extractCalendarFields(year, month, day, hour, minute, dummy);
										sec_int = static_cast<int>(std::floor(dummy));
									}
								}
							}
							picosec = pic;
						}
					}
					else
					{
						// 自動幅：次のリテラルまで数字列を読み取り、12桁以下ならゼロ埋め、13桁以上なら四捨五入
						if (i_s >= N_s || !std::isdigit(s[i_s]))
							throw std::invalid_argument("TimePoint parse: expected digit in fraction");
						size_t start = i_s;
						while (i_s < N_s && std::isdigit(s[i_s])) ++i_s;
						std::string token = s.substr(start, i_s - start);
						if (token.empty())
							throw std::invalid_argument("TimePoint parse: empty fraction");

						if (token.size() <= 12)
						{
							// 12 桁以下 → 末尾をゼロ埋め
							int64_t val = 0;
							for (char c : token)
								val = val * 10 + (c - '0');
							for (size_t k = token.size(); k < 12; ++k)
								val *= 10;
							picosec = val;
						}
						else
						{
							// 13 桁以上 → 四捨五入
							std::string head12 = token.substr(0, 12);
							char digit13 = token[12];
							int64_t pic = 0;
							for (char c : head12)
								pic = pic * 10 + (c - '0');

							if (digit13 >= '5')
							{
								pic += 1;
								if (pic >= 1000000000000LL)
								{
									pic = 0;
									sec_int++;
									if (sec_int >= 60)
									{
										TimePoint tmp(year, month, day, hour, minute, static_cast<double>(sec_int - 1));
										tmp.addSeconds(1);
										double dummy;
										tmp.extractCalendarFields(year, month, day, hour, minute, dummy);
										sec_int = static_cast<int>(std::floor(dummy));
									}
								}
							}
							picosec = pic;
						}
					}
					break;
				}
				default:
					throw std::invalid_argument("TimePoint parse: unknown specifier");
			}
		}
	}
	// (3) 入力文字列を最後まで使い切ったかチェック
	if (i_s != N_s)
		throw std::invalid_argument("TimePoint parse: leftover characters");

	// (4) 秒＋小数部(double) にまとめる
	double frac_double = static_cast<double>(picosec) * 1e-12;
	double sec_full = static_cast<double>(sec_int) + frac_double;

	// (5) もうひとつのカレンダー→秒変換を使わずに、
	//     既存のコンストラクタを呼んでこの TimePoint を上書きする
	//std::cout << year << ", " << month << ", " << day << ", " << hour << ", " << minute << ", " << sec_full << std::endl;
	*this = TimePoint(year, month, day, hour, minute, sec_full);
	// std::cout << seconds_ << ", " << milliseconds_ << ", " << microseconds_ << ", " << nanoseconds_ << ", " << picoseconds_ << std::endl;
}

inline void TimePoint::parse(const std::string &s, std::string fmt = "%Y/%m/%d %H:%M:%S.%f")
{
	//std::cout << s << std::endl;
	TimePoint tp(s, fmt);

	seconds_ = tp.seconds_;
	milliseconds_ = tp.milliseconds_;
	microseconds_ = tp.microseconds_;
	nanoseconds_ = tp.nanoseconds_;
	picoseconds_ = tp.picoseconds_;

	return;
}

inline void TimePoint::normalize()
{
	//――――――――――――――――――――――――――――――――――――――――――
	// (1) 下位単位から上位単位へ 1000 倍ごとにキャリーを行う
	//    ・1,000 ps = 1 ns
	//    ・1,000 ns = 1 μs
	//    ・1,000 μs = 1 ms
	//    ・1,000 ms = 1 s
	//――――――――――――――――――――――――――――――――――――――――――

	// (1-1) picoseconds_ → nanoseconds
	if (picoseconds_ >= 1000 || picoseconds_ <= -1000)
	{
		std::int64_t carry_ns = picoseconds_ / 1000;
		nanoseconds_   += carry_ns;
		picoseconds_   -= carry_ns * 1000;
	}
	// もし負の余りがある場合
	if (picoseconds_ < 0)
	{
		nanoseconds_ -= 1;
		picoseconds_ += 1000;
	}
	// ここで picoseconds_ ∈ [0..999]

	// (1-2) nanoseconds → microseconds
	if (nanoseconds_ >= 1000 || nanoseconds_ <= -1000)
	{
		std::int64_t carry_us = nanoseconds_ / 1000;
		microseconds_ += carry_us;
		nanoseconds_  -= carry_us * 1000;
	}
	if (nanoseconds_ < 0)
	{
		microseconds_ -= 1;
		nanoseconds_  += 1000;
	}
	// ここで nanoseconds_ ∈ [0..999]

	// (1-3) microseconds_ → milliseconds_
	if (microseconds_ >= 1000 || microseconds_ <= -1000)
	{
		std::int64_t carry_ms = microseconds_ / 1000;
		milliseconds_ += carry_ms;
		microseconds_ -= carry_ms * 1000;
	}
	if (microseconds_ < 0)
	{
		milliseconds_ -= 1;
		microseconds_  += 1000;
	}
	// ここで microseconds ∈ [0..999]

	// (1-4) milliseconds → seconds
	if (milliseconds_ >= 1000 || milliseconds_ <= -1000)
	{
		std::int64_t carry_s = milliseconds_ / 1000;
		seconds_       += carry_s;
		milliseconds_  -= carry_s * 1000;
	}
	if (milliseconds_ < 0)
	{
		seconds_      -= 1;
		milliseconds_ += 1000;
	}
	// ここで milliseconds_ ∈ [0..999]
	// seconds_ は任意の整数（正負両対応）

	//――――――――――――――――――――――――――――――――――――――――――
	// (2) この時点で各フィールドは以下の範囲保証が成り立つ：
	//     picoseconds_  ∈ [0..999]
	//     nanoseconds_  ∈ [0..999]
	//     microseconds_ ∈ [0..999]
	//     milliseconds_ ∈ [0..999]
	//     seconds_      ∈ Z（整数全域）
	//――――――――――――――――――――――――――――――――――――――――――

	return;
}

inline void TimePoint::extractCalendarFields(int &year, int &month, int &day, int &hour, int &minute, double &second_full) const
{
	// 整数秒部を取り出す
	std::int64_t total_sec = seconds_;
	std::int64_t days = total_sec / 86400LL;
	std::int64_t rem_sec = total_sec % 86400LL;
	if (rem_sec < 0)
	{
		days -= 1;
		rem_sec = rem_sec + 86400LL;
	}

	hour   = static_cast<int>(rem_sec / 3600LL);
	rem_sec = rem_sec % 3600LL;
	minute = static_cast<int>(rem_sec / 60LL);
	int sec_int = static_cast<int>(rem_sec % 60LL);

	// days → 年/月/日（プロレプティブ・グレゴリオ暦）
	std::int64_t z = days - 60;  // 0000-03-01 を 0 日目 → 0000-01-01 を 0 日目に補正
	const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
	const std::int64_t doe = z - era * 146097;            // 0 ≤ doe < 146097
	const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	const std::int64_t y = yoe + era * 400;
	const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	const std::int64_t mp = (5 * doy + 2) / 153;
	day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
	month = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
	year = static_cast<int>(y + (mp < 10 ? 0 : 1));

	// 「整数秒 + ミリ/マイクロ/ナノ/ピコ秒」を小数秒にまとめる
	//
	//　　millisecond は 10^-3 秒単位、microsecond は 10^-6 秒単位、
	//　　nanosecond は 10^-9 秒単位、picosecond は 10^-12 秒単位
	//
	//　　double による近似誤差を最小限にするため、順序を高精度→低精度で足す：
	double frac_sec = static_cast<double>(milliseconds_) * 1e-3 + static_cast<double>(microseconds_) * 1e-6 + static_cast<double>(nanoseconds_) * 1e-9 + static_cast<double>(picoseconds_) * 1e-12;

	second_full = static_cast<double>(sec_int) + frac_sec;
}

template<std::integral Int> inline void TimePoint::addSeconds(Int delta_sec)
{
	seconds_ += static_cast<std::int64_t>(delta_sec);
	normalize();
}

template<std::floating_point Real> inline void TimePoint::addSeconds(Real delta_sec)
{
	std::int64_t int_part  = static_cast<std::int64_t>(std::floor(delta_sec));
	double frac_part = delta_sec - static_cast<double>(int_part);
	seconds_ += int_part;
	std::int64_t add_picoseconds = static_cast<std::int64_t>(std::llround(frac_part * static_cast<double>(PICO_PER_SEC)));
	picoseconds_ += add_picoseconds;
	normalize();
}

template<std::integral Int> inline void TimePoint::addMinutes(Int delta_min)
{
	addSeconds(static_cast<std::int64_t>(delta_min * 60LL));
}

template<std::floating_point Real> inline void TimePoint::addMinutes(Real delta_min)
{
	addSeconds(delta_min * 60.0);
}

template<std::integral Int> inline void TimePoint::addHours(Int delta_hour)
{
	addSeconds(static_cast<std::int64_t>(delta_hour * 3600LL));
}

template<std::floating_point Real> inline void TimePoint::addHours(Real delta_hour)
{
	addSeconds(delta_hour * 3600.0);
}

template<std::integral Int> inline void TimePoint::addDays(Int delta_days)
{
	addSeconds(static_cast<std::int64_t>(delta_days * 86400LL));
}

template<std::floating_point Real> inline void TimePoint::addDays(Real delta_days)
{
	addSeconds(delta_days * 86400.0);
}

template<std::integral Int> inline void TimePoint::addMilliseconds(Int delta_ms)
{
	milliseconds_ += static_cast<std::int64_t>(delta_ms);
	normalize();
}

template<std::floating_point Real> inline void TimePoint::addMilliseconds(Real delta_ms)
{
	std::int64_t int_ms = static_cast<std::int64_t>(std::floor(delta_ms));
	double frac_ms = delta_ms - static_cast<double>(int_ms);
	milliseconds_ += int_ms;
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_ms * static_cast<double>(PICO_PER_MSEC)));
	picoseconds_ += add_ps;
	normalize();
}

template<std::integral Int> inline void TimePoint::addMicroseconds(Int delta_us)
{
	microseconds_ += static_cast<std::int64_t>(delta_us);
	normalize();
}

template<std::floating_point Real> inline void TimePoint::addMicroseconds(Real delta_us)
{
	std::int64_t int_us = static_cast<std::int64_t>(std::floor(delta_us));
	double frac_us = delta_us - static_cast<double>(int_us);
	microseconds_ += int_us;
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_us * static_cast<double>(PICO_PER_USEC)));
	picoseconds_ += add_ps;
	normalize();
}

template<std::integral Int> inline void TimePoint::addNanoseconds(Int delta_ns)
{
	nanoseconds_ += static_cast<std::int64_t>(delta_ns);
	normalize();
}

template<std::floating_point Real> inline void TimePoint::addNanoseconds(Real delta_ns)
{
	std::int64_t int_ns = static_cast<std::int64_t>(std::floor(delta_ns));
	double frac_ns = delta_ns - static_cast<double>(int_ns);
	nanoseconds_ += int_ns;
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_ns * static_cast<double>(PICO_PER_NSEC)));
	picoseconds_ += add_ps;
	normalize();
}

template<std::integral Int> inline void TimePoint::addPicoseconds(Int delta_ps)
{
	picoseconds_ += static_cast<std::int64_t>(delta_ps);
	normalize();
}

template<std::floating_point Real> inline void TimePoint::addPicoseconds(Real delta_ps)
{
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(delta_ps));
	picoseconds_ += add_ps;
	normalize();
}

inline void TimePoint::extractToStrings(std::string &out_yyyy, std::string &out_mm, std::string &out_dd, std::string &out_hh, std::string &out_mi, std::string &out_ss, std::string &out_fraction, int fracDigits = 9) const
{
	// (1) fracDigits を 0..12 の範囲に丸める
	if (fracDigits < 0) fracDigits = 0;
	if (fracDigits > 12) fracDigits = 12;

	// (2) まず既存の数値版 extractCalendarFields を呼んで
	//     年, 月, 日, 時, 分, 秒(小数付き) を取得
	int year, month, day, hour, minute;
	double sec_full; // 0.0 <= sec_full < 60.0
	extractCalendarFields(year, month, day, hour, minute, sec_full);
	// std::cout << year << "/" << month << "/" << day << " " << hour << ":" << minute << ":" << sec_full << std::endl;

	// (3) 整数秒部と小数秒部に分離
	std::int64_t sec_int = static_cast<std::int64_t>(std::floor(sec_full)); // 0..59
	double sec_frac = sec_full - static_cast<double>(sec_int);        // [0,1)
	// std::cout << sec_int << ", " << sec_frac << std::endl;

	// (4) 合計ピコ秒 (0 .. 10^12-1) を計算
	//     → (ミリ秒×10^9) + (マイクロ秒×10^6) + (ナノ秒×10^3) + (ピコ秒)
	std::int64_t frac_pico = 
		static_cast<std::int64_t>(milliseconds_) * PICO_PER_MSEC
		+ static_cast<std::int64_t>(microseconds_) * PICO_PER_USEC
		+ static_cast<std::int64_t>(nanoseconds_)  * PICO_PER_NSEC
		+ static_cast<std::int64_t>(picoseconds_); // 0..10^12-1
	// std::cout << frac_pico << std::endl;

	// (5) 「小数秒(ピコ秒)」を fracDigits 桁へ丸めつつキャリーを処理する
	std::int64_t pow10 = 1;
	for (int i = 0; i < 12 - fracDigits; ++i)
	{
		pow10 *= 10; // 10^(12 - fracDigits)
	}
	// 四捨五入
	std::int64_t rounded = (frac_pico + pow10 / 2) / pow10; // 0..10^fracDigits
	// std::cout << pow10 << ", " << rounded << std::endl;
	if (rounded == static_cast<std::int64_t>(std::pow(10, fracDigits)))
	{
		// 例: 小数部が 0.9995… で四捨五入してちょうど 1.000… になった場合
		rounded = 0;
		++sec_int;
	}

	// (6) sec_int が 60 以上になるなら、分→時→日にキャリー
	int minute_c = minute;
	int hour_c   = hour;
	int day_c    = day;
	int month_c  = month;
	int year_c   = year;

	if (sec_int >= 60)
	{
		sec_int -= 60;
		++minute_c;

		if (minute_c >= 60)
		{
			minute_c -= 60;
			++hour_c;

			if (hour_c >= 24)
			{
				hour_c -= 24;
				// 日付キャリーは一旦 TimePoint を使って正確に計算
				TimePoint tmp_tp(year_c, month_c, day_c, hour_c, minute_c, static_cast<double>(sec_int));
				tmp_tp.addDays(1);
				double dummy;
				tmp_tp.extractCalendarFields(year_c, month_c, day_c, hour_c, minute_c, dummy);
				sec_int = static_cast<std::int64_t>(std::floor(dummy)); // dummy == 0.0 想定
			}
		}
	}

	// (7) 年 → 文字列化 (4 桁ゼロ埋 or 5 桁以上はそのまま)
	{
		std::ostringstream oss;
		int abs_y = (year_c < 0 ? -year_c : year_c);
		if (abs_y <= 9999)
		{
			// 絶対値が 0 .. 9999 なら 4 桁ゼロ埋
			if (year_c < 0)
			{
				oss << '-';
			}
			oss << std::setw(4) << std::setfill('0') << abs_y;
		}
		else
		{
			// 5 桁以上は符号含めそのまま
			oss << year_c;
		}
		out_yyyy = oss.str();
	}

	// (8) 月 (2 桁ゼロ埋)
	{
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << month_c;
		out_mm = oss.str();
	}

	// (9) 日 (2 桁ゼロ埋)
	{
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << day_c;
		out_dd = oss.str();
	}

	// (10) 時 (2 桁ゼロ埋)
	{
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << hour_c;
		out_hh = oss.str();
	}

	// (11) 分 (2 桁ゼロ埋)
	{
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << minute_c;
		out_mi = oss.str();
	}

	// (12) 秒整数部 (2 桁ゼロ埋)
	{
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << sec_int;
		out_ss = oss.str();
	}

	// (13) 小数秒部 (fracDigits 桁ゼロ埋) を文字列化して out_fraction にセット
	if (fracDigits > 0)
	{
		// rounded ∈ [0..10^fracDigits - 1]
		std::ostringstream oss;
		oss << std::setw(fracDigits) << std::setfill('0') << rounded;
		out_fraction = oss.str();
	}
	else
	{
		out_fraction.clear();
	}
}

inline std::string TimePoint::format(std::string fmt = "%Y/%m/%d %2H:%2M:%2S") const
{
	//――――――――――――――――――――――――――――――――――――――――――
	// (A) まず "%<digits>f" を探して frac_width を決める
	//――――――――――――――――――――――――――――――――――――――――――
	int frac_width = 0;
	bool is_fraction = false;
	for (size_t i = 0; i + 1 < fmt.size(); ++i)
	{
		if (fmt[i] == '%')
		{
			is_fraction = true;
			if(std::isdigit(fmt[i + 1]))
			{
				size_t j = i + 1;
				while (j < fmt.size() && std::isdigit(fmt[j])) ++j;
				if (j < fmt.size() && fmt[j] == 'f')
				{
					//is_fraction = true;
					frac_width = std::stoi(fmt.substr(i + 1, j - (i + 1)));
					i = j; // "%...f" 部分を飛ばす
				}
			}
		}
	}

	//std::cout << frac_width << ", " << is_fraction << std::endl;

	std::string yyyy_str, mm_str, dd_str, hh_str, mi_str, ss_str, fraction_str;
	if (frac_width == 0)
	{
		if (is_fraction)
		{
			// %f はあるが幅指定なし → まず 12 桁を取り、あとで末尾ゼロ削除する
			extractToStrings(yyyy_str, mm_str, dd_str, hh_str, mi_str, ss_str, fraction_str, 12);
			//std::cout << fraction_str << std::endl;
		}
		else
		{
			// %f も書かれていない → 小数部は不要
			extractToStrings(yyyy_str, mm_str, dd_str, hh_str, mi_str, ss_str, fraction_str, 0);
		}
	}
	else
	{
		// 幅指定あり (例: "%3f" や "%5f")
		extractToStrings(yyyy_str, mm_str, dd_str, hh_str, mi_str, ss_str, fraction_str, frac_width);
	}

	// 数字の先頭に不要なゼロが入っているケースを一度 int→string で揃える
	yyyy_str = std::to_string(std::stoi(yyyy_str));
	mm_str   = std::to_string(std::stoi(mm_str));
	dd_str   = std::to_string(std::stoi(dd_str));
	hh_str   = std::to_string(std::stoi(hh_str));
	mi_str   = std::to_string(std::stoi(mi_str));
	ss_str   = std::to_string(std::stoi(ss_str));

	//――――――――――――――――――――――――――――――――――――――――――
	// (B) フォーマット文字列を再度走査しながら置換していく
	//――――――――――――――――――――――――――――――――――――――――――
	std::string out;
	out.reserve(fmt.size() + 32);

	for (size_t i = 0; i < fmt.size(); ++i)
	{
		if (fmt[i] == '%')
		{
			// "%%" → '%'
			if (i + 1 < fmt.size() && fmt[i + 1] == '%')
			{
				out.push_back('%');
				++i;
				continue;
			}

			// '%' のあとに幅指定（数字列）があるかチェック
			++i;
			int width = 0;
			bool hasWidth = false;
			size_t j = i;
			while (j < fmt.size() && std::isdigit(fmt[j]))
			{
				hasWidth = true;
				++j;
			}
			if (hasWidth)
			{
				// 幅指定を整数化
				width = std::stoi(fmt.substr(i, j - i));
				i = j;
			}
			if (i >= fmt.size()) break;

			// specごとに対応文字列を生成
			switch (fmt[i])
			{
				case 'Y':
				{
					// 年
					if (hasWidth)
					{
						bool negative = (yyyy_str[0] == '-');
						std::string body = negative ? yyyy_str.substr(1) : yyyy_str;
						if ((int)body.size() < width)
						{
							body = std::string(width - body.size(), '0') + body;
						}
						yyyy_str = negative ? "-" + body : body;
					}
					out += yyyy_str;
					break;
				}
				case 'm':
				{
					// 月
					if (hasWidth && (int)mm_str.size() < width)
					{
						mm_str = std::string(width - mm_str.size(), '0') + mm_str;
					}
					out += mm_str;
					break;
				}
				case 'd':
				{
					// 日
					if (hasWidth && (int)dd_str.size() < width)
					{
						dd_str = std::string(width - dd_str.size(), '0') + dd_str;
					}
					out += dd_str;
					break;
				}
				case 'H':
				{
					// 時
					if (hasWidth && (int)hh_str.size() < width)
					{
						hh_str = std::string(width - hh_str.size(), '0') + hh_str;
					}
					out += hh_str;
					break;
				}
				case 'M':
				{
					// 分
					if (hasWidth && (int)mi_str.size() < width)
					{
						mi_str = std::string(width - mi_str.size(), '0') + mi_str;
					}
					out += mi_str;
					break;
				}
				case 'S':
				{
					// 秒（整数部）
					if (hasWidth && (int)ss_str.size() < width)
					{
						ss_str = std::string(width - ss_str.size(), '0') + ss_str;
					}
					out += ss_str;
					break;
				}
				case 'f':
				{
					// 小数部
					if (frac_width == 0)
					{
						// 「%f」だが幅指定なし → 末尾ゼロを削る
						while (!fraction_str.empty() && fraction_str.back() == '0')
							fraction_str.pop_back();
						if (!fraction_str.empty())
							out += fraction_str;
						else
							out.push_back('0');
					}
					else
					{
						// 幅指定あり（例: "%3f" など） → すでに extractToStrings でゼロ埋め済み
						std::ostringstream oss;
						oss << std::setw(frac_width) << std::setfill('0') << fraction_str;
						out += oss.str();
					}
					break;
				}
				default:
				{
					// 未知の "%X" → "%X" のまま出力
					out.push_back('%');
					out.push_back(fmt[i]);
					break;
				}
			}
		}
		else
		{
			// 普通の文字はそのまま出力
			out.push_back(fmt[i]);
		}
	}

	return out;
}

inline bool TimePoint::isValidDateTime(int y, int m, int d, int h, int mi, double sec_full) const
{
	// 1) 月
	if (m < 1 || m > 12) return false;

	// 2) 日
	int dim = daysInMonth(y, m);
	if (d < 1 || d > dim) return false;

	// 3) 時間
	if (h < 0 || h > 23) return false;

	// 4) 分
	if (mi < 0 || mi > 59) return false;

	// 5) 秒 (整数部 0..59, 小数部 0.0≦x<1.0)
	if (!(sec_full >= 0.0 && sec_full < 60.0)) return false;

	return true;
}

inline std::int64_t TimePoint::seconds(void) const
{
	return seconds_;
}

inline std::int64_t TimePoint::milliseconds(void) const
{
	return milliseconds_;
}

inline std::int64_t TimePoint::microseconds(void) const
{
	return microseconds_;
}

inline std::int64_t TimePoint::nanoseconds(void) const
{
	return nanoseconds_;
}

inline std::int64_t TimePoint::picoseconds(void) const
{
	return picoseconds_;
}

template<std::integral Int> inline void TimePoint::seconds(Int sec)
{
	seconds_ = sec;
	return;
}

template<std::integral Int> inline void TimePoint::milliseconds(Int millisec)
{
	milliseconds_ = millisec;
	return;
}

template<std::integral Int> inline void TimePoint::microseconds(Int microsec)
{
	microseconds_ = microsec;
	return;
}

template<std::integral Int> inline void TimePoint::nanoseconds(Int nanosec)
{
	nanoseconds_ = nanosec;
	return;
}

template<std::integral Int> inline void TimePoint::picoseconds(Int picosec)
{
	picoseconds_ = picosec;
	return;
}

inline std::int64_t TimePoint::days_from_epoch(int y, unsigned m, unsigned d)
{
	if (m <= 2)
	{
		y -= 1;
		m += 12;
	}

	const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
	const unsigned yoe = static_cast<unsigned>(y - static_cast<int>(era * 400));
	const unsigned doy = (153 * (m - 3) + 2) / 5 + (d - 1);
	const std::int64_t doe = static_cast<std::int64_t>(yoe) * 365 + static_cast<std::int64_t>(yoe / 4) - static_cast<std::int64_t>(yoe / 100) + static_cast<std::int64_t>(doy);

	//std::cout << era * 146097 + doe + 60 << " days from 0000-01-01 00:00:00" << std::endl;

	return era * 146097 + doe + 60;
}

inline bool TimePoint::isLeapYear(int yy) const
{
	if ((yy % 4 == 0 && yy % 100 != 0) || (yy % 400 == 0))
	{
		return true;
	}
	return false;
}

inline int TimePoint::daysInMonth(int yy, int mo) const
{
	switch (mo)
	{
		case 1:  case 3:  case 5:  case 7:
		case 8:  case 10: case 12:
			return 31;
		case 4: case 6: case 9: case 11:
			return 30;
		case 2:
			return (isLeapYear(yy) ? 29 : 28);
		default:
			return 0;
	}
}

inline bool TimePoint::isValidDateTimeString(const std::string &s)
{
	// 最低でも "YYYY-MM-DD HH:MM:SS" は長さ19文字
	if (s.size() < 19) return false;

	int yy, mo, dd, hh, mi;
	char c1, c2, c3, c4, c5;
	std::istringstream iss(s);
	iss >> std::noskipws;

	// "YYYY-MM-DD HH:MM:" 部分を読み込む
	iss >> yy >> c1 >> mo >> c2 >> dd >> c3 >> hh >> c4 >> mi >> c5;
	//std::cout << yy << ", " << c1 << ", " << mo << ", " << c2 << ", " << dd << ", " << c3 << ", " << hh << ", " << c4 << ", " << mi << ", " << c5 << std::endl;
	if (iss.fail() || c1 != '-' || c2 != '-' || c3 != ' ' || c4 != ':' || c5 != ':')
	{
		return false;
	}

	// 残りを秒の文字列として読み取り (例 "56.123456" or "56")
	double sec_full;
	if (!(iss >> sec_full)) return false;

	// 秒が数字として OK でも余分な文字が残っていないか確認
	//   例: "12:34:56abc" のように余分な文字があれば無効
	if (!iss.eof()) return false;

	// 上で取得した数値で日時として有効かチェック
	return isValidDateTime(yy, mo, dd, hh, mi, sec_full);
}

inline Duration::Duration()
{
	seconds_ = 0;
	milliseconds_ = 0;
	microseconds_ = 0;
	nanoseconds_ = 0;
	picoseconds_ = 0;

	return;
}

inline Duration::Duration(std::int64_t sec, std::int64_t millisec, std::int64_t microsec, std::int64_t nanosec, std::int64_t picosec)
{
	seconds_ = sec;
	milliseconds_ = millisec;
	microseconds_ = microsec;
	nanoseconds_ = nanosec;
	picoseconds_ = picosec;

	normalize();
}

inline Duration::Duration(double seconds_full)
{
	// std::cout << static_cast<std::int64_t>(std::llround(seconds_full * double(PICO_PER_SEC))) << std::endl;
	
	if(seconds_full < 0.0)
	{
		positive_ = false;
		seconds_full *= -1.0;
	}
	else
	{
		positive_ = true;
	}

	std::int64_t sec_int = static_cast<std::int64_t>(std::trunc(seconds_full));
	double frac_sec = seconds_full - static_cast<double>(sec_int);

	//std::cout << static_cast<std::int64_t>(frac_sec * 1.0E14) << std::endl;

	seconds_ = sec_int;
	picoseconds_ = static_cast<std::int64_t>(std::llround(frac_sec * double(PICO_PER_SEC)));
	milliseconds_ = 0;
	microseconds_ = 0;
	nanoseconds_ = 0;

	normalize();
}

inline Duration::Duration(std::int64_t days, int hours, int minutes, double seconds_full)
{
	//――――――――――――――――――――――――――――――――――
	// 1) 「日→秒」「時→秒」「分→秒」を整数演算で加算
	//――――――――――――――――――――――――――――――――――
	seconds_ = days * 86400LL;
	seconds_ += static_cast<std::int64_t>(hours)   * 3600LL;
	seconds_ += static_cast<std::int64_t>(minutes) *   60LL;

	//――――――――――――――――――――――――――――――――――
	// 2) seconds_full を整数秒部と小数秒部に分離
	//――――――――――――――――――――――――――――――――――
	std::int64_t sec_int = static_cast<std::int64_t>(std::floor(seconds_full));
	double frac_sec = seconds_full - static_cast<double>(sec_int);

	// (整数秒部) を加算
	seconds_ += sec_int;

	//――――――――――――――――――――――――――――――――――
	// 3) 小数秒部 → ピコ秒単位の整数 に丸め
	//    （1 秒 = 10^12 ピコ秒）
	//――――――――――――――――――――――――――――――――――
	// (a) 小数秒部を「最も近い整数ピコ秒」に丸める
	std::int64_t total_picosec = static_cast<std::int64_t>(std::llround(frac_sec * 1e12));

	// (b) もし丸めでちょうど 1 秒分 (=10^12 ピコ秒) を超えていたら、
	//     1 秒を繰り上げ、残りを再設定する
	if (total_picosec >= PICO_PER_SEC)
	{
		total_picosec -= PICO_PER_SEC;
		seconds_ += 1;
	}

	//――――――――――――――――――――――――――――――――――
	// 4) 「total_picosec (0 ～ 10^12-1)」を
	//    ミリ/マイクロ/ナノ/ピコ に分解して格納
	//――――――――――――――――――――――――――――――――――
	// ミリ秒部 (0..999)
	milliseconds_ = static_cast<std::int64_t>( total_picosec / PICO_PER_MSEC );
	std::int64_t rem = total_picosec % PICO_PER_MSEC;

	// マイクロ秒部 (0..999)
	microseconds_ = static_cast<std::int64_t>( rem / PICO_PER_USEC );
	rem %= PICO_PER_USEC;

	// ナノ秒部   (0..999)
	nanoseconds_ = static_cast<std::int64_t>( rem / PICO_PER_NSEC );
	rem %= PICO_PER_NSEC;

	// ピコ秒部   (0..999)
	picoseconds_ = static_cast<std::int64_t>( rem );

	//――――――――――――――――――――――――――――――――――
	// 5) 各フィールドは範囲内になっているので正規化は不要
	//    （念のため normalize() を呼んでもかまわない）
	//――――――――――――――――――――――――――――――――――
	normalize();  // 必要に応じて呼ぶ
}

inline Duration::Duration(std::string s, std::string fmt = "%+ %d-%2H:%2M:%2S")
{
	//std::cout << s << std::endl;
	// (1) 初期化
	seconds_ = 0;
	milliseconds_ = 0;
	microseconds_ = 0;
	nanoseconds_ = 0;
	picoseconds_ = 0;
	positive_    = true;

	size_t i_fmt = 0;   // fmt のインデックス
	size_t i_s   = 0;   // s のインデックス

	while (i_fmt < fmt.size())
	{
		if (fmt[i_fmt] != '%')
		{
			// リテラル文字にマッチさせる
			if (i_s >= s.size() || s[i_s] != fmt[i_fmt])
			{
				throw std::invalid_argument("Duration parse: literal mismatch at `" + std::string(1, fmt[i_fmt]) + "`");
			}
			++i_fmt;
			++i_s;
		}
		else
		{
			// '%' を発見 → 次の文字を見に行く
			++i_fmt;
			if (i_fmt >= fmt.size())
			{
				throw std::invalid_argument("Duration parse: '%' at end of format");
			}

			// --- ① 符号指定子 "%+" の場合 ---
			if (fmt[i_fmt] == '+')
			{
				// 文字列 s の先頭に '+' か '-' があるか確認する
				if (i_s < s.size() && (s[i_s] == '+' || s[i_s] == '-'))
				{
					if (s[i_s] == '-')
					{
						positive_ = false;
					}
					// '+' だったら特に何もしない（+1 のまま）
					++i_s;
				}
				// もし s[i_s] に符号がなければ、そのまま正(+1)とみなす
				++i_fmt;
			}
			else
			{
				// --- ② 幅指定（例: "%2H" など）があるか探す ---
				bool hasWidth = false;
				int width = 0;
				size_t j = i_fmt;

				while (j < fmt.size() && std::isdigit(fmt[j]))
				{
					hasWidth = true;
					width = width * 10 + (fmt[j] - '0');
					++j;
				}
				if (hasWidth)
				{
					i_fmt = j;

					if (i_fmt >= fmt.size())
					{
						throw std::invalid_argument("Duration parse: missing specifier after width");
					}
				}

				char spec = fmt[i_fmt++];

				switch (spec)
				{
					case 'd':
					{
						// 幅指定があるならその文字数だけ読み込み
						if (hasWidth)
						{
							if (i_s + width > s.size())
							{
								throw std::invalid_argument("Duration parse: day width overflow");
							}
							std::string token = s.substr(i_s, width);
							// 数字チェック
							for (char c : token)
							{
								if (!std::isdigit(c))
								{
									throw std::invalid_argument("Duration parse: invalid day digits");
								}
							}

							std::int64_t days = std::stoll(token);
							seconds_ += days * 86400LL;
							i_s += width;
						}
						else
						{
							// 幅指定なしなら区切り文字までの数字をすべて読む
							size_t start = i_s;
							while (i_s < s.size() && std::isdigit(s[i_s]))
							{
								++i_s;
							}
							if (start == i_s)
							{
								throw std::invalid_argument("Duration parse: expected digits for days");
							}

							std::int64_t days = std::stoll(s.substr(start, i_s - start));
							seconds_ += days * 86400LL;

							//std::cout << days << std::endl;
						}
						break;
					}
					case 'H':
					{
						if (hasWidth)
						{
							if (i_s + width > s.size())
							{
								throw std::invalid_argument("Duration parse: hour width overflow");
							}

							std::string token = s.substr(i_s, width);

							for (char c : token)
							{
								if (!std::isdigit(c))
								{
									throw std::invalid_argument("Duration parse: invalid hour digits");
								}
							}

							int hour = std::stoi(token);
							seconds_ += static_cast<std::int64_t>(hour) * 3600LL;
							i_s += width;
						}
						else
						{
							// 幅指定なしなら区切り文字までの数字をすべて読む
							size_t start = i_s;
							while (i_s < s.size() && std::isdigit(s[i_s]))
							{
								++i_s;
							}

							if (start == i_s)
							{
								throw std::invalid_argument("Duration parse: expected digits for hour");
							}

							int hour = std::stoi(s.substr(start, i_s - start));
							seconds_ += static_cast<std::int64_t>(hour) * 3600LL;
						}
						break;
					}
					case 'M':
					{
						if (hasWidth)
						{
							if (i_s + width > s.size())
							{
								throw std::invalid_argument("Duration parse: minute width overflow");
							}

							std::string token = s.substr(i_s, width);

							for (char c : token)
							{
								if (!std::isdigit(c))
								{
									throw std::invalid_argument("Duration parse: invalid minute digits");
								}
							}

							int minute = std::stoi(token);
							seconds_ += static_cast<std::int64_t>(minute) * 60LL;
							i_s += width;
						}
						else
						{
							size_t start = i_s;
							while (i_s < s.size() && std::isdigit(s[i_s]))
							{
								++i_s;
							}

							if (start == i_s)
							{
								throw std::invalid_argument("Duration parse: expected digits for minute");
							}

							int minute = std::stoi(s.substr(start, i_s - start));
							seconds_ += static_cast<std::int64_t>(minute) * 60LL;
						}
						break;
					}
					case 'S':
					{
						if (hasWidth)
						{
							if (i_s + width > s.size())
							{
								throw std::invalid_argument("Duration parse: second width overflow");
							}

							std::string token = s.substr(i_s, width);

							for (char c : token)
							{
								if (!std::isdigit(c))
								{
									throw std::invalid_argument("Duration parse: invalid second digits");
								}
							}

							int sec_int = std::stoi(token);
							seconds_ += static_cast<std::int64_t>(sec_int);
							i_s += width;
						}
						else
						{
							size_t start = i_s;
							while (i_s < s.size() && std::isdigit(s[i_s]))
							{
								++i_s;
							}

							if (start == i_s)
							{
								throw std::invalid_argument("Duration parse: expected digits for second");
							}

							int sec_int = std::stoi(s.substr(start, i_s - start));
							seconds_ += static_cast<std::int64_t>(sec_int);
						}
						break;
					}
					case 'f':
					{
						// 「%f」の場合は、幅指定にかかわらず
						// s[i_s] 以降の数字をリテラルまで読み切る
						if (i_s >= s.size() || !std::isdigit(s[i_s]))
						{
							throw std::invalid_argument("Duration parse: expected digit for fraction");
						}
						// 次のフォーマット文字（リテラル）を探すためのインデックス
						// fmt[i_fmt] が次に来る「リテラル文字」または '%' の位置を想定
						// しかしここでは簡単化して、「文字列 s の次に数字以外が来たところまでを分数部とみなす」
						size_t start = i_s;
						while (i_s < s.size() && std::isdigit(s[i_s]))
						{
							++i_s;
						}
						std::string token = s.substr(start, i_s - start);
						// token には「連続する数字」が入っている
						// 例: "12345" → 0.12345 秒 → ピコ秒に変換 etc.

						// 今回の設計では「小数点以下をピコ秒単位で保持する」ので、
						// token を 12 桁に満たすよう右側をゼロ埋め or 13 桁以上なら四捨五入して12桁にする処理をします。
						// 例：token="123" → "123000000000"
						//     token="1234567890123" → "123456789012" ＋ アップ／四捨五入


						std::int64_t total_ps;
						// std::cout << token.size() << std::endl;

						if (token.size() > 12)
						{
							bool roundUp = (token[12] >= '5');
							token.resize(12);
							total_ps = std::stoll(token);
							
							if (roundUp)
							{
								total_ps ++;
							}
						}
						else if (token.size() <= 12)
						{
							token.append(12 - token.size(), '0');
							total_ps = std::stoll(token);
						}
						
						picoseconds_ = total_ps;
						// ここで token は必ず長さ 12 の数字文字列
						break;
					}
					default:
						// もし未定義の "%X" が来たらエラー
						throw std::invalid_argument(std::string("Duration parse: unknown specifier `%") + spec + "`");
				}
			}
		}
	}

	normalize();
}

inline void Duration::parse(std::string s, std::string fmt = "%+ %d-%2H:%2M:%2S")
{
	Duration dr(s, fmt);

	positive_ = dr.positive_;
	seconds_ = dr.seconds_;
	milliseconds_ = dr.milliseconds_;
	microseconds_ = dr.microseconds_;
	nanoseconds_ = dr.nanoseconds_;
	picoseconds_ = dr.picoseconds_;

	return;
}

inline void Duration::normalize(void)
{
	//――――――――――――――――――――――――――――――――――――――――――
	// （1）まず、ピコ秒→ナノ秒→マイクロ秒→ミリ秒→秒　の順にキャリーしていく
	//――――――――――――――――――――――――――――――――――――――――――

	// (1-1) picoseconds_ → nanoseconds にキャリー
	//    1000 ps = 1 ns
	if (picoseconds_ >= 1000 || picoseconds_ <= -1000)
	{
		std::int64_t carry_ns = picoseconds_ / 1000;
		nanoseconds_   += carry_ns;
		picoseconds_   -= carry_ns * 1000;
	}
	// ここで picoseconds_ は -999 .. +999 の範囲になる
	// もし負の余りが出たら、たとえば picoseconds_ = -5 のとき → nanoseconds -= 1, picoseconds_ += 1000 のように補正
	if (picoseconds_ < 0)
	{
		nanoseconds_   -= 1;
		picoseconds_   += 1000;
	}

	// (1-2) nanoseconds → microseconds にキャリー
	//    1000 ns = 1 μs
	if (nanoseconds_ >= 1000 || nanoseconds_ <= -1000)
	{
		std::int64_t carry_us = nanoseconds_ / 1000;
		microseconds_ += carry_us;
		nanoseconds_  -= carry_us * 1000;
	}
	if (nanoseconds_ < 0)
	{
		microseconds_ += -1;
		nanoseconds_  += 1000;
	}

	// (1-3) microseconds_ → milliseconds_ にキャリー
	//    1000 μs = 1 ms
	if (microseconds_ >= 1000 || microseconds_ <= -1000)
	{
		std::int64_t carry_ms = microseconds_ / 1000;
		milliseconds_ += carry_ms;
		microseconds_ -= carry_ms * 1000;
	}
	if (microseconds_ < 0)
	{
		milliseconds_ += -1;
		microseconds_  += 1000;
	}

	// (1-4) milliseconds_ → seconds_ にキャリー
	//    1000 ms = 1 s
	if (milliseconds_ >= 1000 || milliseconds_ <= -1000)
	{
		std::int64_t carry_s = milliseconds_ / 1000;
		seconds_       += carry_s;
		milliseconds_  -= carry_s * 1000;
	}
	if (milliseconds_ < 0)
	{
		seconds_      += -1;
		milliseconds_ += 1000;
	}

	// ここまでで例えば -2.3s -> -3s + 700ms, -0.7s -> -1s + 300msなどとなっている
	if(seconds_ < 0)
	{
		if(positive_ == false)
		{
			positive_ = true;
		}
		else
		{
			positive_ = false;
		}

		seconds_ *= -1;
		seconds_ -= 1;

		std::int64_t total_ps = milliseconds_ * PICO_PER_MSEC + microseconds_ * PICO_PER_USEC + nanoseconds_ * PICO_PER_NSEC + picoseconds_;
		total_ps = PICO_PER_SEC - total_ps;

		milliseconds_ = 0;
		microseconds_ = 0;
		nanoseconds_ = 0;
		picoseconds_ = total_ps;

		normalize();
	}

	if(seconds_ == 0 && milliseconds_ == 0 && microseconds_ == 0 && nanoseconds_ == 0 && picoseconds_ == 0)
	{
		positive_ = true;
	}

	//std::cout << positive_ << std::endl;

	// これで-3s + 700ms -> -(2s + 300ms), -1s + 300ms -> -(0s + 700ms)となるはず
}

inline int Duration::extractFracWidth(std::string fmt)
{
	const size_t N = fmt.size();
	for (size_t i = 0; i + 1 < N; ++i)
	{
		if (fmt[i] == '%')
		{
			size_t j = i + 1;
			// まず「幅指定の数字列」が続いているかチェック
			int width = 0;
			bool hasWidth = false;
			while (j < N && std::isdigit((unsigned char)fmt[j]))
			{
				hasWidth = true;
				width = width * 10 + (fmt[j] - '0');
				++j;
			}
			// 次の文字が 'f' なら、"%<digits>f" or "%f" にマッチ
			if (j < N && fmt[j] == 'f')
			{
				if (hasWidth)
				{
					return width;
				}
				else
				{
					// 幅指定なしの %f を「12桁」とみなしたいならここで 12 を返す
					// 幅指定無しは「可変長で末尾ゼロ削除」などの実装をしたい場合は 0 を返して
					// 呼び出し側で「hasWidth==false → special handling」としてもOKです。
					return 0;
				}
			}
			// もし "%+" のような別のspecifier の場合はスキップして next へ
		}
	}
	// %f が見つからなかったら 0 を返す
	return 0;
}

inline std::string Duration::format(std::string fmt = "%+(%d-%2H:%2M:%2S.%2f)")
{
	// (B) 符号文字列を準備
	

	int frac_width = extractFracWidth(fmt);
	std::int64_t total_ps = milliseconds_ * PICO_PER_MSEC + microseconds_ * PICO_PER_USEC + nanoseconds_ * PICO_PER_NSEC + picoseconds_;
	total_ps = std::llround(total_ps / std::pow(10, 12 - (frac_width==0?12:frac_width))) * std::pow(10, 12 - (frac_width==0?12:frac_width));
	//std::cout << total_ps << std::endl;
	Duration temp;
	temp.positive_ = positive_;
	temp.seconds_ = seconds_;
	temp.milliseconds_ = 0;
	temp.microseconds_ = 0;
	temp.nanoseconds_ = 0;
	temp.picoseconds_ = total_ps;
	temp.normalize();
	total_ps = temp.milliseconds_ * PICO_PER_MSEC + temp.microseconds_ * PICO_PER_USEC + temp.nanoseconds_ * PICO_PER_NSEC + temp.picoseconds_;
	// (C) 小数部をまず 12 桁の文字列（ゼロ埋め）で生成し、末尾不要ゼロは残しておく
	std::ostringstream ofs_frac12;
	ofs_frac12 << std::setw(12) << std::setfill('0') << total_ps;
	std::string frac12 = ofs_frac12.str();  // 例: "003000000000" など
	// ただし、ここでは「format 中の %f」については末尾ゼロは削らず、
	// 幅指定に応じて必要な桁数だけ切り取る/ゼロ埋めする運用とします。

	char sign_char = (temp.positive_ ? '+' : '-');
	//std::cout << temp.positive_ << std::endl;

	// (A) まず、"seconds" を日・時・分・秒 に分解する
	std::int64_t total_sec = temp.seconds_;
	std::int64_t days_part = total_sec / 86400;
	std::int64_t rem = total_sec % 86400;
	int hour = static_cast<int>(rem / 3600);
	rem %= 3600;
	int minute  = static_cast<int>(rem / 60);
	int sec_int = static_cast<int>(rem % 60);

	// (D) 書式文字列を左から順に解釈して置換
	std::string out;
	out.reserve(fmt.size() + 32);

	size_t i_fmt = 0;
	while (i_fmt < fmt.size())
	{
		if (fmt[i_fmt] != '%')
		{
			// リテラル文字そのままコピー
			out.push_back(fmt[i_fmt]);
			++i_fmt;
		}
		else
		{
			// '%' を発見 → 次を読む
			++i_fmt;
			if (i_fmt >= fmt.size())
			{
				throw std::invalid_argument("Duration::format: '%' at end of format");
			}

			// (1) "%+" の場合
			if (fmt[i_fmt] == '+')
			{
				// 符号を出力（正の場合は '+' を出してもよいが省略しても OK）
				if (! temp.positive_)
				{
					out.push_back('-');
				}
				else
				{
					out.push_back('+');
				}
				++i_fmt;
				continue;
			}

			// (2) 幅指定 (例: "%2d", "%3H", "%4f" など) があるかチェック
			bool hasWidth = false;
			int  width    = 0;
			size_t j = i_fmt;
			while (j < fmt.size() && std::isdigit(fmt[j]))
			{
				hasWidth = true;
				width = width * 10 + (fmt[j] - '0');
				++j;
			}
			if (hasWidth)
			{
				i_fmt = j;
				if (i_fmt >= fmt.size())
				{
					throw std::invalid_argument("Duration::format: missing specifier after width");
				}
			}

			char spec = fmt[i_fmt++];
			switch (spec)
			{
				case 'd':
				{
					// 日数を出力
					std::ostringstream oss;
					std::string ds = std::to_string(days_part);
					if (hasWidth && static_cast<int>(ds.size()) < width)
					{
						ds = std::string(width - ds.size(), '0') + ds;
					}
					oss << ds;
					out += oss.str();
					break;
				}
				case 'H':
				{
					// 時間 (0..23) を出力
					std::ostringstream oss;
					int h = hour;
					if (hasWidth)
					{
						oss << std::setw(width) << std::setfill('0') << h;
					}
					else
					{
						oss << std::setw(2) << std::setfill('0') << h;
					}
					out += oss.str();
					break;
				}
				case 'M':
				{
					// 分 (0..59) を出力
					std::ostringstream oss;
					int m = minute;
					if (hasWidth)
					{
						oss << std::setw(width) << std::setfill('0') << m;
					}
					else
					{
						oss << std::setw(2) << std::setfill('0') << m;
					}
					out += oss.str();
					break;
				}
				case 'S':
				{
					// 秒 (0..59) を出力
					std::ostringstream oss;
					int s = sec_int;
					if (hasWidth)
					{
						oss << std::setw(width) << std::setfill('0') << s;
					}
					else
					{
						oss << std::setw(2) << std::setfill('0') << s;
					}
					out += oss.str();
					break;
				}
				case 'f':
				{
					// 小数部。幅指定があればその桁数だけ、指定がなければ 12 桁すべて出力する
					int w = hasWidth ? width : 12;
					if (w < 0 || w > 12)
					{
						throw std::invalid_argument("Duration::format: invalid width for %f");
					}

					// frac12 は "012345678901" のように常に長さ 12 のゼロ埋め済み文字列
					// → 先頭から w 文字を取り出す
					std::string sub = frac12.substr(0, w);

					// もし w が指定になく（=12）なら、末尾ゼロを省いてもよい：
					if (!hasWidth || frac_width == 0)
					{
						while (sub.size() > 1 && sub.back() == '0')
						{
							sub.pop_back();
						}
					}
					out += sub;
					break;
				}
				default:
					// 未知の指定子はエラー
					throw std::invalid_argument(std::string("Duration::format: unknown specifier `%") + spec + "`");
			}
		}
	}

	return out;
}

template<std::integral Int> inline void Duration::addSeconds(Int delta_sec)
{
	//std::cout << deltaSec << std::endl;
	if(positive_)
	{
		seconds_ += static_cast<std::int64_t>(delta_sec);
	}
	else
	{
		seconds_ -= static_cast<std::int64_t>(delta_sec);
	}
	normalize();
}

template<std::floating_point Real> inline void Duration::addSeconds(Real delta_sec)
{
	double sn = delta_sec >= 0.0 ? 1.0 : -1.0;
	delta_sec = std::abs(delta_sec);

	std::int64_t int_part  = static_cast<std::int64_t>(std::floor(delta_sec));
	double frac_part = delta_sec - static_cast<double>(int_part);
	std::int64_t add_picoseconds = static_cast<std::int64_t>(std::llround(frac_part * static_cast<double>(PICO_PER_SEC)));

	if(positive_)
	{
		seconds_ += sn * int_part;
		picoseconds_ += sn * add_picoseconds;
	}
	else
	{
		seconds_ -= sn * int_part;
		picoseconds_ -= sn * add_picoseconds;
	}

	normalize();
}

template<std::integral Int> inline void Duration::addMinutes(Int delta_min)
{
	// 1 分 = 60 秒
	addSeconds(static_cast<std::int64_t>(delta_min * 60));
}

template<std::floating_point Real> inline void Duration::addMinutes(Real delta_min)
{
	addSeconds(delta_min * 60.0);
}

template<std::integral Int> inline void Duration::addHours(Int delta_hour)
{
	// 1 時間 = 3600 秒
	addSeconds(static_cast<std::int64_t>(delta_hour * 3600));
}

template<std::floating_point Real> inline void Duration::addHours(Real delta_hour)
{
	addSeconds(delta_hour * 3600.0);
}

template<std::integral Int> inline void Duration::addDays(Int delta_days)
{
	// 1 日 = 86400 秒
	addSeconds(static_cast<std::int64_t>(delta_days * 86400));
}

template<std::floating_point Real> inline void Duration::addDays(Real delta_days)
{
	addSeconds(delta_days * 86400.0);
}

template<std::integral Int> inline void Duration::addMilliseconds(Int delta_ms)
{
	if(positive_)
	{
		milliseconds_ += static_cast<std::int64_t>(delta_ms);
	}
	else
	{
		milliseconds_ -= static_cast<std::int64_t>(delta_ms);
	}

	normalize();
}

template<std::floating_point Real> inline void Duration::addMilliseconds(Real delta_ms)
{
	double sn = delta_ms >= 0.0 ? 1.0 : -1.0;
	delta_ms = std::abs(delta_ms);

	std::int64_t int_ms = static_cast<std::int64_t>(std::floor(delta_ms));
	double frac_ms = delta_ms - static_cast<double>(int_ms);
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_ms * static_cast<double>(PICO_PER_MSEC)));

	if(positive_)
	{
		milliseconds_ += sn * int_ms;
		picoseconds_ += sn * add_ps;
	}
	else
	{
		milliseconds_ -= sn * int_ms;
		picoseconds_ -= sn * add_ps;
	}

	normalize();
}

template<std::integral Int> inline void Duration::addMicroseconds(Int delta_us)
{
	if(positive_)
	{
		microseconds_ += static_cast<std::int64_t>(delta_us);
	}
	else
	{
		microseconds_ -= static_cast<std::int64_t>(delta_us);
	}

	normalize();
}

template<std::floating_point Real> inline void Duration::addMicroseconds(Real delta_us)
{
	double sn = delta_us >= 0.0 ? 1.0 : -1.0;
	delta_us = std::abs(delta_us);

	std::int64_t int_us = static_cast<std::int64_t>(std::floor(delta_us));
	double frac_us = delta_us - static_cast<double>(int_us);
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_us * static_cast<double>(PICO_PER_USEC)));

	if(positive_)
	{
		microseconds_ += sn * int_us;
		picoseconds_ += sn * add_ps;
	}
	else
	{
		microseconds_ -= sn * int_us;
		picoseconds_ -= sn * add_ps;
	}

	normalize();
}

template<std::integral Int> inline void Duration::addNanoseconds(Int delta_ns)
{
	if(positive_)
	{
		nanoseconds_ += static_cast<std::int64_t>(delta_ns);
	}
	else
	{
		nanoseconds_ -= static_cast<std::int64_t>(delta_ns);
	}

	normalize();
}

template<std::floating_point Real> inline void Duration::addNanoseconds(Real delta_ns)
{
	double sn = delta_ns >= 0.0 ? 1.0 : -1.0;
	delta_ns = std::abs(delta_ns);

	std::int64_t int_ns = static_cast<std::int64_t>(std::floor(delta_ns));
	double frac_ns = delta_ns - static_cast<double>(int_ns);
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(frac_ns * static_cast<double>(PICO_PER_NSEC)));

	if(positive_)
	{
		nanoseconds_ += sn * int_ns;
		picoseconds_ += sn * add_ps;
	}
	else
	{
		nanoseconds_ -= sn * int_ns;
		picoseconds_ -= sn * add_ps;
	}

	normalize();
}

template<std::integral Int> inline void Duration::addPicoseconds(Int delta_ps)
{
	if(positive_)
	{
		picoseconds_ += static_cast<std::int64_t>(delta_ps);
	}
	else
	{
		picoseconds_ -= static_cast<std::int64_t>(delta_ps);
	}

	normalize();
}

template<std::floating_point Real> inline void Duration::addPicoseconds(Real delta_ps)
{
	std::int64_t add_ps = static_cast<std::int64_t>(std::llround(delta_ps));
	addPicoseconds(add_ps);
	normalize();
}

inline bool Duration::positive(void) const
{
	return positive_;
}

inline std::int64_t Duration::seconds(void) const
{
	return seconds_;
}

inline std::int64_t Duration::milliseconds(void) const
{
	return milliseconds_;
}

inline std::int64_t Duration::microseconds(void) const
{
	return microseconds_;
}

inline std::int64_t Duration::nanoseconds(void) const
{
	return nanoseconds_;
}

inline std::int64_t Duration::picoseconds(void) const
{
	return picoseconds_;
}

inline void Duration::positive(bool pos)
{
	positive_ = pos;
	return;
}

template<std::integral Int> inline void Duration::seconds(Int sec)
{
	seconds_ = sec;
	return;
}

template<std::integral Int> inline void Duration::milliseconds(Int millisec)
{
	milliseconds_ = millisec;
	return;
}

template<std::integral Int> inline void Duration::microseconds(Int microsec)
{
	microseconds_ = microsec;
	return;
}

template<std::integral Int> inline void Duration::nanoseconds(Int nanosec)
{
	nanoseconds_ = nanosec;
	return;
}

template<std::integral Int> inline void Duration::picoseconds(Int picosec)
{
	picoseconds_ = picosec;
	return;
}

inline Duration Duration::operator+(const Duration &other) const
{
	int sign = positive_?1:-1;
	int sign_other = other.positive_?1:-1;
	Duration sum;
	sum.positive_ = positive_;
	sum.seconds_ = seconds_ + sign * sign_other * other.seconds_;
	sum.milliseconds_ = milliseconds_ + sign * sign_other * other.milliseconds_;
	sum.microseconds_ = microseconds_ + sign * sign_other * other.microseconds_;
	sum.nanoseconds_ = nanoseconds_ + sign * sign_other * other.nanoseconds_;
	sum.picoseconds_ = picoseconds_ + sign * sign_other * other.picoseconds_;
	sum.normalize();
	return sum;
}

inline Duration Duration::operator-(const Duration &other) const
{
	auto other2 = other;

	other2.positive_ = !other2.positive_;
	Duration diff = *this + other2;
	diff.normalize();
	return diff;
}

inline double Duration::operator/(const Duration &other) const
{
	// それぞれを「秒＋サブ秒」の double 値に変換して割る
	int sign = positive_?1:-1;
	int sign_other = other.positive_?1:-1;
	double ta = static_cast<double>(seconds_) + static_cast<double>(milliseconds_) * 1e-3 + static_cast<double>(microseconds_) * 1e-6 + static_cast<double>(nanoseconds_)  * 1e-9 + static_cast<double>(picoseconds_)  * 1e-12;
	double tb = static_cast<double>(other.seconds_) + static_cast<double>(other.milliseconds_) * 1e-3 + static_cast<double>(other.microseconds_) * 1e-6 + static_cast<double>(other.nanoseconds_)  * 1e-9 + static_cast<double>(other.picoseconds_)  * 1e-12;
	return sign * sign_other * ta / tb;
}

inline bool Duration::operator==(const Duration &other) const
{
	return
		positive_ == other.positive_ &&
		seconds_ == other.seconds_ &&
		milliseconds_ == other.milliseconds_ &&
		microseconds_ == other.microseconds_ &&
		nanoseconds_ == other.nanoseconds_ &&
		picoseconds_ == other.picoseconds_;
}

inline bool Duration::operator!=(const Duration &other) const
{
	return !(*this == other);
}

inline bool Duration::operator<(const Duration &other) const
{
	if (positive_ != other.positive_)
	{
		if(positive_)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	if (positive_)
	{
		if (seconds_ != other.seconds_) return seconds_ < other.seconds_;
		if (milliseconds_ != other.milliseconds_) return milliseconds_ < other.milliseconds_;
		if (microseconds_ != other.microseconds_) return microseconds_ < other.microseconds_;
		if (nanoseconds_ != other.nanoseconds_)   return nanoseconds_ < other.nanoseconds_;
		return picoseconds_ < other.picoseconds_;
	}
	else
	{
		if (seconds_ != other.seconds_) return seconds_ > other.seconds_;
		if (milliseconds_ != other.milliseconds_) return milliseconds_ > other.milliseconds_;
		if (microseconds_ != other.microseconds_) return microseconds_ > other.microseconds_;
		if (nanoseconds_ != other.nanoseconds_)   return nanoseconds_ > other.nanoseconds_;
		return picoseconds_ > other.picoseconds_;
	}		
}

inline bool Duration::operator<=(const Duration &other) const
{
	return (*this < other) || (*this == other);
}

inline bool Duration::operator>(const Duration &other) const
{
	return other < *this;
}

inline bool Duration::operator>=(const Duration &other) const
{
	return other <= *this;
}

inline double Duration::toTotalPicoseconds(void)
{
	double sign = positive_?1.0:-1.0;
	double total = static_cast<double>(seconds_) * 1.0E12;
	total += static_cast<double>(milliseconds_) * 1.0E9;
	total += static_cast<double>(microseconds_) * 1.0E6;
	total += static_cast<double>(nanoseconds_) * 1.0E3;
	total += static_cast<double>(picoseconds_);

	return sign * total;
}

inline double Duration::toTotalNanoseconds(void)
{
	double sign = positive_?1.0:-1.0;
	double total = static_cast<double>(seconds_) * 1.0E9;
	total += static_cast<double>(milliseconds_) * 1.0E6;
	total += static_cast<double>(microseconds_) * 1.0E3;
	total += static_cast<double>(nanoseconds_);
	total += static_cast<double>(picoseconds_) * 1.0E-3;

	return sign * total;
}

inline double Duration::toTotalMicroseconds(void)
{
	double sign = positive_?1.0:-1.0;
	double total = static_cast<double>(seconds_) * 1.0E6;
	total += static_cast<double>(milliseconds_) * 1.0E3;
	total += static_cast<double>(microseconds_);
	total += static_cast<double>(nanoseconds_) * 1.0E-3;
	total += static_cast<double>(picoseconds_) * 1.0E-6;
	return sign * total;
}

inline double Duration::toTotalMilliseconds(void)
{
	double sign = positive_?1.0:-1.0;
	double total = static_cast<double>(seconds_) * 1.0E3;
	total += static_cast<double>(milliseconds_);
	total += static_cast<double>(microseconds_) * 1.0E-3;
	total += static_cast<double>(nanoseconds_) * 1.0E-6;
	total += static_cast<double>(picoseconds_) * 1.0E-9;
	return sign * total;
}

inline double Duration::toTotalSeconds(void)
{
	double sign = positive_?1.0:-1.0;
	double total = static_cast<double>(seconds_);
	total += static_cast<double>(milliseconds_) * 1.0E-3;
	total += static_cast<double>(microseconds_) * 1.0E-6;
	total += static_cast<double>(nanoseconds_) * 1.0E-9;
	total += static_cast<double>(picoseconds_) * 1.0E-12;
	return sign * total;
}

inline double Duration::toTotalMinutes(void)
{
	// 1 分 = 60 秒
	return toTotalSeconds() / 60.0;
}

inline double Duration::toTotalHours(void)
{
	// 1 時間 = 3600 秒
	return toTotalSeconds() / 3600.0;
}

inline double Duration::toTotalDays(void)
{
	// 1 日 = 86400 秒
	return toTotalSeconds() / 86400.0;
}

inline TimePoint importCalendarFields(int year, int month, int day, int hour, int minute, double secondFull)
{
	return TimePoint(year, month, day, hour, minute, secondFull);
}

inline Duration fromSeconds(double sec_full)
{
	Duration d;
	
	if(sec_full < 0.0)
	{
		d.positive(false);
		sec_full *= -1.0;
	}
	
	std::int64_t sec_int = static_cast<std::int64_t>(std::trunc(sec_full));
	double frac_sec      = sec_full - static_cast<double>(sec_int);

	d.seconds(sec_int);
	d.milliseconds(0);
	d.microseconds(0);
	d.nanoseconds(0);
	d.picoseconds(0);

	// (2) 小数秒部をミリ秒単位に換算
	//     例: frac_sec = 0.123456789012 → 123.456789012 ms
	double ms_total = frac_sec * 1000.0;
	std::int64_t ms_int = static_cast<std::int64_t>(std::trunc(ms_total));
	double frac_ms = ms_total - static_cast<double>(ms_int);
	d.milliseconds(ms_int);

	// (3) 小数ミリ秒部をマイクロ秒単位に換算
	//     例: frac_ms = 0.456789012 → 456.789012 μs
	double us_total = frac_ms * 1000.0;
	std::int64_t us_int = static_cast<std::int64_t>(std::trunc(us_total));
	double frac_us = us_total - static_cast<double>(us_int);
	d.microseconds(us_int);

	// (4) 小数マイクロ秒部をナノ秒単位に換算
	//     例: frac_us = 0.789012 → 789.012 ns
	double ns_total = frac_us * 1000.0;
	std::int64_t ns_int = static_cast<std::int64_t>(std::trunc(ns_total));
	double frac_ns = ns_total - static_cast<double>(ns_int);
	d.nanoseconds(ns_int);

	// (5) 小数ナノ秒部をピコ秒単位に換算
	//     例: frac_ns = 0.012 → 12.0 ps
	double ps_total = frac_ns * 1000.0;
	std::int64_t ps_int = static_cast<std::int64_t>(std::llround(ps_total));
	d.picoseconds(ps_int);

	// (6) 最後に normalize() を呼んでキャリー／借りを調整
	d.normalize();

	return d;
}

inline Duration fromMinutes(double min_full)
{
	// 1 分 = 60 秒
	return fromSeconds(min_full * 60.0);
}

inline Duration fromHours(double hr_full)
{
	// 1 時間 = 3600 秒
	return fromSeconds(hr_full * 3600.0);
}

inline Duration fromDays(double day_full)
{
	// 1 日 = 86400 秒
	return fromSeconds(day_full * 86400.0);
}

inline Duration fromMilliseconds(double ms_full)
{
	Duration d;

	if(ms_full < 0.0)
	{
		d.positive(false);
		ms_full *= -1.0;
	}
	
	std::int64_t ms_int = static_cast<std::int64_t>(std::trunc(ms_full));
	double frac_ms      = ms_full - static_cast<double>(ms_int);

	d.seconds(0);
	d.milliseconds(ms_int);
	d.microseconds(0);
	d.nanoseconds(0);
	d.picoseconds(0);

	// (2) 小数ミリ秒部（0 ≤ frac_ms < 1）をマイクロ秒に換算
	//     例: frac_ms = 0.789 → 789.0 μs
	double us_total = frac_ms * 1000.0;
	std::int64_t us_int = static_cast<std::int64_t>(std::trunc(us_total));
	double frac_us     = us_total - static_cast<double>(us_int);
	d.microseconds(us_int);

	// (3) 小数マイクロ秒部をナノ秒単位に換算
	double ns_total = frac_us * 1000.0;
	std::int64_t ns_int = static_cast<std::int64_t>(std::trunc(ns_total));
	double frac_ns = ns_total - static_cast<double>(ns_int);
	d.nanoseconds(ns_int);

	// (4) 小数ナノ秒部をピコ秒単位に換算
	double ps_total = frac_ns * 1000.0;
	std::int64_t ps_int = static_cast<std::int64_t>(std::llround(ps_total));
	d.picoseconds(ps_int);

	d.normalize();
	return d;
}

inline Duration fromMicroseconds(double us_full)
{
	Duration d;

	if(us_full < 0.0)
	{
		d.positive(false);
		us_full *= -1.0;
	}

	std::int64_t us_int  = static_cast<std::int64_t>(std::trunc(us_full));
	double frac_us       = us_full - static_cast<double>(us_int);

	d.seconds(0);
	d.milliseconds(0);
	d.microseconds(us_int);
	d.nanoseconds(0);
	d.picoseconds(0);

	// 小数マイクロ秒部をナノ秒に
	double ns_total = frac_us * 1000.0;
	std::int64_t ns_int = static_cast<std::int64_t>(std::trunc(ns_total));
	double frac_ns = ns_total - static_cast<double>(ns_int);
	d.nanoseconds(ns_int);

	// 小数ナノ秒部をピコ秒に
	double ps_total = frac_ns * 1000.0;
	std::int64_t ps_int = static_cast<std::int64_t>(std::llround(ps_total));
	d.picoseconds(ps_int);

	d.normalize();
	return d;
}

inline Duration fromNanoseconds(double ns_full)
{
	Duration d;

	if(ns_full < 0.0)
	{
		d.positive(false);
		ns_full *= -1.0;
	}
	
	std::int64_t ns_int  = static_cast<std::int64_t>(std::trunc(ns_full));
	double frac_ns       = ns_full - static_cast<double>(ns_int);

	d.seconds(0);
	d.milliseconds(0);
	d.microseconds(0);
	d.nanoseconds(ns_int);
	d.picoseconds(0);

	// 小数ナノ秒部をピコ秒に
	double ps_total = frac_ns * 1000.0;
	std::int64_t ps_int = static_cast<std::int64_t>(std::llround(ps_total));
	d.picoseconds(ps_int);

	d.normalize();
	return d;
}

inline Duration fromPicoseconds(double ps_full)
{
	Duration d;
	if(ps_full < 0.0)
	{
		d.positive(false);
		ps_full *= -1.0;
	}
	std::int64_t ps_int = static_cast<std::int64_t>(std::llround(ps_full));
	d.seconds(0);
	d.milliseconds(0);
	d.microseconds(0);
	d.nanoseconds(0);
	d.picoseconds(ps_int);
	d.normalize();
	return d;
}

inline TimePoint operator+(const TimePoint &tp, const Duration &d)
{
	TimePoint result = tp;

	int sign = d.positive()?1:-1;

	// (1) 整数秒を加算
	result.seconds(result.seconds() + sign * d.seconds());
	// (2) ミリ秒～ピコ秒をそれぞれ加算
	result.milliseconds(result.milliseconds() + sign * d.milliseconds());
	result.microseconds(result.microseconds() + sign * d.microseconds());
	result.nanoseconds(result.nanoseconds() + sign * d.nanoseconds());
	result.picoseconds(result.picoseconds() + sign * d.picoseconds());

	// (3) 正規化してキャリー／借り入れを補正
	result.normalize();
	return result;
}

inline TimePoint operator-(const TimePoint &tp, const Duration &d)
{
	TimePoint result = tp;

	int sign = d.positive()?1:-1;

	// (1) 整数秒を減算
	result.seconds(result.seconds() - sign * d.seconds());
	// (2) ミリ秒～ピコ秒をそれぞれ減算
	result.milliseconds(result.milliseconds() - sign * d.milliseconds());
	result.microseconds(result.microseconds() - sign * d.microseconds());
	result.nanoseconds(result.nanoseconds() - sign * d.nanoseconds());
	result.picoseconds(result.picoseconds() - sign * d.picoseconds());

	// (3) 正規化してキャリー／借り入れを補正
	result.normalize();
	return result;
}

inline Duration operator-(const TimePoint &t2, const TimePoint &t1)
{
	Duration delta;

	// (1) 整数秒の差分
	delta.seconds(t2.seconds() - t1.seconds());
	// (2) 各サブ秒フィールドの差分
	delta.milliseconds(t2.milliseconds() - t1.milliseconds());
	delta.microseconds(t2.microseconds() - t1.microseconds());
	delta.nanoseconds(t2.nanoseconds() - t1.nanoseconds());
	delta.picoseconds(t2.picoseconds() - t1.picoseconds());

	// (3) 正規化してキャリー／借り入れを補正
	delta.normalize();
	return delta;
}

inline Duration operator*(const Duration &d, double k)
{
	k = d.positive()?k:-k;
	// (1) 秒成分を k 倍し、整数部と小数部に分離
	double raw_sec = static_cast<double>(d.seconds()) * k;
	std::int64_t new_sec_int = static_cast<std::int64_t>(std::trunc(raw_sec));
	double sec_frac = raw_sec - static_cast<double>(new_sec_int);

	// (2) 小数秒部をミリ秒単位で取り出し、d.milliseconds_×k を加算
	double raw_msec_from_sec_frac = sec_frac * 1000.0;
	double raw_msec_from_old_msec = static_cast<double>(d.milliseconds()) * k;
	double total_msec = raw_msec_from_sec_frac + raw_msec_from_old_msec;
	std::int64_t new_msec_int = static_cast<std::int64_t>(std::trunc(total_msec));
	double msec_frac = total_msec - static_cast<double>(new_msec_int);

	// (3) ミリ→マイクロへ
	double raw_usec_from_msec_frac = msec_frac * 1000.0;
	double raw_usec_from_old_usec = static_cast<double>(d.microseconds()) * k;
	double total_usec = raw_usec_from_msec_frac + raw_usec_from_old_usec;
	std::int64_t new_usec_int = static_cast<std::int64_t>(std::trunc(total_usec));
	double usec_frac = total_usec - static_cast<double>(new_usec_int);

	// (4) マイクロ→ナノへ
	double raw_nsec_from_usec_frac = usec_frac * 1000.0;
	double raw_nsec_from_old_nsec = static_cast<double>(d.nanoseconds()) * k;
	double total_nsec = raw_nsec_from_usec_frac + raw_nsec_from_old_nsec;
	std::int64_t new_nsec_int = static_cast<std::int64_t>(std::trunc(total_nsec));
	double nsec_frac = total_nsec - static_cast<double>(new_nsec_int);

	// (5) ナノ→ピコへ
	double raw_psec_from_nsec_frac = nsec_frac * 1000.0;
	double raw_psec_from_old_psec = static_cast<double>(d.picoseconds()) * k;
	double total_psec = raw_psec_from_nsec_frac + raw_psec_from_old_psec;
	std::int64_t new_psec_int = static_cast<std::int64_t>(std::llround(total_psec));

	// (6) できあがった各成分を normalize に任せる
	return Duration(new_sec_int, static_cast<int64_t>(new_msec_int), static_cast<int64_t>(new_usec_int), static_cast<int64_t>(new_nsec_int), static_cast<int64_t>(new_psec_int));
}

inline Duration operator*(double k, const Duration &d)
{
	return d * k;
}

inline Duration operator/(const Duration &d, double k)
{
	// (1) 秒成分を k で割り、整数部と小数部に分離
	double raw_sec = static_cast<double>(d.seconds()) / k;
	std::int64_t new_sec_int = static_cast<std::int64_t>(std::trunc(raw_sec));
	double sec_frac = raw_sec - static_cast<double>(new_sec_int);

	// (2) 小数秒→ミリ秒単位
	double raw_msec_from_sec_frac = sec_frac * 1000.0;
	double raw_msec_from_old_msec = static_cast<double>(d.milliseconds()) / k;
	double total_msec = raw_msec_from_sec_frac + raw_msec_from_old_msec;
	std::int64_t new_msec_int = static_cast<std::int64_t>(std::trunc(total_msec));
	double msec_frac = total_msec - static_cast<double>(new_msec_int);

	// (3) ミリ秒→マイクロ秒
	double raw_usec_from_msec_frac = msec_frac * 1000.0;
	double raw_usec_from_old_usec = static_cast<double>(d.microseconds()) / k;
	double total_usec = raw_usec_from_msec_frac + raw_usec_from_old_usec;
	std::int64_t new_usec_int = static_cast<std::int64_t>(std::trunc(total_usec));
	double usec_frac = total_usec - static_cast<double>(new_usec_int);

	// (4) マイクロ秒→ナノ秒
	double raw_nsec_from_usec_frac = usec_frac * 1000.0;
	double raw_nsec_from_old_nsec = static_cast<double>(d.nanoseconds()) / k;
	double total_nsec = raw_nsec_from_usec_frac + raw_nsec_from_old_nsec;
	std::int64_t new_nsec_int = static_cast<std::int64_t>(std::trunc(total_nsec));
	double nsec_frac = total_nsec - static_cast<double>(new_nsec_int);

	// (5) ナノ秒→ピコ秒
	double raw_psec_from_nsec_frac = nsec_frac * 1000.0;
	double raw_psec_from_old_psec = static_cast<double>(d.picoseconds()) / k;
	double total_psec = raw_psec_from_nsec_frac + raw_psec_from_old_psec;
	std::int64_t new_psec_int = static_cast<std::int64_t>(std::llround(total_psec));

	// (6) できあがった各成分を normalize に任せる
	return Duration(new_sec_int, static_cast<int64_t>(new_msec_int), static_cast<int64_t>(new_usec_int), static_cast<int64_t>(new_nsec_int), static_cast<int64_t>(new_psec_int));
}

inline bool operator>(const TimePoint &t1, const TimePoint &t2)
{
	return (t1 - t2).positive() ? true : false;
}

inline bool operator<(const TimePoint &t1, const TimePoint &t2)
{
	return (t2 - t1).positive() ? true : false;
}

inline bool operator==(const TimePoint &t1, const TimePoint &t2)
{
	return !((t1 - t2).positive() || (t2 - t1).positive());
}

inline bool operator!=(const TimePoint &t1, const TimePoint &t2)
{
	return !(t1 == t2);
}

inline const TimePoint epoch("1970/01/01 00:00:00", "%Y/%m/%d %H:%M:%S");

inline TimePoint now(double time_zone = 0.0)
{
	std::timespec ts;
	std::timespec_get(&ts, TIME_UTC);
	TimePoint now = epoch;
	now.seconds(epoch.seconds() + ts.tv_sec + static_cast<int>(3600 * time_zone));
	now.nanoseconds(epoch.nanoseconds() + ts.tv_nsec);
	now.normalize();
	// now.addSeconds(ts.tv_sec);
	// now.addNanoseconds(ts.tv_nsec);
	// now.addHours(time_zone);

	return now;
}

}