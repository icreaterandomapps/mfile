#define UNICODE
#define _UNICODE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#ifndef IDI_APPICON
#define IDI_APPICON 101
#endif
#ifndef IDR_PNG_BACK
#define IDR_PNG_BACK      201
#define IDR_PNG_FORWARD   202
#define IDR_PNG_UP        203
#define IDR_PNG_REFRESH   204
#define IDR_PNG_DESKTOP   205
#define IDR_PNG_DOWNLOADS 206
#define IDR_PNG_DOCS      207
#define IDR_PNG_PICS      208
#define IDR_PNG_MUSIC     209
#define IDR_PNG_VIDEOS    210
#define IDR_PNG_CD        211
#endif

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shldisp.h>
#include <commdlg.h>
#include <ole2.h>
#include <uxtheme.h>
#include <winreg.h>
#include <gdiplus.h>
#include <stdio.h>
#include <wchar.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwchar>
#include <cwctype>
#include <ctime>

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP 0x00004000
#endif
#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER 0x0004
#endif
#ifndef CSIDL_MYVIDEO
#define CSIDL_MYVIDEO 0x000e
#endif
#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 0x000d
#endif
#ifndef CSIDL_MYPICTURES
#define CSIDL_MYPICTURES 0x0027
#endif
#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES 0x0026
#endif
#ifndef CSIDL_PROGRAM_FILESX86
#define CSIDL_PROGRAM_FILESX86 0x002a
#endif
#ifndef HDF_SORTUP
#define HDF_SORTUP 0x0400
#endif
#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN 0x0200
#endif
#ifndef LVM_SETSELECTEDCOLUMN
#define LVM_SETSELECTEDCOLUMN (LVM_FIRST + 140)
#endif
#ifndef CFSTR_PREFERREDDROPEFFECT
#define CFSTR_PREFERREDDROPEFFECT TEXT("Preferred DropEffect")
#endif
#ifndef DROPEFFECT_COPY
#define DROPEFFECT_COPY 1
#endif
#ifndef DROPEFFECT_MOVE
#define DROPEFFECT_MOVE 2
#endif
#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

// ListView group compatibility (ComCtl32 v6 features; defined if missing)
#ifndef LVM_ENABLEGROUPVIEW
#define LVM_ENABLEGROUPVIEW (LVM_FIRST + 157)
#endif
#ifndef LVM_INSERTGROUP
#define LVM_INSERTGROUP (LVM_FIRST + 145)
#endif
#ifndef LVM_REMOVEALLGROUPS
#define LVM_REMOVEALLGROUPS (LVM_FIRST + 160)
#endif
#ifndef LVM_SETITEMGROUPID
#define LVM_SETITEMGROUPID (LVM_FIRST + 148)
#endif
#ifndef LVM_SETGROUPMETRICS
#define LVM_SETGROUPMETRICS (LVM_FIRST + 155)
#endif
#ifndef LVIF_GROUPID
#define LVIF_GROUPID 0x0100
#endif
#ifndef LVGF_HEADER
#define LVGF_HEADER 0x00000001
#endif
#ifndef LVGF_GROUPID
#define LVGF_GROUPID 0x00000010
#endif
#ifndef LVGS_NORMAL
#define LVGS_NORMAL 0x00000000
#endif
#ifndef LVGS_COLLAPSIBLE
#define LVGS_COLLAPSIBLE 0x00000008
#endif
#ifndef LVGA_HEADER_LEFT
#define LVGA_HEADER_LEFT 0x00000001
#endif
#ifndef LVGF_ALIGN
#define LVGF_ALIGN 0x00000008
#endif
#ifndef LVGF_STATE
#define LVGF_STATE 0x00000004
#endif
#ifndef LVGMF_NONE
#define LVGMF_NONE 0x00000000
#endif
#ifndef LVGMF_TEXTCOLOR
#define LVGMF_TEXTCOLOR 0x00000004
#endif

#ifndef LVGROUPMETRICS_DEFINED_COMPAT
typedef struct tagLVGROUPMETRICS_COMPAT {
    UINT cbSize;
    UINT mask;
    UINT Left;
    UINT Top;
    UINT Right;
    UINT Bottom;
    COLORREF crLeft;
    COLORREF crTop;
    COLORREF crRight;
    COLORREF crBottom;
    COLORREF crHeader;
    COLORREF crFooter;
} LVGROUPMETRICS_COMPAT, *PLVGROUPMETRICS_COMPAT;
#endif

#if !defined(LVGROUP_V5_SIZE)
typedef struct tagLVGROUP_COMPAT {
    UINT    cbSize;
    UINT    mask;
    LPWSTR  pszHeader;
    int     cchHeader;
    LPWSTR  pszFooter;
    int     cchFooter;
    int     iGroupId;
    UINT    stateMask;
    UINT    state;
    UINT    uAlign;
} LVGROUP_COMPAT, *PLVGROUP_COMPAT;
#else
typedef LVGROUP LVGROUP_COMPAT;
typedef PLVGROUP PLVGROUP_COMPAT;
#endif

HWND mainWnd = NULL;
HWND addressBar = NULL;
HWND goBtn = NULL;
HWND treeView = NULL;
HWND fileList = NULL;
HWND statusBar = NULL;
HWND backBtn = NULL, forwardBtn = NULL, upBtn = NULL, refreshBtn = NULL;
HWND sortBtn = NULL, darkBtn = NULL, newFolderBtn = NULL;
HWND searchBox = NULL, searchBtn = NULL;
HFONT appFont = NULL;
HIMAGELIST g_sysSmall = NULL;
HICON g_appIconBig = NULL;
HICON g_appIconSmall = NULL;
bool g_comInit = false;
bool g_updatingTreeSelection = false;

std::wstring currentPath;
std::vector<std::wstring> history;
int historyPos = -1;

int g_leftPaneWidth = 200;
bool g_dragSplitter = false;
bool g_darkMode = false;
bool g_clipCut = false; // cut vs copy

HBRUSH g_darkBrush = NULL;
HBRUSH g_darkEditBrush = NULL;
COLORREF g_darkBg = RGB(32, 32, 32);
COLORREF g_darkBg2 = RGB(43, 43, 43);
COLORREF g_darkText = RGB(232, 232, 232);

ULONG_PTR g_gdiplusToken = 0;
Gdiplus::Image* g_imgBack = NULL;
Gdiplus::Image* g_imgForward = NULL;
Gdiplus::Image* g_imgUp = NULL;
Gdiplus::Image* g_imgRefresh = NULL;
Gdiplus::Image* g_imgDesktop = NULL;
Gdiplus::Image* g_imgDownloads = NULL;
Gdiplus::Image* g_imgDocs = NULL;
Gdiplus::Image* g_imgPics = NULL;
Gdiplus::Image* g_imgMusic = NULL;
Gdiplus::Image* g_imgVideos = NULL;
Gdiplus::Image* g_imgCD = NULL;

static const int TOPBAR_HEIGHT = 38;
static const int SPLITTER_HIT = 6;

enum {
    ID_BACK = 100, ID_FORWARD, ID_UP, ID_REFRESH,
    ID_ADDRESS, ID_GO, ID_TREE, ID_LIST,
    ID_SORTBTN, ID_DARKBTN, ID_NEWFOLDERBTN,
    ID_SEARCH, ID_SEARCHBTN,

    ID_CTX_OPEN = 2001,
    ID_CTX_COPY, ID_CTX_CUT, ID_CTX_PASTE,
    ID_CTX_DELETE, ID_CTX_RENAME, ID_CTX_NEWFOLDER,
    ID_CTX_PROPERTIES, ID_CTX_REFRESH, ID_CTX_SELECTALL,
    ID_CTX_COMPRESS, ID_CTX_OPENZIP, ID_CTX_EXTRACT, ID_CTX_EXTRACT_FOLDER,
    ID_CTX_COPY_PATH, ID_CTX_COPY_NAME, ID_CTX_OPEN_CMD, ID_CTX_OPEN_EXPLORER,

    ID_SORT_NAME_ASC = 3001, ID_SORT_NAME_DESC,
    ID_SORT_DATE_ASC, ID_SORT_DATE_DESC,
    ID_SORT_TYPE_ASC, ID_SORT_TYPE_DESC,
    ID_SORT_SIZE_ASC, ID_SORT_SIZE_DESC,
    ID_SORT_DATE_GROUPED, ID_SORT_SHOW_HIDDEN,

    ID_CMD_FOCUS_ADDRESS = 4001, ID_CMD_FOCUS_SEARCH,
    ID_CMD_UP, ID_CMD_BACK, ID_CMD_FORWARD
};

enum SortMode {
    SORT_NAME_ASC = 0, SORT_NAME_DESC,
    SORT_DATE_ASC, SORT_DATE_DESC,
    SORT_TYPE_ASC, SORT_TYPE_DESC,
    SORT_SIZE_ASC, SORT_SIZE_DESC
};

enum UiIconKind { UIICON_NONE = 0, UIICON_DESKTOP, UIICON_DOWNLOADS, UIICON_DOCS, UIICON_PICS, UIICON_MUSIC, UIICON_VIDEOS, UIICON_CD };

struct FileItem {
    std::wstring name;
    std::wstring fullPath;
    std::wstring typeText;
    bool isDir;
    bool isZip;
    bool isHidden;
    ULONGLONG sizeBytes;
    FILETIME ftModified;
    FILETIME ftCreated;
    SYSTEMTIME stModifiedLocal;
    SYSTEMTIME stCreatedLocal;
    int icon;
};

std::vector<FileItem> g_items;
std::vector<FileItem> g_allItems; // unfiltered items for current directory
bool g_dirTruncated = false;
size_t g_dirTotalCount = 0;
const size_t MAX_DIR_ITEMS = 50000;  // safety cap for very large folders/drives
SortMode g_sortMode = SORT_NAME_ASC;
bool g_groupByDate = false;
std::wstring g_statusText;

bool g_searchMode = false;      // showing recursive search results
bool g_searching = false;       // reentrancy guard
std::wstring g_searchRoot;
std::wstring g_currentFilter;   // filter within current directory view
bool g_showHidden = false;
bool g_useContextFiles = false;
std::vector<std::wstring> g_contextFiles;

WNDPROC g_oldEditProc = NULL;
WNDPROC g_oldSearchProc = NULL;
WNDPROC g_oldHeaderProc = NULL;
HACCEL g_accel = NULL;

// Forward declarations
bool Navigate(const std::wstring& path, bool record = true);
void SelectTreePath(const std::wstring& path);
void RefreshView();
void ApplyTheme();
void ApplyThemeToChildren(HWND hwnd);
std::vector<std::wstring> SelectedFiles();
void PerformSearch();
void ClearSearchMode();
void LoadDirectory(const std::wstring& path);
void SetListColumnsForMode();
void ApplyCurrentFilter();
void UpdateStatusBar();
void OpenSelected();
void DeleteSelected(bool toRecycleBin);
void CreateNewFolder();
void ShowItemContextMenu(int x, int y, bool onItem);

std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == L'\\' || a.back() == L'/') return a + b;
    return a + L"\\" + b;
}

const wchar_t* THIS_PC_PATH = L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";

bool IsThisPCPath(const std::wstring& p) {
    if (p.empty()) return false;
    if (lstrcmpiW(p.c_str(), THIS_PC_PATH) == 0) return true;
    if (lstrcmpiW(p.c_str(), L"This PC") == 0) return true;
    if (lstrcmpiW(p.c_str(), L"Computer") == 0) return true;
    return false;
}

std::wstring NormalizeDirPath(std::wstring p) {
    if (IsThisPCPath(p)) return THIS_PC_PATH;
    while (p.size() > 3 && (p.back() == L'\\' || p.back() == L'/')) p.erase(p.size() - 1);
    return p;
}

bool PathEqualsInsensitive(const std::wstring& a, const std::wstring& b) {
    if (IsThisPCPath(a) && IsThisPCPath(b)) return true;
    return lstrcmpiW(NormalizeDirPath(a).c_str(), NormalizeDirPath(b).c_str()) == 0;
}

bool IsDriveRootPath(const std::wstring& p) {
    std::wstring n = NormalizeDirPath(p);
    return n.size() == 3 && ((n[0] >= L'A' && n[0] <= L'Z') || (n[0] >= L'a' && n[0] <= L'z')) && n[1] == L':' && (n[2] == L'\\' || n[2] == L'/');
}

bool IsDownloadsPath(const std::wstring& p) {
    wchar_t buf[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, buf)) && buf[0]) {
        std::wstring d = JoinPath(buf, L"Downloads");
        return PathEqualsInsensitive(p, d);
    }
    return false;
}

void Status(const std::wstring& s) {
    g_statusText = s;
    if (statusBar) SendMessageW(statusBar, SB_SETTEXTW, 0 | SBT_OWNERDRAW, (LPARAM)g_statusText.c_str());
}

std::wstring FormatSizeShort(ULONGLONG bytes) {
    const wchar_t* units[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    double v = (double)bytes;
    int u = 0;
    while (v >= 1024.0 && u < 4) { v /= 1024.0; ++u; }
    wchar_t buf[64] = {};
    if (u == 0) _snwprintf(buf, 64, L"%I64u %s", (unsigned long long)bytes, units[u]);
    else _snwprintf(buf, 64, L"%.1f %s", v, units[u]);
    buf[63] = 0;
    return buf;
}

void UpdateStatusBar() {
    if (!statusBar) return;
    int total = (int)g_items.size();
    int selected = 0;
    ULONGLONG selBytes = 0;
    bool selHasDir = false;
    if (fileList) {
        selected = ListView_GetSelectedCount(fileList);
        if (selected > 0) {
            int i = -1;
            while ((i = ListView_GetNextItem(fileList, i, LVNI_SELECTED)) != -1) {
                LVITEMW lvi = {}; lvi.mask = LVIF_PARAM; lvi.iItem = i;
                if (!ListView_GetItem(fileList, &lvi)) continue;
                size_t idx = (size_t)lvi.lParam;
                if (idx >= g_items.size()) continue;
                if (g_items[idx].isDir) selHasDir = true;
                else selBytes += g_items[idx].sizeBytes;
            }
        }
    }

    wchar_t text[256] = {};
    if (g_searchMode) {
        if (selected > 0)
            _snwprintf(text, 256, L"%d selected    %d result%s", selected, total, total == 1 ? L"" : L"s");
        else
            _snwprintf(text, 256, L"%d result%s", total, total == 1 ? L"" : L"s");
    } else if (selected > 0) {
        if (selHasDir)
            _snwprintf(text, 256, L"%d items    %d selected", total, selected);
        else
            _snwprintf(text, 256, L"%d items    %d selected (%s)", total, selected, FormatSizeShort(selBytes).c_str());
        if (g_dirTruncated && g_currentFilter.empty()) {
            wchar_t extra[64] = {};
            _snwprintf(extra, 64, L" (first %I64u of %I64u)", (unsigned long long)g_allItems.size(), (unsigned long long)g_dirTotalCount);
            extra[63] = 0;
            size_t cur = wcslen(text);
            if (cur < 255) wcsncat(text, extra, 255 - cur);
        }
    } else {
        _snwprintf(text, 256, L"%d items", total);
        if (!g_currentFilter.empty()) {
            _snwprintf(text, 256, L"%d of %d items", total, (int)g_allItems.size());
            if (g_dirTruncated) {
                wchar_t extra[64] = {};
                _snwprintf(extra, 64, L" (first %I64u of %I64u)", (unsigned long long)g_allItems.size(), (unsigned long long)g_dirTotalCount);
                extra[63] = 0;
                size_t cur = wcslen(text);
                if (cur < 255) wcsncat(text, extra, 255 - cur);
            }
        } else if (g_dirTruncated) {
            _snwprintf(text, 256, L"Showing %d of %I64u items", total, (unsigned long long)g_dirTotalCount);
        }
    }

    // append free space for current drive
    if (!g_searchMode && !currentPath.empty()) {
        ULARGE_INTEGER freeBytes = {}, totalBytes = {};
        std::wstring root = currentPath.size() >= 3 ? currentPath.substr(0, 3) : currentPath;
        if (GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, NULL)) {
            wchar_t extra[128] = {};
            _snwprintf(extra, 128, L"    %s free", FormatSizeShort(freeBytes.QuadPart).c_str());
            extra[127] = 0;
            size_t cur = wcslen(text);
            if (cur < 255) wcsncat(text, extra, 255 - cur);
        }
    }
    text[255] = 0;
    Status(text);
}

