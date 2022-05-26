//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <string>
#include <sstream>

#include <shlobj.h>

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"PNG Files (*.png)",          L"*.png"},
    {L"All Documents (*.*)",        L"*.*"}
};

std::string wstring2string(std::wstring wstr);

std::string OpenFileDialog();

#endif // __DIALOG_H__