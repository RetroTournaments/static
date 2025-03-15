////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2025 Matthew Deutsch
//
// Static is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// Static is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Static; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#ifndef STATIC_SMB_TASCOMPARE_HEADER
#define STATIC_SMB_TASCOMPARE_HEADER

#include "smb/smbdb.h"
#include "rgmui/rgmui.h"
#include "smb/rgms.h"

namespace sta::smbui
{

struct TasCompareTAS
{
    std::string name;
    std::string path;
    int start;
    std::vector<rgms::SMBMessageProcessorOutput> outputs;
    std::vector<nes::Frame> frames;
    float opacity;
};

struct Shot
{
    smb::AreaID aid;
    int apx;
    int width;
    int follow_tas_index;
};

struct TasCompareInfo
{
    int start_frame;
    int end_frame;
    int current_frame;
    std::vector<TasCompareTAS> tases;
    std::vector<Shot> shots;
};


class SMBTasCompareApplication : public rgmui::IApplication
{
public:
    SMBTasCompareApplication(smb::SMBDatabase* db);
    ~SMBTasCompareApplication();

    bool OnFrame();

private:
    smb::SMBDatabase* m_db;
    TasCompareInfo m_info;
};

class SMBTasCompareTases : public rgmui::IApplicationComponent
{
public:
    SMBTasCompareTases(smb::SMBDatabase* db, TasCompareInfo* info);
    ~SMBTasCompareTases() = default;

    void OnFrame();

private:

    smb::SMBDatabase* m_db;
    TasCompareInfo* m_info;
};

class SMBTasCompareIndividual : public rgmui::IApplicationComponent
{
public:
    SMBTasCompareIndividual(smb::SMBDatabase* db, TasCompareInfo* info);
    ~SMBTasCompareIndividual() = default;

    void OnFrame();

private:
    smb::SMBDatabase* m_db;
    TasCompareInfo* m_info;
    int m_individual_index;
};

class SMBTasCompareCombined : public rgmui::IApplicationComponent
{
public:
    SMBTasCompareCombined(smb::SMBDatabase* db, TasCompareInfo* info);
    ~SMBTasCompareCombined() = default;

    void OnFrame();

private:
    smb::SMBDatabase* m_db;
    TasCompareInfo* m_info;
};

}

#endif