void LoadAppIcon() {
    HINSTANCE hi = GetModuleHandleW(NULL);
    g_appIconBig = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 32, 32, 0);
    g_appIconSmall = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 16, 16, 0);
    if (!g_appIconBig) {
        g_appIconBig = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    }
    if (g_appIconBig || g_appIconSmall) {
        if (!g_appIconSmall) g_appIconSmall = g_appIconBig;
        return;
    }

    // Fallback: load from icons\mfile.ico or mfile.ico next to the executable
    wchar_t exePath[MAX_PATH] = {};
    if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) return;
    std::wstring dir = exePath;
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) dir = dir.substr(0, pos + 1);
    else dir.clear();

    const wchar_t* candidates[] = { L"icons\\mfile.ico", L"mfile.ico" };
    for (int i = 0; i < 2 && !g_appIconBig && !g_appIconSmall; ++i) {
        std::wstring icoPath = dir + candidates[i];
        g_appIconBig = (HICON)LoadImageW(NULL, icoPath.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        g_appIconSmall = (HICON)LoadImageW(NULL, icoPath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        if (!g_appIconBig) {
            g_appIconBig = (HICON)LoadImageW(NULL, icoPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        }
        if (!g_appIconSmall) g_appIconSmall = g_appIconBig;
    }
}

void UpdateButtons() {
    if (backBtn) EnableWindow(backBtn, historyPos > 0);
    if (forwardBtn) EnableWindow(forwardBtn, historyPos >= 0 && historyPos < (int)history.size() - 1);
    if (upBtn) EnableWindow(upBtn, !IsThisPCPath(currentPath));
}

std::wstring GetSpecialFolderPath(int csidl) {
    wchar_t path[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, path)) && path[0] != L'\0') {
        return path;
    }
    return L"";
}

std::wstring GetDownloadsPath() {
    std::wstring p = GetSpecialFolderPath(CSIDL_PROFILE);
    if (!p.empty()) {
        std::wstring d = JoinPath(p, L"Downloads");
        DWORD attr = GetFileAttributesW(d.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) return d;
    }
    return L"";
}

bool IsZipName(const std::wstring& p) {
    if (p.length() < 4) return false;
    size_t n = p.length() - 4;
    return towlower((wint_t)p[n]) == L'.' &&
           towlower((wint_t)p[n + 1]) == L'z' &&
           towlower((wint_t)p[n + 2]) == L'i' &&
           towlower((wint_t)p[n + 3]) == L'p';
}

ULONGLONG FileTimeToULL(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

int CompareFileTimeDesc(const FILETIME& a, const FILETIME& b) {
    ULONGLONG ua = FileTimeToULL(a), ub = FileTimeToULL(b);
    if (ua == ub) return 0;
    return (ua > ub) ? -1 : 1; // desc
}

int DateGroupId(const SYSTEMTIME& st) {
    // 0 Today, 1 Yesterday, 2 Earlier this week, 3 Last week, 4 Earlier this month, 5 Earlier this year, 6 A long time ago
    if (st.wYear == 0) return 6;

    SYSTEMTIME now = {};
    GetLocalTime(&now);

    SYSTEMTIME today = now;
    today.wHour = 0; today.wMinute = 0; today.wSecond = 0; today.wMilliseconds = 0;
    SYSTEMTIME itemDay = st;
    itemDay.wHour = 0; itemDay.wMinute = 0; itemDay.wSecond = 0; itemDay.wMilliseconds = 0;

    FILETIME ftToday = {}, ftItem = {};
    if (!SystemTimeToFileTime(&today, &ftToday) || !SystemTimeToFileTime(&itemDay, &ftItem)) return 6;

    ULONGLONG uToday = FileTimeToULL(ftToday);
    ULONGLONG uItem = FileTimeToULL(ftItem);
    const ULONGLONG DAY = 864000000000ULL;

    if (uItem >= uToday) return 0; // Today (or future)
    int days = (int)((uToday - uItem) / DAY);
    if (days <= 0) return 0;
    if (days == 1) return 1; // Yesterday

    int dow = (int)now.wDayOfWeek; // 0 = Sunday
    if (days <= dow) return 2;          // Earlier this week
    if (days <= dow + 7) return 3;      // Last week

    if ((int)st.wYear == (int)now.wYear && (int)st.wMonth == (int)now.wMonth) return 4; // Earlier this month
    if ((int)st.wYear == (int)now.wYear) return 5; // Earlier this year

    return 6; // A long time ago
}

const wchar_t* DateGroupName(int id) {
    switch (id) {
        case 0: return L"Today";
        case 1: return L"Yesterday";
        case 2: return L"Earlier this week";
        case 3: return L"Last week";
        case 4: return L"Earlier this month";
        case 5: return L"Earlier this year";
        default: return L"A long time ago";
    }
}

bool EnsureCom() {
    if (g_comInit) return true;
    HRESULT hr = OleInitialize(NULL);
    if (SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE) {
        g_comInit = true;
        return true;
    }
    return false;
}

HRESULT DispGetDispatch(IDispatch* disp, const wchar_t* name, int argc, VARIANT* argv, IDispatch** outDisp) {
    if (outDisp) *outDisp = NULL;
    if (!disp || !name || !outDisp) return E_INVALIDARG;
    DISPID dispid = 0; LPOLESTR oleName = (LPOLESTR)name;
    HRESULT hr = disp->GetIDsOfNames(IID_NULL, &oleName, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = {}; params.rgvarg = argv; params.cArgs = (UINT)argc;
    VARIANT result; VariantInit(&result); EXCEPINFO ex = {}; UINT argErr = 0;
    hr = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &params, &result, &ex, &argErr);
    if (SUCCEEDED(hr)) {
        if (result.vt == VT_DISPATCH && result.pdispVal) { *outDisp = result.pdispVal; result.vt = VT_EMPTY; result.pdispVal = NULL; }
        else hr = E_NOINTERFACE;
    }
    VariantClear(&result); return hr;
}

HRESULT DispInvokeVoid(IDispatch* disp, const wchar_t* name, int argc, VARIANT* argv) {
    if (!disp || !name) return E_INVALIDARG;
    DISPID dispid = 0; LPOLESTR oleName = (LPOLESTR)name;
    HRESULT hr = disp->GetIDsOfNames(IID_NULL, &oleName, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = {}; params.rgvarg = argv; params.cArgs = (UINT)argc;
    VARIANT result; VariantInit(&result); EXCEPINFO ex = {}; UINT argErr = 0;
    hr = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &result, &ex, &argErr);
    VariantClear(&result); return hr;
}

// ---- 7-Zip integration (optional; uses installed or bundled 7zG.exe/7z.exe) ----
std::wstring g_7zGPath;
std::wstring g_7zPath;
std::wstring g_7zFMPath;
bool g_7zChecked = false;

bool FileExistsWPath(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring GetExeDir() {
    wchar_t exePath[MAX_PATH] = {};
    if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) return L"";
    std::wstring dir = exePath;
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) dir = dir.substr(0, pos + 1);
    else dir.clear();
    return dir;
}

void InitGdiplus() {
    if (g_gdiplusToken) return;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Ok) {
        g_gdiplusToken = 0;
    }
}

void ShutdownGdiplus() {
    if (g_gdiplusToken) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
}

Gdiplus::Image* LoadUiImageFromResource(int resId) {
    if (!g_gdiplusToken) return NULL;
    HRSRC hrsrc = FindResourceW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(resId), RT_RCDATA);
    if (!hrsrc) return NULL;
    DWORD size = SizeofResource(GetModuleHandleW(NULL), hrsrc);
    HGLOBAL hResData = LoadResource(GetModuleHandleW(NULL), hrsrc);
    if (!hResData || size == 0) return NULL;
    void* pResData = LockResource(hResData);
    if (!pResData) return NULL;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) return NULL;
    void* pMem = GlobalLock(hMem);
    if (!pMem) { GlobalFree(hMem); return NULL; }
    memcpy(pMem, pResData, size);
    GlobalUnlock(hMem);

    IStream* stream = NULL;
    if (CreateStreamOnHGlobal(hMem, TRUE, &stream) != S_OK || !stream) {
        GlobalFree(hMem);
        return NULL;
    }
    Gdiplus::Image* img = Gdiplus::Image::FromStream(stream);
    stream->Release();
    if (!img || img->GetLastStatus() != Gdiplus::Ok) {
        delete img;
        return NULL;
    }
    return img;
}

Gdiplus::Image* LoadUiImage(const std::wstring& relPath, int resId) {
    Gdiplus::Image* img = LoadUiImageFromResource(resId);
    if (img) return img;
    if (!g_gdiplusToken) return NULL;
    std::wstring path = GetExeDir() + relPath;
    img = Gdiplus::Image::FromFile(path.c_str());
    if (!img || img->GetLastStatus() != Gdiplus::Ok) {
        delete img;
        return NULL;
    }
    return img;
}

void LoadToolbarIcons() {
    if (!g_gdiplusToken) return;
    if (g_imgBack || g_imgForward || g_imgUp || g_imgRefresh) return;
    g_imgBack = LoadUiImage(L"icons\\leftarrow.png", IDR_PNG_BACK);
    g_imgForward = LoadUiImage(L"icons\\rightarrow.png", IDR_PNG_FORWARD);
    g_imgUp = LoadUiImage(L"icons\\uparrow.png", IDR_PNG_UP);
    g_imgRefresh = LoadUiImage(L"icons\\refresh.png", IDR_PNG_REFRESH);
    g_imgDesktop = LoadUiImage(L"icons\\desktop.png", IDR_PNG_DESKTOP);
    g_imgDownloads = LoadUiImage(L"icons\\downloads.png", IDR_PNG_DOWNLOADS);
    g_imgDocs = LoadUiImage(L"icons\\docs.png", IDR_PNG_DOCS);
    g_imgPics = LoadUiImage(L"icons\\pics.png", IDR_PNG_PICS);
    g_imgMusic = LoadUiImage(L"icons\\music.png", IDR_PNG_MUSIC);
    g_imgVideos = LoadUiImage(L"icons\\videos.png", IDR_PNG_VIDEOS);
    g_imgCD = LoadUiImage(L"icons\\cd.png", IDR_PNG_CD);
}

void UnloadToolbarIcons() {
    if (g_imgBack) { delete g_imgBack; g_imgBack = NULL; }
    if (g_imgForward) { delete g_imgForward; g_imgForward = NULL; }
    if (g_imgUp) { delete g_imgUp; g_imgUp = NULL; }
    if (g_imgRefresh) { delete g_imgRefresh; g_imgRefresh = NULL; }
    if (g_imgDesktop) { delete g_imgDesktop; g_imgDesktop = NULL; }
    if (g_imgDownloads) { delete g_imgDownloads; g_imgDownloads = NULL; }
    if (g_imgDocs) { delete g_imgDocs; g_imgDocs = NULL; }
    if (g_imgPics) { delete g_imgPics; g_imgPics = NULL; }
    if (g_imgMusic) { delete g_imgMusic; g_imgMusic = NULL; }
    if (g_imgVideos) { delete g_imgVideos; g_imgVideos = NULL; }
    if (g_imgCD) { delete g_imgCD; g_imgCD = NULL; }
}

Gdiplus::Image* GetUiIconImage(UiIconKind kind) {
    switch (kind) {
        case UIICON_DESKTOP: return g_imgDesktop;
        case UIICON_DOWNLOADS: return g_imgDownloads;
        case UIICON_DOCS: return g_imgDocs;
        case UIICON_PICS: return g_imgPics;
        case UIICON_MUSIC: return g_imgMusic;
        case UIICON_VIDEOS: return g_imgVideos;
        case UIICON_CD: return g_imgCD;
        default: return NULL;
    }
}

std::wstring ReadRegString(HKEY root, const wchar_t* subkey, const wchar_t* value) {
    HKEY h = NULL;
    if (RegOpenKeyExW(root, subkey, 0, KEY_READ, &h) != ERROR_SUCCESS) return L"";
    wchar_t buf[MAX_PATH * 2] = {};
    DWORD type = 0, cb = sizeof(buf) - sizeof(wchar_t);
    LONG r = RegQueryValueExW(h, value, NULL, &type, (LPBYTE)buf, &cb);
    RegCloseKey(h);
    if (r != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ)) return L"";
    buf[(cb / sizeof(wchar_t))] = 0;
    return std::wstring(buf);
}

void Detect7Zip() {
    if (g_7zChecked) return;
    g_7zChecked = true;
    g_7zGPath.clear(); g_7zPath.clear(); g_7zFMPath.clear();

    auto tryDir = [](const std::wstring& dir) {
        if (dir.empty()) return;
        std::wstring d = dir;
        if (!d.empty() && d.back() != L'\\' && d.back() != L'/') d += L"\\";
        if (g_7zGPath.empty() && FileExistsWPath(d + L"7zG.exe")) g_7zGPath = d + L"7zG.exe";
        if (g_7zPath.empty() && FileExistsWPath(d + L"7z.exe")) g_7zPath = d + L"7z.exe";
        if (g_7zFMPath.empty() && FileExistsWPath(d + L"7zFM.exe")) g_7zFMPath = d + L"7zFM.exe";
        if (g_7zPath.empty() && FileExistsWPath(d + L"7za.exe")) g_7zPath = d + L"7za.exe";
    };

    tryDir(GetExeDir());

    std::wstring reg = ReadRegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\7-Zip", L"Path");
    if (!reg.empty()) tryDir(reg);
    reg = ReadRegString(HKEY_CURRENT_USER, L"SOFTWARE\\7-Zip", L"Path");
    if (!reg.empty()) tryDir(reg);

    wchar_t pf[MAX_PATH] = {}, pf86[MAX_PATH] = {};
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, pf) == S_OK) tryDir(std::wstring(pf) + L"\\7-Zip");
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, pf86) == S_OK) tryDir(std::wstring(pf86) + L"\\7-Zip");

    if (g_7zGPath.empty() || g_7zPath.empty() || g_7zFMPath.empty()) {
        wchar_t found[MAX_PATH] = {};
        if (g_7zGPath.empty() && SearchPathW(NULL, L"7zG.exe", NULL, MAX_PATH, found, NULL)) g_7zGPath = found;
        if (g_7zPath.empty() && SearchPathW(NULL, L"7z.exe", NULL, MAX_PATH, found, NULL)) g_7zPath = found;
        if (g_7zFMPath.empty() && SearchPathW(NULL, L"7zFM.exe", NULL, MAX_PATH, found, NULL)) g_7zFMPath = found;
        if (g_7zPath.empty() && SearchPathW(NULL, L"7za.exe", NULL, MAX_PATH, found, NULL)) g_7zPath = found;
    }
}

bool Has7Zip() { Detect7Zip(); return !g_7zGPath.empty() || !g_7zPath.empty(); }
bool Has7ZipGUI() { Detect7Zip(); return !g_7zGPath.empty(); }

std::wstring QuoteArg(const std::wstring& s) {
    std::wstring r = L"\"";
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == L'"') r += L"\\\""; else r += s[i];
    }
    r += L"\"";
    return r;
}

