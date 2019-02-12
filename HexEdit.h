#pragma once


// CHexEdit

class CHexEdit : public CEdit
{
    DECLARE_DYNAMIC(CHexEdit)

public:
    CHexEdit();
    virtual ~CHexEdit();

protected:
    afx_msg void OnKeyDown( UINT, UINT, UINT );
    afx_msg void OnChar( UINT, UINT, UINT );
    afx_msg void OnLButtonDown( UINT, CPoint );
    afx_msg void OnLButtonUp( UINT, CPoint );
    afx_msg void OnMouseMove( UINT, CPoint );
    DECLARE_MESSAGE_MAP()
};

