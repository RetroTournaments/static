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

namespace sta::smbui
{

class SMBTasCompareApplication : public rgmui::IApplication
{
public:
    SMBTasCompareApplication(smb::SMBDatabase* db);
    ~SMBTasCompareApplication();
};

}

#endif
