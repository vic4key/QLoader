#pragma once

#include <string>
#include <functional>

struct Node
{
  Node(const std::string& name, void* pData = nullptr) :  m_pData(pData), m_pTV(nullptr)
  {
    std::wstring s(name.cbegin(), name.cend());
    m_Name = s.c_str();
  }

  Node(const CString& name, void* pData = nullptr) : m_Name(name), m_pData(pData), m_pTV(nullptr) {}
  virtual ~Node() {}

  Node& operator=(const Node& right)
  {
    m_Name  = right.m_Name;
    m_pData = right.m_pData;
    m_pTV = right.m_pTV;
    return *this;
  }

  CString m_Name;
  void* m_pData;
  TVITEM* m_pTV;
};

class EasyTreeCtrl : public CTreeCtrl
{
  DECLARE_DYNAMIC(EasyTreeCtrl)

  enum class eNotifyType
  {
    SELECTING,

    BEFORE_INSERTING,
    AFTER_INSERTING,

    BEFORE_MODIFYING,
    AFTER_MODIFYING,

    BEFORE_DELETING,
    AFTER_DELETING,
  };

public:
  EasyTreeCtrl(CDialog* pParent = nullptr);
  virtual ~EasyTreeCtrl();

  void Populate(std::function<void(HTREEITEM& root)> pfn, const CString& name = L"<root>");
  void Iterate(HTREEITEM pItem, std::function<void(HTREEITEM pItem)> pfn);
  HTREEITEM InsertNode(HTREEITEM& pParent, Node* pNode);

public:
  void OnNotify(std::function<bool(eNotifyType action, Node* pNode)> pfn);

public:
  afx_msg void OnDestroy();

  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnContextMenuHandler(UINT ID);

  afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);

  afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

  afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

private:
  HTREEITEM GetpItemFocusing();
  bool EditItem(HTREEITEM pItem);
  bool Notify(eNotifyType action, HTREEITEM pItem);
  bool Notify(eNotifyType action, Node* pNode);
  void Cleanup(HTREEITEM pItem);

private:
  CDialog* m_pParent;
  std::function<bool(eNotifyType action, Node* pNode)> m_pfnOnNotify;
};