bool RunProcessAsync(const std::wstring& exe, const std::wstring& args, const std::wstring& workDir) {
    std::wstring cmd = QuoteArg(exe);
    if (!args.empty()) { cmd += L" "; cmd += args; }
    std::vector<wchar_t> buf(cmd.begin(), cmd.end());
    buf.push_back(0);
    STARTUPINFOW si = {}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(exe.c_str(), &buf[0], NULL, NULL, FALSE, 0, NULL, workDir.empty() ? NULL : workDir.c_str(), &si, &pi);
    if (ok) { CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
    return ok != FALSE;
}

bool IsArchiveExt(const std::wstring& extLower) {
    return extLower == L"zip" || extLower == L"7z" || extLower == L"rar" || extLower == L"tar" ||
           extLower == L"gz" || extLower == L"tgz" || extLower == L"bz2" || extLower == L"tbz2" ||
           extLower == L"xz" || extLower == L"txz" || extLower == L"iso" || extLower == L"wim" ||
           extLower == L"cab" || extLower == L"arj" || extLower == L"lzma" || extLower == L"z" ||
           extLower == L"001";
}

bool IsArchivePath(const std::wstring& p) {
    const wchar_t* dot = wcsrchr(p.c_str(), L'.');
    if (!dot || !dot[1]) return false;
    std::wstring ext = dot + 1;
    for (size_t i = 0; i < ext.size(); ++i) ext[i] = (wchar_t)towlower((wint_t)ext[i]);
    // handle .tar.gz style? basic support
    if (ext == L"gz" || ext == L"bz2" || ext == L"xz") {
        std::wstring pre(p.c_str(), dot - p.c_str());
        const wchar_t* dot2 = wcsrchr(pre.c_str(), L'.');
        if (dot2 && dot2[1]) {
            std::wstring e2 = dot2 + 1;
            for (size_t i = 0; i < e2.size(); ++i) e2[i] = (wchar_t)towlower((wint_t)e2[i]);
            if (e2 == L"tar") return true;
        }
    }
    return IsArchiveExt(ext);
}

bool CreateEmptyZip(const std::wstring& zipPath);
bool CompressToZip(const std::vector<std::wstring>& files, const std::wstring& zipPath, std::wstring& err);
bool ExtractZip(const std::wstring& zipPath, const std::wstring& destFolder, std::wstring& err);

bool ExtractArchive(const std::wstring& archive, const std::wstring& destFolder, std::wstring& err) {
    Detect7Zip();
    std::wstring tool = !g_7zGPath.empty() ? g_7zGPath : g_7zPath;
    if (!tool.empty()) {
        std::wstring args = L"x " + QuoteArg(archive) + L" -o" + QuoteArg(destFolder) + L" -aos";
        // 7zG prompts for password if needed and shows GUI
        if (RunProcessAsync(tool, args, currentPath)) return true;
        err = L"Failed to launch 7-Zip.";
        return false;
    }
    // fallback: Shell ZIP (only .zip)
    if (IsZipName(archive)) return ExtractZip(archive, destFolder, err);
    err = L"No 7-Zip found. Install 7-Zip or place 7zG.exe next to mfile.exe to extract this archive.";
    return false;
}

bool CompressWith7Zip(const std::vector<std::wstring>& files, const std::wstring& outPath, const std::wstring& type, std::wstring& err) {
    if (files.empty()) { err = L"No files selected."; return false; }
    Detect7Zip();
    std::wstring tool = !g_7zGPath.empty() ? g_7zGPath : g_7zPath;
    if (tool.empty()) {
        if (type == L"zip") return CompressToZip(files, outPath, err);
        err = L"No 7-Zip found. Install 7-Zip or place 7zG.exe next to mfile.exe.";
        return false;
    }
    std::wstring args = L"a";
    if (!type.empty()) { args += L" -t"; args += type; }
    args += L" " + QuoteArg(outPath);
    for (size_t i = 0; i < files.size(); ++i) args += L" " + QuoteArg(files[i]);
    if (RunProcessAsync(tool, args, currentPath)) return true;
    err = L"Failed to launch 7-Zip.";
    return false;
}

bool ShellCopyToFolder(const std::vector<std::wstring>& sources, const std::wstring& destFolder, std::wstring& err) {
    if (!EnsureCom()) { err = L"COM initialization failed."; return false; }
    static const GUID CLSID_Shell_Local = {0x13709620,0xC279,0x11CE,{0xA4,0x9E,0x44,0x45,0x53,0x54,0x00,0x00}};
    IDispatch* pShell = NULL;
    HRESULT hr = CoCreateInstance(CLSID_Shell_Local, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pShell);
    if (FAILED(hr) || !pShell) { err = L"Could not create Shell.Application."; return false; }
    VARIANT vDestPath; VariantInit(&vDestPath); vDestPath.vt = VT_BSTR; vDestPath.bstrVal = SysAllocString(destFolder.c_str());
    VARIANT argNS[1]; argNS[0] = vDestPath;
    IDispatch* pDestFolder = NULL;
    hr = DispGetDispatch(pShell, L"NameSpace", 1, argNS, &pDestFolder);
    VariantClear(&vDestPath);
    if (FAILED(hr) || !pDestFolder) { pShell->Release(); err = L"Could not open destination folder."; return false; }
    bool ok = true;
    for (size_t i = 0; i < sources.size(); ++i) {
        VARIANT vOpt; VariantInit(&vOpt); vOpt.vt = VT_I4; vOpt.lVal = 16 | 1024;
        VARIANT vItem; VariantInit(&vItem); vItem.vt = VT_BSTR; vItem.bstrVal = SysAllocString(sources[i].c_str());
        VARIANT args[2]; args[0] = vOpt; args[1] = vItem;
        hr = DispInvokeVoid(pDestFolder, L"CopyHere", 2, args);
        VariantClear(&vItem); VariantClear(&vOpt);
        if (FAILED(hr)) { ok = false; err = L"CopyHere failed."; break; }
    }
    pDestFolder->Release(); pShell->Release(); return ok;
}

bool CreateEmptyZip(const std::wstring& zipPath) {
    HANDLE h = CreateFileW(zipPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    const unsigned char eocd[22] = {0x50,0x4B,0x05,0x06,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    DWORD written = 0; BOOL ok = WriteFile(h, eocd, sizeof(eocd), &written, NULL); CloseHandle(h);
    return ok && written == sizeof(eocd);
}

bool CompressToZip(const std::vector<std::wstring>& files, const std::wstring& zipPath, std::wstring& err) {
    if (files.empty()) { err = L"No files selected."; return false; }
    if (!CreateEmptyZip(zipPath)) { err = L"Could not create ZIP file."; return false; }
    if (!ShellCopyToFolder(files, zipPath, err)) return false;
    return true;
}

bool ExtractZip(const std::wstring& zipPath, const std::wstring& destFolder, std::wstring& err) {
    if (!EnsureCom()) { err = L"COM initialization failed."; return false; }
    static const GUID CLSID_Shell_Local = {0x13709620,0xC279,0x11CE,{0xA4,0x9E,0x44,0x45,0x53,0x54,0x00,0x00}};
    IDispatch* pShell = NULL;
    HRESULT hr = CoCreateInstance(CLSID_Shell_Local, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pShell);
    if (FAILED(hr) || !pShell) { err = L"Could not create Shell.Application."; return false; }
    VARIANT vZipPath; VariantInit(&vZipPath); vZipPath.vt = VT_BSTR; vZipPath.bstrVal = SysAllocString(zipPath.c_str());
    VARIANT argZip[1]; argZip[0] = vZipPath;
    IDispatch* pZipFolder = NULL; hr = DispGetDispatch(pShell, L"NameSpace", 1, argZip, &pZipFolder); VariantClear(&vZipPath);
    if (FAILED(hr) || !pZipFolder) { pShell->Release(); err = L"Could not open ZIP archive."; return false; }
    IDispatch* pItems = NULL; hr = DispGetDispatch(pZipFolder, L"Items", 0, NULL, &pItems);
    if (FAILED(hr) || !pItems) { pZipFolder->Release(); pShell->Release(); err = L"Could not read ZIP contents."; return false; }
    VARIANT vDestPath; VariantInit(&vDestPath); vDestPath.vt = VT_BSTR; vDestPath.bstrVal = SysAllocString(destFolder.c_str());
    VARIANT argDest[1]; argDest[0] = vDestPath;
    IDispatch* pDestFolder = NULL; hr = DispGetDispatch(pShell, L"NameSpace", 1, argDest, &pDestFolder); VariantClear(&vDestPath);
    if (FAILED(hr) || !pDestFolder) { pItems->Release(); pZipFolder->Release(); pShell->Release(); err = L"Could not open destination folder."; return false; }
    VARIANT vOpt; VariantInit(&vOpt); vOpt.vt = VT_I4; vOpt.lVal = 16 | 1024;
    VARIANT vItems; VariantInit(&vItems); vItems.vt = VT_DISPATCH; vItems.pdispVal = pItems;
    VARIANT args[2]; args[0] = vOpt; args[1] = vItems;
    hr = DispInvokeVoid(pDestFolder, L"CopyHere", 2, args);
    pDestFolder->Release(); pItems->Release(); pZipFolder->Release(); pShell->Release();
    if (FAILED(hr)) { err = L"Extraction failed."; return false; }
    return true;
}

// ---- Clipboard (CF_HDROP copy/cut/paste) ----
bool CopyFilesToClipboard(const std::vector<std::wstring>& paths, bool cut) {
    if (paths.empty()) return false;
    size_t bytes = sizeof(DROPFILES);
    for (size_t i = 0; i < paths.size(); ++i) bytes += (paths[i].size() + 1) * sizeof(wchar_t);
    bytes += sizeof(wchar_t); // double null

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
    if (!hMem) return false;
    DROPFILES* df = (DROPFILES*)GlobalLock(hMem);
    if (!df) { GlobalFree(hMem); return false; }
    df->pFiles = sizeof(DROPFILES);
    df->fWide = TRUE;
    wchar_t* dst = (wchar_t*)((BYTE*)df + sizeof(DROPFILES));
    for (size_t i = 0; i < paths.size(); ++i) {
        size_t len = paths[i].size();
        memcpy(dst, paths[i].c_str(), len * sizeof(wchar_t));
        dst += len;
        *dst++ = L'\0';
    }
    *dst = L'\0';
    GlobalUnlock(hMem);

    if (!OpenClipboard(mainWnd)) { GlobalFree(hMem); return false; }
    EmptyClipboard();
    SetClipboardData(CF_HDROP, hMem); // ownership transferred

    UINT fmt = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
    if (fmt) {
        HGLOBAL hEff = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
        if (hEff) {
            DWORD* pEff = (DWORD*)GlobalLock(hEff);
            if (pEff) { *pEff = cut ? DROPEFFECT_MOVE : DROPEFFECT_COPY; GlobalUnlock(hEff); }
            SetClipboardData(fmt, hEff);
        }
    }
    CloseClipboard();
    g_clipCut = cut;
    return true;
}

void PasteFromClipboard() {
    if (!OpenClipboard(mainWnd)) return;
    bool doMove = false;
    UINT fmt = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
    if (fmt) {
        HGLOBAL hEff = (HGLOBAL)GetClipboardData(fmt);
        if (hEff) {
            DWORD* pEff = (DWORD*)GlobalLock(hEff);
            if (pEff) { doMove = ((*pEff & DROPEFFECT_MOVE) != 0); GlobalUnlock(hEff); }
        }
    }
    HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
    if (!hDrop) { CloseClipboard(); return; }
    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
    std::vector<std::wstring> files;
    for (UINT i = 0; i < count; ++i) {
        UINT len = DragQueryFileW(hDrop, i, NULL, 0);
        if (len == 0) continue;
        std::vector<wchar_t> buf(len + 1, 0);
        DragQueryFileW(hDrop, i, &buf[0], len + 1);
        files.push_back(&buf[0]);
    }
    CloseClipboard();

    if (files.empty()) return;

    // SHFileOperationW with double-null-terminated "from"
    std::wstring from;
    for (size_t i = 0; i < files.size(); ++i) { from += files[i]; from.push_back(L'\0'); }
    from.push_back(L'\0');
    std::wstring to = currentPath;
    to.push_back(L'\0'); to.push_back(L'\0');

    SHFILEOPSTRUCTW op = {};
    op.hwnd = mainWnd;
    op.wFunc = doMove ? FO_MOVE : FO_COPY;
    op.pFrom = from.c_str();
    op.pTo = to.c_str();
    op.fFlags = FOF_ALLOWUNDO;
    int res = SHFileOperationW(&op);
    if (res == 0 && !op.fAnyOperationsAborted) {
        if (doMove) {
            // clear clipboard after cut-paste
            if (OpenClipboard(mainWnd)) { EmptyClipboard(); CloseClipboard(); }
            g_clipCut = false;
        }
        Navigate(currentPath, false);
    } else {
        Status(L"Paste operation failed or was cancelled.");
    }
}

void DeleteFiles(const std::vector<std::wstring>& files, bool toRecycleBin) {
    if (files.empty()) return;
    std::wstring from;
    for (size_t i = 0; i < files.size(); ++i) { from += files[i]; from.push_back(L'\0'); }
    from.push_back(L'\0');
    SHFILEOPSTRUCTW op = {};
    op.hwnd = mainWnd;
    op.wFunc = FO_DELETE;
    op.pFrom = from.c_str();
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
    if (!toRecycleBin) op.fFlags &= ~FOF_ALLOWUNDO;
    int confirm = MessageBoxW(mainWnd, toRecycleBin ? L"Move selected item(s) to Recycle Bin?" : L"Permanently delete selected item(s)?",
        L"mFile", MB_ICONWARNING | MB_YESNO);
    if (confirm != IDYES) return;
    int res = SHFileOperationW(&op);
    if (res == 0) Navigate(currentPath, false);
    else Status(L"Delete failed or was cancelled.");
}

void DeleteSelected(bool toRecycleBin) {
    DeleteFiles(SelectedFiles(), toRecycleBin);
}

void ShowProperties(const std::wstring& path) {
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.hwnd = mainWnd;
    sei.lpVerb = L"properties";
    sei.lpFile = path.c_str();
    sei.nShow = SW_SHOW;
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    ShellExecuteExW(&sei);
}

void CopyTextToClipboard(const std::wstring& text, const wchar_t* statusMsg) {
    if (text.empty()) return;
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
    if (!hMem) return;
    void* p = GlobalLock(hMem);
    if (p) { memcpy(p, text.c_str(), bytes); GlobalUnlock(hMem); }
    if (OpenClipboard(mainWnd)) {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
        if (statusMsg) Status(statusMsg);
    } else {
        GlobalFree(hMem);
    }
}

void CopyPathsToClipboard(const std::vector<std::wstring>& paths) {
    if (paths.empty()) return;
    std::wstring text;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i) text += L"\r\n";
        text += paths[i];
    }
    CopyTextToClipboard(text, paths.size() == 1 ? L"Path copied." : L"Paths copied.");
}

void CopyNamesToClipboard(const std::vector<std::wstring>& names) {
    if (names.empty()) return;
    std::wstring text;
    for (size_t i = 0; i < names.size(); ++i) {
        if (i) text += L"\r\n";
        text += names[i];
    }
    CopyTextToClipboard(text, names.size() == 1 ? L"Name copied." : L"Names copied.");
}

void OpenInExplorer(const std::wstring& path) {
    std::wstring p = path;
    DWORD a = GetFileAttributesW(p.c_str());
    if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) {
        // select file in explorer
        std::wstring args = L"/select,\"" + p + L"\"";
        ShellExecuteW(mainWnd, L"open", L"explorer.exe", args.c_str(), NULL, SW_SHOWNORMAL);
    } else {
        ShellExecuteW(mainWnd, L"open", p.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void OpenInCommandPrompt(const std::wstring& dir) {
    std::wstring d = dir;
    DWORD a = GetFileAttributesW(d.c_str());
    if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY)) {
        size_t pos = d.find_last_of(L"\\/");
        if (pos != std::wstring::npos) d = d.substr(0, pos);
    }
    if (d.empty()) return;
    ShellExecuteW(mainWnd, L"open", L"cmd.exe", NULL, d.c_str(), SW_SHOWNORMAL);
}

void GoUp() {
    if (IsThisPCPath(currentPath)) return;
    if (g_searchMode) { g_searchMode = false; g_searchRoot.clear(); SetListColumnsForMode(); LoadDirectory(currentPath); ApplyCurrentFilter(); return; }
    if (IsDriveRootPath(currentPath)) { Navigate(THIS_PC_PATH); return; }
    if (currentPath.size() > 3) {
        size_t pos = currentPath.find_last_of(L'\\', currentPath.size() - 2);
        Navigate((pos == std::wstring::npos || pos == 2) ? currentPath.substr(0, 3) : currentPath.substr(0, pos));
    }
}

void GoBack() {
    if (historyPos > 0) { int t = historyPos - 1; if (Navigate(history[t], false)) historyPos = t; UpdateButtons(); }
}

void GoForward() {
    if (historyPos >= 0 && historyPos + 1 < (int)history.size()) { int t = historyPos + 1; if (Navigate(history[t], false)) historyPos = t; UpdateButtons(); }
}

void CreateNewFolder() {
    std::wstring base = JoinPath(currentPath, L"New folder");
    std::wstring path = base;
    int n = 1;
    while (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
        wchar_t buf[32]; _snwprintf(buf, 32, L" (%d)", n++); buf[31] = 0;
        path = base + buf;
    }
    if (CreateDirectoryW(path.c_str(), NULL)) {
        Navigate(currentPath, false);
        // try to select and rename
        int count = ListView_GetItemCount(fileList);
        std::wstring name = path.substr(path.find_last_of(L'\\') + 1);
        for (int i = 0; i < count; ++i) {
            wchar_t nm[MAX_PATH] = {};
            ListView_GetItemText(fileList, i, 0, nm, MAX_PATH);
            if (lstrcmpiW(nm, name.c_str()) == 0) {
                ListView_SetItemState(fileList, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_SetItemState(fileList, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(fileList, i, FALSE);
                SetFocus(fileList);
                ListView_EditLabel(fileList, i);
                break;
            }
        }
    } else {
        Status(L"Could not create folder.");
    }
}

std::vector<std::wstring> SelectedFilesOrCurrent() {
    std::vector<std::wstring> sel = SelectedFiles();
    if (!sel.empty()) return sel;
    if (!currentPath.empty()) sel.push_back(currentPath);
    return sel;
}

// ---- Sorting & view model ----
static const FILETIME& ItemGroupTime(const FileItem& it) {
    // For Downloads/grouped view, use creation time when available so newly created folders/files land in the right day bucket.
    if (FileTimeToULL(it.ftCreated) != 0) return it.ftCreated;
    return it.ftModified;
}

static const SYSTEMTIME& ItemGroupSt(const FileItem& it) {
    if (it.stCreatedLocal.wYear != 0) return it.stCreatedLocal;
    return it.stModifiedLocal;
}

bool ItemLess(const FileItem& a, const FileItem& b) {
    // Downloads-style grouped view: group by date, files first in each group (newest first), folders at bottom of each group
    if (g_groupByDate) {
        int ga = DateGroupId(ItemGroupSt(a));
        int gb = DateGroupId(ItemGroupSt(b));
        if (ga != gb) return ga < gb; // Today (0) first
        if (a.isDir != b.isDir) return !a.isDir; // files before folders within the same group
        int c = CompareFileTimeDesc(ItemGroupTime(a), ItemGroupTime(b));
        if (c != 0) return c < 0; // newest first
        return lstrcmpiW(a.name.c_str(), b.name.c_str()) < 0;
    }

    auto cmpName = [&]() { return lstrcmpiW(a.name.c_str(), b.name.c_str()); };
    switch (g_sortMode) {
        case SORT_NAME_ASC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            return cmpName() < 0;
        case SORT_NAME_DESC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            return cmpName() > 0;
        case SORT_DATE_ASC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            { int c = CompareFileTimeDesc(a.ftModified, b.ftModified); if (c != 0) return c > 0; }
            return cmpName() < 0;
        case SORT_DATE_DESC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            { int c = CompareFileTimeDesc(a.ftModified, b.ftModified); if (c != 0) return c < 0; }
            return cmpName() < 0;
        case SORT_TYPE_ASC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            { int c = lstrcmpiW(a.typeText.c_str(), b.typeText.c_str()); if (c != 0) return c < 0; }
            return cmpName() < 0;
        case SORT_TYPE_DESC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            { int c = lstrcmpiW(a.typeText.c_str(), b.typeText.c_str()); if (c != 0) return c > 0; }
            return cmpName() < 0;
        case SORT_SIZE_ASC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            if (a.sizeBytes != b.sizeBytes) return a.sizeBytes < b.sizeBytes;
            return cmpName() < 0;
        case SORT_SIZE_DESC:
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            if (a.sizeBytes != b.sizeBytes) return a.sizeBytes > b.sizeBytes;
            return cmpName() < 0;
    }
    return cmpName() < 0;
}

void UpdateSortHeader() {
    HWND hHeader = ListView_GetHeader(fileList);
    if (!hHeader) return;
    for (int i = 0; i < 4; ++i) {
        HDITEMW hdi = {}; hdi.mask = HDI_FORMAT;
        Header_GetItem(hHeader, i, &hdi);
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        bool asc = true; int col = -1;
        switch (g_sortMode) {
            case SORT_NAME_ASC: col = 0; asc = true; break;
            case SORT_NAME_DESC: col = 0; asc = false; break;
            case SORT_DATE_ASC: col = 1; asc = true; break;
            case SORT_DATE_DESC: col = 1; asc = false; break;
            case SORT_TYPE_ASC: col = 2; asc = true; break;
            case SORT_TYPE_DESC: col = 2; asc = false; break;
            case SORT_SIZE_ASC: col = 3; asc = true; break;
            case SORT_SIZE_DESC: col = 3; asc = false; break;
        }
        if (g_groupByDate && i == 1) { col = 1; asc = false; }
        if (i == col) hdi.fmt |= asc ? HDF_SORTUP : HDF_SORTDOWN;
        Header_SetItem(hHeader, i, &hdi);
    }
}

void ApplyListGroupColors() {
    if (!fileList) return;
    LVGROUPMETRICS_COMPAT gm = {};
    gm.cbSize = sizeof(gm);
    gm.mask = LVGMF_TEXTCOLOR;
    gm.crHeader = g_darkMode ? g_darkText : CLR_DEFAULT;
    SendMessageW(fileList, LVM_SETGROUPMETRICS, 0, (LPARAM)&gm);
}

void ApplyDarkListColors() {
    if (!fileList || !treeView) return;
    if (g_darkMode) {
        ListView_SetBkColor(fileList, g_darkBg);
        ListView_SetTextBkColor(fileList, CLR_NONE);
        ListView_SetTextColor(fileList, g_darkText);
        TreeView_SetBkColor(treeView, g_darkBg2);
        TreeView_SetTextColor(treeView, g_darkText);
        SendMessageW(fileList, LVM_SETTEXTCOLOR, 0, (LPARAM)g_darkText);
        SendMessageW(fileList, LVM_SETBKCOLOR, 0, (LPARAM)g_darkBg);
        SendMessageW(fileList, LVM_SETTEXTBKCOLOR, 0, (LPARAM)CLR_NONE);
    } else {
        ListView_SetBkColor(fileList, CLR_NONE);
        ListView_SetTextBkColor(fileList, CLR_NONE);
        ListView_SetTextColor(fileList, CLR_NONE);
        SendMessageW(fileList, LVM_SETTEXTCOLOR, 0, (LPARAM)CLR_DEFAULT);
        SendMessageW(fileList, LVM_SETBKCOLOR, 0, (LPARAM)CLR_NONE);
        SendMessageW(fileList, LVM_SETTEXTBKCOLOR, 0, (LPARAM)CLR_NONE);
        TreeView_SetBkColor(treeView, (COLORREF)-1);
        TreeView_SetTextColor(treeView, CLR_NONE);
    }
    ApplyListGroupColors();
    InvalidateRect(fileList, NULL, TRUE);
    InvalidateRect(treeView, NULL, TRUE);
}

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

typedef int (WINAPI *PFN_SetPreferredAppMode)(int);
typedef BOOL (WINAPI *PFN_AllowDarkModeForWindow)(HWND, BOOL);
typedef void (WINAPI *PFN_FlushMenuThemes)(void);

void ApplyAppDarkModePreference(bool dark) {
    HMODULE hUx = GetModuleHandleW(L"uxtheme.dll");
    if (!hUx) hUx = LoadLibraryW(L"uxtheme.dll");
    if (!hUx) return;

    PFN_SetPreferredAppMode pSetPreferredAppMode = (PFN_SetPreferredAppMode)GetProcAddress(hUx, MAKEINTRESOURCEA(135));
    PFN_AllowDarkModeForWindow pAllowDarkModeForWindow = (PFN_AllowDarkModeForWindow)GetProcAddress(hUx, MAKEINTRESOURCEA(133));
    PFN_FlushMenuThemes pFlushMenuThemes = (PFN_FlushMenuThemes)GetProcAddress(hUx, MAKEINTRESOURCEA(136));

    if (pSetPreferredAppMode) {
        // 0=Default, 1=AllowDark, 2=ForceDark, 3=ForceLight
        pSetPreferredAppMode(dark ? 2 : 3);
    }
    if (pAllowDarkModeForWindow && mainWnd) {
        pAllowDarkModeForWindow(mainWnd, dark ? TRUE : FALSE);
    }
    if (pFlushMenuThemes) {
        pFlushMenuThemes();
    }
}

void ApplyWindowDarkTitleBar(bool dark) {
    if (!mainWnd) return;
    BOOL useDark = dark ? TRUE : FALSE;
    HMODULE hDwm = GetModuleHandleW(L"dwmapi.dll");
    if (!hDwm) hDwm = LoadLibraryW(L"dwmapi.dll");
    if (hDwm) {
        typedef HRESULT (WINAPI *PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
        PFN_DwmSetWindowAttribute pDwmSetWindowAttribute =
            (PFN_DwmSetWindowAttribute)GetProcAddress(hDwm, "DwmSetWindowAttribute");
        if (pDwmSetWindowAttribute) {
            pDwmSetWindowAttribute(mainWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));
            pDwmSetWindowAttribute(mainWnd, 19, &useDark, sizeof(useDark));
        }
    }
    ApplyAppDarkModePreference(dark);
}

void ApplyControlThemes() {
    const wchar_t* dark = g_darkMode ? L"DarkMode_Explorer" : NULL;
    if (treeView) SetWindowTheme(treeView, dark, NULL);
    if (fileList) SetWindowTheme(fileList, dark, NULL);

    HWND hdr = fileList ? ListView_GetHeader(fileList) : NULL;
    if (hdr) SetWindowTheme(hdr, g_darkMode ? L"DarkMode_ItemsView" : L"ItemsView", NULL);

    if (addressBar) SetWindowTheme(addressBar, g_darkMode ? L"DarkMode_CFD" : NULL, NULL);
    if (searchBox) SetWindowTheme(searchBox, g_darkMode ? L"DarkMode_CFD" : NULL, NULL);

    // Re-apply colors after theme changes
    ApplyDarkListColors();
    if (addressBar) InvalidateRect(addressBar, NULL, TRUE);
    if (searchBox) InvalidateRect(searchBox, NULL, TRUE);
    if (statusBar) InvalidateRect(statusBar, NULL, TRUE);
}

void ApplyTheme() {
    if (g_darkBrush) { DeleteObject(g_darkBrush); g_darkBrush = NULL; }
    if (g_darkEditBrush) { DeleteObject(g_darkEditBrush); g_darkEditBrush = NULL; }
    g_darkBrush = CreateSolidBrush(g_darkBg);
    g_darkEditBrush = CreateSolidBrush(g_darkBg2);

    ApplyDarkListColors();
    ApplyWindowDarkTitleBar(g_darkMode);
    ApplyControlThemes();
    if (darkBtn) SetWindowTextW(darkBtn, g_darkMode ? L"Light" : L"Dark");
    if (mainWnd) {
        RedrawWindow(mainWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
    }
}

// ComCtl32 v6 activation context (required for ListView group view / visual styles)
static HANDLE g_actCtxHandle = INVALID_HANDLE_VALUE;
static ULONG_PTR g_actCtxCookie = 0;
static std::wstring g_actCtxManifestPath;

bool ActivateComCtlV6() {
    wchar_t tempPath[MAX_PATH] = {};
    wchar_t tempFile[MAX_PATH] = {};
    if (!GetTempPathW(MAX_PATH, tempPath)) return false;
    if (!GetTempFileNameW(tempPath, L"mfm", 0, tempFile)) return false;

    const char manifest[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"
        "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\r\n"
        "  <dependency>\r\n"
        "    <dependentAssembly>\r\n"
        "      <assemblyIdentity type=\"win32\" name=\"Microsoft.Windows.Common-Controls\" version=\"6.0.0.0\" processorArchitecture=\"*\" publicKeyToken=\"6595b64144ccf1df\" language=\"*\"/>\r\n"
        "    </dependentAssembly>\r\n"
        "  </dependency>\r\n"
        "</assembly>\r\n";

    HANDLE hf = CreateFileW(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    BOOL ok = WriteFile(hf, manifest, (DWORD)(sizeof(manifest) - 1), &written, NULL);
    CloseHandle(hf);
    if (!ok) {
        DeleteFileW(tempFile);
        return false;
    }

    ACTCTXW actctx = {};
    actctx.cbSize = sizeof(actctx);
    actctx.lpSource = tempFile;
    g_actCtxHandle = CreateActCtxW(&actctx);
    if (g_actCtxHandle == INVALID_HANDLE_VALUE) {
        DeleteFileW(tempFile);
        return false;
    }
    if (!ActivateActCtx(g_actCtxHandle, &g_actCtxCookie)) {
        ReleaseActCtx(g_actCtxHandle);
        g_actCtxHandle = INVALID_HANDLE_VALUE;
        DeleteFileW(tempFile);
        return false;
    }
    g_actCtxManifestPath = tempFile;
    return true;
}

void DeactivateComCtlV6() {
    if (g_actCtxCookie) {
        DeactivateActCtx(0, g_actCtxCookie);
        g_actCtxCookie = 0;
    }
    if (g_actCtxHandle != INVALID_HANDLE_VALUE) {
        ReleaseActCtx(g_actCtxHandle);
        g_actCtxHandle = INVALID_HANDLE_VALUE;
    }
    if (!g_actCtxManifestPath.empty()) {
        DeleteFileW(g_actCtxManifestPath.c_str());
        g_actCtxManifestPath.clear();
    }
}

void PopulateListView() {
    if (!fileList) return;
    SendMessageW(fileList, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(fileList);
    SendMessageW(fileList, LVM_ENABLEGROUPVIEW, FALSE, 0);
    SendMessageW(fileList, LVM_REMOVEALLGROUPS, 0, 0);

    std::sort(g_items.begin(), g_items.end(), ItemLess);

    if (g_groupByDate) {
        SendMessageW(fileList, LVM_ENABLEGROUPVIEW, TRUE, 0);
        // insert only groups that have items, in chronological order
        bool present[7] = {false, false, false, false, false, false, false};
        for (size_t i = 0; i < g_items.size(); ++i) {
            int gid = DateGroupId(ItemGroupSt(g_items[i]));
            if (gid < 0) gid = 6;
            if (gid > 6) gid = 6;
            present[gid] = true;
        }
        for (int gi = 0; gi < 7; ++gi) {
            if (!present[gi]) continue;
            LVGROUP_COMPAT grp = {};
            grp.cbSize = sizeof(LVGROUP_COMPAT);
            grp.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN | LVGF_STATE;
            grp.pszHeader = (LPWSTR)DateGroupName(gi);
            grp.iGroupId = gi;
            grp.uAlign = LVGA_HEADER_LEFT;
            grp.state = LVGS_COLLAPSIBLE;
            grp.stateMask = LVGS_COLLAPSIBLE;
            SendMessageW(fileList, LVM_INSERTGROUP, (WPARAM)-1, (LPARAM)&grp);
        }
    }

    for (size_t i = 0; i < g_items.size(); ++i) {
        const FileItem& it = g_items[i];
        LVITEMW lvi = {};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = (int)i;
        if (g_sysSmall) {
            lvi.mask |= LVIF_IMAGE;
            lvi.iImage = it.icon;
        }
        lvi.pszText = (LPWSTR)it.name.c_str();
        lvi.lParam = (LPARAM)i;
        if (g_groupByDate) {
            lvi.mask |= LVIF_GROUPID;
            int gid = DateGroupId(ItemGroupSt(it));
            if (gid < 0) gid = 6;
            if (gid > 6) gid = 6;
            lvi.iGroupId = gid;
        }
        int row = ListView_InsertItem(fileList, &lvi);
        if (row < 0) continue;

        const SYSTEMTIME& shownSt = g_groupByDate ? ItemGroupSt(it) : it.stModifiedLocal;
        wchar_t date[64] = {};
        if (shownSt.wYear != 0) {
            _snwprintf(date, 64, L"%02u/%02u/%04u %02u:%02u",
                (unsigned)shownSt.wMonth, (unsigned)shownSt.wDay, (unsigned)shownSt.wYear,
                (unsigned)shownSt.wHour, (unsigned)shownSt.wMinute);
        }
        date[63] = 0;

        wchar_t sizeText[48] = {};
        if (!it.isDir) {
            ULONGLONG bytes = it.sizeBytes;
            if (bytes >= (1024ULL * 1024ULL * 1024ULL))
                _snwprintf(sizeText, 48, L"%I64u GB", (bytes + 1024ULL*1024ULL*1024ULL - 1) / (1024ULL*1024ULL*1024ULL));
            else if (bytes >= (1024ULL * 1024ULL))
                _snwprintf(sizeText, 48, L"%I64u MB", (bytes + 1024ULL*1024ULL - 1) / (1024ULL*1024ULL));
            else if (bytes >= 1024ULL)
                _snwprintf(sizeText, 48, L"%I64u KB", (bytes + 1023ULL) / 1024ULL);
            else if (bytes > 0)
                _snwprintf(sizeText, 48, L"1 KB");
            else
                lstrcpynW(sizeText, L"0 KB", 48);
            sizeText[47] = 0;
        }

        ListView_SetItemText(fileList, row, 1, date);
        ListView_SetItemText(fileList, row, 2, (LPWSTR)it.typeText.c_str());
        ListView_SetItemText(fileList, row, 3, sizeText);
    }

    SendMessageW(fileList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(fileList, NULL, TRUE);
    UpdateSortHeader();
    UpdateStatusBar();
}

std::wstring ToLowerStr(const std::wstring& s) {
    std::wstring r = s;
    for (size_t i = 0; i < r.size(); ++i) r[i] = (wchar_t)towlower((wint_t)r[i]);
    return r;
}

FileItem MakeFileItem(const WIN32_FIND_DATAW& d, const std::wstring& parentDir, bool searchResult) {
    FileItem it = {};
    it.name = d.cFileName;
    it.fullPath = JoinPath(parentDir, it.name);
    it.isDir = (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    it.isZip = !it.isDir && IsZipName(it.name);
    it.isHidden = (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;

    it.ftModified = d.ftLastWriteTime;
    it.ftCreated = d.ftCreationTime;

    ULARGE_INTEGER uli;
    uli.HighPart = d.nFileSizeHigh;
    uli.LowPart = d.nFileSizeLow;
    it.sizeBytes = uli.QuadPart;

    FILETIME local = {};
    if (FileTimeToLocalFileTime(&it.ftModified, &local)) FileTimeToSystemTime(&local, &it.stModifiedLocal);
    else ZeroMemory(&it.stModifiedLocal, sizeof(it.stModifiedLocal));
    local = {};
    if (FileTimeToLocalFileTime(&it.ftCreated, &local)) FileTimeToSystemTime(&local, &it.stCreatedLocal);
    else ZeroMemory(&it.stCreatedLocal, sizeof(it.stCreatedLocal));

    // Fast icon lookup by file attributes/name (avoids expensive per-file shell access on large folders).
    SHFILEINFOW sfi = {};
    DWORD fileAttrs = d.dwFileAttributes;
    if (!fileAttrs) fileAttrs = it.isDir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    DWORD_PTR hres = SHGetFileInfoW(it.name.c_str(), fileAttrs, &sfi, sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
    if (!hres && it.isDir) {
        ZeroMemory(&sfi, sizeof(sfi));
        hres = SHGetFileInfoW(L"folder", FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
    }
    it.icon = hres ? sfi.iIcon : 0;

    if (searchResult) {
        it.typeText = parentDir;
    } else if (it.isDir) {
        it.typeText = L"File folder";
    } else {
        const wchar_t* ext = wcsrchr(it.name.c_str(), L'.');
        if (ext && ext != it.name.c_str() && ext[1]) {
            std::wstring e = ext + 1;
            for (size_t k = 0; k < e.size(); ++k) e[k] = (wchar_t)towupper((wint_t)e[k]);
            it.typeText = e + L" File";
        } else {
            it.typeText = L"File";
        }
    }
    return it;
}

void SetListColumnsForMode() {
    if (!fileList) return;
    // update column 2 header/text meaning
    HWND hdr = ListView_GetHeader(fileList);
    if (!hdr) return;
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT;
    if (g_searchMode) {
        col.pszText = (LPWSTR)L"Folder";
    } else {
        col.pszText = (LPWSTR)L"Type";
    }
    ListView_SetColumn(fileList, 2, &col);
}

void ApplyCurrentFilter() {
    if (g_searchMode) return;
    g_items.clear();
    g_items.reserve(g_allItems.size());

    std::wstring qLower;
    if (!g_currentFilter.empty()) qLower = ToLowerStr(g_currentFilter);

    for (size_t i = 0; i < g_allItems.size(); ++i) {
        if (!g_showHidden && g_allItems[i].isHidden) continue;
        if (!qLower.empty()) {
            if (ToLowerStr(g_allItems[i].name).find(qLower) == std::wstring::npos) continue;
        }
        g_items.push_back(g_allItems[i]);
    }
    PopulateListView();
    UpdateStatusBar();
}

void SearchRecursiveCollect(const std::wstring& root, const std::wstring& qLower, std::vector<FileItem>& out, int depth) {
    if (depth > 32) return; // safety
    WIN32_FIND_DATAW d;
    std::wstring pattern = JoinPath(root, L"*");
    HANDLE h = FindFirstFileW(pattern.c_str(), &d);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (wcscmp(d.cFileName, L".") == 0 || wcscmp(d.cFileName, L"..") == 0) continue;
        bool isDir = (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool isHidden = (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
        std::wstring name = d.cFileName;
        std::wstring lowerName = ToLowerStr(name);
        if ((g_showHidden || !isHidden) && lowerName.find(qLower) != std::wstring::npos) {
            out.push_back(MakeFileItem(d, root, true));
            if (out.size() >= 2000) { FindClose(h); return; } // limit results
        }
        if (isDir && (g_showHidden || !isHidden)) {
            SearchRecursiveCollect(JoinPath(root, name), qLower, out, depth + 1);
            if (out.size() >= 2000) { FindClose(h); return; }
            // keep UI responsive
            MSG msg;
            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) { PostQuitMessage((int)msg.wParam); FindClose(h); return; }
                if (!IsDialogMessageW(mainWnd, &msg)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
            }
        }
    } while (FindNextFileW(h, &d));
    FindClose(h);
}

void PerformSearch() {
    if (g_searching || !searchBox) return;
    int len = GetWindowTextLengthW(searchBox);
    std::wstring query;
    if (len > 0) {
        std::vector<wchar_t> buf((size_t)len + 1, 0);
        GetWindowTextW(searchBox, &buf[0], len + 1);
        query.assign(&buf[0]);
    }
    size_t s = query.find_first_not_of(L" \t");
    size_t e = query.find_last_not_of(L" \t");
    if (s == std::wstring::npos) query.clear();
    else query = query.substr(s, e - s + 1);

    if (query.empty()) {
        if (g_searchMode) {
            g_searchMode = false;
            g_searchRoot.clear();
            SetListColumnsForMode();
            LoadDirectory(currentPath);
        } else {
            g_currentFilter.clear();
            ApplyCurrentFilter();
        }
        return;
    }

    g_searching = true;
    g_searchMode = true;
    g_searchRoot = currentPath;
    g_currentFilter.clear();
    g_groupByDate = false;
    g_sortMode = SORT_NAME_ASC;
    SetListColumnsForMode();

    g_items.clear();
    Status(L"Searching...");
    SearchRecursiveCollect(currentPath, ToLowerStr(query), g_items, 0);

    PopulateListView();
    UpdateStatusBar();
    g_searching = false;
}

LRESULT CALLBACK SearchBoxProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_RETURN) { PerformSearch(); return 0; }
        if (wp == VK_ESCAPE) {
            SetWindowTextW(hwnd, L"");
            if (g_searchMode) {
                g_searchMode = false;
                g_searchRoot.clear();
                SetListColumnsForMode();
                LoadDirectory(currentPath);
            } else {
                g_currentFilter.clear();
                ApplyCurrentFilter();
            }
            return 0;
        }
    }
    if (msg == WM_CHAR && (wp == '\r' || wp == '\n')) return 0;
    return CallWindowProcW(g_oldSearchProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK HeaderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_ERASEBKGND || msg == WM_PAINT) {
        // Let default paint first
        LRESULT res = CallWindowProcW(g_oldHeaderProc, hwnd, msg, wp, lp);
        HDC hdc = (msg == WM_ERASEBKGND) ? (HDC)wp : GetDC(hwnd);
        if (hdc) {
            RECT rc; GetClientRect(hwnd, &rc);
            int cols = Header_GetItemCount(hwnd);
            int totalW = 0;
            for (int i = 0; i < cols; ++i) {
                RECT ir; if (Header_GetItemRect(hwnd, i, &ir)) { if (ir.right > totalW) totalW = ir.right; }
            }
            if (totalW < rc.right) {
                RECT fill = rc;
                fill.left = totalW;
                COLORREF bg = g_darkMode ? g_darkBg2 : GetSysColor(COLOR_WINDOW);
                HBRUSH b = CreateSolidBrush(bg);
                FillRect(hdc, &fill, b);
                DeleteObject(b);
            }
            if (msg == WM_PAINT) ReleaseDC(hwnd, hdc);
        }
        return res;
    }
    return CallWindowProcW(g_oldHeaderProc, hwnd, msg, wp, lp);
}

void LoadThisPCItems(std::vector<FileItem>& out) {
    wchar_t drives[512] = {};
    if (GetLogicalDriveStringsW(511, drives) == 0) return;
    wchar_t* p = drives;
    while (*p) {
        std::wstring d = p;
        std::wstring root = NormalizeDirPath(d);
        UINT type = GetDriveTypeW(root.c_str());

        FileItem it = {};
        wchar_t volName[MAX_PATH] = {};
        BOOL hasVol = GetVolumeInformationW(root.c_str(), volName, MAX_PATH, NULL, NULL, NULL, NULL, 0);
        if (hasVol && volName[0]) it.name = std::wstring(volName) + L" (" + root.substr(0, 2) + L")";
        else if (type == DRIVE_FIXED) it.name = L"Local Disk (" + root.substr(0, 2) + L")";
        else if (type == DRIVE_REMOVABLE) it.name = L"Removable Disk (" + root.substr(0, 2) + L")";
        else if (type == DRIVE_CDROM) it.name = L"CD Drive (" + root.substr(0, 2) + L")";
        else if (type == DRIVE_REMOTE) it.name = L"Network Drive (" + root.substr(0, 2) + L")";
        else it.name = L"Drive (" + root.substr(0, 2) + L")";

        it.fullPath = root;
        it.isDir = true;
        it.isZip = false;
        it.isHidden = false;
        it.sizeBytes = 0;
        memset(&it.ftModified, 0, sizeof(it.ftModified));
        memset(&it.ftCreated, 0, sizeof(it.ftCreated));
        memset(&it.stModifiedLocal, 0, sizeof(it.stModifiedLocal));
        memset(&it.stCreatedLocal, 0, sizeof(it.stCreatedLocal));

        ULARGE_INTEGER totalBytes = {}, freeBytes = {};
        if (GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, NULL)) {
            it.sizeBytes = totalBytes.QuadPart;
        }

        if (type == DRIVE_FIXED) it.typeText = L"Local Disk";
        else if (type == DRIVE_REMOVABLE) it.typeText = L"Removable Disk";
        else if (type == DRIVE_CDROM) it.typeText = L"CD Drive";
        else if (type == DRIVE_REMOTE) it.typeText = L"Network Drive";
        else it.typeText = L"Drive";

        SHFILEINFOW sfi = {};
        DWORD_PTR hres = SHGetFileInfoW(root.c_str(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
        if (!hres) hres = SHGetFileInfoW(root.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
        it.icon = hres ? sfi.iIcon : 0;

        out.push_back(it);
        p += lstrlenW(p) + 1;
    }
}

void LoadDirectory(const std::wstring& path) {
    g_allItems.clear();
    g_dirTruncated = false;
    g_dirTotalCount = 0;
    if (IsThisPCPath(path)) {
        LoadThisPCItems(g_allItems);
        g_dirTotalCount = g_allItems.size();
        g_groupByDate = false;
        g_sortMode = SORT_NAME_ASC;
        SetListColumnsForMode();
        ApplyCurrentFilter();
        return;
    }

    WIN32_FIND_DATAW d;
    std::wstring search = JoinPath(path, L"*");
    HANDLE h = FindFirstFileW(search.c_str(), &d);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(d.cFileName, L".") == 0 || wcscmp(d.cFileName, L"..") == 0) continue;
            ++g_dirTotalCount;
            if (g_allItems.size() < MAX_DIR_ITEMS) {
                g_allItems.push_back(MakeFileItem(d, path, false));
            } else {
                g_dirTruncated = true;
            }
        } while (FindNextFileW(h, &d));
        FindClose(h);
    }

    // Downloads: grouped by date with newest files first and folders at bottom of each day group
    if (IsDownloadsPath(path) && !g_dirTruncated) {
        g_groupByDate = true;
        g_sortMode = SORT_DATE_DESC;
    } else {
        g_groupByDate = false;
        g_sortMode = SORT_NAME_ASC;
    }

    SetListColumnsForMode();
    ApplyCurrentFilter();
}

bool Navigate(const std::wstring& path, bool record) {
    std::wstring target = NormalizeDirPath(path);
    if (target.empty()) return false;

    if (IsThisPCPath(target)) {
        g_searchMode = false;
        g_searchRoot.clear();
        g_currentFilter.clear();
        currentPath = THIS_PC_PATH;
        if (addressBar) SetWindowTextW(addressBar, L"This PC");
        if (searchBox) SetWindowTextW(searchBox, L"");
        LoadDirectory(currentPath);
        if (record) {
            history.resize(historyPos + 1);
            history.push_back(currentPath);
            historyPos = (int)history.size() - 1;
        }
        UpdateButtons();
        SelectTreePath(currentPath);
        return true;
    }

    DWORD attr = GetFileAttributesW(target.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY) && IsZipName(target)) {
            ShellExecuteW(mainWnd, L"open", target.c_str(), NULL, NULL, SW_SHOWNORMAL);
            return true;
        }
        Status(L"Folder not found.");
        MessageBeep(MB_ICONWARNING);
        return false;
    }
    g_searchMode = false;
    g_searchRoot.clear();
    g_currentFilter.clear();
    currentPath = target;
    if (addressBar) SetWindowTextW(addressBar, target.c_str());
    if (searchBox) SetWindowTextW(searchBox, L"");

    LoadDirectory(target);

    if (record) {
        history.resize(historyPos + 1);
        history.push_back(target);
        historyPos = (int)history.size() - 1;
    }
    UpdateButtons();
    SelectTreePath(target);
    return true;
}

FileItem* GetItemByListIndex(int listIndex) {
    if (listIndex < 0 || listIndex >= ListView_GetItemCount(fileList)) return NULL;
    LVITEMW lvi = {}; lvi.mask = LVIF_PARAM; lvi.iItem = listIndex;
    if (!ListView_GetItem(fileList, &lvi)) return NULL;
    size_t idx = (size_t)lvi.lParam;
    if (idx >= g_items.size()) return NULL;
    return &g_items[idx];
}

std::vector<std::wstring> SelectedFiles() {
    std::vector<std::wstring> files;
    if (!fileList) return files;
    int i = -1;
    while ((i = ListView_GetNextItem(fileList, i, LVNI_SELECTED)) != -1) {
        FileItem* it = GetItemByListIndex(i);
        if (it) files.push_back(it->fullPath);
    }
    return files;
}

void OpenFiles(const std::vector<std::wstring>& files) {
    if (files.empty()) return;
    for (size_t i = 0; i < files.size(); ++i) {
        DWORD a = GetFileAttributesW(files[i].c_str());
        if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) { Navigate(files[i]); return; }
    }
    if (files.size() == 1 && IsArchivePath(files[0])) {
        Detect7Zip();
        if (!g_7zFMPath.empty()) {
            RunProcessAsync(g_7zFMPath, QuoteArg(files[0]), currentPath);
            return;
        }
        if (Has7ZipGUI() && !IsZipName(files[0])) {
            RunProcessAsync(g_7zGPath, L"x " + QuoteArg(files[0]) + L" -o" + QuoteArg(currentPath) + L" -aos", currentPath);
            Status(L"Extraction started (7-Zip).");
            return;
        }
    }
    for (size_t i = 0; i < files.size(); ++i) {
        ShellExecuteW(mainWnd, L"open", files[i].c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void OpenSelected() {
    OpenFiles(SelectedFiles());
}

void ClearSearchMode() {
    if (!g_searchMode) return;
    g_searchMode = false;
    g_searchRoot.clear();
    SetListColumnsForMode();
    LoadDirectory(currentPath);
}

void RefreshView() {
    if (g_searchMode) {
        PerformSearch();
    } else if (!currentPath.empty()) {
        Navigate(currentPath, false);
    }
}

// ---- Context menus ----
void ShowSortMenu(HWND anchor) {
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_NAME_ASC?MF_CHECKED:0), ID_SORT_NAME_ASC, L"Name (A-Z)");
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_NAME_DESC?MF_CHECKED:0), ID_SORT_NAME_DESC, L"Name (Z-A)");
    AppendMenuW(m, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_DATE_DESC && !g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_DESC, L"Date modified (newest first)");
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_DATE_ASC && !g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_ASC, L"Date modified (oldest first)");
    AppendMenuW(m, MF_STRING | (g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_GROUPED, L"Group by date (Downloads style)");
    AppendMenuW(m, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_TYPE_ASC?MF_CHECKED:0), ID_SORT_TYPE_ASC, L"Type (A-Z)");
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_TYPE_DESC?MF_CHECKED:0), ID_SORT_TYPE_DESC, L"Type (Z-A)");
    AppendMenuW(m, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_SIZE_ASC?MF_CHECKED:0), ID_SORT_SIZE_ASC, L"Size (smallest first)");
    AppendMenuW(m, MF_STRING | (g_sortMode==SORT_SIZE_DESC?MF_CHECKED:0), ID_SORT_SIZE_DESC, L"Size (largest first)");
    AppendMenuW(m, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m, MF_STRING | (g_showHidden?MF_CHECKED:0), ID_SORT_SHOW_HIDDEN, L"Show hidden items");
    RECT rc; GetWindowRect(anchor, &rc);
    int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom, 0, mainWnd, NULL);
    DestroyMenu(m);
    switch (cmd) {
        case ID_SORT_NAME_ASC: g_sortMode = SORT_NAME_ASC; g_groupByDate = false; break;
        case ID_SORT_NAME_DESC: g_sortMode = SORT_NAME_DESC; g_groupByDate = false; break;
        case ID_SORT_DATE_ASC: g_sortMode = SORT_DATE_ASC; g_groupByDate = false; break;
        case ID_SORT_DATE_DESC: g_sortMode = SORT_DATE_DESC; g_groupByDate = false; break;
        case ID_SORT_TYPE_ASC: g_sortMode = SORT_TYPE_ASC; g_groupByDate = false; break;
        case ID_SORT_TYPE_DESC: g_sortMode = SORT_TYPE_DESC; g_groupByDate = false; break;
        case ID_SORT_SIZE_ASC: g_sortMode = SORT_SIZE_ASC; g_groupByDate = false; break;
        case ID_SORT_SIZE_DESC: g_sortMode = SORT_SIZE_DESC; g_groupByDate = false; break;
        case ID_SORT_DATE_GROUPED: g_groupByDate = true; g_sortMode = SORT_DATE_DESC; break;
        case ID_SORT_SHOW_HIDDEN: g_showHidden = !g_showHidden; ApplyCurrentFilter(); return;
        default: return;
    }
    PopulateListView();
}

