// PreferencesToolPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PreferencesToolPage.h"
#include "tdcenum.h"

#include "..\shared\enstring.h"
#include "..\shared\misc.h"
#include "..\shared\filemisc.h"
#include "..\shared\enfiledialog.h"
#include "..\shared\enmenu.h"
#include "..\shared\dialoghelper.h"
#include "..\shared\themed.h"

#include "..\3rdparty\ini.h"

#include "..\Interfaces\Preferences.h"

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreferencesToolPage property page

static LPCTSTR REALQUOTE = _T("\"");
static LPCTSTR SAFEQUOTE = _T("{QUOTES}");

IMPLEMENT_DYNCREATE(CPreferencesToolPage, CPreferencesPageBase)

CPreferencesToolPage::CPreferencesToolPage(int nMaxNumTools) 
	: 
	CPreferencesPageBase(CPreferencesToolPage::IDD),
	m_eToolPath(FES_COMBOSTYLEBTN | FES_ALLOWURL),
	m_eIconPath(FES_COMBOSTYLEBTN | FES_DISPLAYSIMAGES),
	m_nMaxNumTools(nMaxNumTools),
	m_btnArgMenu(IDR_MISC, MM_TOOLARGS, MBS_DOWN)
{
	//{{AFX_DATA_INIT(CPreferencesToolPage)
	m_sToolPath = _T("");
	m_sCommandLine = _T("");
	m_bRunMinimized = FALSE;
	m_sIconPath = _T("");
	//}}AFX_DATA_INIT
}

CPreferencesToolPage::~CPreferencesToolPage()
{
}

void CPreferencesToolPage::DoDataExchange(CDataExchange* pDX)
{
	CPreferencesPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreferencesToolPage)
	DDX_Control(pDX, IDC_ARGMENUBTN, m_btnArgMenu);
	DDX_Control(pDX, IDC_TOOLPATH, m_eToolPath);
	DDX_Control(pDX, IDC_TOOLLIST, m_lcTools);
	DDX_Text(pDX, IDC_TOOLPATH, m_sToolPath);
	DDX_Text(pDX, IDC_CMDLINE, m_sCommandLine);
	DDX_Check(pDX, IDC_RUNMINIMIZED, m_bRunMinimized);
	DDX_Text(pDX, IDC_ICONPATH, m_sIconPath);
	DDX_Control(pDX, IDC_ICONPATH, m_eIconPath);
	DDX_Control(pDX, IDC_CMDLINE, m_eCmdLine);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_DISPLAYUDTSINTOOLBAR, m_bDisplayUDTsInToolbar);
}

BEGIN_MESSAGE_MAP(CPreferencesToolPage, CPreferencesPageBase)
	//{{AFX_MSG_MAP(CPreferencesToolPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_UDTPREFS_NEW, OnNewTool)
	ON_COMMAND(ID_UDTPREFS_DELETE, OnDeleteTool)
	ON_COMMAND(ID_UDTPREFS_COPY, OnCopyTool)
	ON_COMMAND(ID_UDTPREFS_EDIT, OnEditToolName)
	ON_COMMAND(ID_UDTPREFS_MOVEDOWN, OnMoveToolDown)
	ON_COMMAND(ID_UDTPREFS_MOVEUP, OnMoveToolUp)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_NEW, OnUpdateCmdUINewTool)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_DELETE, OnUpdateCmdUIDeleteTool)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_COPY, OnUpdateCmdUICopyTool)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_EDIT, OnUpdateCmdUIEditToolName)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_MOVEDOWN, OnUpdateCmdUIMoveToolDown)
	ON_UPDATE_COMMAND_UI(ID_UDTPREFS_MOVEUP, OnUpdateCmdUIMoveToolUp)

	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_TOOLLIST, OnEndlabeleditToollist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TOOLLIST, OnItemchangedToollist)
	ON_EN_CHANGE(IDC_TOOLPATH, OnChangeToolpath)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TOOLLIST, OnKeydownToollist)
	ON_EN_CHANGE(IDC_CMDLINE, OnChangeCmdline)
	ON_COMMAND_RANGE(ID_TOOLARG_PATHNAME, ID_TOOLARG_SELTASKCUSTOM, OnInsertPlaceholder)
	ON_BN_CLICKED(IDC_RUNMINIMIZED, OnRunminimized)
	ON_BN_CLICKED(IDC_TESTTOOL, OnTestTool)
	ON_EN_CHANGE(IDC_ICONPATH, OnChangeIconPath)
	ON_BN_CLICKED(IDC_IMPORT, OnImportTools)
	ON_REGISTERED_MESSAGE(WM_FE_GETFILEICON, OnGetFileIcon)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreferencesToolPage message handlers

