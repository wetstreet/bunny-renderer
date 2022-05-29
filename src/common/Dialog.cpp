#include "Dialog.h"

#include <sstream>

#include <shlobj.h>

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    //{L"PNG Files (*.png)",          L"*.png"},
    {L"Image (*.png; *.tga)",       L"*.png;*.tga"},
    {L"OBJ Files (*.obj)",          L"*.obj"},
    {L"All Documents (*.*)",        L"*.*"}
};

std::string wstring2string(std::wstring wstr)
{
    std::string result;
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    if (len <= 0)return result;
    char* buffer = new char[len + 1];
    if (buffer == NULL)return result;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    result.append(buffer);
    delete[] buffer;
    return result;
}

std::string FileDialog(const CLSID id)
{
    std::string s;
    // CoCreate the File Open Dialog object.
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(id, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the options on the dialog.
        DWORD dwFlags;

        pfd->GetOptions(&dwFlags);
        pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
        pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
        pfd->SetFileTypeIndex(1);
        pfd->SetDefaultExtension(L"png");

        // Show the dialog
        hr = pfd->Show(NULL);
        if (SUCCEEDED(hr))
        {
            // Obtain the result, once the user clicks the 'Open' button.
            // The result is an IShellItem object.
            IShellItem* psiResult;
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr))
            {
                // We are just going to print out the name of the file for sample sake.
                PWSTR pszFilePath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                std::wstringstream ss;
                ss << pszFilePath;
                s = wstring2string(ss.str());

                CoTaskMemFree(pszFilePath);
                psiResult->Release();
            }
        }

        pfd->Release();
    }
    return s;
}

std::string OpenFileDialog(int index)
{
    std::string s;
    // CoCreate the File Open Dialog object.
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the options on the dialog.
        DWORD dwFlags;

        pfd->GetOptions(&dwFlags);
        pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
        pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
        pfd->SetFileTypeIndex(index);

        // Show the dialog
        hr = pfd->Show(NULL);
        if (SUCCEEDED(hr))
        {
            // Obtain the result, once the user clicks the 'Open' button.
            // The result is an IShellItem object.
            IShellItem* psiResult;
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr))
            {
                // We are just going to print out the name of the file for sample sake.
                PWSTR pszFilePath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (pszFilePath != NULL)
                {
                    std::wstringstream ss;
                    ss << pszFilePath;
                    s = wstring2string(ss.str());

                    CoTaskMemFree(pszFilePath);
                }

                psiResult->Release();
            }
        }

        pfd->Release();
    }
    return s;
}

std::string SaveFileDialog()
{
    return FileDialog(CLSID_FileSaveDialog);
}