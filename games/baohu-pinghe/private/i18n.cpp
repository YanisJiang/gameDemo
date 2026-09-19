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
    {"cover.tagline", "有勇无度", "Courage without measure"},
    {"cover.prompt", "按任意键", "Press any key"},
    {"select.level", "选择关卡  1-6 进入", "Select stage  1-6 start"},
    {"select.lang", "Z 中文   E English   Q 退出", "Z 中文   E English   Q quit"},
    {"game.title", "暴虎冯河", "Bao Hu Ping He"},
    {"level.1.title", "试水", "Wading"},
    {"level.2.title", "逼择", "Hard Choice"},
    {"level.3.title", "诡步", "Strange Steps"},
    {"level.4.title", "渡口", "Ferry"},
    {"level.5.title", "狂者", "Mad One"},
    {"level.6.title", "妄执", "Delusion"},
    {"hud.rules",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.tutorial",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.pressure",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.feint",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.ferry",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.mad",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.hint.puppet",
     "空格蓄力松手冲刺 · 撑过计时过关 · 红区或落水则死",
     "Hold Space, release to dash · Survive the timer · Red zone or river kills"},
    {"hud.level", "关卡", "Level"},
    {"end.melee", "贴身死 · 有勇无距", "Mauled · Courage without distance"},
    {"end.river", "落水死 · 退亦险境", "Drowned · Retreat into peril"},
    {"end.level_clear", "本关通过", "Stage cleared"},
    {"end.level_clear_hint", "Enter/N 下一关   R 重玩本关   Q 返回选关",
     "Enter/N next   R replay   Q level select"},
    {"end.all_clear", "六关通关 · 勇而有度", "All clear · Brave, yet measured"},
    {"end.retry_level", "R 重开本关  Q 返回选关", "R retry stage  Q level select"},
    {"end.retry_all", "R 从第1关重开  Q 返回选关", "R restart  Q level select"},
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