BOOL CPreferencesToolPage::OnInitDialog() 
{
	CPreferencesPageBase::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreferencesToolPage::OnFirstShow()
{
	CPreferencesPageBase::OnFirstShow();

	VERIFY(InitializeToolbar());

	m_ilSys.Initialize();
	m_lcTools.SetImageList(m_ilSys.GetImageList(), LVSIL_SMALL);

	m_eToolPath.SetCurrentFolder(FileMisc::GetAppFolder());
	m_eIconPath.SetCurrentFolder(FileMisc::GetAppFolder());

	m_btnArgMenu.SetTooltip(CEnString(IDS_PTP_PLACEHOLDERS));

	CRect rList;
	m_lcTools.GetClientRect(rList);

	m_lcTools.InsertColumn(0, CEnString(IDS_PTP_TOOLNAME), LVCFMT_LEFT, 150);
	m_lcTools.InsertColumn(1, CEnString(IDS_PTP_TOOLPATH), LVCFMT_LEFT, 250);
	m_lcTools.InsertColumn(2, CEnString(IDS_PTP_ARGUMENTS), LVCFMT_LEFT, rList.Width() - 400);
	m_lcTools.InsertColumn(3, CEnString(IDS_PTP_ICONPATH), LVCFMT_LEFT, 0);

	m_lcTools.SetExtendedStyle(m_lcTools.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	
	// add tools we loaded from the registry
	for (int nTool = 0; nTool < m_aTools.GetSize(); nTool++)
	{
		const USERTOOL& tool = m_aTools[nTool];
		VERIFY(AddListTool(tool) != -1);

	}
	RebuildListCtrlImages();

	m_lcTools.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	OnItemchangedToollist(NULL, NULL);

	CThemed::SetWindowTheme(&m_lcTools, _T("Explorer"));

	EnableControls();
	m_toolbar.RefreshButtonStates(FALSE);
}

int CPreferencesToolPage::AddListTool(const USERTOOL& tool, int nPos, BOOL bRebuildImages)
{
	// special case
	if (nPos == -1)
		nPos = m_lcTools.GetItemCount();

	// Sanity check
	if ((nPos < 0) || (nPos > m_lcTools.GetItemCount()))
	{
		ASSERT(0);
		return -1;
	}

	int nIndex = m_lcTools.InsertItem(nPos, tool.sToolName, -1);

	if (nIndex == -1)
		return -1;
	
	m_lcTools.SetItemText(nIndex, 1, tool.sToolPath);
	m_lcTools.SetItemText(nIndex, 2, tool.sCmdline);
	m_lcTools.SetItemText(nIndex, 3, tool.sIconPath);
	m_lcTools.SetItemData(nIndex, tool.bRunMinimized);

	if (bRebuildImages)
		RebuildListCtrlImages();
	
	return nIndex;
}

void CPreferencesToolPage::OnNewTool() 
{
	ASSERT(m_lcTools.GetItemCount() < m_nMaxNumTools);

	int nIndex = m_lcTools.InsertItem(m_lcTools.GetItemCount(), CEnString(IDS_PTP_NEWTOOL), -1);
	m_lcTools.SetItemText(nIndex, 2, MapCmdIDToPlaceholder(ID_TOOLARG_PATHNAME));

	m_lcTools.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
	m_lcTools.SetFocus();
	m_lcTools.EditLabel(nIndex);

	m_toolbar.RefreshButtonStates(TRUE);

	CPreferencesPageBase::OnControlChange();
}

void CPreferencesToolPage::OnUpdateCmdUINewTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_lcTools.GetItemCount() < m_nMaxNumTools);
}

