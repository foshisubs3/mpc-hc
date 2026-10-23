/*
 * Subtitle style presets button (Normal / Movies / Anime) shown in the player toolbar,
 * just before the mute button and the volume slider.
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "CMPCThemeMenu.h"

class CMainFrame;

class CSubStylePresetButton : public CWnd
{
    DECLARE_DYNAMIC(CSubStylePresetButton)

public:
    CSubStylePresetButton();
    virtual ~CSubStylePresetButton();

    bool Create(CWnd* pParentWnd, CMainFrame* pMainFrame);

    // Width needed to show the widest preset name plus the drop-down arrow
    int GetIdealWidth();
    // Redraw after the active preset changed
    void Refresh();

    static CString PresetName(int idx);

protected:
    CMainFrame* m_pMainFrame;
    CMPCThemeMenu m_menu;
    CFont m_font;
    bool m_bHover;
    bool m_bTracking;

    void EnsureFont();
    void ShowPresetMenu();

    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};
