#pragma once

enum class Locale { Zh, En };

void setLocale(Locale locale);
Locale currentLocale();
const char* tr(const char* key);