void CPreferencesToolPage::OnDeleteTool() 
{
	int nSel = GetCurSel();

	if (nSel != -1)
	{
		m_lcTools.DeleteItem(nSel);

		// Select next previous item
		if (nSel == m_lcTools.GetItemCount())
			nSel--;

		SetCurSel(nSel);
		
		if (nSel == -1)
		{
			m_sToolPath.Empty();
			m_sIconPath.Empty();
			
			EnableControls();
			UpdateData(FALSE);
		}
	}

	CPreferencesPageBase::OnControlChange();
}

void CPreferencesToolPage::OnUpdateCmdUIDeleteTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurSel() != -1);
}

void CPreferencesToolPage::OnEditToolName() 
{
	int nSel = GetCurSel();
	
	if (nSel != -1)
	{
		m_lcTools.SetFocus();
		m_lcTools.EditLabel(nSel);
	}
}

void CPreferencesToolPage::OnUpdateCmdUIEditToolName(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurSel() != -1);
}

void CPreferencesToolPage::OnCopyTool() 
{
	ASSERT(m_lcTools.GetItemCount() < m_nMaxNumTools);
	int nSel = GetCurSel();
	
	if (nSel != -1)
	{
		USERTOOL tool;
		VERIFY(GetListTool(nSel, tool));
		
		int nCopy = AddListTool(tool, (nSel + 1), TRUE);
		SetCurSel(nCopy);
		
		m_lcTools.SetFocus();
		m_lcTools.EditLabel(nCopy);
		
		m_toolbar.RefreshButtonStates(TRUE);
		
		CPreferencesPageBase::OnControlChange();
	}
}

void CPreferencesToolPage::OnUpdateCmdUICopyTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetCurSel() != -1) && (m_lcTools.GetItemCount() < m_nMaxNumTools));
}

void CPreferencesToolPage::OnMoveToolUp() 
{
	int nSel = GetCurSel();
	
	if (nSel > 0)
	{
		USERTOOL tool;
		VERIFY(GetListTool(nSel, tool));

		m_lcTools.DeleteItem(nSel);
		nSel = AddListTool(tool, (nSel - 1), TRUE);
		SetCurSel(nSel);

		m_lcTools.SetFocus();

		CPreferencesPageBase::OnControlChange();
	}
}

void CPreferencesToolPage::OnUpdateCmdUIMoveToolUp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurSel() > 0);
}

void CPreferencesToolPage::OnMoveToolDown() 
{
	int nSel = GetCurSel();
	
	if ((nSel >= 0) && (nSel < (m_lcTools.GetItemCount() - 1)))
	{
		USERTOOL tool;
		VERIFY(GetListTool(nSel, tool));
		
		m_lcTools.DeleteItem(nSel);
		nSel = AddListTool(tool, (nSel + 1), TRUE);
		SetCurSel(nSel);

		m_lcTools.SetFocus();
		
		CPreferencesPageBase::OnControlChange();
	}
}

void CPreferencesToolPage::OnUpdateCmdUIMoveToolDown(CCmdUI* pCmdUI)
{
	int nSel = GetCurSel();

	pCmdUI->Enable((nSel >= 0) && (nSel < (m_lcTools.GetItemCount() - 1)));
}

void CPreferencesToolPage::OnEndlabeleditToollist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	if (pDispInfo->item.pszText)
	{
		int nSel = GetCurSel();

		if (nSel >= 0)
		{
			m_lcTools.SetItemText(nSel, 0, pDispInfo->item.pszText);

			GetDlgItem(IDC_TOOLPATH)->SetFocus();
		}
	}
	
	*pResult = 0;

	CPreferencesPageBase::OnControlChange();
}

