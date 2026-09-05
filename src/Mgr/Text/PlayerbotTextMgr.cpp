/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotTextMgr.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"    // Required due to a poor implementation by AC
#include "Random.h"
#include "WorldSessionMgr.h"

void PlayerbotTextMgr::replaceAll(std::string& str, std::string const& from, std::string const& to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void PlayerbotTextMgr::LoadBotTexts()
{
    LOG_INFO("playerbots", "Loading playerbots texts...");

    std::map<std::string, std::vector<BotTextEntry>> loadedBotTexts;
    std::map<uint32, std::vector<uint32>> loadedReplyTextIndexes;
    uint32 count = 0;
    if (PreparedQueryResult result =
            PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT)))
    {
        do
        {
            std::map<uint32, std::string> text;
            Field* fields = result->Fetch();
            std::string name = fields[0].Get<std::string>();
            text[0] = fields[1].Get<std::string>();
            uint8 sayType = fields[2].Get<uint8>();
            uint8 replyType = fields[3].Get<uint8>();
            for (uint8 i = 1; i < TOTAL_LOCALES; ++i)
            {
                text[i] = fields[i + 3].Get<std::string>();
            }

            std::vector<BotTextEntry>& textList = loadedBotTexts[name];
            if (name == "reply")
                loadedReplyTextIndexes[replyType].push_back(static_cast<uint32>(textList.size()));

            textList.push_back(BotTextEntry(name, text, sayType, replyType));
            ++count;
        } while (result->NextRow());
    }

    {
        std::lock_guard<std::mutex> lock(botTextMutex);
        botTexts.swap(loadedBotTexts);
        replyTextIndexes.swap(loadedReplyTextIndexes);
        textSelectionStates.clear();
    }

    LOG_INFO("playerbots", "{} playerbots texts loaded", count);
}

void PlayerbotTextMgr::LoadBotTextChance()
{
    if (botTextChance.empty())
    {
        QueryResult results = PlayerbotsDatabase.Query("SELECT name, probability FROM ai_playerbot_texts_chance");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string name = fields[0].Get<std::string>();
                uint32 probability = fields[1].Get<uint32>();

                botTextChance[name] = probability;
            } while (results->NextRow());
        }
    }
}

// general texts

uint32 PlayerbotTextMgr::SelectTextIndex(std::string const& selectionKey, uint32 poolSize)
{
    TextSelectionState& state = textSelectionStates[selectionKey];

    if (state.poolSize != poolSize)
    {
        state.remainingIndexes.clear();
        state.lastIndex = std::numeric_limits<uint32>::max();
        state.poolSize = poolSize;
    }

    if (state.remainingIndexes.empty())
    {
        // Drawing from the remaining indexes is a lightweight shuffle bag: every line is used once
        // before the pool refills, without reshuffling or scanning on each chat message.
        state.remainingIndexes.reserve(poolSize);
        for (uint32 index = 0; index < poolSize; ++index)
            state.remainingIndexes.push_back(index);
    }

    uint32 choice = urand(0, static_cast<uint32>(state.remainingIndexes.size() - 1));
    if (poolSize > 1 && state.remainingIndexes.size() == poolSize &&
        state.remainingIndexes[choice] == state.lastIndex)
    {
        choice = choice == 0 ? 1 : 0;
    }

    uint32 selectedIndex = state.remainingIndexes[choice];
    state.remainingIndexes[choice] = state.remainingIndexes.back();
    state.remainingIndexes.pop_back();
    state.lastIndex = selectedIndex;
    return selectedIndex;
}

std::string PlayerbotTextMgr::GetBotText(std::string name)
{
    std::lock_guard<std::mutex> lock(botTextMutex);
    if (botTexts.empty())
    {
        LOG_ERROR("playerbots", "Can't get bot text {}! No bots texts loaded!", name);
        return "";
    }

    auto textList = botTexts.find(name);
    if (textList == botTexts.end() || textList->second.empty())
    {
        LOG_ERROR("playerbots", "Can't get bot text {}! No bots texts for this name!", name);
        return "";
    }

    std::vector<BotTextEntry> const& list = textList->second;
    BotTextEntry const& textEntry = list[SelectTextIndex(name, static_cast<uint32>(list.size()))];
    uint32 locale = GetLocalePriority();
    std::string const& localizedText = textEntry.m_text.at(locale);
    return !localizedText.empty() ? localizedText : textEntry.m_text.at(0);
}

std::string PlayerbotTextMgr::GetBotText(std::string name, std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name);
    if (botText.empty())
        return "";

    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        replaceAll(botText, i->first, i->second);

    return botText;
}

std::string PlayerbotTextMgr::GetBotTextOrDefault(std::string name, std::string defaultText,
    std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name, placeholders);
    if (botText.empty())
    {
        for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        {
            replaceAll(defaultText, i->first, i->second);
        }
        return defaultText;
    }

    return botText;
}

// chat replies

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders)
{
    std::string botText;
    {
        std::lock_guard<std::mutex> lock(botTextMutex);
        if (botTexts.empty())
        {
            LOG_ERROR("playerbots", "Can't get bot text reply {}! No bots texts loaded!", replyType);
            return "";
        }
        auto textList = botTexts.find("reply");
        auto replyList = replyTextIndexes.find(replyType);
        if (textList == botTexts.end() || replyList == replyTextIndexes.end() || replyList->second.empty())
        {
            LOG_ERROR("playerbots", "Can't get bot text reply {}! No bots texts replies!", replyType);
            return "";
        }

        std::string selectionKey = "reply:" + std::to_string(replyType);
        std::vector<uint32> const& indexes = replyList->second;
        uint32 selectedIndex = SelectTextIndex(selectionKey, static_cast<uint32>(indexes.size()));
        BotTextEntry const& textEntry = textList->second[indexes[selectedIndex]];
        uint32 locale = GetLocalePriority();
        std::string const& localizedText = textEntry.m_text.at(locale);
        botText = !localizedText.empty() ? localizedText : textEntry.m_text.at(0);
    }

    for (auto& placeholder : placeholders)
        replaceAll(botText, placeholder.first, placeholder.second);

    return botText;
}

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::string name)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%s"] = name;

    return GetBotText(replyType, placeholders);
}

// probabilities

bool PlayerbotTextMgr::rollTextChance(std::string name)
{
    if (!botTextChance[name])
        return true;

    return urand(0, 100) < botTextChance[name];
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name);
    return !text.empty();
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name, placeholders);
    return !text.empty();
}

void PlayerbotTextMgr::AddLocalePriority(uint32 locale)
{
    if (locale >= TOTAL_LOCALES)
    {
        LOG_WARN("playerbots", "Ignoring locale {} for bot texts because it exceeds TOTAL_LOCALES ({})", locale, TOTAL_LOCALES - 1);
        return;
    }

    botTextLocalePriority[locale]++;
}

uint32 PlayerbotTextMgr::GetLocalePriority()
{
    // if no real players online, reset top locale
    uint32 const activeSessions = sWorldSessionMgr->GetActiveSessionCount();
    if (!activeSessions)
    {
        ResetLocalePriority();
        return 0;
    }

    uint32 topLocale = 0;
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        if (botTextLocalePriority[i] > botTextLocalePriority[topLocale])
            topLocale = i;
    }

    return topLocale;
}

void PlayerbotTextMgr::ResetLocalePriority()
{
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        botTextLocalePriority[i] = 0;
    }
}
