/*
 * Subtitle style presets button (Normal / Movies / Anime) shown in the player toolbar.
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "SubStylePresetButton.h"
#include "MainFrm.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

namespace
{
    // Arabic labels, written as escapes so the source file stays plain ASCII
    const wchar_t* const kPresetNames[CAppSettings::SUB_PRESET_COUNT] = {
        L"\u0637\u0628\u064A\u0639\u064A",          // Normal
        L"\u0623\u0641\u0644\u0627\u0645",          // Movies
        L"\u0623\u0646\u0645\u064A",                // Anime
    };
    const wchar_t* const kEditPrefix = L"\u062A\u0639\u062F\u064A\u0644 \u0633\u062A\u0627\u064A\u0644 "; // "Edit style: "
    // ASS handling modes (radio group)
    const wchar_t* const kModeNone = L"\u0628\u062F\u0648\u0646 \u0627\u0633\u062A\u0628\u062F\u0627\u0644 (\u0633\u062A\u0627\u064A\u0644 \u0627\u0644\u0645\u0644\u0641 \u0643\u0645\u0627 \u0647\u0648)";       // keep the file's styles
    const wchar_t* const kModeDefault = L"\u0627\u0633\u062A\u0628\u062F\u0627\u0644 \u0633\u062A\u0627\u064A\u0644 Default \u0641\u0642\u0637";    // replace "Default" only
    const wchar_t* const kModeMain = L"\u0627\u0633\u062A\u0628\u062F\u0627\u0644 \u0627\u0644\u0633\u062A\u0627\u064A\u0644 \u0627\u0644\u0631\u0626\u064A\u0633\u064A \u062A\u0644\u0642\u0627\u0626\u064A\u064B\u0627";       // replace the detected main style
    const wchar_t* const kModeMainNow = L" (\u0627\u0644\u062D\u0627\u0644\u064A: ";    // " (current: "
    const wchar_t* const kModeAll = L"\u0627\u0633\u062A\u0628\u062F\u0627\u0644 \u0643\u0644 \u0633\u062A\u0627\u064A\u0644\u0627\u062A ASS";        // replace every style
    const wchar_t* const kArrow = L" \u25BE";

    enum {
        CMD_PRESET_FIRST = 1,                                       // 1..3 select a preset
        CMD_EDIT_FIRST = CMD_PRESET_FIRST + CAppSettings::SUB_PRESET_COUNT, // edit preset 1..2
        CMD_MODE_NONE = 20,
        CMD_MODE_DEFAULT,
        CMD_MODE_MAIN,
        CMD_MODE_ALL,
    };

    const int kHorzPadding = 8;
}

IMPLEMENT_DYNAMIC(CSubStylePresetButton, CWnd)

BEGIN_MESSAGE_MAP(CSubStylePresetButton, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

CSubStylePresetButton::CSubStylePresetButton()
    : m_pMainFrame(nullptr)
    , m_bHover(false)
    , m_bTracking(false)
{
}

CSubStylePresetButton::~CSubStylePresetButton()
{
}

CString CSubStylePresetButton::PresetName(int idx)
{
    if (idx < 0 || idx >= CAppSettings::SUB_PRESET_COUNT) {
        idx = CAppSettings::SUB_PRESET_NORMAL;
    }
    return kPresetNames[idx];
}

bool CSubStylePresetButton::Create(CWnd* pParentWnd, CMainFrame* pMainFrame)
{
    m_pMainFrame = pMainFrame;
    CString wndClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_HAND));
    return !!CreateEx(0, wndClass, L"", WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, 0);
}

void CSubStylePresetButton::EnsureFont()
{
    if (!m_font.GetSafeHandle()) {
        CMPCThemeUtil::getFontByType(m_font, this, CMPCThemeUtil::StatusFont);
    }
}

int CSubStylePresetButton::GetIdealWidth()
{
    if (!m_hWnd) {
        return 0;
    }
    // re-create the font so DPI changes are picked up
    if (m_font.GetSafeHandle()) {
        m_font.DeleteObject();
    }
    EnsureFont();

    CClientDC dc(this);
    CFont* pOld = dc.SelectObject(&m_font);
    int maxWidth = 0;
    for (int i = 0; i < CAppSettings::SUB_PRESET_COUNT; i++) {
        CSize sz = dc.GetTextExtent(PresetName(i) + kArrow);
        maxWidth = std::max(maxWidth, (int)sz.cx);
    }
    dc.SelectObject(pOld);
    return maxWidth + 2 * kHorzPadding;
}

void CSubStylePresetButton::Refresh()
{
    if (m_hWnd) {
        Invalidate();
    }
}

BOOL CSubStylePresetButton::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // everything is drawn in OnPaint
}

void CSubStylePresetButton::OnPaint()
{
    CPaintDC dc(this);
    CRect r;
    GetClientRect(&r);

    const bool themed = AppIsThemeLoaded();
    COLORREF bg = themed ? CMPCTheme::PlayerBGColor : ::GetSysColor(COLOR_BTNFACE);
    COLORREF fg = themed ? CMPCTheme::TextFGColor : ::GetSysColor(COLOR_BTNTEXT);

    dc.FillSolidRect(r, bg);
    if (m_bHover) {
        COLORREF hot = themed ? CMPCTheme::PlayerButtonHotColor : ::GetSysColor(COLOR_3DLIGHT);
        COLORREF border = themed ? CMPCTheme::PlayerButtonBorderColor : ::GetSysColor(COLOR_3DSHADOW);
        dc.FillSolidRect(r, hot);
        CBrush b;
        b.CreateSolidBrush(border);
        dc.FrameRect(r, &b);
    }

    EnsureFont();
    CFont* pOld = dc.SelectObject(&m_font);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(fg);
    CString text = PresetName(AfxGetAppSettings().nSubStylePreset) + kArrow;
    dc.DrawText(text, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    dc.SelectObject(pOld);
}

void CSubStylePresetButton::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bTracking) {
        TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd, 0 };
        m_bTracking = !!TrackMouseEvent(&tme);
    }
    if (!m_bHover) {
        m_bHover = true;
        Invalidate();
    }
    __super::OnMouseMove(nFlags, point);
}

LRESULT CSubStylePresetButton::OnMouseLeave(WPARAM, LPARAM)
{
    m_bTracking = false;
    if (m_bHover) {
        m_bHover = false;
        Invalidate();
    }
    return 0;
}

void CSubStylePresetButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    ShowPresetMenu();
}

void CSubStylePresetButton::ShowPresetMenu()
{
    if (!m_pMainFrame) {
        return;
    }
    CAppSettings& s = AfxGetAppSettings();

    m_menu.DestroyMenu();
    m_menu.CreatePopupMenu();
    for (int i = 0; i < CAppSettings::SUB_PRESET_COUNT; i++) {
        m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_PRESET_FIRST + i, PresetName(i));
    }
    m_menu.CheckMenuRadioItem(CMD_PRESET_FIRST, CMD_PRESET_FIRST + CAppSettings::SUB_PRESET_COUNT - 1,
                              CMD_PRESET_FIRST + s.nSubStylePreset, MF_BYCOMMAND);
    m_menu.AppendMenu(MF_SEPARATOR);
    for (int i = CAppSettings::SUB_PRESET_MOVIES; i < CAppSettings::SUB_PRESET_COUNT; i++) {
        m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_EDIT_FIRST + i - 1, CString(kEditPrefix) + PresetName(i) + L"...");
    }
    m_menu.AppendMenu(MF_SEPARATOR);
    CString mainLabel = kModeMain;
    CString detected = m_pMainFrame->GetDetectedMainSubStyle();
    if (!detected.IsEmpty()) {
        mainLabel += CString(kModeMainNow) + detected + L")";
    }
    m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_MODE_NONE, kModeNone);
    m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_MODE_DEFAULT, kModeDefault);
    m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_MODE_MAIN, mainLabel);
    m_menu.AppendMenu(MF_STRING | MF_ENABLED, CMD_MODE_ALL, kModeAll);
    UINT mode = s.ActiveSubOverrideAll() ? CMD_MODE_ALL
                : !s.ActiveSubOverrideDefault() ? CMD_MODE_NONE
                : s.ActiveSubAutoMain() ? CMD_MODE_MAIN : CMD_MODE_DEFAULT;
    m_menu.CheckMenuRadioItem(CMD_MODE_NONE, CMD_MODE_ALL, mode, MF_BYCOMMAND);

    if (AppIsThemeLoaded()) {
        m_menu.fulfillThemeReqs();
    }

    CRect wr;
    GetWindowRect(&wr);
    // open upwards from the top-left corner of the button, like the other toolbar menus
    UINT cmd = m_menu.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_LEFTALIGN, wr.left, wr.top, m_pMainFrame);

    m_bHover = false;
    Invalidate();

    if (cmd >= CMD_PRESET_FIRST && cmd < CMD_PRESET_FIRST + (UINT)CAppSettings::SUB_PRESET_COUNT) {
        m_pMainFrame->SetSubStylePreset((int)(cmd - CMD_PRESET_FIRST));
    } else if (cmd >= CMD_EDIT_FIRST && cmd < CMD_EDIT_FIRST + (UINT)(CAppSettings::SUB_PRESET_COUNT - 1)) {
        m_pMainFrame->EditSubStylePreset((int)(cmd - CMD_EDIT_FIRST) + 1);
    } else if (cmd >= CMD_MODE_NONE && cmd <= CMD_MODE_ALL) {
        s.ActiveSubOverrideAll() = (cmd == CMD_MODE_ALL);
        s.ActiveSubOverrideDefault() = (cmd == CMD_MODE_DEFAULT || cmd == CMD_MODE_MAIN);
        s.ActiveSubAutoMain() = (cmd == CMD_MODE_MAIN);
        m_pMainFrame->ApplySubStylePresetChange(true);
    }
}