std::vector<std::wstring> ActiveContextFiles() {
    return g_useContextFiles ? g_contextFiles : SelectedFiles();
}

void ShowItemContextMenu(int x, int y, bool onItem) {
    std::vector<std::wstring> sel = ActiveContextFiles();
    HMENU m = CreatePopupMenu();
    if (onItem && !sel.empty()) {
        AppendMenuW(m, MF_STRING | MF_DEFAULT, ID_CTX_OPEN, L"Open");
        if (sel.size() == 1 && IsArchivePath(sel[0])) {
            AppendMenuW(m, MF_STRING, ID_CTX_EXTRACT, L"Extract Here");
            AppendMenuW(m, MF_STRING, ID_CTX_EXTRACT_FOLDER, L"Extract to Folder...");
            if (Has7ZipGUI() || !g_7zFMPath.empty()) {
                // keep simple; 7zG/7zFM used when available
            }
        }
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_CUT, L"Cut\tCtrl+X");
        AppendMenuW(m, MF_STRING, ID_CTX_COPY, L"Copy\tCtrl+C");
        AppendMenuW(m, MF_STRING, ID_CTX_COPY_NAME, L"Copy name");
        AppendMenuW(m, MF_STRING, ID_CTX_COPY_PATH, L"Copy as path");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_DELETE, L"Delete\tDel");
        if (sel.size() == 1) AppendMenuW(m, MF_STRING, ID_CTX_RENAME, L"Rename\tF2");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        if (Has7Zip()) {
            AppendMenuW(m, MF_STRING, ID_CTX_COMPRESS, L"Add to ZIP archive...");
            AppendMenuW(m, MF_STRING, ID_CTX_OPENZIP, L"Add to 7z archive...");
        } else {
            AppendMenuW(m, MF_STRING, ID_CTX_COMPRESS, L"Compress to ZIP...");
        }
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_OPEN_EXPLORER, sel.size() == 1 ? L"Show in Explorer" : L"Open in Explorer");
        AppendMenuW(m, MF_STRING, ID_CTX_OPEN_CMD, L"Open Command Prompt here");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_PROPERTIES, L"Properties");
    } else {
        AppendMenuW(m, MF_STRING, ID_CTX_REFRESH, L"Refresh\tF5");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        bool canPaste = IsClipboardFormatAvailable(CF_HDROP) != 0;
        AppendMenuW(m, MF_STRING | (canPaste ? 0 : MF_GRAYED), ID_CTX_PASTE, L"Paste\tCtrl+V");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_NEWFOLDER, L"New Folder\tCtrl+N");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_SELECTALL, L"Select All\tCtrl+A");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_COPY_NAME, L"Copy folder name");
        AppendMenuW(m, MF_STRING, ID_CTX_COPY_PATH, L"Copy folder path");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        HMENU sortSub = CreatePopupMenu();
        AppendMenuW(sortSub, MF_STRING | (g_sortMode==SORT_NAME_ASC?MF_CHECKED:0), ID_SORT_NAME_ASC, L"Name (A-Z)");
        AppendMenuW(sortSub, MF_STRING | (g_sortMode==SORT_NAME_DESC?MF_CHECKED:0), ID_SORT_NAME_DESC, L"Name (Z-A)");
        AppendMenuW(sortSub, MF_STRING | (g_sortMode==SORT_DATE_DESC && !g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_DESC, L"Date modified (newest)");
        AppendMenuW(sortSub, MF_STRING | (g_sortMode==SORT_DATE_ASC && !g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_ASC, L"Date modified (oldest)");
        AppendMenuW(sortSub, MF_STRING | (g_groupByDate?MF_CHECKED:0), ID_SORT_DATE_GROUPED, L"Group by date (Downloads style)");
        AppendMenuW(sortSub, MF_SEPARATOR, 0, NULL);
        AppendMenuW(sortSub, MF_STRING | (g_showHidden?MF_CHECKED:0), ID_SORT_SHOW_HIDDEN, L"Show hidden items");
        AppendMenuW(m, MF_POPUP, (UINT_PTR)sortSub, L"Sort by");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_OPEN_CMD, L"Open Command Prompt here");
        AppendMenuW(m, MF_STRING, ID_CTX_OPEN_EXPLORER, L"Open folder in Explorer");
        AppendMenuW(m, MF_SEPARATOR, 0, NULL);
        AppendMenuW(m, MF_STRING, ID_CTX_PROPERTIES, L"Properties");
    }

    int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, mainWnd, NULL);
    DestroyMenu(m);

    // context files remain valid for command handling below
    {
        std::vector<std::wstring> s = ActiveContextFiles();
        auto namesFromPaths = [&](const std::vector<std::wstring>& paths) {
            std::vector<std::wstring> names;
            for (size_t i = 0; i < paths.size(); ++i) {
                std::wstring p = paths[i];
                while (p.size() > 3 && (p.back() == L'\\' || p.back() == L'/')) p.pop_back();
                size_t pos = p.find_last_of(L"\\/");
                names.push_back((pos == std::wstring::npos) ? p : p.substr(pos + 1));
            }
            return names;
        };
        switch (cmd) {
            case ID_CTX_OPEN: OpenFiles(s); break;
            case ID_CTX_COPY: { if (CopyFilesToClipboard(s, false)) Status(L"Copied."); } break;
            case ID_CTX_CUT: { if (CopyFilesToClipboard(s, true)) Status(L"Cut."); } break;
            case ID_CTX_COPY_PATH: {
                std::vector<std::wstring> paths = s;
                if (!onItem || paths.empty()) {
                    // background: copy current folder path
                    paths.clear();
                    if (!currentPath.empty()) paths.push_back(currentPath);
                }
                CopyPathsToClipboard(paths);
                break;
            }
            case ID_CTX_COPY_NAME: {
                std::vector<std::wstring> paths = s;
                if (!onItem || paths.empty()) {
                    paths.clear();
                    if (!currentPath.empty()) paths.push_back(currentPath);
                }
                CopyNamesToClipboard(namesFromPaths(paths));
                break;
            }
            case ID_CTX_PASTE: PasteFromClipboard(); break;
            case ID_CTX_DELETE: DeleteFiles(s, true); break;
            case ID_CTX_RENAME: {
                if (s.size() == 1) {
                    int idx = -1;
                    for (int i = 0; i < ListView_GetItemCount(fileList); ++i) {
                        FileItem* it = GetItemByListIndex(i);
                        if (it && lstrcmpiW(it->fullPath.c_str(), s[0].c_str()) == 0) { idx = i; break; }
                    }
                    if (idx >= 0) {
                        ListView_SetItemState(fileList, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                        ListView_SetItemState(fileList, idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        SetFocus(fileList);
                        ListView_EditLabel(fileList, idx);
                    }
                }
                break;
            }
            case ID_CTX_NEWFOLDER: CreateNewFolder(); break;
            case ID_CTX_PROPERTIES: {
                if (s.size() == 1) ShowProperties(s[0]);
                else ShowProperties(currentPath);
                break;
            }
            case ID_CTX_REFRESH: RefreshView(); break;
            case ID_CTX_SELECTALL: ListView_SetItemState(fileList, -1, LVIS_SELECTED, LVIS_SELECTED); SetFocus(fileList); break;
            case ID_CTX_OPEN_CMD: {
                std::wstring dir = currentPath;
                if (s.size() == 1) {
                    DWORD a = GetFileAttributesW(s[0].c_str());
                    if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) dir = s[0];
                    else {
                        size_t pos = s[0].find_last_of(L"\\/");
                        if (pos != std::wstring::npos) dir = s[0].substr(0, pos);
                    }
                }
                OpenInCommandPrompt(dir);
                break;
            }
            case ID_CTX_OPEN_EXPLORER: {
                if (s.size() == 1) OpenInExplorer(s[0]);
                else OpenInExplorer(currentPath);
                break;
            }
            case ID_CTX_COMPRESS: {
                if (s.empty()) { Status(L"Select files or folders to compress."); break; }
                wchar_t output[MAX_PATH] = L"Archive.zip";
                OPENFILENAMEW ofn = {}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = mainWnd;
                ofn.lpstrFilter = L"ZIP archive (*.zip)\0*.zip\0All files (*.*)\0*.*\0"; ofn.lpstrFile = output; ofn.nMaxFile = MAX_PATH;
                ofn.lpstrDefExt = L"zip"; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
                if (GetSaveFileNameW(&ofn)) {
                    std::wstring err; Status(L"Creating archive...");
                    bool ok = Has7Zip() ? CompressWith7Zip(s, output, L"zip", err) : CompressToZip(s, output, err);
                    if (ok) {
                        Status(Has7ZipGUI() ? L"Compression started (7-Zip)." : L"ZIP archive created.");
                        if (!Has7ZipGUI()) RefreshView();
                    } else { Status(err); MessageBeep(MB_ICONERROR); }
                }
                break;
            }
            case ID_CTX_OPENZIP: { // Add to 7z archive...
                if (s.empty()) { Status(L"Select files or folders to compress."); break; }
                wchar_t output[MAX_PATH] = L"Archive.7z";
                OPENFILENAMEW ofn = {}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = mainWnd;
                ofn.lpstrFilter = L"7z archive (*.7z)\0*.7z\0ZIP archive (*.zip)\0*.zip\0TAR archive (*.tar)\0*.tar\0All files (*.*)\0*.*\0";
                ofn.lpstrFile = output; ofn.nMaxFile = MAX_PATH;
                ofn.lpstrDefExt = L"7z"; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
                if (GetSaveFileNameW(&ofn)) {
                    std::wstring out = output;
                    std::wstring type = L"7z";
                    size_t dot = out.find_last_of(L'.');
                    if (dot != std::wstring::npos) {
                        std::wstring ext = out.substr(dot + 1);
                        for (size_t i = 0; i < ext.size(); ++i) ext[i] = (wchar_t)towlower((wint_t)ext[i]);
                        if (ext == L"zip") type = L"zip";
                        else if (ext == L"tar") type = L"tar";
                        else if (ext == L"7z") type = L"7z";
                    }
                    std::wstring err; Status(L"Creating archive...");
                    if (CompressWith7Zip(s, out, type, err)) Status(L"Compression started (7-Zip).");
                    else { Status(err); MessageBeep(MB_ICONERROR); }
                }
                break;
            }
            case ID_CTX_EXTRACT: {
                if (s.size() == 1) {
                    std::wstring err; Status(L"Extracting...");
                    if (ExtractArchive(s[0], currentPath, err)) {
                        Status(Has7ZipGUI() ? L"Extraction started (7-Zip)." : L"Extraction completed.");
                        if (!Has7ZipGUI()) RefreshView();
                    } else { Status(err); MessageBeep(MB_ICONERROR); }
                }
                break;
            }
            case ID_CTX_EXTRACT_FOLDER: {
                if (s.size() == 1) {
                    std::wstring zipPath = s[0];
                    size_t slash = zipPath.find_last_of(L'\\'); size_t start = (slash==std::wstring::npos)?0:slash+1;
                    std::wstring fileName = zipPath.substr(start);
                    std::wstring base = fileName;
                    for (int pass = 0; pass < 2; ++pass) {
                        size_t dot = base.find_last_of(L'.');
                        if (dot == std::wstring::npos || dot == 0) break;
                        std::wstring e = base.substr(dot + 1);
                        for (size_t i = 0; i < e.size(); ++i) e[i] = (wchar_t)towlower((wint_t)e[i]);
                        if (IsArchiveExt(e) || e == L"tar") base = base.substr(0, dot);
                        else break;
                    }
                    if (base.empty()) base = L"extracted";
                    std::wstring dest = JoinPath(currentPath, base);
                    CreateDirectoryW(dest.c_str(), NULL);
                    std::wstring err; Status(L"Extracting...");
                    if (ExtractArchive(zipPath, dest, err)) {
                        Status(Has7ZipGUI() ? L"Extraction started (7-Zip)." : L"Extraction completed.");
                        if (!Has7ZipGUI()) RefreshView();
                    } else { Status(err); MessageBeep(MB_ICONERROR); }
                }
                break;
            }
            case ID_SORT_NAME_ASC: g_sortMode = SORT_NAME_ASC; g_groupByDate = false; PopulateListView(); break;
            case ID_SORT_NAME_DESC: g_sortMode = SORT_NAME_DESC; g_groupByDate = false; PopulateListView(); break;
            case ID_SORT_DATE_ASC: g_sortMode = SORT_DATE_ASC; g_groupByDate = false; PopulateListView(); break;
            case ID_SORT_DATE_DESC: g_sortMode = SORT_DATE_DESC; g_groupByDate = false; PopulateListView(); break;
            case ID_SORT_DATE_GROUPED: g_groupByDate = true; g_sortMode = SORT_DATE_DESC; PopulateListView(); break;
            case ID_SORT_SHOW_HIDDEN: g_showHidden = !g_showHidden; ApplyCurrentFilter(); break;
        }
    }
}

