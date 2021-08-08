#include "pch.h"
#include "EasyTreeCtrl.h"
#include "resource.h"

#include <cassert>

#pragma warning(disable : 26454)

#define ID_CONTEXT_MENU_INSERT WM_USER + 100
#define ID_CONTEXT_MENU_MODIFY WM_USER + 101
#define ID_CONTEXT_MENU_DELETE WM_USER + 102

IMPLEMENT_DYNAMIC(EasyTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(EasyTreeCtrl, CTreeCtrl)
  ON_WM_DESTROY()
  ON_WM_CONTEXTMENU()
  ON_COMMAND_RANGE(ID_CONTEXT_MENU_INSERT, ID_CONTEXT_MENU_DELETE, OnContextMenuHandler)

  ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
  ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)

  ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
END_MESSAGE_MAP()

EasyTreeCtrl::EasyTreeCtrl(CDialog* pParent) : CTreeCtrl(), m_pParent(pParent), m_pfnOnNotify(nullptr)
{
}

EasyTreeCtrl::~EasyTreeCtrl()
{
}

void EasyTreeCtrl::OnDestroy()
{
  this->Cleanup(this->GetRootItem());
}

void EasyTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (point.x == -1 && point.y == -1) // SHIFT F10
  {
    DWORD position = GetMessagePos();
    point.x = GET_X_LPARAM(position);
    point.y = GET_Y_LPARAM(position);
  }

  // manually select a new item that changed by right-click

  if (auto pItem = this->GetpItemFocusing())
  {
    this->SelectItem(pItem);
  }

  // display context menu

  CMenu thePopupMenu;
  thePopupMenu.CreatePopupMenu();
  // thePopupMenu.AppendMenu(MF_STRING | MF_POPUP, ID_CONTEXT_MENU_INSERT, L"Insert");
  thePopupMenu.AppendMenu(MF_STRING | MF_POPUP, ID_CONTEXT_MENU_MODIFY, L"Modify");
  // thePopupMenu.AppendMenu(MF_STRING | MF_POPUP, ID_CONTEXT_MENU_DELETE, L"Delete");
  thePopupMenu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}

void EasyTreeCtrl::OnContextMenuHandler(UINT ID)
{
  auto pItem = this->GetSelectedItem();
  assert(pItem != nullptr);

  switch (ID)
  {
  case ID_CONTEXT_MENU_INSERT:
    {
      if (this->Notify(eNotifyType::BEFORE_INSERTING, pItem))
      {
        // DO NOTHING
        this->Notify(eNotifyType::AFTER_INSERTING, pItem);
      }
    }
    break;

  case ID_CONTEXT_MENU_MODIFY:
    {
      this->EditItem(pItem);
    }
    break;

  case ID_CONTEXT_MENU_DELETE:
    {
      auto pNode = reinterpret_cast<Node*>(this->GetItemData(pItem));

      if (this->Notify(eNotifyType::BEFORE_DELETING, pItem))
      {
        this->Cleanup(this->GetChildItem(pItem)); // cleanup all child items before deleting
        this->DeleteItem(pItem);
        this->Notify(eNotifyType::AFTER_DELETING, pNode);
      }

      if (pNode != nullptr) // after notify after deleting, cleanup the item
      {
        delete pNode;
      }
    }
    break;

  default:
    break;
  }
}

void EasyTreeCtrl::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
  auto pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  assert(pNMTreeView != nullptr);

  auto pNode = reinterpret_cast<Node*>(this->GetItemData(pNMTreeView->itemNew.hItem));
  if (pNode != nullptr)
  {
    this->Notify(eNotifyType::SELECTING, pNode);
  }
}

void EasyTreeCtrl::OnRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
  CPoint point;
  GetCursorPos(&point);

  this->SendMessage(WM_CONTEXTMENU, WPARAM(GetSafeHwnd()), MAKELPARAM(point.x, point.y));

  *pResult = TRUE;
}

void EasyTreeCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
  auto pItem = this->GetpItemFocusing();
  assert(pItem != nullptr);
  
  this->EditItem(pItem);
  
  *pResult = TRUE;
}

void EasyTreeCtrl::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
  *pResult = FALSE;
}

void EasyTreeCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
  auto pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
  assert(pTVDispInfo != nullptr);

  auto pItem = &pTVDispInfo->item;
  assert(pItem != nullptr);

  if (pItem->pszText != nullptr)
  {
    if (this->Notify(eNotifyType::BEFORE_MODIFYING, pItem->hItem))
    {
      pItem->mask = TVIF_TEXT;
      this->SetItem(pItem);
      this->Notify(eNotifyType::AFTER_MODIFYING, pItem->hItem);
    }
  }

  *pResult = FALSE;
}

void EasyTreeCtrl::OnNotify(std::function<bool(eNotifyType action, Node* pNode)> pfn)
{
  m_pfnOnNotify = pfn;
}

bool EasyTreeCtrl::Notify(eNotifyType action, HTREEITEM pItem)
{
  if (pItem == nullptr)
  {
    return true;
  }

  auto pNode = reinterpret_cast<Node*>(this->GetItemData(pItem));
  if (pNode != nullptr)
  {
    TVITEM tv = { 0 };
    tv.hItem = pItem;
    tv.mask = TVIF_TEXT;
    tv.stateMask = TVIF_TEXT;

    WCHAR s[MAXBYTE] = { 0 };
    tv.pszText = LPWSTR(&s);
    tv.cchTextMax = ARRAYSIZE(s);

    this->GetItem(&tv);
    pNode->m_ptr_tv = &tv;
  }

  return this->Notify(action, pNode);
}

bool EasyTreeCtrl::Notify(eNotifyType action, Node* pNode)
{
  if (m_pfnOnNotify == nullptr)
  {
    return true;
  }

  return m_pfnOnNotify(action, pNode);
}

void EasyTreeCtrl::Populate(std::function<void(HTREEITEM& root)> pfn, const CString& name)
{
  if (pfn != nullptr)
  {
    auto root = this->InsertItem(name, TVI_ROOT);
    pfn(root);
    this->Expand(root, TVE_EXPAND);
  }
}

HTREEITEM EasyTreeCtrl::InsertNode(HTREEITEM& pParent, Node* pNode)
{
  assert(pParent != nullptr && pNode != nullptr);

  auto item = this->InsertItem(pNode->m_name, pParent);
  this->SetItemData(item, DWORD_PTR(pNode));

  return item;
}

HTREEITEM EasyTreeCtrl::GetpItemFocusing()
{
  CPoint point;
  GetCursorPos(&point);
  this->ScreenToClient(&point);
  return this->HitTest(point, nullptr);
}

bool EasyTreeCtrl::EditItem(HTREEITEM pItem)
{
  if (pItem == nullptr)
  {
    return false;
  }

  if (this->EndEditLabelNow(TRUE))
  {
    this->SelectItem(pItem);
    this->EditLabel(pItem);
  }

  return true;
}

void EasyTreeCtrl::Cleanup(HTREEITEM pItem)
{
  this->Iterate(pItem, [&](HTREEITEM pItem) -> void
  {
    if (auto pNode = reinterpret_cast<Node*>(this->GetItemData(pItem)))
    {
      delete pNode;
    }
  });
}

void EasyTreeCtrl::Iterate(HTREEITEM hItem, std::function<void(HTREEITEM pItem)> pfn)
{
  if (hItem == nullptr || pfn == nullptr)
  {
    return;
  }

  do
  {
    pfn(hItem);

    if (this->ItemHasChildren(hItem))
    {
      this->Iterate(this->GetChildItem(hItem), pfn);
    }

  } while (hItem = this->GetNextSiblingItem(hItem));
}