void CPreferencesToolPage::OnItemchangedToollist(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/) 
{
	EnableControls();
	int nSel = GetCurSel();

	if (nSel >= 0)
	{
		m_sToolPath = m_lcTools.GetItemText(nSel, 1);
		m_sCommandLine = m_lcTools.GetItemText(nSel, 2);
		m_sIconPath = m_lcTools.GetItemText(nSel, 3);
		m_bRunMinimized = m_lcTools.GetItemData(nSel);
	}
	else
	{
		m_sToolPath.Empty();
		m_sCommandLine.Empty();
		m_bRunMinimized = FALSE;
		m_sIconPath.Empty();
	}

	UpdateData(FALSE);
	m_toolbar.RefreshButtonStates(FALSE);
}

void CPreferencesToolPage::EnableControls()
{
	int nSel = GetCurSel();

	GetDlgItem(IDC_TOOLPATH)->EnableWindow(nSel >= 0);
	GetDlgItem(IDC_ICONPATH)->EnableWindow(nSel >= 0);
	GetDlgItem(IDC_CMDLINE)->EnableWindow(nSel >= 0);
	GetDlgItem(IDC_RUNMINIMIZED)->EnableWindow(nSel >= 0);

	m_btnArgMenu.EnableWindow(nSel >= 0);
}

int CPreferencesToolPage::GetCurSel() const
{
	int nSel = -1;
	POSITION pos = m_lcTools.GetFirstSelectedItemPosition();

	if (pos)
		nSel = m_lcTools.GetNextSelectedItem(pos);

	return nSel;
}

BOOL CPreferencesToolPage::SetCurSel(int nSel)
{
	if (nSel != -1)
		m_lcTools.SetItemState(nSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	// else
	return FALSE;
}

void CPreferencesToolPage::OnChangeToolpath() 
{
	int nSel = GetCurSel();

	if (nSel == CB_ERR)
	{
		ASSERT (0);
		return;
	}

	UpdateData();

	m_lcTools.SetItemText(nSel, 1, m_sToolPath);

	RebuildListCtrlImages();

	CPreferencesPageBase::OnControlChange();
}

void CPreferencesToolPage::RebuildListCtrlImages()
{
	m_ilSys.Initialize();
	m_lcTools.SetImageList(m_ilSys.GetImageList(), LVSIL_SMALL);

	int nTool = m_lcTools.GetItemCount();

	while (nTool--)
	{
		LVITEM lvi = { 0 };
		
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = nTool;
		
		CString sIconPath = m_lcTools.GetItemText(nTool, 3);

		if (sIconPath.IsEmpty())
		{
			CString sToolPath = m_lcTools.GetItemText(nTool, 1);

			if (!CTDCToolsCmdlineParser::PrepareToolPath(sToolPath))
				FileMisc::MakeFullPath(sToolPath, FileMisc::GetAppFolder());

			lvi.iImage = m_ilSys.GetFileImageIndex(sToolPath);	
		}
		else
		{
			FileMisc::MakeFullPath(sIconPath, FileMisc::GetAppFolder());

			HICON hIcon = CEnBitmap::LoadImageFileAsIcon(sIconPath, CLR_NONE, 16, 16);

			if (hIcon)
			{
				lvi.iImage = m_ilSys.GetImageList()->Add(hIcon);
				::DestroyIcon(hIcon);
			}
			else
			{
				lvi.iImage = m_ilSys.GetFileImageIndex(sIconPath);
			}
		}
		
		m_lcTools.SetItem(&lvi);
	}
}

void CPreferencesToolPage::OnChangeIconPath() 
{
	int nSel = GetCurSel();

	if (nSel == CB_ERR)
	{
		ASSERT (0);
		return;
	}

	UpdateData();

	m_lcTools.SetItemText(nSel, 3, m_sIconPath);

	RebuildListCtrlImages();

	CPreferencesPageBase::OnControlChange();
}

int CPreferencesToolPage::GetUserTools(CUserToolArray& aTools) const
{
	aTools.Copy(m_aTools);

	return aTools.GetSize();
}

BOOL CPreferencesToolPage::GetUserTool(int nTool, USERTOOL& tool) const
{
	if (nTool >= 0 && nTool < m_aTools.GetSize())
	{
		tool = m_aTools[nTool];
		return TRUE;
	}

	return FALSE;
}

void CPreferencesToolPage::OnOK() 
{
	CPreferencesPageBase::OnOK();
	
	// save tools to m_aTools
	m_aTools.RemoveAll();
	int nToolCount = m_lcTools.GetItemCount();

	for (int nTool = 0; nTool < nToolCount; nTool++)
	{
		USERTOOL ut;
		VERIFY(GetListTool(nTool, ut));

		// GetPrivateProfileString strips a leading/trailing quote pairs if 
		// it finds them so we replace quotes with safe quotes
		ut.sCmdline.Replace(REALQUOTE, SAFEQUOTE);
		
		m_aTools.Add(ut);
	}
}

BOOL CPreferencesToolPage::GetListTool(int nTool, USERTOOL& ut) const
{
	if ((nTool < 0) || (nTool >= m_lcTools.GetItemCount()))
	{
		ASSERT(0);
		return FALSE;
	}

	ut.sToolName = m_lcTools.GetItemText(nTool, 0);
	ut.sToolPath = m_lcTools.GetItemText(nTool, 1);
	ut.sCmdline = m_lcTools.GetItemText(nTool, 2);
	ut.sIconPath = m_lcTools.GetItemText(nTool, 3);
	ut.bRunMinimized = m_lcTools.GetItemData(nTool);

	return TRUE;
}

void CPreferencesToolPage::OnKeydownToollist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
	
	switch (pLVKeyDown->wVKey)
	{
	case VK_DELETE:
		OnDeleteTool();
		break;

	case VK_F2:
		{
			int nSel = GetCurSel();

			if (nSel >= 0)
				m_lcTools.EditLabel(nSel);
		}
		break;
	}
	
	*pResult = 0;
}

