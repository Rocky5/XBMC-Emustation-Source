/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "include.h"
#include "GUIInfoTypes.h"
#include "utils/CharsetConverter.h"
#include "GUIInfoManager.h"
#include "LocalizeStrings.h"
#include "GUIColorManager.h"
#include "GUIListItem.h"
#include "guilib/SkinInfo.h"

using namespace std;

CGUIInfoBool::CGUIInfoBool(bool value)
{
  m_info = 0;
  m_value = value;
}

void CGUIInfoBool::Parse(const CStdString &info)
{
  m_info = g_infoManager.TranslateString(info);
  if (m_info == SYSTEM_ALWAYS_TRUE)
  {
    m_value = true;
    m_info = 0;
  }
  else if (m_info == SYSTEM_ALWAYS_FALSE)
  {
    m_value = false;
    m_info = 0;
  }
  else
    m_info = g_infoManager.GetBool(m_info);
}

void CGUIInfoBool::Update(int parentID, const CGUIListItem *item)
{
  if (m_info)
    m_value = g_infoManager.GetBool(m_info, parentID, item);
}


CGUIInfoColor::CGUIInfoColor(uint32_t color)
{
  m_color = color;
  m_info = 0;
}

const CGUIInfoColor &CGUIInfoColor::operator=(color_t color)
{
  m_color = color;
  m_info = 0;
  return *this;
}

const CGUIInfoColor &CGUIInfoColor::operator=(const CGUIInfoColor &color)
{
  m_color = color.m_color;
  m_info = color.m_info;
  return *this;
}

void CGUIInfoColor::Update()
{
  if (!m_info)
    return; // no infolabel

  // Expand the infolabel, and then convert it to a color
  CStdString infoLabel(g_infoManager.GetLabel(m_info));
  if (!infoLabel.IsEmpty())
    m_color = g_colorManager.GetColor(infoLabel.c_str());
  else
    m_color = 0;
}

void CGUIInfoColor::Parse(const CStdString &label, int context)
{
  // Check for the standard $INFO[] block layout, and strip it if present
  CStdString label2 = label;
  if (label.Equals("-", false))
    return;

  if (label.Left(4).Equals("$VAR", false))
  {
    label2 = label.Mid(5, label.length() - 6);
    m_info = g_infoManager.TranslateSkinVariableString(label2, context);
    if (!m_info)
      m_info = g_infoManager.RegisterSkinVariableString(g_SkinInfo.CreateSkinVariable(label2, context));
    return;
  }

  if (label.Left(5).Equals("$INFO", false))
    label2 = label.Mid(6, label.length()-7);

  m_info = g_infoManager.TranslateString(label2);
  if (!m_info)
    m_color = g_colorManager.GetColor(label);
}

CGUIInfoLabel::CGUIInfoLabel()
{
}

CGUIInfoLabel::CGUIInfoLabel(const CStdString &label, const CStdString &fallback, int context)
{
  SetLabel(label, fallback, context);
}

void CGUIInfoLabel::SetLabel(const CStdString &label, const CStdString &fallback, int context)
{
  m_fallback = fallback;
  Parse(label, context);
}

CStdString CGUIInfoLabel::GetLabel(int contextWindow, bool preferImage) const
{
  CStdString label;
  for (unsigned int i = 0; i < m_info.size(); i++)
  {
    const CInfoPortion &portion = m_info[i];
    if (portion.m_info)
    {
      CStdString infoLabel;
      if (preferImage)
        infoLabel = g_infoManager.GetImage(portion.m_info, contextWindow);
      if (infoLabel.IsEmpty())
        infoLabel = g_infoManager.GetLabel(portion.m_info, contextWindow);
      if (!infoLabel.IsEmpty())
        label += portion.GetLabel(infoLabel);
    }
    else
    { // no info, so just append the prefix
      label += portion.m_prefix;
    }
  }
  if (label.IsEmpty())  // empty label, use the fallback
    return m_fallback;
  return label;
}

CStdString CGUIInfoLabel::GetItemLabel(const CGUIListItem *item, bool preferImages) const
{
  if (!item->IsFileItem()) return "";
  CStdString label;
  for (unsigned int i = 0; i < m_info.size(); i++)
  {
    const CInfoPortion &portion = m_info[i];
    if (portion.m_info)
    {
      CStdString infoLabel;
      if (preferImages)
        infoLabel = g_infoManager.GetItemImage((const CFileItem *)item, portion.m_info);
      else
        infoLabel = g_infoManager.GetItemLabel((const CFileItem *)item, portion.m_info);
      if (!infoLabel.IsEmpty())
        label += portion.GetLabel(infoLabel);
    }
    else
    { // no info, so just append the prefix
      label += portion.m_prefix;
    }
  }
  if (label.IsEmpty())
    return m_fallback;
  return label;
}

bool CGUIInfoLabel::IsEmpty() const
{
  return m_info.size() == 0;
}

bool CGUIInfoLabel::IsConstant() const
{
  return m_info.size() == 0 || (m_info.size() == 1 && m_info[0].m_info == 0);
}

typedef CStdString (*StringReplacerFunc) (const CStdString &str);

void ReplaceString(CStdString &work, const std::string &str, StringReplacerFunc func)
{
  // Replace all $str[number] with the real string
  size_t pos1 = work.find("$" + str + "[");
  while (pos1 != std::string::npos)
  {
    size_t pos2 = pos1 + str.length() + 2;
    size_t pos3 = StringUtils::FindEndBracket(work, '[', ']', pos2);
    if (pos3 != std::string::npos)
    {
      CStdString left = work.substr(0, pos1);
      CStdString right = work.substr(pos3 + 1);
      CStdString replace = func(work.substr(pos2, pos3 - pos2));
      work = left + replace + right;
    }
    else
    {
      CLog::Log(LOGERROR, "Error parsing label - missing ']' in \"%s\"", work.c_str());
      return;
    }
    pos1 = work.find("$" + str + "[", pos1);
  }
}

