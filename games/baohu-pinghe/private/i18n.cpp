#include "i18n.hpp"

#include <cstring>

namespace {

Locale g_locale = Locale::Zh;

struct Entry {
  const char* key;
  const char* zh;
  const char* en;
};

const Entry kTable[] = {
    {"lang.prompt", "选择语言  1=中文  2=English", "Language  1=Chinese  2=English"},
    {"game.title", "暴虎冯河", "Bao Hu Ping He"},
    {"level.1.title", "试水：先活下来", "Trial: Just Survive"},
    {"level.2.title", "逼岸：河在涨", "Rising: The Bank Shrinks"},
    {"level.3.title", "狂虎：不可大意", "Berserk: Do Not Overreach"},
    {"hud.hint", "空格蓄力松手冲刺", "Hold Space to charge, release to dash"},
    {"hud.level", "关卡", "Level"},
    {"end.melee", "贴身死 · 有勇无距", "Mauled · Courage without distance"},
    {"end.river", "落水死 · 退亦险境", "Drowned · Retreat into peril"},
    {"end.level_clear", "本关通过", "Stage cleared"},
    {"end.level_clear_hint", "Enter/N 下一关   R 重玩本关   Q 退出",
     "Enter/N next   R replay   Q quit"},
    {"end.all_clear", "三关通关 · 勇而有度", "All clear · Brave, yet measured"},
    {"end.retry_level", "R 重开本关  Q 退出", "R retry stage  Q quit"},
    {"end.retry_all", "R 从第1关重开  Q 退出", "R restart from stage 1  Q quit"},
};

const char* lookup(const char* key, Locale loc) {
  for (const Entry& e : kTable) {
    if (std::strcmp(e.key, key) == 0) {
      return loc == Locale::En ? e.en : e.zh;
    }
  }
  return nullptr;
}

}  // namespace

void setLocale(Locale locale) { g_locale = locale; }

Locale currentLocale() { return g_locale; }

const char* tr(const char* key) {
  if (const char* s = lookup(key, g_locale)) {
    return s;
  }
  if (g_locale != Locale::Zh) {
    if (const char* s = lookup(key, Locale::Zh)) {
      return s;
    }
  }
  return key;
}