// ---- Tree view ----
struct TreePathItem { std::wstring path; UiIconKind iconKind; TreePathItem() : iconKind(UIICON_NONE) {} };
std::vector<TreePathItem*> g_treeItems;

HTREEITEM AddTreeItem(HTREEITEM parent, const std::wstring& text, const std::wstring& path, int icon, int iconSel, bool hasChildren, UiIconKind iconKind = UIICON_NONE) {
    TreePathItem* tpi = new TreePathItem(); tpi->path = path; tpi->iconKind = iconKind; g_treeItems.push_back(tpi);
    TVINSERTSTRUCTW tvis = {}; tvis.hParent = parent; tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    if (hasChildren) tvis.item.mask |= TVIF_CHILDREN;
    tvis.item.pszText = (LPWSTR)text.c_str(); tvis.item.iImage = icon; tvis.item.iSelectedImage = iconSel;
    tvis.item.cChildren = hasChildren ? 1 : 0; tvis.item.lParam = (LPARAM)tpi;
    return (HTREEITEM)SendMessageW(treeView, TVM_INSERTITEMW, 0, (LPARAM)&tvis);
}

void ClearTreeItems() { for (size_t i=0;i<g_treeItems.size();++i) delete g_treeItems[i]; g_treeItems.clear(); }