void CPreferencesToolPage::OnChangeCmdline() 
{
	int nSel = GetCurSel();

	if (nSel >= 0)
	{
		UpdateData();

		m_lcTools.SetItemText(nSel, 2, m_sCommandLine);

		CPreferencesPageBase::OnControlChange();
	}
}

void CPreferencesToolPage::OnInsertPlaceholder(UINT nCmdID) 
{
	m_eCmdLine.ReplaceSel(MapCmdIDToPlaceholder(nCmdID), TRUE);

	CPreferencesPageBase::OnControlChange();
}

void CPreferencesToolPage::OnRunminimized() 
{
	int nSel = GetCurSel();

	if (nSel >= 0)
	{
		UpdateData();

		m_lcTools.SetItemData(nSel, m_bRunMinimized);

		CPreferencesPageBase::OnControlChange();
	}
}

LRESULT CPreferencesToolPage::OnGetFileIcon(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDC_TOOLPATH:
		{
			CString sToolPath((LPCTSTR)lParam);
			
			if (CTDCToolsCmdlineParser::PrepareToolPath(sToolPath))
			{
				static HICON hIcon = m_ilSys.ExtractFileIcon(sToolPath);
				return (LRESULT)hIcon;
			}
		}
		break;
	}

	return 0L;
}

void CPreferencesToolPage::OnTestTool() 
{
	int nTool = GetCurSel();
	
	if (nTool != -1)
	{
		USERTOOL ut;
		VERIFY(GetListTool(nTool, ut));
		
		GetParent()->SendMessage(WM_PTP_TESTTOOL, 0, (LPARAM)&ut);
	}
}

