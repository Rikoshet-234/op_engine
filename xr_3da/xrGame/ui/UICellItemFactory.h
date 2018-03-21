#pragma once

class CUICellItem;
class CInventoryItem;

CUICellItem*	create_cell_item(CInventoryItem* itm);
CUICellItem*	create_cell_item(shared_str itemSection);
CUICellItem*	create_cell_item(shared_str itemSection,float condition);