std::wstring GetTreeItemPath(HTREEITEM hItem) {
    if (!hItem) return L"";
    TVITEMW tvi = {}; tvi.mask = TVIF_PARAM; tvi.hItem = hItem;
    if (SendMessageW(treeView, TVM_GETITEMW, 0, (LPARAM)&tvi)) { TreePathItem* tpi = (TreePathItem*)tvi.lParam; if (tpi) return tpi->path; }
    return L"";
}

void PopulateComputerChildren(HTREEITEM parent) {
    wchar_t drives[512] = {};
    if (GetLogicalDriveStringsW(511, drives) == 0) return;
    wchar_t* p = drives;
    while (*p) {
        std::wstring d = p; UINT type = GetDriveTypeW(d.c_str());
        std::wstring label;
        wchar_t volName[MAX_PATH] = {};
        if (GetVolumeInformationW(d.c_str(), volName, MAX_PATH, NULL, NULL, NULL, NULL, 0) && volName[0]) label = std::wstring(volName) + L" (" + d.substr(0,2) + L")";
        else if (type == DRIVE_FIXED) label = L"Local Disk (" + d.substr(0,2) + L")";
        else if (type == DRIVE_REMOVABLE) label = L"Removable Disk (" + d.substr(0,2) + L")";
        else if (type == DRIVE_CDROM) label = L"CD Drive (" + d.substr(0,2) + L")";
        else label = L"Drive (" + d.substr(0,2) + L")";
        SHFILEINFOW sfi = {}; SHGetFileInfoW(d.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES);
        int ic = sfi.iIcon; SHGetFileInfoW(d.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES|SHGFI_OPENICON);
        UiIconKind kind = (type == DRIVE_CDROM) ? UIICON_CD : UIICON_NONE;
        AddTreeItem(parent, label, NormalizeDirPath(d), ic, sfi.iIcon, false, kind);
        p += lstrlenW(p) + 1;
    }
}