CString CPreferencesToolPage::MapCmdIDToPlaceholder(UINT nCmdID) 
{
	CLA_TYPE nType = MapCmdIDToType(nCmdID);
	CString sPlaceHolder = CTDCToolsCmdlineParser::GetPlaceHolder(nType), sText;

	switch (nCmdID)
	{
	case ID_TOOLARG_FILETITLE:			
	case ID_TOOLARG_FOLDER:				
	case ID_TOOLARG_FILENAME:			
	case ID_TOOLARG_SELTASKID:			
	case ID_TOOLARG_TODAYSDATE:	
		sText.Format(_T("$(%s)"), sPlaceHolder);
		break;
		
	case ID_TOOLARG_PATHNAME:			
	case ID_TOOLARG_SELTASKTITLE:		
	case ID_TOOLARG_TODOLIST:			
	case ID_TOOLARG_SELTASKEXTID:		
	case ID_TOOLARG_SELTASKCOMMENTS:	
	case ID_TOOLARG_SELTASKFILELINK:	
	case ID_TOOLARG_SELTASKALLOCBY:		
	case ID_TOOLARG_SELTASKALLOCTO:		
		sText.Format(_T("\"$(%s)\""), sPlaceHolder);
		break;
		
	case ID_TOOLARG_USERDATE:		
		sText.Format(_T("$(%s, var_date1, \"%s\", default_date)"), sPlaceHolder, CEnString(IDS_USERTOOL_DATEPROMPT));       
		break;
		
	case ID_TOOLARG_USERFILEPATH:	
		sText.Format(_T("\"$(%s, var_file1, \"%s\", default_path)\""), sPlaceHolder, CEnString(IDS_USERTOOL_FILEPROMPT));   
		break;
		
	case ID_TOOLARG_USERFOLDER:		
		sText.Format(_T("$(%s, var_folder1, \"%s\", default_folder)"), sPlaceHolder, CEnString(IDS_USERTOOL_FOLDERPROMPT));     
		break;
		
	case ID_TOOLARG_USERTEXT:		
		sText.Format(_T("$(%s, var_text1, \"%s\", default_text)"), sPlaceHolder, CEnString(IDS_USERTOOL_TEXTPROMPT));       
		break;

	case ID_TOOLARG_SELTASKCUSTOM:		
		sText.Format(_T("$(%s, var_cust1)"), sPlaceHolder);
		break;
	}
	
	return sText;
}

CLA_TYPE CPreferencesToolPage::MapCmdIDToType(UINT nCmdID)
{
	switch (nCmdID)
	{
	case ID_TOOLARG_PATHNAME:			return CLAT_PATHNAME;		
	case ID_TOOLARG_FILETITLE:			return CLAT_FILETITLE;
	case ID_TOOLARG_FOLDER:				return CLAT_FOLDER; 
	case ID_TOOLARG_FILENAME:			return CLAT_FILENAME;
	case ID_TOOLARG_SELTASKID:			return CLAT_SELTASKID;
	case ID_TOOLARG_SELTASKTITLE:		return CLAT_SELTASKTITLE;
	case ID_TOOLARG_TODAYSDATE:			return CLAT_TODAYSDATE;
	case ID_TOOLARG_TODOLIST:			return CLAT_TODOLIST;
	case ID_TOOLARG_SELTASKEXTID:		return CLAT_SELTASKEXTID;
	case ID_TOOLARG_SELTASKCOMMENTS:	return CLAT_SELTASKCOMMENTS;
	case ID_TOOLARG_SELTASKFILELINK:	return CLAT_SELTASKFILELINK;
	case ID_TOOLARG_SELTASKALLOCBY:		return CLAT_SELTASKALLOCBY;
	case ID_TOOLARG_SELTASKALLOCTO:		return CLAT_SELTASKALLOCTO;	
	case ID_TOOLARG_SELTASKCUSTOM:		return CLAT_SELTASKCUSTATTRIB;	
	case ID_TOOLARG_USERDATE:			return CLAT_USERDATE;
	case ID_TOOLARG_USERFILEPATH:		return CLAT_USERFILE;
	case ID_TOOLARG_USERFOLDER:			return CLAT_USERFOLDER;
	case ID_TOOLARG_USERTEXT:			return CLAT_USERTEXT;
	}

	// all else
	ASSERT(0);
	return CLAT_NONE;
}