CStdString LocalizeReplacer(const CStdString &str)
{
  CStdString replace = g_localizeStringsTemp.Get(atoi(str.c_str()));
  if (replace == "")
    replace = g_localizeStrings.Get(atoi(str.c_str()));
  return replace;
}

CStdString AddonReplacer(const CStdString &str)
{
  // assumes "addon.id #####"
  size_t length = str.find(" ");
  CStdString id = str.substr(0, length);
  int stringid = atoi(str.substr(length + 1).c_str());
  return g_localizeStringsTemp.Get(stringid);
}

CStdString NumberReplacer(const CStdString &str)
{
  return str;
}

CStdString CGUIInfoLabel::ReplaceLocalize(const CStdString &label)
{
  CStdString work(label);
  ReplaceString(work, "LOCALIZE", LocalizeReplacer);
  ReplaceString(work, "NUMBER", NumberReplacer);
  return work;
}

CStdString CGUIInfoLabel::ReplaceAddonStrings(const CStdString &label)
{
  CStdString work(label);
  ReplaceString(work, "ADDON", AddonReplacer);
  return work;
}

enum EINFOFORMAT { NONE = 0, FORMATINFO, FORMATESCINFO, FORMATVAR };

typedef struct
{
  const char *str;
  EINFOFORMAT  val;
} infoformat;

const static infoformat infoformatmap[] = {{ "$INFO[",    FORMATINFO },
                                           { "$ESCINFO[", FORMATESCINFO},
                                           { "$VAR[",     FORMATVAR}};

void CGUIInfoLabel::Parse(const CStdString &label, int context)
{
  m_info.clear();
  // Step 1: Replace all $LOCALIZE[number] with the real string
  CStdString work = ReplaceLocalize(label);
  // Step 2: Replace all $ADDON[id number] with the real string
  work = ReplaceAddonStrings(work);
  // Step 3: Find all $INFO[info,prefix,postfix] blocks
  EINFOFORMAT format;
  do
  {
    format = NONE;
    int pos1 = work.size();
    int pos2;
    int len = 0;
    for (size_t i = 0; i < sizeof(infoformatmap) / sizeof(infoformat); i++)
    {
      pos2 = work.Find(infoformatmap[i].str);
      if (pos2 != (int)string::npos && pos2 < pos1)
      {
        pos1 = pos2;
        len = strlen(infoformatmap[i].str);
        format = infoformatmap[i].val;
      }
    }

    if (format != NONE)
    {
      if (pos1 > 0)
        m_info.push_back(CInfoPortion(0, work.Left(pos1), ""));

      pos2 = StringUtils::FindEndBracket(work, '[', ']', pos1 + len);
      if (pos2 > pos1)
      {
        // decipher the block
        CStdString block = work.Mid(pos1 + len, pos2 - pos1 - len);
        CStdStringArray params;
        StringUtils::SplitString(block, ",", params);
        int info;
        if (format == FORMATVAR)
        {
          info = g_infoManager.TranslateSkinVariableString(params[0], context);
          if (info == 0)
            info = g_infoManager.RegisterSkinVariableString(g_SkinInfo.CreateSkinVariable(params[0], context));
          if (info == 0) // skinner didn't define this conditional label!
            CLog::Log(LOGWARNING, "Label Formating: $VAR[%s] is not defined", params[0].c_str());
        }
        else
          info = g_infoManager.TranslateString(params[0]);
        CStdString prefix, postfix;
        if (params.size() > 1)
          prefix = params[1];
        if (params.size() > 2)
          postfix = params[2];
        m_info.push_back(CInfoPortion(info, prefix, postfix, format == FORMATESCINFO));
        // and delete it from our work string
        work = work.Mid(pos2 + 1);
      }
      else
      {
        CLog::Log(LOGERROR, "Error parsing label - missing ']' in \"%s\"", label.c_str());
        return;
      }
    }
  }
  while (format != NONE);

  if (!work.IsEmpty())
    m_info.push_back(CInfoPortion(0, work, ""));
}

CGUIInfoLabel::CInfoPortion::CInfoPortion(int info, const CStdString &prefix, const CStdString &postfix, bool escaped)
{
  m_info = info;
  m_prefix = prefix;
  m_postfix = postfix;
  m_escaped = escaped;
  // filter our prefix and postfix for comma's
  m_prefix.Replace("$COMMA", ",");
  m_postfix.Replace("$COMMA", ",");
  m_prefix.Replace("$LBRACKET", "["); m_prefix.Replace("$RBRACKET", "]");
  m_postfix.Replace("$LBRACKET", "["); m_postfix.Replace("$RBRACKET", "]");
}

CStdString CGUIInfoLabel::CInfoPortion::GetLabel(const CStdString &info) const
{
  CStdString label = m_prefix + info + m_postfix;
  if (m_escaped) // escape all quotes, then quote
  {
    label.Replace("\"", "\\\"");
    return "\"" + label + "\"";
  }
  return label;
}

CStdString CGUIInfoLabel::GetLabel(const CStdString &label, int contextWindow, bool preferImage)
{ // translate the label
  CGUIInfoLabel info(label, "", contextWindow);
  return info.GetLabel(contextWindow, preferImage);
}