void BuildTree() {
    if (!treeView) return;
    g_updatingTreeSelection = true;
    TreeView_DeleteAllItems(treeView);
    ClearTreeItems();

    auto addKnown = [&](int csidl, const wchar_t* label, bool useDownloads, UiIconKind kind) {
        std::wstring p = useDownloads ? GetDownloadsPath() : GetSpecialFolderPath(csidl);
        if (p.empty()) return;
        SHFILEINFOW sfi = {}; SHGetFileInfoW(p.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES);
        int ic = sfi.iIcon; SHGetFileInfoW(p.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES|SHGFI_OPENICON);
        AddTreeItem(TVI_ROOT, label, p, ic, sfi.iIcon, false, kind);
    };

    addKnown(CSIDL_DESKTOPDIRECTORY, L"Desktop", false, UIICON_DESKTOP);
    addKnown(0, L"Downloads", true, UIICON_DOWNLOADS);
    addKnown(CSIDL_PERSONAL, L"Documents", false, UIICON_DOCS);
    addKnown(CSIDL_MYPICTURES, L"Pictures", false, UIICON_PICS);
    addKnown(CSIDL_MYMUSIC, L"Music", false, UIICON_MUSIC);
    addKnown(CSIDL_MYVIDEO, L"Videos", false, UIICON_VIDEOS);

    SHFILEINFOW sfi = {}; memset(&sfi, 0, sizeof(sfi));
    if (!SHGetFileInfoW(THIS_PC_PATH, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON)) {
        memset(&sfi, 0, sizeof(sfi));
        SHGetFileInfoW(L"C:\\", FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
    }
    HTREEITEM hComputer = AddTreeItem(TVI_ROOT, L"This PC", THIS_PC_PATH, sfi.iIcon, sfi.iIcon, true);
    if (hComputer) { PopulateComputerChildren(hComputer); TreeView_Expand(treeView, hComputer, TVE_EXPAND); }
    g_updatingTreeSelection = false;
}

HTREEITEM FindTreeItemByPathRec(HTREEITEM parent, const std::wstring& path) {
    HTREEITEM item = (parent == (HTREEITEM)-1) ? TreeView_GetRoot(treeView) : TreeView_GetChild(treeView, parent);
    while (item) {
        std::wstring p = GetTreeItemPath(item);
        if (!p.empty() && PathEqualsInsensitive(p, path)) return item;
        HTREEITEM child = FindTreeItemByPathRec(item, path);
        if (child) return child;
        item = TreeView_GetNextSibling(treeView, item);
    }
    return NULL;
}

void SelectTreePath(const std::wstring& path) {
    if (!treeView || path.empty()) return;
    HTREEITEM item = FindTreeItemByPathRec((HTREEITEM)-1, path);
    if (!item && path.size() >= 2) item = FindTreeItemByPathRec((HTREEITEM)-1, path.substr(0,3));
    if (item) { g_updatingTreeSelection = true; TreeView_SelectItem(treeView, item); g_updatingTreeSelection = false; }
}

// ---- UI ----
LRESULT CALLBACK AddressBarProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        int len = GetWindowTextLengthW(hwnd);
        if (len > 0) { std::vector<wchar_t> buf((size_t)len + 1, 0); GetWindowTextW(hwnd, &buf[0], len + 1); Navigate(&buf[0]); }
        return 0;
    }
    if (msg == WM_CHAR && wp == VK_RETURN) return 0;
    return CallWindowProcW(g_oldEditProc, hwnd, msg, wp, lp);
}

void Layout(HWND hwnd) {
    RECT r; GetClientRect(hwnd, &r);
    int width = r.right - r.left;
    int height = r.bottom - r.top;
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    int margin = 6, top = 6, btnH = 24, btnW = 30, gap = 4;

    int x = margin;
    if (backBtn) MoveWindow(backBtn, x, top, btnW, btnH, TRUE);
    x += btnW + gap;
    if (forwardBtn) MoveWindow(forwardBtn, x, top, btnW, btnH, TRUE);
    x += btnW + gap;
    if (upBtn) MoveWindow(upBtn, x, top, btnW, btnH, TRUE);
    x += btnW + gap;
    if (refreshBtn) MoveWindow(refreshBtn, x, top, btnW, btnH, TRUE);
    x += btnW + gap + 4;

    int goW = 34, searchW = 140, findW = 40, darkW = 46, sortW = 46, newW = 40;
    int rightW = goW + gap + searchW + gap + findW + gap + darkW + gap + sortW + gap + newW;
    int addrLeft = x;
    int addrW = width - addrLeft - margin - rightW - gap;
    if (addrW < 60) addrW = 60;
    if (addressBar) MoveWindow(addressBar, addrLeft, top, addrW, btnH, TRUE);
    int ax = addrLeft + addrW + gap;
    if (goBtn) MoveWindow(goBtn, ax, top, goW, btnH, TRUE);
    ax += goW + gap;
    if (searchBox) MoveWindow(searchBox, ax, top, searchW, btnH, TRUE);
    ax += searchW + gap;
    if (searchBtn) MoveWindow(searchBtn, ax, top, findW, btnH, TRUE);
    ax += findW + gap;
    if (darkBtn) MoveWindow(darkBtn, ax, top, darkW, btnH, TRUE);
    ax += darkW + gap;
    if (sortBtn) MoveWindow(sortBtn, ax, top, sortW, btnH, TRUE);
    ax += sortW + gap;
    if (newFolderBtn) MoveWindow(newFolderBtn, ax, top, newW, btnH, TRUE);

    if (statusBar) SendMessageW(statusBar, WM_SIZE, 0, 0);
    RECT sbRect = {};
    int sbH = 0;
    if (statusBar && GetWindowRect(statusBar, &sbRect)) {
        sbH = sbRect.bottom - sbRect.top;
        if (sbH < 0) sbH = 0;
    }

    int contentTop = TOPBAR_HEIGHT;
    int contentH = height - contentTop - sbH - margin;
    if (contentH < 0) contentH = 0;

    int leftW = g_leftPaneWidth;
    if (leftW < 140) leftW = 140;
    if (leftW > width - 220) leftW = width - 220;
    g_leftPaneWidth = leftW;

    int treeX = margin;
    int listX = treeX + leftW + margin;
    int listW = width - listX - margin;
    if (listW < 120) listW = 120;

    if (treeView) MoveWindow(treeView, treeX, contentTop, leftW, contentH, TRUE);
    if (fileList) MoveWindow(fileList, listX, contentTop, listW, contentH, TRUE);
}

