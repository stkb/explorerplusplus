#ifndef TREEVIEW_INCLUDED
#define TREEVIEW_INCLUDED

#include <commctrl.h>
#include <shlwapi.h>
#include <list>
#include "../Helper/Buffer.h"
#include "../Helper/iDirectoryMonitor.h"
#include "../Helper/DropHandler.h"

using namespace std;

#define WM_USER_TREEVIEW				WM_APP + 70
#define WM_USER_TREEVIEW_GAINEDFOCUS	(WM_USER_TREEVIEW + 2)

class CMyTreeView : public IDropTarget, public IDropSource
{
public:

	/* IUnknown methods. */
	HRESULT __stdcall	QueryInterface(REFIID iid,void **ppvObject);
	ULONG __stdcall		AddRef(void);
	ULONG __stdcall		Release(void);

	/* Contructor/Deconstructor. */
	CMyTreeView(HWND hTreeView,HWND hParent,IDirectoryMonitor *pDirMon);
	~CMyTreeView();

	/* Drop source functions. */
	HRESULT _stdcall	QueryContinueDrag(BOOL fEscapePressed,DWORD gfrKeyState);
	HRESULT _stdcall	GiveFeedback(DWORD dwEffect);

	/* User functions. */
	LRESULT CALLBACK	TreeViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
	HRESULT				AddDirectory(HTREEITEM hParent,TCHAR *szParsingPath);
	HRESULT				AddDirectory(HTREEITEM hParent,LPITEMIDLIST pidlDirectory);
	void				AddDirectoryInternal(IShellFolder *pShellFolder,LPITEMIDLIST pidlDirectory,HTREEITEM hParent);
	LPITEMIDLIST		BuildPath(HTREEITEM hTreeItem);
	HTREEITEM			LocateItem(TCHAR *szParsingPath);
	HTREEITEM			LocateItem(LPITEMIDLIST pidlDirectory);
	HTREEITEM			LocateItemByPath(TCHAR *szItemPath,BOOL bExpand);
	void				EraseItems(HTREEITEM hParent);
	HTREEITEM			CheckAgainstDesktop(TCHAR *szFullFileName);
	BOOL				QueryDragging(void);
	DWORD WINAPI		Thread_SubFolders(LPVOID pParam);
	DWORD WINAPI		Thread_AddDirectoryInternal(IShellFolder *pShellFolder,LPITEMIDLIST pidlDirectory,HTREEITEM hParent);
	void				SetShowHidden(BOOL bShowHidden);
	void				RefreshAllIcons(void);

	static void DirectoryAlteredCallback(TCHAR *szFileName,DWORD dwAction,void *pData);

	/* Drag and Drop. */
	HRESULT _stdcall	DragEnter(IDataObject *pDataObject,DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);
	HRESULT _stdcall	DragOver(DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);
	HRESULT _stdcall	DragLeave(void);
	HRESULT _stdcall	Drop(IDataObject *pDataObject,DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);

	void		MonitorDrivePublic(TCHAR *szDrive);

	int					m_iProcessing;

private:

	/* Message handlers. */
	LRESULT CALLBACK	OnNotify(HWND hwnd,UINT Msg,WPARAM wParam,LPARAM lParam);
	LRESULT		OnSetCursor(void);

	void		DirectoryModified(DWORD Action,TCHAR *szFullFileName);

	void		DirectoryAltered(void);
	HTREEITEM	AddRoot(void);
	void		AddItem(TCHAR *szFullFileName);
	void		AddItemInternal(HTREEITEM hParent,TCHAR *szFullFileName);
	void		AddDrive(TCHAR *szDrive);
	void		RenameItem(HTREEITEM hItem,TCHAR *szFullFileName);
	void		RemoveItem(TCHAR *szFullFileName);
	LRESULT CALLBACK	OnDeviceChange(WPARAM wParam,LPARAM lParam);

	/* Item id's. */
	int			GenerateUniqueItemId(void);

	/* Drag and drop. */
	HRESULT		InitializeDragDropHelpers(void);
	void		RestoreState(void);
	DWORD		GetCurrentDragEffect(DWORD grfKeyState,DWORD dwCurrentEffect,POINTL *ptl);
	BOOL		CheckItemLocations(IDataObject *pDataObject,HTREEITEM hItem,int iDroppedItem);
	HRESULT		OnBeginDrag(int iItemId,DragTypes_t DragType);

	/* Icon refresh. */
	void		RefreshAllIconsInternal(HTREEITEM hFirstSibling);

	HTREEITEM	LocateExistingItem(TCHAR *szParsingPath);
	HTREEITEM	LocateExistingItem(LPITEMIDLIST pidlDirectory);
	HTREEITEM	LocateItemInternal(LPITEMIDLIST pidlDirectory,BOOL bOnlyLocateExistingItem);
	void		MonitorAllDrives(void);
	void		MonitorDrive(TCHAR *szDrive);
	HTREEITEM	DetermineDriveSortedPosition(HTREEITEM hParent,TCHAR *szItemName);
	HTREEITEM	DetermineItemSortedPosition(HTREEITEM hParent,TCHAR *szItem);




	/* ------ Internal state. ------ */

	typedef struct
	{
		TCHAR szFileName[MAX_PATH];
		DWORD dwAction;
	} AlteredFiles_t;

	typedef struct
	{
		LPITEMIDLIST	pidl;
	} ItemInfo_t;

	typedef struct
	{
		TCHAR szPath[MAX_PATH];
		CMyTreeView *pMyTreeView;
	} DirectoryAltered_t;

	typedef struct
	{
		TCHAR	szDrive[MAX_PATH];
		HANDLE	hDrive;
		int		iMonitorId;
	} DriveEvent_t;

	HWND				m_hTreeView;
	HWND				m_hParent;
	int					m_iRefCount;
	IDirectoryMonitor	*m_pDirMon;
	TCHAR				m_szOldName[MAX_PATH];
	BOOL				m_bRightClick;
	BOOL				m_bShowHidden;

	/* Item id's and info. */
	int					*m_uItemMap;
	ItemInfo_t			*m_pItemInfo;
	int					m_iCurrentItemAllocation;

	/* Drag and drop. */
	IDragSourceHelper	*m_pDragSourceHelper;
	IDropTargetHelper	*m_pDropTargetHelper;
	IDataObject			*m_pDataObject;
	BOOL				m_bDragging;
	BOOL				m_bDragCancelled;
	BOOL				m_bDragAllowed;
	BOOL				m_bDataAccept;
	DragTypes_t			m_DragType;

	/* Directory modification. */
	AlteredFiles_t		*m_pAlteredFiles;
	int					m_iAlteredAllocation;
	int					m_nAltered;
	CRITICAL_SECTION	m_cs;

	/* Hardware events. */
	list<DriveEvent_t>	m_pDriveList;
	BOOL				m_bQueryRemoveCompleted;
	TCHAR				m_szQueryRemove[MAX_PATH];
};

typedef struct
{
	HWND			hTreeView;
	HTREEITEM		hParent;
	LPITEMIDLIST	pidl;
	CMyTreeView		*pMyTreeView;

	IShellFolder	*pShellFolder;
} ThreadInfo_t;

#endif