void CPreferencesToolPage::OnImportTools() 
{
	BOOL bContinue = TRUE;
	CPreferences prefs;

	while (bContinue)
	{
		CFileOpenDialog dialog(IDS_IMPORTPREFS_TITLE, 
								_T("ini"), 
								NULL, 
								OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, 
								CEnString(IDS_INIFILEFILTER));
		
		if (dialog.DoModal(prefs) == IDOK)
		{
			CIni ini(dialog.GetPathName());

			int nNumTools = ini.GetInt(_T("Tools"), _T("ToolCount"), 0);
			nNumTools = min(nNumTools, m_nMaxNumTools);

			if (!nNumTools)
			{
				bContinue = (AfxMessageBox(IDS_INIHASNOTOOLS, MB_YESNO) == IDYES);
			}
			else
			{
				for (int nTool = 0; nTool < nNumTools; nTool++)
				{
					CString sKey = Misc::MakeKey(_T("Tools\\Tool%d"), nTool + 1);
					USERTOOL ut;

					ut.sToolName = ini.GetString(sKey, _T("Name"));
					ut.sToolPath = ini.GetString(sKey, _T("Path"));
					ut.sIconPath = ini.GetString(sKey, _T("IconPath"));
					ut.bRunMinimized = ini.GetBool(sKey, _T("RunMinimized"), FALSE);
					ut.sCmdline = ini.GetString(sKey, _T("Cmdline"));

					// replace safe quotes with real quotes
					ut.sCmdline.Replace(SAFEQUOTE, REALQUOTE);

					// add tool to list
					VERIFY(AddListTool(ut) != -1);
				}

				bContinue = FALSE;

				CPreferencesPageBase::OnControlChange();
			}
		}
		else
			bContinue = FALSE; // cancelled
	}
}

void CPreferencesToolPage::LoadPreferences(const IPreferences* pPrefs, LPCTSTR szKey)
{
	// load tools
	int nToolCount = pPrefs->GetProfileInt(_T("Tools"), _T("ToolCount"), 0);
	nToolCount = min(nToolCount, m_nMaxNumTools);

	for (int nTool = 1; nTool <= nToolCount; nTool++)
	{
		CString sKey = Misc::MakeKey(_T("Tools\\Tool%d"), nTool);

		USERTOOL ut;
		ut.sToolName = pPrefs->GetProfileString(sKey, _T("Name"), _T(""));
		ut.sToolPath = pPrefs->GetProfileString(sKey, _T("Path"), _T(""));
		ut.sCmdline = pPrefs->GetProfileString(sKey, _T("CmdLine"), _T("")); 
		ut.bRunMinimized = pPrefs->GetProfileInt(sKey, _T("RunMinimized"), FALSE);
		ut.sIconPath = pPrefs->GetProfileString(sKey, _T("IconPath"), _T(""));
		
		// replace safe quotes with real quotes
		ut.sCmdline.Replace(SAFEQUOTE, REALQUOTE);

		m_aTools.Add(ut);
	}

	m_bDisplayUDTsInToolbar = pPrefs->GetProfileInt(szKey, _T("DisplayUDTsInToolbar"), TRUE);
}

void CPreferencesToolPage::SavePreferences(IPreferences* pPrefs, LPCTSTR szKey) const
{
	// save tools to registry and m_aTools
	int nToolCount = m_aTools.GetSize();

	for (int nTool = 0; nTool < nToolCount; nTool++)
	{
		USERTOOL ut = m_aTools[nTool];

        CString sKey = Misc::MakeKey(_T("Tools\\Tool%d"), nTool + 1);
		
		pPrefs->WriteProfileString(sKey, _T("Name"), ut.sToolName);
		pPrefs->WriteProfileString(sKey, _T("Path"), ut.sToolPath);
		pPrefs->WriteProfileString(sKey, _T("IconPath"), ut.sIconPath);
		pPrefs->WriteProfileInt(sKey, _T("RunMinimized"), ut.bRunMinimized);
		
		// GetPrivateProfileString strips a leading/trailing quote pairs if 
		// it finds them so we replace quotes with safe quotes
		ut.sCmdline.Replace(REALQUOTE, SAFEQUOTE);
		pPrefs->WriteProfileString(sKey, _T("Cmdline"), ut.sCmdline);
	}

	pPrefs->WriteProfileInt(_T("Tools"), _T("ToolCount"), nToolCount);
	pPrefs->WriteProfileInt(szKey, _T("DisplayUDTsInToolbar"), m_bDisplayUDTsInToolbar);
}