void CreateUI(HWND hwnd) {
    NONCLIENTMETRICSW ncm = {}; ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) appFont = CreateFontIndirectW(&ncm.lfMessageFont);
    if (!appFont) appFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    const DWORD btnStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
    backBtn = CreateWindowW(L"BUTTON", L"<", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_BACK, NULL, NULL);
    forwardBtn = CreateWindowW(L"BUTTON", L">", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_FORWARD, NULL, NULL);
    upBtn = CreateWindowW(L"BUTTON", L"^", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_UP, NULL, NULL);
    refreshBtn = CreateWindowW(L"BUTTON", L"R", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_REFRESH, NULL, NULL);

    addressBar = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_ADDRESS, NULL, NULL);
    if (addressBar) g_oldEditProc = (WNDPROC)SetWindowLongPtrW(addressBar, GWLP_WNDPROC, (LONG_PTR)AddressBarProc);

    goBtn = CreateWindowW(L"BUTTON", L"Go", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_GO, NULL, NULL);
    searchBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_SEARCH, NULL, NULL);
    if (searchBox) {
        g_oldSearchProc = (WNDPROC)SetWindowLongPtrW(searchBox, GWLP_WNDPROC, (LONG_PTR)SearchBoxProc);
        SendMessageW(searchBox, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search");
    }
    searchBtn = CreateWindowW(L"BUTTON", L"Find", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_SEARCHBTN, NULL, NULL);
    darkBtn = CreateWindowW(L"BUTTON", L"Dark", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_DARKBTN, NULL, NULL);
    sortBtn = CreateWindowW(L"BUTTON", L"Sort", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_SORTBTN, NULL, NULL);
    newFolderBtn = CreateWindowW(L"BUTTON", L"New", btnStyle, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_NEWFOLDERBTN, NULL, NULL);

    treeView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP|TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_TREE, NULL, NULL);
    fileList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_EDITLABELS, 0,0,0,0, hwnd, (HMENU)(INT_PTR)ID_LIST, NULL, NULL);

    ListView_SetExtendedListViewStyle(fileList, LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP|LVS_EX_HEADERDRAGDROP);
    LVCOLUMNW c = {}; c.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
    c.pszText = (LPWSTR)L"Name"; c.cx = 280; c.iSubItem = 0; ListView_InsertColumn(fileList, 0, &c);
    c.pszText = (LPWSTR)L"Date modified"; c.cx = 140; c.iSubItem = 1; ListView_InsertColumn(fileList, 1, &c);
    c.pszText = (LPWSTR)L"Type"; c.cx = 120; c.iSubItem = 2; ListView_InsertColumn(fileList, 2, &c);
    c.pszText = (LPWSTR)L"Size"; c.cx = 80; c.iSubItem = 3; ListView_InsertColumn(fileList, 3, &c);

    statusBar = CreateWindowW(STATUSCLASSNAMEW, L"", WS_CHILD|WS_VISIBLE, 0,0,0,0, hwnd, NULL, NULL, NULL);

    HWND controls[] = {backBtn,forwardBtn,upBtn,refreshBtn,addressBar,goBtn,searchBox,searchBtn,darkBtn,sortBtn,newFolderBtn,treeView,fileList,statusBar};
    for (size_t i=0;i<sizeof(controls)/sizeof(controls[0]);++i) if (controls[i] && appFont) SendMessageW(controls[i], WM_SETFONT, (WPARAM)appFont, TRUE);

    HWND hdr = ListView_GetHeader(fileList);
    if (hdr) g_oldHeaderProc = (WNDPROC)SetWindowLongPtrW(hdr, GWLP_WNDPROC, (LONG_PTR)HeaderProc);

    SHFILEINFOW sfi = {};
    g_sysSmall = (HIMAGELIST)SHGetFileInfoW(L"C:\\", 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
    if (g_sysSmall) { TreeView_SetImageList(treeView, g_sysSmall, TVSIL_NORMAL); ListView_SetImageList(fileList, g_sysSmall, LVSIL_SMALL); }

    BuildTree();
    ApplyTheme();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        mainWnd = hwnd;
        CreateUI(hwnd);
        DragAcceptFiles(hwnd, TRUE);
        Navigate(THIS_PC_PATH);
        return 0;
    }
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wp;
        UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
        if (count == 0) { DragFinish(hDrop); return 0; }
        std::vector<std::wstring> files;
        for (UINT i = 0; i < count; ++i) {
            UINT len = DragQueryFileW(hDrop, i, NULL, 0);
            if (len == 0) continue;
            std::vector<wchar_t> buf(len + 1, 0);
            DragQueryFileW(hDrop, i, &buf[0], len + 1);
            files.push_back(&buf[0]);
        }
        POINT pt; DragQueryPoint(hDrop, &pt);
        DragFinish(hDrop);
        if (files.empty()) return 0;

        std::wstring dest = currentPath;
        if (fileList) {
            POINT ptList = pt;
            MapWindowPoints(hwnd, fileList, &ptList, 1);
            LVHITTESTINFO hit = {}; hit.pt = ptList;
            int row = ListView_HitTest(fileList, &hit);
            if (row >= 0) {
                FileItem* it = GetItemByListIndex(row);
                if (it && it->isDir) dest = it->fullPath;
            }
        }

        std::wstring from;
        for (size_t i = 0; i < files.size(); ++i) { from += files[i]; from.push_back(L'\0'); }
        from.push_back(L'\0');
        std::wstring to = dest; to.push_back(L'\0'); to.push_back(L'\0');

        bool move = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        SHFILEOPSTRUCTW op = {};
        op.hwnd = mainWnd;
        op.wFunc = move ? FO_MOVE : FO_COPY;
        op.pFrom = from.c_str();
        op.pTo = to.c_str();
        op.fFlags = FOF_ALLOWUNDO;
        int res = SHFileOperationW(&op);
        if (res == 0 && !op.fAnyOperationsAborted) {
            Status(move ? L"Moved." : L"Copied.");
            Navigate(currentPath, false);
        } else {
            Status(L"Drop operation failed or was cancelled.");
        }
        return 0;
    }
    case WM_SIZE:
        Layout(hwnd);
        return 0;
    case WM_ERASEBKGND: {
        if (g_darkMode) {
            RECT rc; GetClientRect(hwnd, &rc);
            FillRect((HDC)wp, &rc, g_darkBrush);
            return 1;
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        if (g_darkMode) {
            HDC hdc = (HDC)wp;
            SetTextColor(hdc, g_darkText);
            SetBkColor(hdc, (msg==WM_CTLCOLOREDIT)?g_darkBg2:g_darkBg);
            return (LRESULT)((msg==WM_CTLCOLOREDIT)?g_darkEditBrush:g_darkBrush);
        }
        break;
    }
    case WM_LBUTTONDOWN: {
        int x = (short)LOWORD(lp);
        int y = (short)HIWORD(lp);
        RECT r; GetClientRect(hwnd, &r);
        const int margin = 6;
        RECT sbRect = {};
        int sbH = 0;
        if (statusBar && GetWindowRect(statusBar, &sbRect)) sbH = sbRect.bottom - sbRect.top;
        int contentTop = TOPBAR_HEIGHT;
        int contentBottom = (r.bottom - r.top) - sbH - margin;
        int splitX = margin + g_leftPaneWidth + margin / 2;
        if (y >= contentTop && y <= contentBottom && abs(x - splitX) <= SPLITTER_HIT) {
            g_dragSplitter = true;
            SetCapture(hwnd);
            return 0;
        }
        break;
    }
    case WM_MOUSEMOVE: {
        if (g_dragSplitter) {
            int x = (short)LOWORD(lp);
            RECT r; GetClientRect(hwnd, &r);
            int w = r.right - r.left;
            const int margin = 6;
            int newW = x - margin;
            if (newW < 140) newW = 140;
            if (newW > w - 220) newW = w - 220;
            if (newW != g_leftPaneWidth) {
                g_leftPaneWidth = newW;
                Layout(hwnd);
            }
            return 0;
        }
        break;
    }
    case WM_LBUTTONUP:
        if (g_dragSplitter) {
            g_dragSplitter = false;
            ReleaseCapture();
            return 0;
        }
        break;
    case WM_SETCURSOR: {
        POINT pt; GetCursorPos(&pt); ScreenToClient(hwnd, &pt);
        RECT r; GetClientRect(hwnd, &r);
        const int margin = 6;
        RECT sbRect = {};
        int sbH = 0;
        if (statusBar && GetWindowRect(statusBar, &sbRect)) sbH = sbRect.bottom - sbRect.top;
        int contentTop = TOPBAR_HEIGHT;
        int contentBottom = (r.bottom - r.top) - sbH - margin;
        int splitX = margin + g_leftPaneWidth + margin / 2;
        if (pt.y >= contentTop && pt.y <= contentBottom && abs(pt.x - splitX) <= SPLITTER_HIT) {
            SetCursor(LoadCursor(NULL, IDC_SIZEWE));
            return TRUE;
        }
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        int code = HIWORD(wp);
        if (id == ID_SEARCH && code == EN_CHANGE) {
            if (!g_searchMode && !g_searching && searchBox) {
                int len = GetWindowTextLengthW(searchBox);
                std::wstring q;
                if (len > 0) {
                    std::vector<wchar_t> buf((size_t)len + 1, 0);
                    GetWindowTextW(searchBox, &buf[0], len + 1);
                    q.assign(&buf[0]);
                }
                size_t s = q.find_first_not_of(L" \t");
                size_t e = q.find_last_not_of(L" \t");
                if (s == std::wstring::npos) q.clear();
                else q = q.substr(s, e - s + 1);
                if (q != g_currentFilter) {
                    g_currentFilter = q;
                    ApplyCurrentFilter();
                }
            }
            return 0;
        }
        switch (id) {
            case ID_BACK:
            case ID_CMD_BACK: GoBack(); return 0;
            case ID_FORWARD:
            case ID_CMD_FORWARD: GoForward(); return 0;
            case ID_UP:
            case ID_CMD_UP: GoUp(); return 0;
            case ID_REFRESH: RefreshView(); return 0;
            case ID_GO: { int len=GetWindowTextLengthW(addressBar); if(len>0){ std::vector<wchar_t> b((size_t)len+1,0); GetWindowTextW(addressBar,&b[0],len+1); Navigate(&b[0]); } } return 0;
            case ID_DARKBTN: g_darkMode = !g_darkMode; ApplyTheme(); return 0;
            case ID_SORTBTN: ShowSortMenu(sortBtn); return 0;
            case ID_NEWFOLDERBTN: CreateNewFolder(); return 0;
            case ID_SEARCHBTN: PerformSearch(); return 0;
            case ID_CMD_FOCUS_ADDRESS:
                if (addressBar) { SetFocus(addressBar); SendMessageW(addressBar, EM_SETSEL, 0, -1); }
                return 0;
            case ID_CMD_FOCUS_SEARCH:
                if (searchBox) { SetFocus(searchBox); SendMessageW(searchBox, EM_SETSEL, 0, -1); }
                return 0;
        }
        break;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lp;
        if (!dis) break;

        // Status bar (owner-drawn text)
        if (statusBar && dis->hwndItem == statusBar) {
            HDC hdc = dis->hDC;
            RECT rc = dis->rcItem;
            COLORREF bg = g_darkMode ? RGB(40, 40, 40) : GetSysColor(COLOR_BTNFACE);
            COLORREF fg = g_darkMode ? RGB(210, 210, 210) : GetSysColor(COLOR_BTNTEXT);
            HBRUSH br = CreateSolidBrush(bg);
            FillRect(hdc, &rc, br);
            DeleteObject(br);
            // subtle top separator line
            HPEN sep = CreatePen(PS_SOLID, 1, g_darkMode ? RGB(60,60,60) : RGB(200,200,200));
            HGDIOBJ oldPen = SelectObject(hdc, sep);
            MoveToEx(hdc, rc.left, rc.top, NULL);
            LineTo(hdc, rc.right, rc.top);
            SelectObject(hdc, oldPen);
            DeleteObject(sep);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, fg);
            HFONT old = appFont ? (HFONT)SelectObject(hdc, appFont) : NULL;
            RECT tr = rc; tr.left += 8; tr.top += 1;
            const wchar_t* txt = (const wchar_t*)dis->itemData;
            if (txt) DrawTextW(hdc, txt, -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS);
            if (old) SelectObject(hdc, old);
            return TRUE;
        }

        // Buttons
        if (dis->CtlType == ODT_BUTTON) {
            HDC hdc = dis->hDC;
            RECT rc = dis->rcItem;
            bool pressed = (dis->itemState & ODS_SELECTED) != 0;
            bool disabled = (dis->itemState & ODS_DISABLED) != 0;
            bool focus = (dis->itemState & ODS_FOCUS) != 0;

            COLORREF bg, border, fg;
            if (g_darkMode) {
                bg = pressed ? RGB(74, 74, 78) : RGB(56, 56, 60);
                border = RGB(78, 78, 82);
                fg = disabled ? RGB(128, 128, 128) : g_darkText;
            } else {
                bg = pressed ? RGB(225, 225, 225) : GetSysColor(COLOR_BTNFACE);
                border = RGB(170, 170, 170);
                fg = disabled ? RGB(140, 140, 140) : GetSysColor(COLOR_BTNTEXT);
            }

            HBRUSH br = CreateSolidBrush(bg);
            FillRect(hdc, &rc, br);
            DeleteObject(br);

            HPEN pen = CreatePen(PS_SOLID, 1, border);
            HGDIOBJ oldPen = SelectObject(hdc, pen);
            HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);

            RECT tr = rc;
            if (pressed) { tr.left += 1; tr.top += 1; tr.right += 1; tr.bottom += 1; }

            Gdiplus::Image* btnImg = NULL;
            if (dis->hwndItem == backBtn) btnImg = g_imgBack;
            else if (dis->hwndItem == forwardBtn) btnImg = g_imgForward;
            else if (dis->hwndItem == upBtn) btnImg = g_imgUp;
            else if (dis->hwndItem == refreshBtn) btnImg = g_imgRefresh;

            if (btnImg && g_gdiplusToken) {
                Gdiplus::Graphics g(hdc);
                g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                int iw = (int)btnImg->GetWidth();
                int ih = (int)btnImg->GetHeight();
                int maxW = (tr.right - tr.left) - 10;
                int maxH = (tr.bottom - tr.top) - 8;
                if (maxW < 4) maxW = 4;
                if (maxH < 4) maxH = 4;
                int dw = iw, dh = ih;
                if (iw > 0 && ih > 0) {
                    if (dw > maxW) { dh = dh * maxW / dw; dw = maxW; }
                    if (dh > maxH) { dw = dw * maxH / dh; dh = maxH; }
                    if (dw < 1) dw = 1;
                    if (dh < 1) dh = 1;
                }
                int dx = tr.left + (tr.right - tr.left - dw) / 2;
                int dy = tr.top + (tr.bottom - tr.top - dh) / 2;

                bool invert = g_darkMode && (btnImg == g_imgBack || btnImg == g_imgForward || btnImg == g_imgUp || btnImg == g_imgRefresh);
                if (disabled || invert) {
                    Gdiplus::ColorMatrix cm = {{
                        {1,0,0,0,0},
                        {0,1,0,0,0},
                        {0,0,1,0,0},
                        {0,0,0,1,0},
                        {0,0,0,0,1}
                    }};
                    if (invert) {
                        cm.m[0][0] = -1; cm.m[1][1] = -1; cm.m[2][2] = -1;
                        cm.m[4][0] = 1;  cm.m[4][1] = 1;  cm.m[4][2] = 1;
                    }
                    if (disabled) {
                        cm.m[3][3] = 0.35f;
                    }
                    Gdiplus::ImageAttributes ia;
                    ia.SetColorMatrix(&cm, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
                    Gdiplus::Rect dest(dx, dy, dw, dh);
                    g.DrawImage(btnImg, dest, 0, 0, iw, ih, Gdiplus::UnitPixel, &ia);
                } else {
                    g.DrawImage(btnImg, dx, dy, dw, dh);
                }
            } else {
                wchar_t txt[128] = {};
                GetWindowTextW(dis->hwndItem, txt, 128);
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, fg);
                HFONT old = appFont ? (HFONT)SelectObject(hdc, appFont) : NULL;
                DrawTextW(hdc, txt, -1, &tr, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
                if (old) SelectObject(hdc, old);
            }

            if (focus && !disabled) {
                RECT fr = rc; InflateRect(&fr, -3, -3);
                DrawFocusRect(hdc, &fr);
            }
            return TRUE;
        }
        break;
    }
    case WM_NOTIFY: {
        if (!lp) break;
        LPNMHDR hdr = (LPNMHDR)lp;
        if (hdr->idFrom == ID_LIST) {
            if (hdr->code == (UINT)NM_CUSTOMDRAW) {
                NMLVCUSTOMDRAW* cd = (NMLVCUSTOMDRAW*)lp;
                if (cd->nmcd.dwDrawStage == CDDS_PREPAINT) {
                    SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
                    return TRUE;
                }
                if (cd->nmcd.dwDrawStage == CDDS_POSTPAINT) {
                    HWND hdr = ListView_GetHeader(fileList);
                    int cols = hdr ? Header_GetItemCount(hdr) : 0;
                    int colsW = 0;
                    for (int i = 0; i < cols; ++i) colsW += ListView_GetColumnWidth(fileList, i);
                    int xScroll = GetScrollPos(fileList, SB_HORZ);
                    RECT rc; GetClientRect(fileList, &rc);
                    int rightEdge = colsW - xScroll;
                    if (rightEdge < rc.left) rightEdge = rc.left;
                    if (rightEdge < rc.right) {
                        RECT fill = rc;
                        fill.left = rightEdge;
                        COLORREF bg = ListView_GetBkColor(fileList);
                        if (bg == CLR_NONE) bg = g_darkMode ? g_darkBg : GetSysColor(COLOR_WINDOW);
                        HBRUSH b = CreateSolidBrush(bg);
                        FillRect(cd->nmcd.hdc, &fill, b);
                        DeleteObject(b);
                    }
                    SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
            }
            if (hdr->code == (UINT)NM_DBLCLK) { OpenSelected(); return 0; }
            if (hdr->code == (UINT)NM_RCLICK) {
                DWORD pos = GetMessagePos(); POINT pt; pt.x=(short)LOWORD(pos); pt.y=(short)HIWORD(pos);
                POINT client = pt; ScreenToClient(fileList, &client);
                LVHITTESTINFO hit={}; hit.pt=client; int row = ListView_HitTest(fileList, &hit);
                g_useContextFiles = false;
                g_contextFiles.clear();
                if (row >= 0) {
                    bool rowSelected = (ListView_GetItemState(fileList, row, LVIS_SELECTED) & LVIS_SELECTED) != 0;
                    if (rowSelected) {
                        // operate on current selection (may be multi-select)
                        g_useContextFiles = false;
                    } else {
                        // right-click on unselected item: do not change selection, act on this item only
                        FileItem* it = GetItemByListIndex(row);
                        if (it) {
                            g_useContextFiles = true;
                            g_contextFiles.push_back(it->fullPath);
                        }
                    }
                }
                ShowItemContextMenu(pt.x, pt.y, row >= 0);
                g_useContextFiles = false;
                g_contextFiles.clear();
                return 0;
            }
            if (hdr->code == LVN_COLUMNCLICK) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                switch (nmlv->iSubItem) {
                    case 0: g_sortMode = (g_sortMode==SORT_NAME_ASC)?SORT_NAME_DESC:SORT_NAME_ASC; g_groupByDate=false; break;
                    case 1: g_sortMode = (g_sortMode==SORT_DATE_DESC)?SORT_DATE_ASC:SORT_DATE_DESC; g_groupByDate=false; break;
                    case 2: g_sortMode = (g_sortMode==SORT_TYPE_ASC)?SORT_TYPE_DESC:SORT_TYPE_ASC; g_groupByDate=false; break;
                    case 3: g_sortMode = (g_sortMode==SORT_SIZE_ASC)?SORT_SIZE_DESC:SORT_SIZE_ASC; g_groupByDate=false; break;
                }
                PopulateListView();
                return 0;
            }
            if (hdr->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                if (nmlv && (nmlv->uChanged & LVIF_STATE) && ((nmlv->uOldState ^ nmlv->uNewState) & LVIS_SELECTED)) {
                    UpdateStatusBar();
                }
                return 0;
            }
            if (hdr->code == LVN_KEYDOWN) {
                NMLVKEYDOWN* kd = (NMLVKEYDOWN*)lp;
                bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                if (alt) { /* let accelerators/system handle */ }
                else if (ctrl && kd->wVKey == 'C') { std::vector<std::wstring> s = SelectedFiles(); if (CopyFilesToClipboard(s, false)) Status(L"Copied."); return 0; }
                else if (ctrl && kd->wVKey == 'X') { std::vector<std::wstring> s = SelectedFiles(); if (CopyFilesToClipboard(s, true)) Status(L"Cut."); return 0; }
                else if (ctrl && kd->wVKey == 'V') { PasteFromClipboard(); return 0; }
                else if (ctrl && kd->wVKey == 'A') { ListView_SetItemState(fileList, -1, LVIS_SELECTED, LVIS_SELECTED); SetFocus(fileList); return 0; }
                else if (ctrl && kd->wVKey == 'N') { CreateNewFolder(); return 0; }
                else if (kd->wVKey == VK_DELETE) { DeleteSelected(!shift); return 0; }
                else if (kd->wVKey == VK_F2) { int i = ListView_GetNextItem(fileList, -1, LVNI_SELECTED); if (i >= 0) { SetFocus(fileList); ListView_EditLabel(fileList, i); } return 0; }
                else if (kd->wVKey == VK_F5) { RefreshView(); return 0; }
                else if (kd->wVKey == VK_BACK) { GoUp(); return 0; }
                else if (kd->wVKey == VK_RETURN) { OpenSelected(); return 0; }
                else if (kd->wVKey == VK_ESCAPE) {
                    if (g_searchMode || !g_currentFilter.empty()) {
                        if (searchBox) SetWindowTextW(searchBox, L"");
                        if (g_searchMode) { g_searchMode = false; g_searchRoot.clear(); SetListColumnsForMode(); LoadDirectory(currentPath); }
                        else { g_currentFilter.clear(); ApplyCurrentFilter(); }
                        return 0;
                    }
                }
            }
            if (hdr->code == LVN_ENDLABELEDITW) {
                NMLVDISPINFOW* di = (NMLVDISPINFOW*)lp;
                if (di && di->item.pszText && di->item.iItem >= 0) {
                    FileItem* it = GetItemByListIndex(di->item.iItem);
                    if (it) {
                        std::wstring newPath = JoinPath(currentPath, di->item.pszText);
                        if (MoveFileW(it->fullPath.c_str(), newPath.c_str())) {
                            RefreshView();
                        } else {
                            MessageBoxW(mainWnd, L"Rename failed.", L"mFile", MB_ICONERROR);
                        }
                    }
                }
                return FALSE;
            }
        } else if (hdr->idFrom == ID_TREE) {
            if (hdr->code == (UINT)NM_CUSTOMDRAW) {
                NMTVCUSTOMDRAW* cd = (NMTVCUSTOMDRAW*)lp;
                if (!cd) break;
                if (cd->nmcd.dwDrawStage == CDDS_PREPAINT) {
                    SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
                    return TRUE;
                }
                if (cd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                    TreePathItem* tpi = (TreePathItem*)cd->nmcd.lItemlParam;
                    Gdiplus::Image* img = (tpi && g_gdiplusToken) ? GetUiIconImage(tpi->iconKind) : NULL;
                    if (img) {
                        SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
                        return TRUE;
                    }
                    SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
                if (cd->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT) {
                    HTREEITEM hItem = (HTREEITEM)cd->nmcd.dwItemSpec;
                    TreePathItem* tpi = (TreePathItem*)cd->nmcd.lItemlParam;
                    Gdiplus::Image* img = (tpi && g_gdiplusToken) ? GetUiIconImage(tpi->iconKind) : NULL;
                    if (img && hItem) {
                        RECT labelRc;
                        if (TreeView_GetItemRect(treeView, hItem, &labelRc, TRUE)) {
                            int iconSize = 16;
                            HIMAGELIST il = TreeView_GetImageList(treeView, TVSIL_NORMAL);
                            if (il) {
                                int cx = 0, cy = 0;
                                if (ImageList_GetIconSize(il, &cx, &cy) && cx > 0 && cy > 0) {
                                    iconSize = cx;
                                }
                            }
                            int iconX = labelRc.left - iconSize - 2;
                            int iconY = labelRc.top + ((labelRc.bottom - labelRc.top) - iconSize) / 2;
                            RECT iconRc = { iconX - 2, iconY - 2, iconX + iconSize + 2, iconY + iconSize + 2 };

                            COLORREF bg = TreeView_GetBkColor(treeView);
                            if (bg == (COLORREF)-1) bg = g_darkMode ? g_darkBg2 : GetSysColor(COLOR_WINDOW);
                            if (cd->nmcd.uItemState & CDIS_SELECTED) {
                                bg = GetSysColor(COLOR_HIGHLIGHT);
                                if (g_darkMode) bg = RGB(62, 62, 66);
                            }
                            HBRUSH br = CreateSolidBrush(bg);
                            FillRect(cd->nmcd.hdc, &iconRc, br);
                            DeleteObject(br);

                            Gdiplus::Graphics g(cd->nmcd.hdc);
                            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                            g.DrawImage(img, iconX, iconY, iconSize, iconSize);
                        }
                    }
                    SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
                SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, CDRF_DODEFAULT);
                return TRUE;
            }
            if (hdr->code == TVN_SELCHANGEDW) {
                if (!g_updatingTreeSelection) {
                    NMTREEVIEWW* nmtv = (NMTREEVIEWW*)lp;
                    if (nmtv && nmtv->itemNew.hItem) {
                        std::wstring p = GetTreeItemPath(nmtv->itemNew.hItem);
                        if (!p.empty() && !PathEqualsInsensitive(p, currentPath)) Navigate(p);
                    }
                }
                return 0;
            }
        }
        break;
    }
    case WM_DESTROY:
        ClearTreeItems();
        if (g_accel) { DestroyAcceleratorTable(g_accel); g_accel = NULL; }
        if (g_darkBrush) { DeleteObject(g_darkBrush); g_darkBrush=NULL; }
        if (g_darkEditBrush) { DeleteObject(g_darkEditBrush); g_darkEditBrush=NULL; }
        if (appFont && appFont != (HFONT)GetStockObject(DEFAULT_GUI_FONT)) DeleteObject(appFont);
        appFont = NULL;
        UnloadToolbarIcons();
        ShutdownGdiplus();
        if (g_appIconSmall == g_appIconBig) g_appIconSmall = NULL;
        if (g_appIconBig) { DestroyIcon(g_appIconBig); g_appIconBig = NULL; }
        if (g_appIconSmall) { DestroyIcon(g_appIconSmall); g_appIconSmall = NULL; }
        if (g_comInit) { OleUninitialize(); g_comInit = false; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int show) {
    bool actCtxActive = ActivateComCtlV6();

    // Enable dark-mode-aware menus on supported Windows 10 builds (best effort)
    ApplyAppDarkModePreference(g_darkMode);

    INITCOMMONCONTROLSEX ic = {};
    ic.dwSize = sizeof(ic);
    ic.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&ic);

    InitGdiplus();
    LoadAppIcon();
    LoadToolbarIcons();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = h;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = g_appIconBig ? g_appIconBig : LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"mFileWindow";
    if (!RegisterClassW(&wc)) {
        if (actCtxActive) DeactivateComCtlV6();
        return 1;
    }

    HWND w = CreateWindowExW(0, wc.lpszClassName, L"mFile", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 650, NULL, NULL, h, NULL);
    if (!w) {
        if (actCtxActive) DeactivateComCtlV6();
        return 1;
    }

    if (g_appIconBig) SendMessageW(w, WM_SETICON, ICON_BIG, (LPARAM)g_appIconBig);
    if (g_appIconSmall) SendMessageW(w, WM_SETICON, ICON_SMALL, (LPARAM)g_appIconSmall);

    ACCEL accels[] = {
        { FCONTROL | FVIRTKEY, 'L', ID_CMD_FOCUS_ADDRESS },
        { FCONTROL | FVIRTKEY, 'F', ID_CMD_FOCUS_SEARCH },
        { FVIRTKEY, VK_F5, ID_REFRESH },
        { FALT | FVIRTKEY, VK_LEFT, ID_CMD_BACK },
        { FALT | FVIRTKEY, VK_RIGHT, ID_CMD_FORWARD },
        { FALT | FVIRTKEY, VK_UP, ID_CMD_UP },
    };
    g_accel = CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));

    ShowWindow(w, show);
    UpdateWindow(w);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (g_accel && TranslateAcceleratorW(w, g_accel, &msg)) continue;
        if (!IsDialogMessageW(w, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    UnloadToolbarIcons();
    ShutdownGdiplus();
    if (actCtxActive) DeactivateComCtlV6();
    return (int)msg.wParam;
}