void CPreferencesToolPage::OnSize(UINT nType, int cx, int cy) 
{
	CPreferencesPageBase::OnSize(nType, cx, cy);
	
	if (m_lcTools.GetSafeHwnd())
	{
		// calculate border
		CPoint ptBorders = CDialogHelper::GetCtrlRect(this, IDC_TOOLBAR).TopLeft();
		
		// calc offsets
		CPoint ptImport = CDialogHelper::GetCtrlRect(this, IDC_TESTTOOL).BottomRight();
		int nXOffset = cx - (ptImport.x + ptBorders.x);
		int nYOffset = cy - (ptImport.y + ptBorders.y);

		// Update positions and sizes
		CDialogHelper::OffsetCtrl(this, IDC_IMPORT, nXOffset, 0);
		CDialogHelper::OffsetCtrl(this, IDC_PATHLABEL, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_ARGLABEL, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_ICONLABEL, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_TOOLPATH, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_CMDLINE, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_ICONPATH, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_RUNMINIMIZED, 0, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_ARGMENUBTN, nXOffset, nYOffset);
		CDialogHelper::OffsetCtrl(this, IDC_TESTTOOL, nXOffset, nYOffset);

		CDialogHelper::ResizeChild(&m_eToolPath, nXOffset, 0);
		CDialogHelper::ResizeChild(&m_eIconPath, nXOffset, 0);
		CDialogHelper::ResizeChild(&m_eCmdLine, nXOffset, 0);
		CDialogHelper::ResizeChild(&m_lcTools, nXOffset, nYOffset);

		m_lcTools.SetColumnWidth(2, m_lcTools.GetColumnWidth(2) + nXOffset);
	}
}

BOOL CPreferencesToolPage::InitializeToolbar()
{
	if (m_toolbar.GetSafeHwnd())
		return TRUE;
	
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP))
		return FALSE;
	
	if (!m_toolbar.LoadToolBar(IDR_UDTPREFS_TOOLBAR))
		return FALSE;
	
	// toolbar images
// 	if (m_theme.HasToolbarImageFile(_T("CUSTOM_ATTRIB")))
// 	{
// 		COLORREF crMask = CLR_NONE;
// 		CString sImagePath = m_theme.GetToolbarImageFile(_T("CUSTOM_ATTRIB"), crMask);
// 		
// 		VERIFY(m_toolbar.SetImage(sImagePath, crMask));
// 	}
// 	else 
		m_toolbar.SetImage(IDB_UDTPREFS_TOOLBAR_STD, RGB(255, 0, 255));
	
	VERIFY(m_tbHelper.Initialize(&m_toolbar, this));
	
	// very important - turn OFF all the auto positioning and sizing
	// by default have no borders
	UINT nStyle = m_toolbar.GetBarStyle();
	nStyle &= ~(CCS_NORESIZE | CCS_NOPARENTALIGN | CBRS_BORDER_ANY);
	nStyle |= (CBRS_SIZE_FIXED | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_toolbar.SetBarStyle(nStyle);
	
	CRect rToolbar;
	GetDlgItem(IDC_TOOLBAR)->GetWindowRect(rToolbar);
	ScreenToClient(rToolbar);
	m_toolbar.MoveWindow(rToolbar);

	m_toolbar.SetBackgroundColor(m_crback);
	
	return TRUE;
}
